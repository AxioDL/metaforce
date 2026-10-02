#!/bin/bash -ex

if [[ -n "${GITHUB_WORKSPACE:-}" ]]; then
  cd "$GITHUB_WORKSPACE"
fi

cd build/install
arch="$(lipo -archs Metaforce.app/Contents/MacOS/Metaforce | tr ' ' '-')"
ditto -c -k --norsrc --noextattr --noacl --keepParent Metaforce.app "Metaforce-${APP_VERSION}-macos-${arch}.zip"
