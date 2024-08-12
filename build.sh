#!/bin/bash

# Check if the first argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 [sim|hw] [run]"
    exit 1
fi

# Define whether to run the application based on the second argument
RUN_APP=0
if [ "$2" = "run" ]; then
    RUN_APP=1
fi

# Based on the argument, perform the appropriate build
if [ "$1" = "hw" ]; then
    make clean BUILD_BCM2XXX=1
    make BUILD_BCM2XXX=1 RPI_3B_PLUS=1 HW_DRIVER_HC_SR04=1 && cp build/kernel8.img /Volumes/bootfs
elif [ "$1" = "sim" ]; then
    make clean SIMULATOR_BUILD=1
    make SIMULATOR_BUILD=1
else
    echo "Invalid argument: $1"
    echo "Usage: $0 [sim|hw] [run]"
    exit 1
fi

# Run the application if 'run' argument was provided
if [ $RUN_APP -eq 1 ]; then
    if [ "$1" = "hw" ]; then
        echo "Running hardware application..."
        # Add your command to run the hardware application here
    elif [ "$1" = "sim" ]; then
        echo "Running simulator application..."
        ./build/sim/strat_os_sim
    fi
fi
