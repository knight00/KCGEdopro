add_library(clzma STATIC)

target_sources(clzma PRIVATE
	Alloc.c
	Alloc.h
	LzFind.c
	LzFind.h
	LzHash.h
	LzmaDec.c
	LzmaDec.h
	LzmaEnc.c
	LzmaEnc.h
	LzmaLib.c
	LzmaLib.h
	Types.h)
