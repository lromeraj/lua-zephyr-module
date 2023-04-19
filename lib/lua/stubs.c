#include <time.h>
#include <errno.h>
#include <zephyr/fs/fs.h>

clock_t _times(struct tm* tm)
{
	errno = ENOSYS;
	return (clock_t) -1;
}

int _unlink(const char *pathname)
{
	return fs_unlink(pathname);
}

int _link(const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}