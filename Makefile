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

.PHONY: test-examples-llvm
test-show-llvm:
	./tests/scripts/test-show-llvm.sh

.ONESHELL:
.PHONY: code-coverage
code-coverage:
	set -ex
	ninja coverage -C build

.PHONY: test-gen-object
test-gen-object:
	./tests/scripts/test-gen-objects.sh


.PHONY: test-examples
test-examples: test-examples-llvm test-gen-object

.ONESHELL:
.PHONY: run-tests
run-tests: test-sanitizer test-examples


.PHONY: run-test-opt
run-test-opt:
	# it requires a program that reads dot files (e.g. xdot)
	llvm-as < tests/t.ll | opt -analyze -view-cfg

# DOCS
# ====

.PHONY: docs-build
docs-build:
	mkdocs build --clean --site-dir build


.PHONY: docs-watch
docs-watch:
	mkdocs serve --watch build


# CONDA
# =====

.ONESHELL:
.PHONY: conda-build
conda-build: clean-optional
	cd conda/recipe
	mamba update conda conda-build
	conda build purge
	conda mambabuild .
