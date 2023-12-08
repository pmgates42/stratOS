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

ifdef BUILD_BCM2XXX
BCM2XXX_DIR = bcm2xxx
BCM2XXX_C_FILES := $(wildcard $(BCM2XXX_DIR)/*.c)
BCM2XXX_ASM_FILES := $(wildcard $(BCM2XXX_DIR)/*.S)
BCM2XXX_OBJ_FILES := $(BCM2XXX_C_FILES:$(BCM2XXX_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(BCM2XXX_ASM_FILES:$(BCM2XXX_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(BCM2XXX_OBJ_FILES)
COPTNS += -I$(BCM2XXX_DIR)/include

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

SCHED_DIR = ssched
SCHED_C_FILES := $(wildcard $(SCHED_DIR)/*.c)
SCHED_ASM_FILES := $(wildcard $(SCHED_DIR)/*.S)
SCHED_OBJ_FILES := $(SCHED_C_FILES:$(SCHED_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(SCHED_ASM_FILES:$(SCHED_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(SCHED_OBJ_FILES)
COPTNS += -I$(SCHED_DIR)/include

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

# Rule for building scheduler C files
$(BUILD_DIR)/%_c.o: $(SCHED_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC)-gcc $(COPTNS) -MMD -c $< -o $@

C_FILES := $(wildcard $(SRC_DIR)/*.c)
ASM_FILES := $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES += $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o) $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES := $(OBJ_FILES:.o=.d)
-include $(DEP_FILES)

# Build rule for kernel file using linker script and object files, then convert to a binary file
kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGCC)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGCC)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
