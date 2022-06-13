#!/bin/bash -ex
cd build/install
for i in metaforce crashpad_handler; do
  codesign --timestamp --options runtime -s "$CODESIGN_IDENT" metaforce.app/Contents/MacOS/$i
done
create-dmg metaforce.app --identity="$CODESIGN_IDENT" .
xcrun altool -t osx -f *.dmg --primary-bundle-id com.axiodl.Metaforce \
  --notarize-app -u "$ASC_USERNAME" -p "$ASC_PASSWORD" --team-id "$ASC_TEAM_ID"
