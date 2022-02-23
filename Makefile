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
.PHONY: conda-build
conda-build: clean
	$(DOCKER) build conda-build
	$(DOCKER) up conda-build -d
	$(DOCKER) exec conda-build bash -c "mamba install --update-specs --yes --quiet --channel conda-forge conda-build pip boa conda-forge-ci-setup=3"
	$(DOCKER) exec conda-build bash -c "mamba update --update-specs --yes --quiet --channel conda-forge conda-build pip boa conda-forge-ci-setup=3"
	$(DOCKER) exec conda-build bash -c "cd /opt/arx/conda/recipe && conda mambabuild ."
	$(DOCKER) stop conda-build
