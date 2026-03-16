#!/usr/bin/env python3

import os
import sys
import json
import shutil
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent
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


def clean_platform(platform_name):
    path = BUILD_DIR / platform_name
    if path.exists():
        print('Removing', path)
        shutil.rmtree(path)


def main():
    if len(sys.argv) < 2:
        print('Usage: compile.py <platform|application> [--app <application>] [--apps-config <path>] [--clean] [--rebuild]')
        sys.exit(1)

    target_name = sys.argv[1]
    args = sys.argv[2:]
    clean = '--clean' in args
    rebuild = ('--rebuild' in args) or ('-r' in args)

    app_name = None
    if '--app' in args:
        app_idx = args.index('--app')
        if app_idx + 1 >= len(args):
            print('ERROR: --app requires an application name')
            sys.exit(1)
        app_name = args[app_idx + 1]

    apps_config_arg = None
    if '--apps-config' in args:
        cfg_idx = args.index('--apps-config')
        if cfg_idx + 1 >= len(args):
            print('ERROR: --apps-config requires a file path')
            sys.exit(1)
        apps_config_arg = args[cfg_idx + 1]

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

    inferred_platform_from_app = False
    selected_app = None

    if app_name is not None:
        selected_app = find_application(app_cfg, app_name)
        if not selected_app:
            print(f'Application "{app_name}" not found in {apps_cfg_path}')
            sys.exit(1)
        platform_name = target_name
    else:
        selected_app = find_application(app_cfg, target_name)
        if selected_app:
            app_name = target_name
            platform_name = selected_app.get('platform')
            inferred_platform_from_app = True
            if not platform_name:
                print(f'Application "{app_name}" does not declare a platform and no platform argument was provided')
                sys.exit(1)
        else:
            platform_name = target_name

    # If platform_name is an alias, map it to the canonical platform name
    if platform_name.lower() in alias_map:
        mapped = alias_map[platform_name.lower()]
        print(f"Using alias: {platform_name} -> {mapped}")
        platform_name = mapped

    if selected_app and selected_app.get('platform') and not inferred_platform_from_app:
        declared_platform = selected_app.get('platform')
        if declared_platform.lower() in alias_map:
            declared_platform = alias_map[declared_platform.lower()]
        if declared_platform != platform_name:
            print(f'ERROR: application "{app_name}" targets platform "{declared_platform}" but build target is "{platform_name}"')
            sys.exit(1)

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

    # fallback heuristics if compiler not provided
    if not compiler:
        if platform_name.lower().find('bcm') != -1 or platform_name.lower().find('rpi') != -1 or platform_name.lower().find('hw') != -1:
            compiler = 'aarch64-elf-gcc'
        else:
            compiler = 'gcc'

    # Verify compiler exists
    if shutil.which(compiler) is None:
        # try adding .exe on Windows
        if sys.platform == 'win32' and not compiler.endswith('.exe') and shutil.which(compiler + '.exe'):
            compiler = compiler + '.exe'
        else:
            print(f"ERROR: compiler '{compiler}' not found in PATH.")
            print("Install a compatible compiler (e.g. MSYS2/mingw-w64 gcc) or run build inside WSL.")
            print("You can also set an absolute path to the compiler in build.json under the platform 'compiler' field.")
            sys.exit(2)

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

    # Basic link for simulator platform
    if platform_name.lower() in ('sim', 'simulator') or platform_cfg.get('link', True):
        out_exe = BUILD_DIR / platform_name / 'strat_os_sim'
        link_executable(compiler, built_objects, out_exe, ldflags)

    print('Build complete')


if __name__ == '__main__':
    main()
