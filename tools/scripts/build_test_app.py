#!/usr/bin/env python3

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


def load_applications(apps_config):
    with apps_config.open('r', encoding='utf-8') as config_file:
        return json.load(config_file).get('applications', [])


def select_application(apps_config):
    applications = load_applications(apps_config)

    names = [app.get('name') for app in applications if app.get('name')]
    if not names:
        raise SystemExit(f'No applications found in {apps_config}')

    print('Available test applications:')
    for index, name in enumerate(names, start=1):
        print(f'  {index}. {name}')

    while True:
        try:
            selection = input(f'Select an application [1-{len(names)}]: ').strip()
        except EOFError:
            raise SystemExit('No application selected')

        if selection.isdigit():
            index = int(selection) - 1
            if 0 <= index < len(names):
                return next(app for app in applications if app.get('name') == names[index])
        elif selection in names:
            return next(app for app in applications if app.get('name') == selection)

        print('Please enter one of the displayed numbers or application names.')


def main():
    parser = argparse.ArgumentParser(description='Select and build an application from an apps manifest.')
    parser.add_argument('--apps-config', default='test_apps/apps.json')
    parser.add_argument('--rebuild', action='store_true')
    parser.add_argument('--run', action='store_true')
    parser.add_argument('--platform')
    parser.add_argument('--debug-output')
    args = parser.parse_args()

    apps_config = Path(args.apps_config)
    if not apps_config.is_absolute():
        apps_config = REPO_ROOT / apps_config
    if not apps_config.exists():
        raise SystemExit(f'Applications config not found: {apps_config}')

    application_config = select_application(apps_config)
    application = application_config['name']
    platforms = application_config.get('platforms') or [application_config.get('platform')]
    if args.platform:
        if args.platform not in platforms:
            raise SystemExit(f'Application "{application}" does not target platform "{args.platform}"')
        platform = args.platform
    elif len(platforms) == 1:
        platform = platforms[0]
    else:
        print('Target platforms:')
        for index, name in enumerate(platforms, start=1):
            print(f'  {index}. {name}')
        selection = input(f'Select a platform [1-{len(platforms)}]: ').strip()
        if not selection.isdigit() or not 1 <= int(selection) <= len(platforms):
            raise SystemExit('Invalid platform selection')
        platform = platforms[int(selection) - 1]
    command = [sys.executable, str(REPO_ROOT / 'strat_build.py'), application,
               '--apps-config', str(apps_config), '--platform', platform]
    if args.rebuild:
        command.append('--rebuild')

    build_result = subprocess.run(command, cwd=REPO_ROOT)
    if build_result.returncode != 0:
        raise SystemExit(build_result.returncode)

    if platform == 'bcm2xxx':
        deploy_command = [
            'bash', str(REPO_ROOT / 'test.sh'), application,
            '--apps-config', str(apps_config), '--platform', platform, '--skip-build'
        ]
        raise SystemExit(subprocess.run(deploy_command, cwd=REPO_ROOT).returncode)

    if platform != 'win-sim':
        raise SystemExit(f'No run hook is configured for platform "{platform}"')

    output_name = application_config.get('output_name', application)
    executable = REPO_ROOT / 'build' / platform / output_name
    if args.debug_output:
        debug_output = Path(args.debug_output)
        if not debug_output.is_absolute():
            debug_output = REPO_ROOT / debug_output
        debug_output.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(executable, debug_output)
    if not args.run:
        raise SystemExit(0)
    raise SystemExit(subprocess.run([str(executable)], cwd=REPO_ROOT).returncode)


if __name__ == '__main__':
    main()