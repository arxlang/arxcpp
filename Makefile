# path
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# build
ROOT=cd arx
COMPILER=$(ROOT) && clang++
CLEAN=0
CXX=clang++
CC=clang

ARGS:=

# build flags
BUILD_TYPE:=release

CMAKE_EXTRA_FLAGS=
CMAKE_BUILD_TYPE=release

# docker
DOCKER=docker-compose --file docker/docker-compose.yaml


.PHONY: clean-optional
clean-optional:
	bash ./scripts/optclean.sh
	mkdir -p build


# CMAKE
# =====

.ONESHELL:
.PHONY: cmake-build
cmake-build: clean-optional
	mkdir -p $(ROOT_DIR)/build/bin
	cd $(ROOT_DIR)/build
	cmake \
		-GNinja \
		-DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
		-DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
		-DCMAKE_C_COMPILER=${CC} \
    	-DCMAKE_CXX_COMPILER=${CXX} \
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
		--log-level=TRACE \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
		${CMAKE_EXTRA_FLAGS} \
		..
	cmake --build .

.PHONY: cmake-build-with-tests
cmake-build-with-tests:
	$(MAKE)	cmake-build \
		CMAKE_EXTRA_FLAGS="-DENABLE_TESTS=on -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE=include-what-you-use"

.ONESHELL:
.PHONY: cmake-install
cmake-install: cmake-build
	cd build
	cmake --install . --config Release -v


# MESON
# =====

.ONESHELL:
.PHONY: meson-build
meson-build: clean-optional
	set -ex
	meson setup \
		--prefix ${CONDA_PREFIX} \
		--libdir ${CONDA_PREFIX}/lib \
		--includedir ${CONDA_PREFIX}/include \
		--buildtype=${BUILD_TYPE} \
		--native-file meson.native ${ARGS} \
		build .
	meson compile -C build

.ONESHELL:
.PHONY: meson-build-with-tests
meson-build-with-tests:
	set -ex
	$(MAKE) meson-build ARGS="-Ddev=enabled"

.ONESHELL:
.PHONY: meson-install
meson-install:
	meson install -C build


# TESTS
# =====

.ONESHELL:
.PHONY: test-sanity
test-sanity:
	set -ex
	meson test -C build -v

.ONESHELL:
.PHONY: test-examples-llvm
test-examples-llvm:
	set -ex
	./build/arx --show-llvm < examples/test_fibonacci.arx
	@python -c "print('=' * 80)"
	./build/arx  --show-llvm  < examples/test_sum.arx

.ONESHELL:
.PHONY: test-coverage
test-coverage:
	set -ex
	meson compile -C build coverage

.ONESHELL:
.PHONY: test-examples-gen-object
test-examples-gen-object:
	set -ex
	./build/arx --output fibonacci < examples/test_fibonacci.arx
	@python -c "print('=' * 80)"
	./build/arx --output sum  < examples/test_sum.arx
	rm -f fibonacci
	rm -f sum


.PHONY: test-examples
test-examples: test-examples-llvm test-examples-gen-object

.ONESHELL:
.PHONY: run-tests
run-tests: test-sanity test-examples


.PHONY: run-test-opt
run-test-opt:
	# it requires a program that reads dot files (e.g. xdot)
	llvm-as < tests/t.ll | opt -analyze -view-cfg


# CONDA
# =====

.ONESHELL:
.PHONY: conda-build
conda-build: clean-optional
	cd conda/recipe
	mamba update conda conda-build
	conda build purge
	conda mambabuild .
