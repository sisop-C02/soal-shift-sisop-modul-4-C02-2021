#define FUSE_USE_VERSION 28

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static const char *dirpath = "/home/maroqi/Downloads";

static int xmp_getattr(const char * path, struct stat * stbuf)
{
  char fpath[PATH_MAX];
  sprintf(fpath, "%s%s", dirpath, path);

  int res;
  res = lstat(fpath, stbuf);
  if (res == -1) {
    return -errno;
  }

  return 0;
}

static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

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

    int res = 0;
    res = (filler(buf, de->d_name, &st, 0));

    if (res != 0) {
      break;
    }
  }

  closedir(dp);

  return 0;
}

static int xmp_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
  (void)fi;

  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

  int fd = 0;
  fd = open(fpath, O_RDONLY);
  if (fd == -1) {
    return -errno;
  }

  int res = 0;
  res = pread(fd, buf, size, offset);
  if (res == -1) {
    res = -errno;
  }

  close(fd);

  return res;
}

static int xmp_mkdir(const char * path, mode_t mode)
{
  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

  char * ret;
  ret = strstr(path, "RX_");
  if (ret) {
    for (int i = strlen(fpath) - 1; i >= 0; i--) {
      if (fpath[i] == '/') {
        break;
      }

      if(isalpha(fpath[i])) { 
        continue;
      }

      // Atbash Cipher
      char offset;
      if (isupper(fpath[i])) {
        offset = 'A';
      } else {
        offset = 'a';
      }
      fpath[i] = (offset + ('z' - 'a' + 1) - 1) - fpath[i] - offset;

      // ROT13 Cipher
      if (isupper(fpath[i])) {
        if (fpath[i] < 'N') {
          fpath[i] +=13;
        } else if (fpath[i] >= 'N') {
          fpath[i] -=13;
        }
      } else if (islower(fpath[i])) {    
        if (fpath[i] >= 'n') {
          fpath[i] -= 13;
        } else if (fpath[i] < 'n') {
          fpath[i] += 13;
        }
      }
    }

    printf("%s\n", fpath);

    int res;
    res = mkdir(fpath, mode);
    if (res == -1) {
      return -errno;
    }
  } else {
    int res;
    res = mkdir(fpath, mode);
    if (res == -1) {
      return -errno;
    }
  }

	return 0;
}

static int xmp_unlink(const char * path)
{
  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

	int res;
	res = unlink(fpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_rmdir(const char * path)
{
  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

	int res;
	res = rmdir(fpath);
	if (res == -1) {
		return -errno;
  }

	return 0;
}

static int xmp_rename(const char * from, const char * to)
{
  char fpath[PATH_MAX];
  if (strcmp(from, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, from);
  }

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

static struct fuse_operations xmp_oper = {
  .getattr  = xmp_getattr,
  .mkdir    = xmp_mkdir,
  .read     = xmp_read,
  .readdir  = xmp_readdir,
	.rename		= xmp_rename,
	.rmdir		= xmp_rmdir,
	.unlink		= xmp_unlink,
};

int main(int argc, char *argv[])
{
  umask(0);

  return fuse_main(argc, argv, &xmp_oper, NULL);
}
