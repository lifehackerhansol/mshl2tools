#: <<"#BUILD_R23_LIBELM"
rm mshl2tools_r23_libelm.7z
rm mshl2tools_internal_r23_libelm.7z
rm -f /opt/devkitpro
ln -s /opt/devkitpro_r23 /opt/devkitpro
echo "#define LIBELM 1" >libprism/fatdriver.h
LIBELM=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libprism/fatdriver.h
mv mshl2tools_r37.7z mshl2tools_r23_libelm.7z
mv mshl2tools_internal.7z mshl2tools_internal_r23_libelm.7z
#BUILD_R23_LIBELM

#: <<"#BUILD_R37_LIBELM"
rm mshl2tools_r37_libelm.7z
rm mshl2tools_internal_r37_libelm.7z
rm -f /opt/devkitpro
ln -s /opt/devkitpro_r37 /opt/devkitpro
echo "#define LIBELM 1" >libprism/fatdriver.h
LIBELM=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libprism/fatdriver.h
mv mshl2tools_r37.7z mshl2tools_r37_libelm.7z
mv mshl2tools_internal.7z mshl2tools_internal_r37_libelm.7z
#BUILD_R37_LIBELM

#: <<"#BUILD_R23_LIBFAT"
rm mshl2tools_r23_libfat.7z
rm mshl2tools_internal_r23_libfat.7z
rm -f /opt/devkitpro
ln -s /opt/devkitpro_r23 /opt/devkitpro
echo "#define LIBFAT 1" >libprism/fatdriver.h
LIBFAT=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libprism/fatdriver.h
mv mshl2tools_r37.7z mshl2tools_r23_libfat.7z
mv mshl2tools_internal.7z mshl2tools_internal_r23_libfat.7z
#BUILD_R23_LIBFAT

#: <<"#BUILD_R37_LIBFAT"
rm mshl2tools_r37_libfat.7z
rm mshl2tools_internal_r37_libfat.7z
rm -f /opt/devkitpro
ln -s /opt/devkitpro_r37 /opt/devkitpro
echo "#define LIBFAT 1" >libprism/fatdriver.h
LIBFAT=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libprism/fatdriver.h
mv mshl2tools_r37.7z mshl2tools_r37_libfat.7z
mv mshl2tools_internal.7z mshl2tools_internal_r37_libfat.7z
#BUILD_R37_LIBFAT
