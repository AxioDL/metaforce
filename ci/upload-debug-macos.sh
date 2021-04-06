#!/bin/bash -ex
cd build/Binaries/metaforce-gui.app/Contents/MacOS
for f in hecl metaforce-gui metaforce visigen; do
  dsymutil $f
done
strip -S hecl metaforce-gui metaforce visigen crashpad_handler
sentry-cli upload-dif --org axiodl --project metaforce {hecl,metaforce-gui,metaforce,visigen}{,.dSYM} --include-sources
rm -r -- *.dSYM
