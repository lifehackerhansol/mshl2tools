NDS's profile information is written in the last 0x200 bytes of the firmware.
But, GW_Installer writes the exploit into the fixed area 0x1FE00-0x1FFFF.
Hence, if launched on DS(lite) (older than firmware v7), it will be bricked.

However, you might want to try R4 or DSTT, which are "autostart".
If you can launch one of them, you can try this unbricker.

Prepare your (or at least the same version of) firmware as fw.bin.

