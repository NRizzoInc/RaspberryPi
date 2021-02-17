# Makefile wrapper for CMake.

SHELL			:= /bin/bash
RM				:= rm -rf
BUILD_ROOT		:= ./build
RELEASE_ROOT	:= ./release
BIN_ROOT		:= ./bin
LIB_ROOT		:= ./lib

all: build # default to non-optimized build mode

.PHONY: build debug
build debug: $(BUILD_ROOT)/Makefile
	@ $(MAKE) -j`nproc` -C $(BUILD_ROOT)

.PHONY: release
release: $(RELEASE_ROOT)/Makefile
	@ $(MAKE) -j`nproc` -C $(RELEASE_ROOT)

./build/Makefile:
	@ mkdir -p $(BUILD_ROOT)
	(cd $(BUILD_ROOT) >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Debug ..)

./release/Makefile:
	@ mkdir -p release
	(cd $(RELEASE_ROOT) >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Release ..)

# cross compile options (define IS_CROSS_COMPILE)
.PHONY: arm_debug arm_build
arm_debug arm_build: $(BUILD_ROOT)/arm/Makefile
	@ $(MAKE) -j`nproc` -C $(BUILD_ROOT)/arm/

.PHONY: arm_release
arm_release: $(RELEASE_ROOT)/arm/Makefile
	@ $(MAKE) -j`nproc` -C $(RELEASE_ROOT)/arm/

./build/arm/Makefile:
	@ mkdir -p $(BUILD_ROOT)/arm/
	(cd $(BUILD_ROOT)/arm/ >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Debug -DIS_CROSS_COMPILE=true ../..)

./release/arm/Makefile:
	@ mkdir -p $(RELEASE_ROOT)/arm/
	(cd $(RELEASE_ROOT)/arm/ >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Release -DIS_CROSS_COMPILE=true ../..)

clean:
	@- $(RM) $(BUILD_ROOT)/ 
	@- $(RM) $(RELEASE_ROOT)/
	@- $(RM) $(BIN_ROOT)/
	@- $(RM) $(LIB_ROOT)/
