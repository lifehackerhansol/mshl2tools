make -f Makefile_r4tf
make -f Makefile_r4tf clean
make -f Makefile_m3r4
make -f Makefile_m3r4 clean
make -f Makefile_g003
make -f Makefile_g003 clean
make -f Makefile_ex4tf
make -f Makefile_ex4tf clean
make -f Makefile_scds
make -f Makefile_scds clean
make -f Makefile_iply
make -f Makefile_iply clean
cp scds.dldi ttio.dldi
binreplace ttio.dldi SCDS#/0/0/0 TTIO#/0/0/0
cp scds.dldi demo.dldi
binreplace demo.dldi SCDS#/0/0/0 DEMO#/0/0/0

cp -f r4tf.dldi ../xenofile/source/dldi/r4tf.dldi
cp -f scds.dldi ../xenofile/source/dldi/scds.dldi
cp -f ttio.dldi ../xenofile/source/dldi/ttio.dldi
cp -f demo.dldi ../xenofile/source/dldi/demo.dldi
