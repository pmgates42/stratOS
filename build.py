#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil

def run_make_command(args):
    try:
        result = subprocess.run(
            ["make"] + args,
            check=True,
            text=True,
            capture_output=True
        )
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("Build failed!")
        print("stdout:")
        print(e.stdout)
        print("stderr:")
        print(e.stderr)
        sys.exit(e.returncode)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} [-a] [sim|hw] [run]")
        sys.exit(1)
    
    # Check if -a flag is present
    delete_build = "-a" in sys.argv
    
    # Remove -a from arguments list if present
    args = [arg for arg in sys.argv[1:] if arg != "-a"]
    
    if len(args) < 1:
        print(f"Usage: {sys.argv[0]} [-a] [sim|hw] [run]")
        sys.exit(1)
    
    build_type = args[0]
    run_app = len(args) > 1 and args[1] == "run"
    
    if delete_build and os.path.exists("build"):
        print("Deleting build folder...")
        shutil.rmtree("build")
    
    if build_type == "hw":
        run_make_command(["BUILD_BCM2XXX=1", "RPI_3B_PLUS=1", "HW_DRIVER_HC_SR04=1"])
    elif build_type == "sim":
        run_make_command(["SIMULATOR_BUILD=1"])
    else:
        print("Invalid argument:", build_type)
        print(f"Usage: {sys.argv[0]} [-a] [sim|hw] [run]")
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
            # you could add hardware run logic here if needed
        elif build_type == "sim":
            print("Running simulator application...")
            try:
                subprocess.run(["./build/sim/strat_os_sim"], check=True)
            except subprocess.CalledProcessError as e:
                print("Simulator run failed!")
                sys.exit(e.returncode)

if __name__ == "__main__":
    main()
