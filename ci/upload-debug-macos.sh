#!/bin/bash -ex
cd build/Binaries/hecl-gui.app/Contents/MacOS
for f in hecl hecl-gui urde visigen; do
  dsymutil $f
done
strip -S hecl hecl-gui urde visigen crashpad_handler
sentry-cli upload-dif --org axiodl --project urde {hecl,hecl-gui,urde,visigen}{,.dSYM} --include-sources
rm -r -- *.dSYM
