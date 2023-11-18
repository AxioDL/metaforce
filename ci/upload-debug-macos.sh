#!/bin/bash -ex
cd build/install
sentry-cli upload-dif --org axiodl --project metaforce metaforce.app/Contents/MacOS/metaforce metaforce.dSYM --include-sources
