# Default PI version is 3
RPI_VERSION ?= 3

# BOOTMNT ?= /media/parallels/boot
ARMGCC ?= aarch64-elf

COPTNS = -DRPI_VERSION=$(RPI_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only

ASMOPTS = -Iinclude

BUILD_DIR = build
SRC_DIR = src

all: kernel8.img

clean:
	rm -rf $(BUILD_DIR)

#----------------------------------------
# CPU configuration
#----------------------------------------

ifdef BUILD_BCM2XXX

ARMGCC = aarch64-elf

BCM2XXX_DIR = bcm2xxx
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

else
$(info )
$(info +---------------------------------------------+)
$(info |                 *** ERROR ***                 )
$(info | Valid CPU configurations are: BUILD_BCM2XXX=1 )
$(info | e.g., make BUILD_BCM2XXX=1  [other options]   )
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

# Rule for building C files
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building assembly files
$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building bcm2xxx C files
$(BUILD_DIR)/%_c.o: $(BCM2XXX_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building bcm2xxx assembly files
$(BUILD_DIR)/%_s.o: $(BCM2XXX_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building aarch64 assembly files
$(BUILD_DIR)/%_s.o: $(AARCH64_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building scheduler C files
$(BUILD_DIR)/%_c.o: $(SCHED_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

# Rule for building HC-SR04 driver C files
$(BUILD_DIR)/%_c.o: $(HW_DRIVER_HC_SR04_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@


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
kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGCC)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGCC)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
