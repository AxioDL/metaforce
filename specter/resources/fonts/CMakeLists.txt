bintoc(droidsans-permissive.c droidsans-permissive.ttf.gz DROIDSANS_PERMISSIVE)
bintoc(bmonofont-i18n.c bmonofont-i18n.ttf.gz BMONOFONT)
bintoc(SpecterCurveGlyphs.c SpecterCurveGlyphs.ttf.gz SPECTERCURVES)
add_library(specter-fonts droidsans-permissive.c bmonofont-i18n.c SpecterCurveGlyphs.c)
