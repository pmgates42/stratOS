WSL install & bare-metal cross-toolchain (concise)

Run these steps in an elevated PowerShell (Administrator). Replace paths/filenames where necessary.

1) Enable required Windows features:
   dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart; dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
   Restart the machine if prompted.

2) Install the WSL2 kernel update (if required):
   Invoke-WebRequest -Uri "https://wslstorestorage.blob.core.windows.net/wslblob/wsl_update_x64.msi" -OutFile "$env:TEMP\\wsl_update_x64.msi"; Start-Process -FilePath "$env:TEMP\\wsl_update_x64.msi" -Wait

3) Import the downloaded distro (.wsl) into WSL:
   New-Item -ItemType Directory -Path 'C:\Distros\Ubuntu' -Force; wsl --import Ubuntu C:\Distros\Ubuntu 'C:\Users\paulm\Downloads\ubuntu-24.04.4-wsl-amd64.wsl' --version 2

4) Launch the distro and create a normal user (first run may be root):
   wsl -d Ubuntu
   # inside WSL:
   adduser <yourname>
   usermod -aG sudo <yourname>
   exit

5) Install build tools inside WSL (as your user):
   wsl -d Ubuntu
   sudo apt update && sudo apt upgrade -y
   sudo apt install -y build-essential make git findutils coreutils

6) Install a bare-metal AArch64 cross-toolchain (recommended for kernel/boot builds)
   - Download a prebuilt aarch64-none-elf (or aarch64-elf) toolchain tar.xz to Windows (e.g. Downloads).
   - In WSL extract and add to PATH (example):
     sudo mkdir -p /opt/gcc-aarch64
     sudo tar -xJf /mnt/c/Users/paulm/Downloads/<toolchain-file>.tar.xz -C /opt/gcc-aarch64 --strip-components=1
     echo 'export PATH=/opt/gcc-aarch64/bin:$PATH' >> ~/.profile
     source ~/.profile
     aarch64-none-elf-gcc --version

   Note: if you only need a Linux userland cross-compiler (not bare-metal), you can use:
     sudo apt install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

7) Build the project (from WSL to use POSIX utilities):
   cd /mnt/c/Users/paulm/repos/stratOS
   # simulator build:
   make SIMULATOR_BUILD=1
   # hardware (bare-metal) build (ensure ARMGCC matches your toolchain prefix):
   make BUILD_BCM2XXX=1 ARMGCC=aarch64-none-elf

8) Notes and alternatives:
   - If you prefer native Windows builds for the simulator, use MSYS2 / MinGW-w64 and ensure gcc is on PATH.
   - For Windows-native hardware cross-toolchains, set absolute compiler paths in build.json or pass ARMGCC to make.
   - If corporate network / policy blocks wsl --install or Store access, importing a downloaded .wsl file (step 3) avoids the Store.

If you want, I can add an automated PowerShell script that performs steps 1–4 and prompts for the downloaded .wsl path.
