ROOT=cd arx
COMPILER=$(ROOT) && clang++
CLEAN=0

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


.PHONY: cmake-build
cmake-build: clean
	cd build \
	&& cmake .. -GNinja \
	&& cmake --build .

.PHONY: cmake-install
cmake-install: clean
	cd build \
	&& cmake -GNinja -DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} .. \
	&& cmake --build .
	&& cmake --install . --config Release -v
