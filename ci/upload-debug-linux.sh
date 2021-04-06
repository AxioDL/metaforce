#!/bin/bash -ex
cd build/Binaries
for f in hecl metaforce-gui metaforce visigen; do
  objcopy --only-keep-debug --compress-debug-sections=zlib $f $f.dbg
done
strip -S hecl metaforce-gui metaforce visigen
sentry-cli upload-dif --org axiodl --project metaforce {hecl,metaforce-gui,metaforce,visigen}{,.dbg} --include-sources
