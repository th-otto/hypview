#include "config.h"
#include "windows_.h"
#include <stdint.h>
#include <gemdos.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "dos.h"


#ifndef O_BINARY
#  ifdef _O_BINARY
#    define O_BINARY _O_BINARY
#  endif
#endif
#ifndef O_BINARY
#  define O_BINARY 0
#endif

void dos_chrout(int c)
{
    putc(c, stdout);
}

void dos_conws(const char *str)
{
	while (*str)
	{
		if (*str != '\r')
			fputc(*str, stdout);
		str++;
	}
	fflush(stdout);
}


/* allocate in ST RAM only */
void *dos_alloc_stram(long nbytes)
{
    return malloc(nbytes);
}


/* get max size of available RAM in ST RAM only */
size_t dos_avail(void)
{
    return LONG_MAX;
}


/* allocate in Alt-RAM (e.g. TT RAM) if possible, otherwise ST RAM */
void *dos_alloc_anyram(long nbytes)
{
    return malloc(nbytes);
}


/* get max size of available RAM in TT RAM or ST RAM */
size_t dos_avail_anyram(void)
{
    return LONG_MAX;
}


int dos_free(void *maddr)
{
    free(maddr);
    return 0;
}


void dos_shrink(void *maddr, long length)
{
	void *newptr;
	
	newptr = realloc(maddr, length);
	if (newptr != maddr)
	{
		abort();
    }
}


int dos_open(const char *name, int mode)
{
	if (mode != 0)
		return -1;
	return open(name, O_RDONLY|O_BINARY);
}


int dos_close(int fd)
{
	if (close(fd) == 0)
		return 0;
	return errno;
}


long dos_read(int fd, long size, void *buf)
{
	return read(fd, buf, size);
}


long dos_write(int fd, long size, const void *buf)
{
	return write(fd, buf, size);
}


long dos_lseek(int fd, int whence, long offset)
{
	return lseek(fd, offset, whence);
}


int dos_rename(const char *oldname, const char *newname)
{
	if (rename(oldname, newname) == 0)
		return 0;
	return errno;
}


uint32_t isdrive(void)
{
	return 1 << 2;
}


DTA *dos_getdta(void)
{
	return NULL;
}


int dos_setdta(DTA *dta)
{
	(void) dta;
	return 0;
}


int dos_sdrv(int drv)
{
	(void) drv;
	return 0;
}


int dos_gdrv(void)
{
	return 2;
}


int dos_sfirst(const char *name, int attrib)
{
	(void) name;
	(void) attrib;
	return E_FILNF;
}


int dos_snext(void)
{
	return E_NMFIL;
}


int dos_gdir(int drive, char *pdrvpath)
{
	(void) drive;
	(void) pdrvpath;
	return E_FILNF;
}
