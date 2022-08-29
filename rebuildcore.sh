: <<"#LIBELM"
cd libelm
make || exit
rm -f /opt/devkitPro/libnds/lib/libelm.a
cp -f lib/libelm.a /opt/devkitPro/libnds/lib/
make clean
cd ..
#LIBELM

cd arm7
make;make clean
cd ..
cd libprism
make;make clean
cd ..
