ROOT=cd arx
COMPILER=$(ROOT) && clang++

.PHONY: clean
clean:
	$(ROOT) && rm -f arx


# `llvm-config --cxxflags --ldflags --system-libs --libs core`
# `bash scripts/getflags.sh`
.PHONY: build
build:
	FLAGS=`scripts/getflags.sh` \
	&& echo $$FLAGS \
	&& ${COMPILER} $$FLAGS src/arx.cpp -o arx


.PHONY: build-ast
build-ast:
	FLAGS=`scripts/getflags.sh` \
	&& echo $$FLAGS \
	&& ${COMPILER} $$FLAGS -O3 -Xclang -disable-llvm-passes \
	  -S -emit-llvm src/arx.cpp -o arx.ll \
	&& opt -S -mem2reg -instnamer arx.ll -o arx-ori.ll


.PHONY: run-test
run-test:
	$(ROOT) && ./arx ../tests/source.arw
