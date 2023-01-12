# build
CLEAN=0
CXX=clang++
CC=clang

ARGS:=

# build flags
BUILD_TYPE:=release

# docker
CONTAINERS=docker-compose \
	--env-file ./.env \
	--file containers/compose.yaml

# release
SEMANTIC_RELEASE=npx --yes \
	-p semantic-release \
	-p "@semantic-release/commit-analyzer" \
	-p "@semantic-release/release-notes-generator" \
	-p "@semantic-release/changelog" \
	-p "@semantic-release/exec" \
	-p "@semantic-release/github" \
	-p "@semantic-release/git" \
	-p "@google/semantic-release-replace-plugin" \
	-p @commitlint/cli \
	-p @commitlint/config-conventional \
	semantic-release



.PHONY: clean-optional
clean-optional:
	bash ./scripts/optclean.sh
	mkdir -p build

.PHONY: clean-gcda
clean-gcda:
	find . -name "*.gcda" -print0 | xargs -0 rm

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
.PHONY: build-dev
build-dev:
	set -ex
	# https://github.com/google/sanitizers/issues/723
	export ASAN_OPTIONS="fast_unwind_on_malloc=0"
	$(MAKE) build \
		BUILD_TYPE="debug" \
		ARGS="-Ddev=enabled -Db_coverage=true -Doptimization=0"  # -Db_sanitize=address

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
.PHONY: code-coverage
code-coverage:
	set -ex
	ninja coverage -C build

.PHONY: test-gen-object
test-gen-object:
	./tests/scripts/test-gen-objects.sh ${ARGS}


.PHONY: test-gen-ast
test-gen-ast:
	./tests/scripts/test-gen-ast.sh ${ARGS}


.PHONY: test-examples
test-examples: test-gen-object test-gen-ast

.ONESHELL:
.PHONY: run-tests
run-tests: test-sanitizer test-examples


.PHONY: run-test-opt
run-test-opt:
	# it requires a program that reads dot files (e.g. xdot)
	llvm-as < tests/t.ll | opt -analyze -view-cfg

.PHONY: run-debug
run-debug:
	LSAN_OPTIONS=verbosity=1:log_threads=1 gdb \
		--args build/arx \
		--input `pwd`/examples/fibonacci.arx \
		--output "/tmp/fibonacci"


# DOCS
# ====

.PHONY: docs-clean
docs-clean:
	rm -rf ./build

.PHONY: docs-api
docs-api:
	mkdir -p build
	doxygen Doxyfile
	./scripts/format_releases.sh

.PHONY: docs-build
docs-build: docs-clean docs-api
	mkdocs build --dirty --site-dir build
	echo "arxlang.org" > ./build/CNAME


.PHONY: docs-watch
docs-watch: docs-clean docs-api
	mkdocs serve --watch docs


# CONDA
# =====

.ONESHELL:
.PHONY: conda-build
conda-build: clean-optional
	cd conda/recipe
	mamba update -y conda conda-build
	conda build purge
	conda mambabuild .


# CONTAINERS
# ==========

.PHONY: create-env-file
create-env-file:
	touch .env
	echo "HOST_UID=`id -u`\nHOST_GID=`id -g`" > .env

.PHONY: container-build
container-build: create-env-file
	$(CONTAINERS) build


.PHONY: container-run
container-run:
	$(CONTAINERS) run arx


# RELEASE
# =======

.PHONY: release
release:
	$(SEMANTIC_RELEASE) --ci


.PHONY: release-dry
release-dry:
	$(SEMANTIC_RELEASE) --dry-run
