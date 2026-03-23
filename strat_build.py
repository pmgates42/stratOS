#!/usr/bin/env python3

import os
import sys
import json
import shutil
import subprocess
from pathlib import Path

REPO_ROOT = Path.cwd().resolve()
BUILD_JSON = REPO_ROOT / 'build.json'
DEFAULT_APPS_JSON = REPO_ROOT / 'apps.json'
BUILD_DIR = REPO_ROOT / 'build'


def load_build_json(path):
    if not path.exists():
        print(f"Error: {path} not found")
        sys.exit(1)
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def load_optional_json(path):
    if not path.exists():
        return {}

    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def find_platform(cfg, name):
    for p in cfg.get('platforms', []):
        if p.get('name') == name:
            return p
    return None


def find_application(app_cfg, name):
    for app in app_cfg.get('applications', []):
        if app.get('name') == name:
            return app
    return None


def collect_modules(cfg, platform_cfg):
    modules = []
    if platform_cfg and 'modules' in platform_cfg and isinstance(platform_cfg['modules'], list):
        for m in platform_cfg['modules']:
            if isinstance(m, dict):
                modules.append(m)
            elif isinstance(m, str):
                # find module by name
                for t in cfg.get('modules', []):
                    if t.get('name') == m:
                        modules.append(t)
                        break
    # fallback
    if not modules:
        if 'modules' in cfg and isinstance(cfg['modules'], list):
            modules = cfg['modules']
        elif 'module' in cfg and isinstance(cfg['module'], dict):
            modules = [cfg['module']]
    return modules


def expand_sources(patterns):
    """Expand glob patterns relative to repo root. Returns list of Path objects."""
    out = []
    for pat in patterns:
        p = Path(REPO_ROOT) / pat
        # If the pattern contains glob tokens, use globbing relative to REPO_ROOT
        if any(ch in pat for ch in ['*', '?', '[']):
            matches = list(Path(REPO_ROOT).glob(pat))
            if not matches:
                # warn but continue
                print(f"Warning: pattern '{pat}' matched no files")
            else:
                out.extend(matches)
        else:
            # literal file path
            if p.exists():
                out.append(p)
            else:
                print(f"Warning: file '{pat}' not found")
    # remove duplicates while preserving order
    seen = set()
    uniq = []
    for x in out:
        k = str(x.resolve())
        if k not in seen:
            seen.add(k)
            uniq.append(x)
    return uniq


def compile_source(compiler, cflags, src_path, out_obj, include_dirs, extra_flags=None):
    src_path = Path(src_path)
    out_obj = Path(out_obj)
    out_obj.parent.mkdir(parents=True, exist_ok=True)

    cmd = [compiler]
    cmd += cflags
    for inc in include_dirs:
        cmd.append(f"-I{inc}")
    if extra_flags:
        cmd += extra_flags

    cmd += ['-MMD', '-c', str(src_path), '-o', str(out_obj)]

    print('Compiling:', ' '.join(cmd))
    try:
        res = subprocess.run(cmd, check=True)
    except FileNotFoundError:
        print(f"ERROR: Compiler '{compiler}' not found. Install GCC (MSYS2/MinGW) or run inside WSL, or update build.json to point to a valid compiler path.")
        sys.exit(2)
    except subprocess.CalledProcessError as e:
        print(f"ERROR: compile failed for {src_path} (returncode {e.returncode})")
        sys.exit(e.returncode)


def link_executable(compiler, objects, out_exe, ldflags=None):
    if not objects:
        print('No objects to link')
        return
    cmd = [compiler]
    cmd += objects
    if ldflags:
        cmd += ldflags
    cmd += ['-o', str(out_exe)]
    print('Linking:', ' '.join(cmd))
    try:
        res = subprocess.run(cmd, check=True)
    except FileNotFoundError:
        print(f"ERROR: Linker/Compiler '{compiler}' not found. Install toolchain or use WSL/MSYS2.")
        sys.exit(2)
    except subprocess.CalledProcessError as e:
        print('ERROR: link failed')
        sys.exit(e.returncode)


def resolve_executable(tool_name):
    if not tool_name:
        return None
    if shutil.which(tool_name) is not None:
        return tool_name
    if sys.platform == 'win32' and not tool_name.endswith('.exe') and shutil.which(tool_name + '.exe'):
        return tool_name + '.exe'
    return None


def run_checked_command(cmd, missing_tool_message, failure_message):
    print('Running:', ' '.join(cmd))
    try:
        subprocess.run(cmd, check=True)
    except FileNotFoundError:
        print(missing_tool_message)
        sys.exit(2)
    except subprocess.CalledProcessError as e:
        print(f"{failure_message} (returncode {e.returncode})")
        sys.exit(e.returncode)


def render_tokenized_args(args, context):
    rendered = []
    for arg in args:
        if not isinstance(arg, str):
            rendered.append(str(arg))
            continue

        out = arg
        for key, value in context.items():
            out = out.replace('{' + key + '}', value)
        rendered.append(out)
    return rendered


def run_platform_link_pipeline(platform_cfg, compiler, built_objects, platform_name, selected_app, ldflags):
    link_cfg = platform_cfg.get('link', True)
    if isinstance(link_cfg, bool):
        if not link_cfg:
            return

        output_name = 'app'
        if selected_app:
            output_name = selected_app.get('output_name', selected_app.get('name', 'app'))
        out_exe = BUILD_DIR / platform_name / output_name
        link_executable(compiler, built_objects, out_exe, ldflags)
        return

    if not isinstance(link_cfg, dict):
        print(f"ERROR: platform '{platform_name}' has invalid 'link' configuration type")
        sys.exit(1)

    if not link_cfg.get('enabled', True):
        return

    link_tool_raw = link_cfg.get('tool', compiler)
    link_tool = resolve_executable(link_tool_raw)
    if not link_tool:
        print(f"ERROR: linker '{link_tool_raw}' not found. Ensure it is available in PATH or use a full path in {BUILD_JSON}.")
        sys.exit(2)

    link_flags = link_cfg.get('flags', []) or []
    linker_script = link_cfg.get('script')

    output_name = 'app'
    if selected_app:
        output_name = selected_app.get('output_name', selected_app.get('name', 'app'))

    elf_output_name = link_cfg.get('output') or link_cfg.get('elf_output') or output_name
    out_elf = BUILD_DIR / platform_name / elf_output_name
    out_elf.parent.mkdir(parents=True, exist_ok=True)

    link_cmd = [link_tool]
    if linker_script:
        script_path = Path(linker_script)
        if not script_path.is_absolute():
            script_path = REPO_ROOT / script_path
        link_cmd += ['-T', str(script_path)]

    link_cmd += link_flags
    link_cmd += built_objects
    link_cmd += ['-o', str(out_elf)]

    run_checked_command(
        link_cmd,
        f"ERROR: linker '{link_tool_raw}' not found. Install toolchain or update build.json.",
        'ERROR: platform link step failed'
    )

    post_link_steps = link_cfg.get('post_link', []) or platform_cfg.get('post_link', []) or []
    token_context = {
        'elf': str(out_elf),
        'build_dir': str(BUILD_DIR / platform_name),
        'platform': platform_name,
        'app': selected_app.get('name', 'app') if selected_app else 'app'
    }

    for idx, step in enumerate(post_link_steps, start=1):
        if not isinstance(step, dict):
            print(f"ERROR: post_link step #{idx} must be an object")
            sys.exit(1)

        step_tool_raw = step.get('tool')
        if not step_tool_raw:
            print(f"ERROR: post_link step #{idx} is missing 'tool'")
            sys.exit(1)

        step_tool = resolve_executable(step_tool_raw)
        if not step_tool:
            print(f"ERROR: post_link tool '{step_tool_raw}' not found. Ensure it is available in PATH or use a full path in {BUILD_JSON}.")
            sys.exit(2)

        step_args = render_tokenized_args(step.get('args', []) or [], token_context)
        step_cmd = [step_tool] + step_args

        run_checked_command(
            step_cmd,
            f"ERROR: post_link tool '{step_tool_raw}' not found. Install toolchain or update build.json.",
            f'ERROR: post_link step #{idx} failed'
        )


def clean_platform(platform_name):
    path = BUILD_DIR / platform_name
    if path.exists():
        print('Removing', path)
        shutil.rmtree(path)


def main():
    if len(sys.argv) < 2:
        print('Usage: strat_build.py <application> [--apps-config <path> | --app-config <path>] [--clean] [--rebuild]')
        sys.exit(1)

    app_name = sys.argv[1]
    args = sys.argv[2:]
    clean = '--clean' in args
    rebuild = ('--rebuild' in args) or ('-r' in args)

    apps_config_arg = None
    for cfg_flag in ['--apps-config', '--app-config']:
        if cfg_flag in args:
            cfg_idx = args.index(cfg_flag)
            if cfg_idx + 1 >= len(args):
                print(f'ERROR: {cfg_flag} requires a file path')
                sys.exit(1)
            apps_config_arg = args[cfg_idx + 1]
            break

    apps_cfg_path = apps_config_arg or os.environ.get('STRATOS_APPS_JSON')
    if apps_cfg_path:
        apps_cfg_path = Path(apps_cfg_path)
        if not apps_cfg_path.is_absolute():
            apps_cfg_path = REPO_ROOT / apps_cfg_path
    else:
        apps_cfg_path = DEFAULT_APPS_JSON

    cfg = load_build_json(BUILD_JSON)
    app_cfg = load_optional_json(apps_cfg_path)
    # Load platform aliases (default + optional user overrides)
    alias_default_path = REPO_ROOT / 'default_alias.json'
    alias_user_path = REPO_ROOT / 'alias.json'
    aliases = {}
    if alias_default_path.exists():
        try:
            with open(alias_default_path, 'r', encoding='utf-8') as f:
                aliases.update(json.load(f))
        except Exception:
            pass
    if alias_user_path.exists():
        try:
            with open(alias_user_path, 'r', encoding='utf-8') as f:
                aliases.update(json.load(f))
        except Exception:
            pass
    # normalize alias keys to lower-case for case-insensitive lookup
    alias_map = {k.lower(): v for k, v in aliases.items() if isinstance(k, str) and isinstance(v, str)}

    selected_app = find_application(app_cfg, app_name)
    if not selected_app:
        print(f'Application "{app_name}" not found in {apps_cfg_path}')
        sys.exit(1)

    platform_name = selected_app.get('platform')
    if not platform_name:
        print(f'ERROR: application "{app_name}" must declare "platform" in {apps_cfg_path}')
        sys.exit(1)

    # If platform_name is an alias, map it to the canonical platform name
    if platform_name.lower() in alias_map:
        mapped = alias_map[platform_name.lower()]
        print(f"Using alias: {platform_name} -> {mapped}")
        platform_name = mapped

    platform_cfg = find_platform(cfg, platform_name)
    if not platform_cfg:
        print(f'Platform "{platform_name}" not found in {BUILD_JSON}')
        sys.exit(1)

    if clean:
        clean_platform(platform_name)
        return

    if rebuild:
        print(f"Rebuild requested: cleaning {platform_name}...")
        clean_platform(platform_name)

    # Determine compiler and cflags
    compiler = platform_cfg.get('compiler')
    cflags = platform_cfg.get('cflags', []) or []
    ldflags = platform_cfg.get('ldflags', []) or []

    if not compiler:
        raise Exception(f"No compiler specified for platform '{platform_name}' in {BUILD_JSON}")

    # Verify compiler exists
    compiler_resolved = resolve_executable(compiler)
    if not compiler_resolved:
        print(f"ERROR: compiler '{compiler}' not found. Ensure compiler is available in path or use full path in {BUILD_JSON}.")
        sys.exit(2)
    compiler = compiler_resolved

    modules = collect_modules(cfg, platform_cfg)
    if not modules:
        print('No modules found to build')
        sys.exit(1)

    # Application sources are configured outside build.json and injected at build time.
    modules = [m for m in modules if m.get('name') != 'app']

    if selected_app:
        app_sources = selected_app.get('sources', []) or []
        if not app_sources:
            print(f'ERROR: application "{app_name}" has no sources configured in {apps_cfg_path}')
            sys.exit(1)

        app_module = {
            'name': f'app_{app_name}',
            'sources': app_sources,
            'includes': selected_app.get('includes', []) or [],
            'cflags': selected_app.get('cflags', []) or []
        }
        modules.append(app_module)

    # The OS is highly configurable. To reduce per-module flag setup, we support
    # this precedence order: app defaults -> platform defaults -> module overrides.
    # Module flags are applied last so module-specific settings can win.
    app_global_cflags = []
    if selected_app:
        app_global_cflags = selected_app.get('platform_cflags', []) or selected_app.get('global_cflags', []) or []

    # Optional hard overrides applied after module flags.
    # Use this when a app must force a value regardless of platform/module defaults.
    app_override_cflags = []
    if selected_app:
        app_override_cflags = selected_app.get('override_cflags', []) or []

    platform_includes = []
    for inc in platform_cfg.get('includes', []) if platform_cfg.get('includes') else []:
        p = REPO_ROOT / inc
        platform_includes.append(str(p) if p.exists() else str(inc))

    built_objects = []

    for mod in modules:
        mod_name = mod.get('name', 'unnamed')
        sources = mod.get('sources', [])
        includes = mod.get('includes', []) or []
        mod_cflags = mod.get('cflags', []) or []

        # normalize include dirs relative to repo
        include_dirs = []
        for inc in includes:
            p = (REPO_ROOT / inc)
            if p.exists():
                include_dirs.append(str(p))
            else:
                include_dirs.append(str(inc))

        # Prepend platform includes so they're searched first, preserve order
        for pi in reversed(platform_includes):
            if pi not in include_dirs:
                include_dirs.insert(0, pi)

        # expand sources (glob patterns)
        src_paths = expand_sources(sources)
        if not src_paths:
            print(f"Warning: no sources found for module '{mod_name}'")
            continue

        # compile each source
        for src_path in src_paths:
            base = Path(src_path).name
            if base.endswith('.c'):
                obj_name = base[:-2] + '_c.o'
            elif base.endswith('.S') or base.endswith('.s'):
                obj_name = base.rsplit('.',1)[0] + '_s.o'
            else:
                print(f'Skipping unknown source type: {src_path}')
                continue

            out_obj = BUILD_DIR / platform_name / mod_name / obj_name

            compile_source(compiler, app_global_cflags + cflags + mod_cflags + app_override_cflags, src_path, out_obj, include_dirs)
            built_objects.append(str(out_obj))

    run_platform_link_pipeline(platform_cfg, compiler, built_objects, platform_name, selected_app, ldflags)

    print('Build complete')


if __name__ == '__main__':
    main()
