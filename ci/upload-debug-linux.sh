#!/bin/bash -ex
cd build/install
sentry-cli upload-dif --org axiodl --project metaforce {hecl,metaforce-gui,metaforce,visigen}{,.dbg} --include-sources
