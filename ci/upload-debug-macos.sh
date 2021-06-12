#!/bin/bash -ex
cd build/install/Metaforce.app/Contents/MacOS
sentry-cli upload-dif --org axiodl --project metaforce {hecl,metaforce-gui,metaforce,visigen}{,.dSYM} --include-sources
