#!/bin/bash -ex
cd build/install/Metaforce.app/Contents/MacOS
sentry-cli upload-dif --org axiodl --project metaforce metaforce{,.dSYM} --include-sources
