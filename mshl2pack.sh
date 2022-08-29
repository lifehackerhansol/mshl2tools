cp -f reset_mse/moonshl2alt.nds .
rm -f mshl2pack_r32.7z
7z a -r0 -mx=9 -xr0!*/.svn/* -xr0!*/.svn mshl2pack_r32.7z arm7 bootlib libfat addons vhbootlib moonshl2alt.nds
