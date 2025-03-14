#include "ireadfile_sqlite.h"
#include <limits>
#include <cstdio>
#include <cstring>
#include <sqlite3.h>
#include <IReadFile.h>

 //===========================================================================

namespace {

#define IRR_VFS_NAME "irr-vfs"

constexpr int iVersion = 1;

struct irrfile_t {
	sqlite3_file base;
	long size;
	irr::io::IReadFile* file;
};

template <typename, auto ret>
struct basefunc;

template <auto ret, typename R, typename ...A>
struct basefunc<R(*)(A...), ret> {
	static R value(A...) {
		return (R)ret;
	}
};

#define MAKEDEFIO(func,ret) basefunc<decltype(sqlite3_io_methods::func), ret>::value
#define MAKEDEFFS(func,ret) basefunc<decltype(sqlite3_vfs::func), ret>::value


int fileRead(sqlite3_file* file, void* buffer, int len, sqlite3_int64 offset) {
	auto* irrfile = reinterpret_cast<irrfile_t*>(file);
	if(!irrfile->file || offset > LONG_MAX)
		return SQLITE_IOERR_SHORT_READ;
	if(!irrfile->file->seek((long)offset) || static_cast<int>(irrfile->file->read(buffer, len)) != len)
		return SQLITE_IOERR_SHORT_READ;
	return SQLITE_OK;
}

int fileFileSize(sqlite3_file* file, sqlite3_int64* size) {
	const auto* irrfile = reinterpret_cast<irrfile_t*>(file);
	*size = irrfile->file ? irrfile->size : 0;
	return SQLITE_OK;
}

int fileCheckReservedLock(sqlite3_file* file, int* result) {
	(void)file;
	*result = 0;
	return SQLITE_OK;
}

constexpr sqlite3_io_methods iomethods{
	iVersion,                                 /* iVersion */
	MAKEDEFIO(xClose, SQLITE_OK),             /* xClose */
	fileRead,                                 /* xRead */
	MAKEDEFIO(xWrite, SQLITE_READONLY),       /* xWrite */
	MAKEDEFIO(xTruncate, SQLITE_READONLY),    /* xTruncate */
	MAKEDEFIO(xSync, SQLITE_OK),              /* xSync */
	fileFileSize,                             /* xFileSize */
	MAKEDEFIO(xLock, SQLITE_OK),              /* xLock */
	MAKEDEFIO(xUnlock, SQLITE_OK),            /* xUnlock */
	fileCheckReservedLock,                    /* xCheckReservedLock */
	MAKEDEFIO(xFileControl, SQLITE_OK),       /* xFileControl */
	MAKEDEFIO(xSectorSize, 0),                /* xSectorSize */
	MAKEDEFIO(xDeviceCharacteristics, 0)      /* xDeviceCharacteristics */
};

//===========================================================================

int vfsOpen(sqlite3_vfs* vfs, const char* path, sqlite3_file* file, int flags, int* outflags) {
	(void)vfs;

	if(!(SQLITE_OPEN_READONLY & flags))
		return SQLITE_ERROR;

	void* ptr;
	if(std::sscanf(path, "%p", &ptr) != 1)
		return SQLITE_ERROR;

	auto* irrfile = reinterpret_cast<irrfile_t*>(file);
	irrfile->base = { &iomethods };
	irrfile->file = static_cast<irr::io::IReadFile*>(ptr);
	irrfile->size = irrfile->file->getSize();

	*outflags = SQLITE_OPEN_READONLY;
	return SQLITE_OK;
}

int vfsAccess(sqlite3_vfs* vfs, const char* path, int flags, int* result) {
	(void)vfs;
	(void)path;
	(void)flags;
	*result = 0;
	return SQLITE_OK;
}

int vfsFullPathname(sqlite3_vfs* vfs, const char* path, int len, char* fullpath) {
	(void)vfs;
	sqlite3_snprintf(len, fullpath, "%s", path);
	return SQLITE_OK;
}

//===========================================================================

constexpr auto mxPathname = std::numeric_limits<uintptr_t>::digits / 2;

}

std::unique_ptr<sqlite3_vfs> irrsqlite_createfilesystem() {
	return std::unique_ptr<sqlite3_vfs>(new sqlite3_vfs
		{
			iVersion,                                 /* iVersion */
			sizeof(irrfile_t),                        /* szOsFile */
			mxPathname,                               /* mxPathname */
			nullptr,                                  /* pNext */
			IRR_VFS_NAME,                             /* zName */
			nullptr,                                  /* pAppData */
			vfsOpen,                                  /* xOpen */
			MAKEDEFFS(xDelete, SQLITE_OK),            /* xDelete */
			vfsAccess,                                /* xAccess */
			vfsFullPathname,                          /* xFullPathname */
			MAKEDEFFS(xDlOpen, nullptr),              /* xDlOpen */
			MAKEDEFFS(xDlError, 0),                   /* xDlError */
			MAKEDEFFS(xDlSym, nullptr),               /* xDlSym */
			MAKEDEFFS(xDlClose, 0),                   /* xDlClose */
			MAKEDEFFS(xRandomness, SQLITE_OK),        /* xRandomness */
			MAKEDEFFS(xSleep, SQLITE_OK),             /* xSleep */
			MAKEDEFFS(xCurrentTime, SQLITE_OK),       /* xCurrentTime */
			MAKEDEFFS(xGetLastError, SQLITE_OK)       /* xGetLastError */
		}
	);
}

int irrdb_open(irr::io::IReadFile* reader, sqlite3 **ppDb, int flags) {
	char buff[mxPathname];
	if(std::snprintf(buff, sizeof(buff), "%p", static_cast<void*>(reader)) >= mxPathname)
		return SQLITE_ERROR;
	return sqlite3_open_v2(buff, ppDb, flags, IRR_VFS_NAME);
}

//===========================================================================
