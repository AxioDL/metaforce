#!/bin/bash -ex

if [[ -n "${GITHUB_WORKSPACE:-}" ]]; then
  cd "$GITHUB_WORKSPACE"
fi

cd build/install
rm -rf Payload
mkdir Payload
cp -R Metaforce.app Payload/
zip -qry "Metaforce-${APP_VERSION}-ios-arm64.ipa" Payload
rm -rf Payload
