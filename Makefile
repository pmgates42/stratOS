# Default PI version is 3 sub-version 1 (3B+)
RPI_VERSION ?= 3
RPI_SUB_VERSION ?= 1

# BOOTMNT ?= /media/parallels/boot
ARMGCC ?= aarch64-elf

#----------------------------------------
# Default compiler to GCC. This will be
# likely be overridden if doing any kind
# of embedded build
#----------------------------------------
GCC = gcc
COMPILER = $(GCC)


ifdef SIMULATOR_BUILD
COPTNS = -DRPI_VERSION=$(RPI_VERSION)  -DRPI_SUB_VERSION=$(RPI_SUB_VERSION) -Wall -Iinclude -Iinclude/public
# Ensure linking against standard libraries
LDFLAGS = -lc -lgcc
else
COPTNS = -DEMBEDDED_BUILD=1 -DRPI_VERSION=$(RPI_VERSION)  -DRPI_SUB_VERSION=$(RPI_SUB_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -Iinclude/public  -mgeneral-regs-only
ASMOPTS = -Iinclude
endif

#----------------------------------------
# Define main source directory
#----------------------------------------
SRC_DIR = src


#----------------------------------------
# Determine the output build directory
#----------------------------------------
ifdef BUILD_BCM2XXX

	PLATFORM = rpi_$(RPI_VERSION)
	ifdef DRPI_SUB_VERSION
		PLATFORM += _$(DRPI_SUB_VERSION)
	endif

else ifdef SIMULATOR_BUILD
	PLATFORM = sim
endif

BUILD_DIR = build/$(PLATFORM)

#----------------------------------------
# Core system files
#----------------------------------------
ifdef SIMULATOR_BUILD
CORE_DIR = core/sim
else
CORE_DIR = core/hw
endif

# Recursively find all C and Assembly files in CORE_DIR
CORE_C_FILES := $(shell find $(CORE_DIR) -name '*.c')
CORE_ASM_FILES := $(shell find $(CORE_DIR) -name '*.S')

# Generate object files from the found source files
CORE_OBJ_FILES := $(CORE_C_FILES:$(CORE_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(CORE_ASM_FILES:$(CORE_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(CORE_OBJ_FILES)

# Include the directory for header files
COPTNS += -I$(CORE_DIR)/include

#----------------------------------------
# Define default target
#----------------------------------------
ifdef SIMULATOR_BUILD
all: simulator
else
all: kernel8.img
endif

clean:
	rm -rf $(BUILD_DIR)

#----------------------------------------
# Comon source files
#----------------------------------------
COMMON_DIR = /common
COMMON_C_FILES := $(wildcard $(COMMON_DIR)/*.c)
OBJ_FILES += $(COMMON_C_FILES)

#----------------------------------------
# Build platform (CPU) specific files
#----------------------------------------

ifdef BUILD_BCM2XXX

	ifdef SIMULATOR_BUILD
		$(info  Simulator build for BUILD_BCM2XXX does not exist currently.)
		$(error Invalid CPU configuration)
	endif

	$(info 	Compiler determined from targeted hardware:)
	$(info 	aarch64-elf-gcc)
	ARMGCC = aarch64-elf
	COMPILER = aarch64-elf-gcc

	BCM2XXX_DIR = platform/bcm2xxx
	BCM2XXX_C_FILES := $(wildcard $(BCM2XXX_DIR)/*.c)
	BCM2XXX_ASM_FILES := $(wildcard $(BCM2XXX_DIR)/*.S)
	BCM2XXX_OBJ_FILES := $(BCM2XXX_C_FILES:$(BCM2XXX_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(BCM2XXX_ASM_FILES:$(BCM2XXX_DIR)/%.S=$(BUILD_DIR)/%_s.o)
	OBJ_FILES += $(BCM2XXX_OBJ_FILES)
	COPTNS += -I$(BCM2XXX_DIR)/include

	AARCH64_DIR = aarch64
	AARCH64_C_FILES := $(wildcard $(AARCH64_DIR)/*.c)
	AARCH64_ASM_FILES := $(wildcard $(AARCH64_DIR)/*.S)
	AARCH64_OBJ_FILES := $(AARCH64_C_FILES:$(AARCH64_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(AARCH64_ASM_FILES:$(AARCH64_DIR)/%.S=$(BUILD_DIR)/%_s.o)
	OBJ_FILES += $(AARCH64_OBJ_FILES)
	COPTNS += -I$(AARCH64_DIR)/include

else ifdef SIMULATOR_BUILD

	# Define the platform/sim directory
	PLATFORM_SIM_DIR = platform/sim

	# Recursively find all C and assembly files in the platform/sim directory
	PLATFORM_SIM_C_FILES := $(wildcard $(PLATFORM_SIM_DIR)/**/*.c)
	PLATFORM_SIM_ASM_FILES := $(wildcard $(PLATFORM_SIM_DIR)/**/*.S)

	# Generate object files for the C and assembly files
	PLATFORM_SIM_OBJ_FILES := $(PLATFORM_SIM_C_FILES:$(PLATFORM_SIM_DIR)/%.c=$(BUILD_DIR)/%_c.o) \
							$(PLATFORM_SIM_ASM_FILES:$(PLATFORM_SIM_DIR)/%.S=$(BUILD_DIR)/%_s.o)

	# Add the object files to the list of objects to build
	OBJ_FILES += $(PLATFORM_SIM_OBJ_FILES)

	# Include the platform/sim directory in the include paths
	COPTNS += -I$(PLATFORM_SIM_DIR)/include
else
$(info )
$(info +---------------------------------------------+)
$(info |                 *** ERROR ***                )
$(info | Must specify platform					      )
$(info | e.g., make BUILD_BCM2XXX=1  			      )
$(info +---------------------------------------------+)
$(info )

$(error Invalid CPU configuration)

endif

#----------------------------------------
# Scheduler configuration
#----------------------------------------

SCHED_DIR = ssched
SCHED_C_FILES := $(wildcard $(SCHED_DIR)/*.c)
SCHED_ASM_FILES := $(wildcard $(SCHED_DIR)/*.S)
SCHED_OBJ_FILES := $(SCHED_C_FILES:$(SCHED_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(SCHED_ASM_FILES:$(SCHED_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(SCHED_OBJ_FILES)
COPTNS += -I$(SCHED_DIR)/include
COPTNS += -DSSCHED_SHOW_DEBUG_DATA

#----------------------------------------
# Hardware driver configurations
#----------------------------------------

ifdef HW_DRIVER_HC_SR04
COPTNS += -DHW_DRIVER_HC_SR04=1

HW_DRIVER_HC_SR04_DIR = drivers/hc_sr04
HW_DRIVER_HC_SR04_C_FILES := $(wildcard $(HW_DRIVER_HC_SR04_DIR)/*.c)
HW_DRIVER_HC_SR04_ASM_FILES := $(wildcard $(HW_DRIVER_HC_SR04_DIR)/*.S)
HW_DRIVER_HC_SR04_OBJ_FILES := $(HW_DRIVER_HC_SR04_C_FILES:$(HW_DRIVER_HC_SR04_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(HW_DRIVER_HC_SR04_ASM_FILES:$(HW_DRIVER_HC_SR04_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(HW_DRIVER_HC_SR04_OBJ_FILES)
COPTNS += -I$(HW_DRIVER_HC_SR04_DIR)/include

else
endif

#----------------------------------------
# RPI specific hardware drivers
#----------------------------------------

ifdef RPI_VERSION

	#----------------------------------------
	# DWC2 USB Driver
	#----------------------------------------
	ifdef SIMULATOR_BUILD
		# don't do anything
	else
		# Include DWC2 driver files for embedded Raspberry Pi build
		DWC2_DIR = drivers/usb/dwc2
		DWC2_C_FILES := $(wildcard $(DWC2_DIR)/*.c)
		DWC2_OBJ_FILES := $(DWC2_C_FILES:$(DWC2_DIR)/%.c=$(BUILD_DIR)/%_c.o)
		OBJ_FILES += $(DWC2_OBJ_FILES)

		# Show debug data by default
		COPTNS += -DDWC2_SHOW_DEBUG_DATA
	endif

endif

#----------------------------------------
# Build rules
#----------------------------------------

# Rule for building C files
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building assembly files
$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building bcm2xxx C files
$(BUILD_DIR)/%_c.o: $(BCM2XXX_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building bcm2xxx assembly files
$(BUILD_DIR)/%_s.o: $(BCM2XXX_DIR)/%.S
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building core assembly files
$(BUILD_DIR)/%_s.o: $(CORE_DIR)/%.S
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building core C files
$(BUILD_DIR)/%_c.o: $(CORE_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building aarch64 assembly files
$(BUILD_DIR)/%_s.o: $(AARCH64_DIR)/%.S
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building scheduler C files
$(BUILD_DIR)/%_c.o: $(SCHED_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building HC-SR04 driver C files
$(BUILD_DIR)/%_c.o: $(HW_DRIVER_HC_SR04_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Rule for building DWC2 driver C files
$(BUILD_DIR)/%_c.o: $(DWC2_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

# Build rules for the platform/sim files
$(BUILD_DIR)/%_c.o: $(PLATFORM_SIM_DIR)/%.c
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(PLATFORM_SIM_DIR)/%.S
	mkdir -p $(@D)
	$(COMPILER) $(COPTNS) -MMD -c $< -o $@

SRC_TOP_DIR := $(SRC_DIR)
VPATH := $(SRC_TOP_DIR):$(shell find $(SRC_TOP_DIR) -type d)

SRC_C_FILES := $(wildcard $(addsuffix /*.c, $(VPATH)))
SRC_ASM_FILES := $(wildcard $(addsuffix /*.S, $(VPATH)))

ASM_FILES := $(wildcard $(SRC_DIR)/*.S)
C_FILES := $(wildcard $(SRC_DIR)/*.c)

all_src_files := $(SRC_C_FILES:$(SRC_TOP_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(ASM_FILES:$(SRC_TOP_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(all_src_files)
OBJ_FILES += $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)

DEP_FILES := $(OBJ_FILES:.o=.d)
-include $(DEP_FILES)

# Build rule for kernel file using linker script and object files, then convert to a binary file
kernel8.img: $(CORE_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGCC)-ld -T $(CORE_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGCC)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img

armstub/build/armstub_s.o: armstub/src/armstub.S
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

armstub: armstub/build/armstub_s.o
	$(ARMGCC)-ld --section-start=.text=0 -o armstub/build/armstub.elf armstub/build/armstub_s.o
	$(ARMGCC)-objcopy armstub/build/armstub.elf -O binary armstub/build/armstub-new.bin

simulator: $(OBJ_FILES)
	$(COMPILER) -o $(BUILD_DIR)/strat_os_sim $(OBJ_FILES)
