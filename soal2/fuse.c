#define FUSE_USE_VERSION 28

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdbool.h>
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

char upper_case[] = {
    'Z', 'Y', 'X', 'W', 'V', 'U',
 		'T', 'S', 'R', 'Q', 'P', 'O',
		'N', 'M', 'L', 'K', 'J', 'I', 
		'H', 'G', 'F', 'E', 'D', 'C',
    'B', 'A'};

char lower_case[] = {
    'z', 'y', 'x', 'w', 'v', 'u',
    't', 's', 'r', 'q', 'p', 'o',
    'n', 'm', 'l', 'k', 'j', 'i',
    'h', 'g', 'f', 'e', 'd', 'c',
    'b', 'a'};

static char atbash_cipher(char ch)
{
  if(isupper(ch)) {
    return upper_case[ch - 65];
  } else {
    return lower_case[ch - 97];
  }
}

static char rot_13_cipher(char ch)
{
  if (isupper(ch)) {
    if (ch < 'N') {
      ch += 13;
    } else if (ch >= 'N') {
      ch -= 13;
    }
  } else if (islower(ch)) {    
    if (ch >= 'n') {
      ch -= 13;
    } else if (ch < 'n') {
      ch += 13;
    }
  }

  return ch;
}

static int xmp_mkdir(const char * path, mode_t mode)
{
  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }

  char check_dir[PATH_MAX];
  strcpy(check_dir, strchr(path, "/"));
  if (!strstr(check_dir, "RX_")) {
    if (strstr(path, "RX_")) {
      for (int i = strlen(fpath) - 1; i >= 0; i--) {
        if (fpath[i] == '/') {
          break;
        }

        if(!isalpha(fpath[i])) { 
          continue;
        }

        fpath[i] = atbash_cipher(fpath[i]);
        fpath[i] = rot_13_cipher(fpath[i]);
      }

      printf("%s\n", fpath);
    }
  }
  
  int res;
  res = mkdir(fpath, mode);
  if (res == -1) {
    return -errno;
  }

	return 0;
}

static char * vigenere_cipher(char * str)
{
  char * key = "SISOP";
  char new_key[strlen(str)];

  for(int i = 0, j = 0; i < strlen(str); i++, j++) {
    if (j == strlen(key)) {
      j = 0;
    }

    new_key[i] = key[j];
  }

  for(int i = 0; i < strlen(str); ++i) {
    if(!isalpha(str[i])) {
      continue;
    }
    
    str[i] = ((str[i] + new_key[i]) % 26) + 'A';
  }

  return str;
}

static void rename_recursively(char * base_path, bool is_vigenere)
{
  DIR *dir = opendir(base_path);
  if (!dir) {
    return;
  }

  struct dirent *dp;
  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
      char * item = dp->d_name;
      char ext[PATH_MAX];

      char path[PATH_MAX];
      strcpy(path, base_path);
      strcat(path, "/");
      strcat(path, dp->d_name);
  
      if (dp->d_type == DT_DIR) {
        rename_recursively(path, is_vigenere);
      } else if (dp->d_type == DT_REG) {
        strcpy(ext, strchr(item, "."));
        item[strlen(item) - (strlen(ext) + 1)] = '\0';
      }

      if (is_vigenere) {
        for (int i = strlen(item) - 1; i >= 0; i--) {
          if(!isalpha(item[i])) {
            continue;
          }

          item[i] = atbash_cipher(item[i]);
        }
        item = vigenere_cipher(item);
      } else {
        for (int i = strlen(item) - 1; i >= 0; i--) {
          if(!isalpha(item[i])) {
            continue;
          }

          item[i] = atbash_cipher(item[i]);
          item[i] = rot_13_cipher(item[i]);
        }
      }

      char new_path[PATH_MAX];
      strcpy(new_path, base_path);
      strcat(new_path, "/");
      strcat(new_path, item);
      if (dp->d_type == DT_REG) {
        strcat(new_path, ext);
      }

      int res;
      res = rename(path, new_path);
      if (res == -1) {
        return -errno;
      }
    }
  }

  closedir(dir);
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

  char * end_fpath = strchr(from, "/");
  char * ret_fpath = strstr(end_fpath, "RX_");
  char * end_dpath = strchr(to, "/");
  char * ret_dpath = strstr(end_dpath, "RX_");

  if (ret_fpath && !ret_dpath) {
    rename_recursively(fpath, false);
  } else if (!ret_fpath && ret_dpath) {
    // TODO: not implemented yet
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
