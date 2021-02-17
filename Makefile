# Makefile wrapper for CMake.

SHELL := /bin/bash
RM    := rm -rf

BUILD_ROOT := build

all: build # default to non-optimized build mode

.PHONY: build debug
build debug: ./build/Makefile
	@ $(MAKE) -j`nproc` -C build

.PHONY: release
release: ./release/Makefile
	@ $(MAKE) -j`nproc` -C release

./build/Makefile:
	@ mkdir -p build
	(cd build >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Debug ..)

./release/Makefile:
	@ mkdir -p release
	(cd release >/dev/null 2>&1 && cmake -DCMAKE_BUILD_TYPE=Release ..)

clean:
	@- $(RM) ./build/ 
	@- $(RM) ./release/
	@- $(RM) ./bin/
	@- $(RM) ./lib/ 
