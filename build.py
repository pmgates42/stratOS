#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil

def main():
    if len(sys.argv) < 2:
        print("Usage: {} [sim|hw] [run]".format(sys.argv[0]))
        sys.exit(1)

    build_type = sys.argv[1]
    run_app = len(sys.argv) > 2 and sys.argv[2] == "run"

    if build_type == "hw":
        subprocess.run(["make", "clean", "BUILD_BCM2XXX=1"], check=True)
        subprocess.run(["make", "BUILD_BCM2XXX=1", "RPI_3B_PLUS=1", "HW_DRIVER_HC_SR04=1"], check=True)
    elif build_type == "sim":
        subprocess.run(["make", "clean", "SIMULATOR_BUILD=1"], check=True)
        subprocess.run(["make", "SIMULATOR_BUILD=1"], check=True)
    else:
        print("Invalid argument:", build_type)
        print("Usage: {} [sim|hw] [run]".format(sys.argv[0]))
        sys.exit(1)


    if build_type == "hw":
        build_dir = "build/rpi_3"
        img_path = os.path.join(build_dir, "kernel8.img")

        destination = "/Volumes/bootfs"

        try:
            shutil.copy(img_path, destination)
            print(f"Copied {img_path} to {destination}")
        except PermissionError:
            print("Error: Permission denied. Try running with sudo.")
        except FileNotFoundError:
            print("Error: /Volumes is not mounted")
        except Exception as e:
            print(f"Error: {e}")

    if run_app:
        if build_type == "hw":
            print("Running hardware application...")
        elif build_type == "sim":
            print("Running simulator application...")
            subprocess.run(["./build/sim/strat_os_sim"], check=True)

if __name__ == "__main__":
    main()
