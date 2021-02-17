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

clean:
	@- $(RM) $(BUILD_ROOT)/ 
	@- $(RM) $(RELEASE_ROOT)/
	@- $(RM) $(BIN_ROOT)/
	@- $(RM) $(LIB_ROOT)/
