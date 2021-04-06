bintoc(droidsans-permissive.cpp droidsans-permissive.ttf.gz DROIDSANS_PERMISSIVE)
bintoc(bmonofont-i18n.cpp bmonofont-i18n.ttf.gz BMONOFONT)
bintoc(SpecterCurveGlyphs.cpp SpecterCurveGlyphs.ttf.gz SPECTERCURVES)
add_library(specter-fonts droidsans-permissive.cpp bmonofont-i18n.cpp SpecterCurveGlyphs.cpp)
