#!/usr/bin/env bash
set -ex

export CXXFLAGS="`echo $CXXFLAGS | sed 's/-fno-exceptions//'`"

meson setup \
  --prefix ${PREFIX} \
  --libdir ${PREFIX}/lib \
  --includedir ${PREFIX}/include \
  --buildtype=release \
  --native-file meson.native \
  build .
meson compile -C build

mv ./build/arx ${PREFIX}/bin/arx
chmod +x ${PREFIX}/bin/arx
