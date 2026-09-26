#!/usr/bin/env python3

import argparse
import json
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
APPS_CONFIG = REPO_ROOT / 'test_apps' / 'apps.json'
BUILD_CONFIG = REPO_ROOT / 'build.json'
APP_TEST_FRAMEWORK = 'app_test'
APP_TEST_PLATFORM_CFLAGS = {
    'win-sim': [
        '-DUNITY_EXCLUDE_STDINT_H'
    ],
    'bcm2xxx': [
        '-DUNITY_EXCLUDE_STDINT_H',
        '-DUNITY_EXCLUDE_FLOAT',
        '-DUNITY_EXCLUDE_DOUBLE',
        '-DUNITY_EXCLUDE_FLOAT_PRINT',
        '-DUNITY_EXCLUDE_SETJMP_H',
        '-DUNITY_OUTPUT_CHAR(a)=putc(0,(char)(a))',
        '-DUNITY_OUTPUT_CHAR_HEADER_DECLARATION=putc(void* p,char c)'
    ]
}


def load_platforms():
    with BUILD_CONFIG.open('r', encoding='utf-8') as config_file:
        return [platform['name'] for platform in json.load(config_file).get('platforms', [])]


def prompt(value, message, choices=None, multiple=False):
    if value:
        values = value if multiple else [value]
    else:
        choices_text = ', '.join(f'{index + 1}: {choice}' for index, choice in enumerate(choices or []))
        print(choices_text)
        answer = input(message).strip()
        values = [item.strip() for item in answer.split(',')] if multiple else [answer]

    if choices:
        resolved = []
        for item in values:
            if item.isdigit() and 1 <= int(item) <= len(choices):
                item = choices[int(item) - 1]
            if item not in choices:
                raise SystemExit(f'Unknown platform "{item}". Choose from: {", ".join(choices)}')
            if item not in resolved:
                resolved.append(item)
        return resolved if multiple else resolved[0]
    return values if multiple else values[0]


def main():
    parser = argparse.ArgumentParser(description='Create a blank OS test application from a template.')
    parser.add_argument('--name')
    parser.add_argument('--platform', action='append')
    args = parser.parse_args()

    name = prompt(args.name, 'Test name: ')
    if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_]*', name):
        raise SystemExit('Test name must be a C identifier using letters, numbers, and underscores.')

    platforms = load_platforms()
    selected_platforms = prompt(args.platform, 'Platforms (comma-separated): ', platforms, multiple=True)

    with APPS_CONFIG.open('r', encoding='utf-8') as config_file:
        manifest = json.load(config_file)
    applications = manifest.setdefault('applications', [])
    if any(app.get('name') == name for app in applications):
        raise SystemExit(f'Application "{name}" already exists in {APPS_CONFIG}')

    source_dir = REPO_ROOT / 'apps' / name
    source_path = source_dir / f'{name}.c'
    source_dir.mkdir(parents=True, exist_ok=False)
    source_path.write_text(
        '#include "generic.h"\n'
        '#include "application/app_interface.h"\n\n'
        '#include "app_test.h"\n\n'
        'void APP_test_setup(void)\n'
        '{\n'
        '}\n\n'
        'void APP_test_teardown(void)\n'
        '{\n'
        '}\n\n'
        f'static boolean APP_{name}_configure(void)\n'
        '{\n'
        '    return TRUE;\n'
        '}\n\n'
        'boolean APP_sched_configure(void)\n'
        '{\n'
        f'    return APP_{name}_configure();\n'
        '}\n',
        encoding='ascii'
    )

    applications.append({
        'name': name,
        'output_name': name,
        'platforms': selected_platforms,
        'test_framework': APP_TEST_FRAMEWORK,
        'platform_config': {
            platform: {'cflags': APP_TEST_PLATFORM_CFLAGS[platform]}
            for platform in selected_platforms
            if platform in APP_TEST_PLATFORM_CFLAGS
        },
        'sources': [str(source_path.relative_to(REPO_ROOT)).replace('\\', '/')]
    })
    APPS_CONFIG.write_text(json.dumps(manifest, indent=4) + '\n', encoding='utf-8')
    print(f'Created {source_path.relative_to(REPO_ROOT)}')
    print(f'Added {name} for {", ".join(selected_platforms)} to {APPS_CONFIG.relative_to(REPO_ROOT)}')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit('Cancelled')