#!/bin/bash -ex
cd build/Binaries
sentry-cli upload-dif --org axiodl --project metaforce {hecl,metaforce-gui,metaforce,visigen}{,.dbg} --include-sources
