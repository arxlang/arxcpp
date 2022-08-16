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

# docker
DOCKER=docker-compose --file docker/docker-compose.yaml


.PHONY: clean-optional
clean-optional:
	bash ./scripts/optclean.sh
	mkdir -p build


.ONESHELL:
.PHONY: build
build: clean-optional
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
.PHONY: build-with-tests
build-with-tests:
	set -ex
	$(MAKE) build ARGS="-Ddev=enabled -Db_coverage=true -Db_sanitize=address"

.ONESHELL:
.PHONY: install
install:
	meson install -C build


# TESTS
# =====

.ONESHELL:
.PHONY: test-sanitizer
test-sanitizer:
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
.PHONY: code-coverage
code-coverage:
	set -ex
	ninja coverage -C build

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
run-tests: test-sanitizer test-examples


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
