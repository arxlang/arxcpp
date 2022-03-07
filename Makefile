# path
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# build
ROOT=cd arx
COMPILER=$(ROOT) && clang++
CLEAN=0
CXX=clang++
CC=clang

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


.PHONY: run-test
run-test:
	$(ROOT) && ./build/arxc ../tests/source.arw

.PHONY: run-test-opt
run-test-opt:
	# it requires a program that reads dot files (e.g. xdot)
	llvm-as < tests/t.ll | opt -analyze -view-cfg

.PHONY: cmake-build
cmake-build: clean
	cd build \
	&& cmake .. -GNinja \
	&& cmake --build .
	mkdir -p bin
	rm -f bin/*
	mv build/arx bin
	chmod +x bin/arx

.PHONY: cmake-install
cmake-install: clean
	cd build \
	&& cmake \
		-GNinja \
		-DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
		-DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
		-DCMAKE_C_COMPILER=${CC} \
    	-DCMAKE_CXX_COMPILER=${CXX} \
		.. \
	&& cmake --build .
	&& cmake --install . --config Release -v


# CONDA
.PHONY: get-conda-forge-scripts
get-conda-forge-staged-recipes:
	rm -rf /tmp/staged-recipes
	git clone --depth 1 https://github.com/conda-forge/staged-recipes.git /tmp/staged-recipes
	cp -R conda/build /tmp/staged-recipes/recipes/arx
	rm -rf /tmp/staged-recipes/recipes/example
	echo $(ROOT_DIR)
	sed -i "s:{{arx_path}}:$(ROOT_DIR):g" /tmp/staged-recipes/recipes/arx/meta.yaml

.PHONY: conda-build
conda-build: clean get-conda-forge-staged-recipes
	cd /tmp/staged-recipes && python build-locally.py linux64
