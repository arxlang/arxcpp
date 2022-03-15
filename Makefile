# path
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# build
ROOT=cd arx
COMPILER=$(ROOT) && clang++
CLEAN=0
CXX=clang++
CC=clang

# build flags
CMAKE_EXTRA_FLAGS=
CMAKE_BUILD_TYPE=release

# docker
DOCKER=docker-compose --file docker/docker-compose.yaml


.PHONY: clean
clean:
	bash ./scripts/optclean.sh
	mkdir -p build


# `llvm-config --cxxflags --ldflags --system-libs --libs core`
# `bash scripts/getflags.sh`
.PHONY: build
build: clean
	FLAGS=`scripts/getflags.sh` \
	&& echo $$FLAGS \
	&& ${COMPILER} $$FLAGS src/arx.cpp -o ../build/arxc


.PHONY: build-ast
build-ast: clean
	FLAGS=`scripts/getflags.sh` \
	&& echo $$FLAGS \
	&& ${COMPILER} $$FLAGS -O3 -Xclang -disable-llvm-passes \
	  -S -emit-llvm src/arx.cpp -o ../build/arx.ll \
	&& opt -S -mem2reg -instnamer ../build/arx.ll -o ../build/arx-ori.ll

.ONESHELL:
.PHONY: test-sanity
test-sanity:
	cd build/tests
	ctest --verbose --label-regex sanity

.ONESHELL:
.PHONY: test-source-code
test-source-code:
	./bin/arx < tests/data/test_fibonacci.arx
	@python -c "print('=' * 80)"
	./bin/arx < tests/data/test_sum.arx

.ONESHELL:
.PHONY: run-tests
run-tests: test-sanity test-sorce-code


.PHONY: run-test-opt
run-test-opt:
	# it requires a program that reads dot files (e.g. xdot)
	llvm-as < tests/t.ll | opt -analyze -view-cfg


.ONESHELL:
.PHONY: cmake-build
cmake-build: clean
	mkdir -p $(ROOT_DIR)/bin
	cd $(ROOT_DIR)/build
	cmake \
		-GNinja \
		-DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
		-DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
		-DCMAKE_C_COMPILER=${CC} \
    	-DCMAKE_CXX_COMPILER=${CXX} \
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
		${CMAKE_EXTRA_FLAGS} \
		..
	cmake --build .

.PHONY: cmake-publish
cmake-publish: cmake-build
	cp $(ROOT_DIR)/build/arx $(ROOT_DIR)/bin
	chmod +x $(ROOT_DIR)/bin/arx

.ONESHELL:
.PHONY: cmake-install
cmake-install: cmake-build
	cd build
	cmake --install . --config Release -v

# CONDA
.ONESHELL:
.PHONY: conda-build
conda-build: clean
	cd conda/build
	conda build purge
	conda mambabuild .
