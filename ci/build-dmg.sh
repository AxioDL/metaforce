#!/bin/bash -ex
cd build/install
# remove debug files before package
rm -r Metaforce.app/Contents/MacOS/*.dSYM
# order is important
for i in visigen hecl metaforce crashpad_handler; do
  codesign --timestamp --options runtime -s "$CODESIGN_IDENT" Metaforce.app/Contents/MacOS/$i
done
macdeployqt Metaforce.app -sign-for-notarization="$CODESIGN_IDENT" -no-strip
create-dmg Metaforce.app --identity="$CODESIGN_IDENT" .
xcrun altool -t osx -f *.dmg --primary-bundle-id com.axiodl.Metaforce \
  --notarize-app -u "$ASC_USERNAME" -p "$ASC_PASSWORD" --team-id "$ASC_TEAM_ID"
