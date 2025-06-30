# my.mk - wrapper around CubeMX Makefile

PROJECT_NAME := testWork
BUILD_DIR := build
TARGET_ELF := $(BUILD_DIR)/$(PROJECT_NAME).elf
TARGET_BIN := $(BUILD_DIR)/$(PROJECT_NAME).bin
CUBEMX_MAKE := Makefile

# CubeMX build
.PHONY: build
build:
	@echo "🔨 Building project..."
	$(MAKE) -f $(CUBEMX_MAKE)

# Flash using st-flash
.PHONY: flash
flash: build
	@echo "⚡ Flashing binary to STM32..."
	st-flash write $(TARGET_BIN) 0x8000000

# Run clang-tidy only on your sources
.PHONY: tidy
tidy:
	@echo "🔍 Running clang-tidy..."
	@if [ -n "$(FILE)" ]; then \
		echo "→ $(FILE)"; \
		clang-tidy-15 "$(FILE)" -- \
			-std=c11 \
			-DSTM32F407xx -DUSE_HAL_DRIVER \
			-I./Core/Inc \
			-I./Drivers/CMSIS/Include \
			-I./Drivers/CMSIS/Device/ST/STM32F4xx/Include \
			-I./Drivers/STM32F4xx_HAL_Driver/Inc || true; \
	else \
		find Core/Src -name "*.c" | while read file; do \
			echo "→ $$file"; \
			clang-tidy-15 "$$file" -- \
				-std=c11 \
				-DSTM32F407xx -DUSE_HAL_DRIVER \
				-I./Core/Inc \
				-I./Drivers/CMSIS/Include \
				-I./Drivers/CMSIS/Device/ST/STM32F4xx/Include \
				-I./Drivers/STM32F4xx_HAL_Driver/Inc || true; \
		done; \
	fi

# Format only your sources (skip Drivers/)
.PHONY: format
format:
	@echo "Formatting code..."
	@find Core -name "*.[ch]" | xargs clang-format-15 --verbose -i  

# Clean
.PHONY: clean
clean:
	@echo "🧹 Cleaning build files..."
	$(MAKE) -f $(CUBEMX_MAKE) clean
