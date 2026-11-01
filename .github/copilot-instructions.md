# StratOS

Bare-metal aarch64 (`bcm2xxx`) and host simulator (`win-sim`) OS. Builds run in WSL.

## Layout
- Public (app-facing) API: `include/uapi/strat_os_<name>.h` + `common/uapi/strat_os_<name>.c`.
- Internal module: `include/<name>.h` + `common/<name>/<name>.c`.
- Drivers in `drivers/<dev>/`; platform code in `platform/{bcm2xxx,sim,hw_common}/`; scheduler in `ssched/`.
- Do not edit vendored code under `test/unit/libs/`.
- Boot/low level: `boot/` (entry, linker script), `aarch64/` (asm, sysregs), `armstub/`. Other: `sfs/`, `tools/sim_frontend/`.

## Build config
- `build.json`: `platforms[]` (`bcm2xxx`, `win-sim`) each with `compiler`, `cflags`, `includes`, `link`, and a `modules` name list; top-level `modules[]` define `sources` (globs) and `includes`. A new module must be defined in `modules[]` and added to each platform's `modules`.
- `apps.json`: product apps (`default`, `default_bcm`); `test_apps/apps.json`: test apps. Each app has `name`, `output_name`, `sources`, and `platform` or `platforms` + `platform_config` (per-platform `cflags` / `override_cflags`).
- `"test_framework": "app_test"` pulls in the `app_test` module (`test/app_test/` + vendored Unity); it is not listed in platform `modules`.
- Output goes to `build/<platform>/` (`win-sim` host binary named by `output_name`; `bcm2xxx` produces `kernel8.elf` / `kernel8.img`).

## Tests
- On-target/sim test apps: `apps/<name>_test/` built from `test_apps/apps.json` (Unity via `test/app_test/`).
- Host unit tests: `test/unit/{ssched,spi}/` using Unity in `test/unit/libs/unity/`.
- `test.sh <app>` builds for `bcm2xxx` and copies `kernel8.img`/`kernel8.elf` to `D:\` (SD card); it needs hardware, so do not run it unprompted.

## Scaffolding
- New component: `python3 tools/scripts/create_strat_os_component.py` (WSL); do not hand-create files or edit `build.json` for it.
- New test app: `python3 tools/scripts/create_test_app.py`; it writes `apps/<name>_test/` and an entry in `test_apps/apps.json`.

## Build
- `python3 strat_build.py <application> [--apps-config test_apps/apps.json] [--platform win-sim] [--rebuild]`, run via `wsl bash -lc "..."`. Prefer the workspace tasks when they fit.
- New modules go in `build.json`; apps are declared in `apps.json` / `test_apps/apps.json` with `platform` or `platforms`.
- The root `Makefile` is legacy; do not use it. `sim_build.sh` / `hw_build.sh` just call `strat_build.py` for `default` / `default_bcm`. Only `test/unit/*` has its own make-based `run.sh`.
- gdb "Source file is more recent than executable" means a clean rebuild is needed.
- After edits, build the affected app for `win-sim` and check for new errors and warnings.
