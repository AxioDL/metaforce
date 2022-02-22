#!/bin/bash -ex
cd build/install
# remove debug files before package
rm -r Metaforce.app/Contents/MacOS/*.dSYM
for i in metaforce crashpad_handler; do
  codesign --timestamp --options runtime -s "$CODESIGN_IDENT" Metaforce.app/Contents/MacOS/$i
done
create-dmg Metaforce.app --identity="$CODESIGN_IDENT" .
xcrun altool -t osx -f *.dmg --primary-bundle-id com.axiodl.Metaforce \
  --notarize-app -u "$ASC_USERNAME" -p "$ASC_PASSWORD" --team-id "$ASC_TEAM_ID"
