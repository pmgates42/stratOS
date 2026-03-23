Build instructions for stratOS

Overview

This repository includes a small Python-based build driver `strat_build.py` that compiles app profiles declared in `apps.json` (or another apps config file).

Quick usage

- Build the default simulator app profile:
  python3 strat_build.py default

- Build the default bare-metal app profile:
  python3 strat_build.py default_bcm

- Remove build artifacts for an app's platform:
  python3 strat_build.py <application> --clean

- Rebuild (clean then build) an app:
  python3 strat_build.py <application> --rebuild
  or
  python3 strat_build.py <application> -r

- Build with a non-default apps config file:
  python3 strat_build.py sim_sched_rate_test --apps-config test_apps/apps.json

- Run a built simulator app:
  ./build/win-sim/default

What strat_build.py does

- Reads `build.json` to determine platforms and modules.
- Reads app profiles from `apps.json` (or `--apps-config` input).
- Requires each app profile to declare a `platform`.
- Compiles sources into `build/<platform>/...` object files.
- By default it links only platforms that request a host-style link. The `bcm` (bare-metal) platform in the default manifest has linking disabled (`"link": false`) because final bare-metal linking requires careful linker-script and objcopy steps.

Prerequisites

Simulator (sim)
- A native GCC toolchain (linux: system gcc; Windows: use MSYS2/MinGW or run inside WSL).
- On Windows builds the simulator links against Winsock; ensure the linker flag `-lws2_32` is available (MSYS2/MinGW provides this).

Bare-metal AArch64 (bcm)
- An AArch64 bare-metal cross-toolchain. Recommended prebuilt toolchain: `arm-gnu-toolchain-*-x86_64-aarch64-none-elf.tar.xz` from Arm GNU Toolchain releases. The tarball provides `aarch64-none-elf-gcc`.
- Typical install (run inside WSL or Linux):
  sudo mkdir -p /opt/gcc-aarch64-none-elf
  sudo tar -xJf /path/to/arm-gnu-toolchain-<version>-x86_64-aarch64-none-elf.tar.xz -C /opt/gcc-aarch64-none-elf --strip-components=1
  echo 'export PATH=/opt/gcc-aarch64-none-elf/bin:$PATH' >> ~/.profile
  source ~/.profile
  aarch64-none-elf-gcc --version

- The build manifest (`build.json`) contains the platform compiler string (e.g. `"aarch64-none-elf-gcc"`). If your toolchain uses a different binary name (for example `aarch64-elf-gcc`), either:
  - edit `build.json` to set the `compiler` field for the `bcm` platform to the installed compiler name, or
  - create a small symlink (in WSL/Linux):
    sudo ln -s /opt/gcc-aarch64-none-elf/bin/aarch64-none-elf-gcc /usr/local/bin/aarch64-elf-gcc

Notes about final bare-metal linking

- `strat_build.py` compiles object files for the `bcm` platform but does not perform the final link / objcopy / image creation steps that the original Makefile performs. To produce a bootable kernel image (kernel8.img) you should run the project's Makefile from a POSIX environment (WSL recommended) with a properly installed cross-toolchain:
  make BUILD_BCM2XXX=1 ARMGCC=aarch64-none-elf

- If you prefer, the Python builder can be extended to run the exact linker command + objcopy sequence. Open an issue or request this and it can be added.

Developer notes

- The build manifest is `build.json` and defines platforms and module source lists.
- The Python driver supports these command-line options: `--apps-config`, `--clean`, `--rebuild` (`-r`).

Commit message suggestion

more improvements to the build system.

Change:
	Use a bespoke build solution for this project (to be
	made generic later).

	can now compile project by running:

  python3 strat_build.py <application>
            [--apps-config <path>] # app config file override
            [--clean] remove generated build artifacts for app platform
            [--rebuild|-r] # Clean and build application

If you want this file moved/renamed or additional details added (linker scripts, exact Makefile link commands, Windows/MSYS2 steps), tell me which items to include and I will update the file.
