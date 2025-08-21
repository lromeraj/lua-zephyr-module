#include <stdio.h>
#include <errno.h>
#include <sys/times.h>
#include <zephyr/fs/fs.h>

/* This stubs should be properly implemented, in zephyr itself or in your
 * project. We provide just a temporal non-working implementation to start
 * testing lua with zephyr, but they will be removed if they become available
 * in the libc libraries shipped with zephyr.
 */


#ifdef CONFIG_PICOLIBC
clock_t times(struct tms* tms) __attribute__((weak));
clock_t times(struct tms* tms)
{
	errno = ENOTSUP;
	return (clock_t) -1;
	
}
#endif

#ifdef CONFIG_NEWLIB_LIBC
int _link(const char *oldpath, const char *newpath) __attribute__((weak));
int _link(const char *oldpath, const char *newpath)
{
	errno = ENOTSUP;
	return -1;
}

int _unlink(const char *pathname) __attribute__((weak));
int _unlink(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}

int _open(const char *pathname) __attribute__((weak));
int _open(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}

int _times(const char *pathname) __attribute__((weak));
int _times(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}
#endif



