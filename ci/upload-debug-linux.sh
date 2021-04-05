#!/bin/bash -ex
cd build/Binaries
for f in hecl hecl-gui urde visigen; do
  objcopy --only-keep-debug --compress-debug-sections=zlib $f $f.dbg
done
strip -S hecl hecl-gui urde visigen
sentry-cli upload-dif --org axiodl --project urde {hecl,hecl-gui,urde,visigen}{,.dbg} --include-sources
