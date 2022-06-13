#!/bin/bash -ex
cd build/install
for i in Metaforce crashpad_handler; do
  codesign --timestamp --options runtime -s "$CODESIGN_IDENT" Metaforce.app/Contents/MacOS/$i
done
create-dmg Metaforce.app --identity="$CODESIGN_IDENT" .
xcrun altool -t osx -f *.dmg --primary-bundle-id com.axiodl.Metaforce \
  --notarize-app -u "$ASC_USERNAME" -p "$ASC_PASSWORD" --team-id "$ASC_TEAM_ID"
