#!/bin/bash -ex
cd build/Binaries
for f in hecl metaforce-gui metaforce visigen; do
  objcopy --only-keep-debug $f $f.dbg
  objcopy --strip-debug --add-gnu-debuglink=$f.dbg $f
done
tar acfv "$GITHUB_WORKSPACE"/debug.tar.xz -- *.dbg