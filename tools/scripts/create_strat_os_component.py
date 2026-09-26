#!/usr/bin/env python3

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
BUILD_CONFIG = REPO_ROOT / 'build.json'
TEST_APP_SCRIPT = REPO_ROOT / 'tools' / 'scripts' / 'create_test_app.py'


def load_platforms():
    with BUILD_CONFIG.open('r', encoding='utf-8') as config_file:
        return [platform['name'] for platform in json.load(config_file).get('platforms', [])]


def prompt_choice(value, message, choices):
    if value:
        answer = value
    else:
        print(', '.join(f'{index + 1}: {choice}' for index, choice in enumerate(choices)))
        answer = input(message).strip()

    if answer.isdigit() and 1 <= int(answer) <= len(choices):
        return choices[int(answer) - 1]
    if answer in choices:
        return answer
    raise SystemExit(f'Unknown choice "{answer}". Choose from: {", ".join(choices)}')


def prompt_yes_no(value, message):
    if value is not None:
        return value
    answer = input(f'{message} [y/N]: ').strip().lower()
    return answer in ('y', 'yes')


def validate_name(name):
    if not re.fullmatch(r'[a-z][a-z0-9_]*', name):
        raise SystemExit('Component name must start with a lowercase letter and contain only lowercase letters, numbers, and underscores.')


def write_external_component(name):
    header_path = REPO_ROOT / 'include' / 'uapi' / f'strat_os_{name}.h'
    source_path = REPO_ROOT / 'common' / 'uapi' / f'strat_os_{name}.c'
    if header_path.exists() or source_path.exists():
        raise SystemExit(f'Component files already exist for "{name}"')

    header_path.write_text(
        '#pragma once\n\n'
        '#include "generic.h"\n\n'
        '#ifdef __cplusplus\n'
        'extern "C" {\n'
        '#endif\n\n'
        f'/* Add the application-facing {name} API here. */\n\n'
        '#ifdef __cplusplus\n'
        '}\n'
        '#endif\n',
        encoding='ascii'
    )
    source_path.write_text(
        f'#include "uapi/strat_os_{name}.h"\n',
        encoding='ascii'
    )
    update_strat_os_module(name)
    return [header_path, source_path]


def write_internal_component(name):
    header_path = REPO_ROOT / 'include' / f'{name}.h'
    source_dir = REPO_ROOT / 'common' / name
    source_path = source_dir / f'{name}.c'
    if header_path.exists() or source_path.exists():
        raise SystemExit(f'Component files already exist for "{name}"')

    source_dir.mkdir(parents=False, exist_ok=False)
    header_path.write_text('#pragma once\n\n', encoding='ascii')
    source_path.write_text(f'#include "{name}.h"\n', encoding='ascii')
    update_internal_build_module(name)
    return [header_path, source_path]


def update_internal_build_module(name):
    with BUILD_CONFIG.open('r', encoding='utf-8') as config_file:
        build_config = json.load(config_file)

    modules = build_config.setdefault('modules', [])
    if not any(module.get('name') == name for module in modules):
        modules.append({
            'name': name,
            'sources': [f'common/{name}/*.c'],
            'includes': [f'common/{name}']
        })

    for platform in build_config.get('platforms', []):
        module_names = platform.setdefault('modules', [])
        if name not in module_names:
            module_names.append(name)

    BUILD_CONFIG.write_text(json.dumps(build_config, indent=4) + '\n', encoding='utf-8')


def update_strat_os_module(name):
    with BUILD_CONFIG.open('r', encoding='utf-8') as config_file:
        build_config = json.load(config_file)

    modules = build_config.setdefault('modules', [])
    strat_os_module = next((module for module in modules if module.get('name') == 'strat_os'), None)
    if strat_os_module is None:
        strat_os_module = {
            'name': 'strat_os',
            'sources': [],
            'includes': ['common/uapi']
        }
        modules.append(strat_os_module)

    sources = strat_os_module.setdefault('sources', [])
    if 'common/uapi/*.c' in sources:
        sources.remove('common/uapi/*.c')
        sources.extend(
            str(path.relative_to(REPO_ROOT)).replace('\\', '/')
            for path in sorted((REPO_ROOT / 'common' / 'uapi').glob('*.c'))
        )

    source = f'common/uapi/strat_os_{name}.c'
    if source not in sources:
        sources.append(source)

    BUILD_CONFIG.write_text(json.dumps(build_config, indent=4) + '\n', encoding='utf-8')


def create_test_app(name, platforms, test_name):
    command = [sys.executable, str(TEST_APP_SCRIPT), '--name', test_name]
    for platform in platforms:
        command.extend(['--platform', platform])
    result = subprocess.run(command, cwd=REPO_ROOT)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description='Create a StratOS component and optionally its integration test app.')
    parser.add_argument('--name')
    parser.add_argument('--interface', choices=['external', 'internal', 'both'])
    parser.add_argument('--platform', action='append')
    parser.add_argument('--with-test', action='store_true')
    parser.add_argument('--without-test', action='store_true')
    parser.add_argument('--test-name')
    args = parser.parse_args()

    name = args.name or input('Component name: ').strip()
    validate_name(name)
    interface = prompt_choice(args.interface, 'Component layers: ', ['external', 'internal', 'both'])
    platforms = args.platform or load_platforms()
    if not args.platform:
        print(f'Targeting all configured platforms: {", ".join(platforms)}')

    if args.with_test and args.without_test:
        raise SystemExit('Use only one of --with-test or --without-test')
    with_test = prompt_yes_no(True if args.with_test else False if args.without_test else None, 'Create an integration test application?')

    files = []
    if interface in ('external', 'both'):
        files.extend(write_external_component(name))
    if interface in ('internal', 'both'):
        files.extend(write_internal_component(name))

    print('Created:')
    for path in files:
        print(f'  {path.relative_to(REPO_ROOT)}')

    if with_test:
        create_test_app(name, platforms, args.test_name or f'{name}_test')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit('Cancelled')