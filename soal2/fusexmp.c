/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

static const char *dirpath = "/home/maroqi/Downloads";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = lstat(fpath, stbuf);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	printf("access:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = access(fpath, mask);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	printf("readlink:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = readlink(fpath, buf, size - 1);
	if (res == -1) {
		return -errno;
  }
	buf[res] = '\0';

	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	printf("readdir:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	DIR *dp;
	dp = opendir(fpath);
	if (dp == NULL) {
		return -errno;
  }

	struct dirent *de;
	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0)) {
			break;
    }
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("mknod:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0) {
			res = close(res);
    }
	} else if (S_ISFIFO(mode)) {
		res = mkfifo(fpath, mode);
  } else {
		res = mknod(fpath, mode, rdev);
  }

	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	printf("mkdir:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = mkdir(fpath, mode);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_unlink(const char *path)
{
	printf("unlink:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = unlink(fpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_rmdir(const char *path)
{
	printf("rmdir:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = rmdir(fpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	printf("symlink:\n");
	printf("%s\n", from);
  char fpath[PATH_MAX];

  if (strcmp(from, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, from);
  }
	printf("%s\n", fpath);

  char dpath[PATH_MAX];

  if (strcmp(to, "/") == 0) {
    sprintf(dpath, "%s", dirpath);
  } else {
    sprintf(dpath, "%s%s", dirpath, to);
  }

	int res;
	res = symlink(fpath, dpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	printf("rename:\n");
	printf("%s\n", from);
  char fpath[PATH_MAX];

  if (strcmp(from, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, from);
  }
	printf("%s\n", fpath);

  char dpath[PATH_MAX];

  if (strcmp(to, "/") == 0) {
    sprintf(dpath, "%s", dirpath);
  } else {
    sprintf(dpath, "%s%s", dirpath, to);
  }

	int res;
	res = rename(fpath, dpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	printf("link:\n");
	printf("%s\n", from);
  char fpath[PATH_MAX];

  if (strcmp(from, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, from);
  }
	printf("%s\n", fpath);

  char dpath[PATH_MAX];

  if (strcmp(to, "/") == 0) {
    sprintf(dpath, "%s", dirpath);
  } else {
    sprintf(dpath, "%s%s", dirpath, to);
  }

	int res;
	res = link(fpath, dpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	printf("chmod:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = chmod(fpath, mode);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("chown:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);
	
	int res;
	res = lchown(fpath, uid, gid);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	printf("truncate:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);
	
	int res;
	res = truncate(fpath, size);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	printf("open:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);
	
	int res;
	res = open(fpath, fi->flags);
	if (res == -1) {
		return -errno;
  }

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	(void) fi;
	printf("read:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int fd;
	fd = open(fpath, O_RDONLY);
	if (fd == -1) {
		return -errno;
  }

	int res;
	res = pread(fd, buf, size, offset);
	if (res == -1) {
		res = -errno;
  }

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	printf("write:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int fd;
	fd = open(fpath, O_WRONLY);
	if (fd == -1) {
		return -errno;
  }

	int res;
	res = pwrite(fd, buf, size, offset);
	if (res == -1) {
		res = -errno;
  }

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	printf("symlink:\n");
	printf("%s\n", path);
  char fpath[PATH_MAX];

  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
	printf("%s\n", fpath);

	int res;
	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		  = xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.open		  = xmp_open,
	.read		  = xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
