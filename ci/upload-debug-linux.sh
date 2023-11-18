#!/bin/bash -ex
cd build/install
sentry-cli upload-dif --org axiodl --project metaforce metaforce{,.dbg} --include-sources
