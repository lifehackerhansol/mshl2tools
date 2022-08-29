As this is very easy libcarddump frontend, only backing up rom is supported...

Well I don't know any better libcarddump sample application...

Please note that some flashcarts cannot initialize DLDI after ejected. This cannot be used for such flashcarts.

To compile on r23, make sure you use my modified libfat (in mshl2pack_r23).
#Or using libfatreduce is also OK.
This does fatUnmount(0), so on normal libfat it can destroy something. (SD unmounting can fail)
