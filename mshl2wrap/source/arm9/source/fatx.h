//Undocumented APIs used for fatx.c
bool _FAT_directory_entryGetAlias(const u8* entryData, char* destName);
size_t _FAT_directory_mbstoucs2(u16* dst, const char* src, size_t len);

void getsfnlfn(const char *path,char *sfn,u16 *lfn);
