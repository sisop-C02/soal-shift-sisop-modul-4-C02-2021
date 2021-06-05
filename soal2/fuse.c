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
#include <time.h>
#include <unistd.h>

static const char * dirpath = "/home/maroqi/Downloads";

static void write_log(char level[], char cmd[], char arg1[], char arg2[])
{
  char message[PATH_MAX];

  time_t t_o = time(NULL);
  struct tm tm_s = * localtime(&t_o);
  sprintf(message, "%s::%02d%02d%d-%02d:%02d:%02d::%s", 
    level, tm_s.tm_mday, tm_s.tm_mon + 1, tm_s.tm_year + 1900,
    tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec, cmd);

  if (arg1 != "") {
    strcat(message, "::");
    strcat(message, arg1);
  }

  if (arg2 != "") {
    strcat(message, "::");
    strcat(message, arg2);
  }

  char * file_name = "/home/maroqi/SinSeiFS.log";
  FILE * log_file;
  log_file = fopen(file_name, "a");
  fprintf(log_file, "%s\n", message);

  fclose(log_file);
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

static void en_de_crypt_12(char * str, bool with_rot)
{
  char item_name[PATH_MAX];
  strcpy(item_name, str);
  char ext[PATH_MAX];

  int size = strlen(str);
  for (int i = strlen(str); i >= 0; i--) {
    if (str[i] == '.') {
      size = i;
      break;
    }
  }

  for (int i = 0; i < size; i++) {
    if(!isalpha(str[i])) {
      continue;
    }

    str[i] = atbash_cipher(str[i]);
    if (with_rot) {
      str[i] = rot_13_cipher(str[i]);
    }
  }
}

static bool is_encrypted(char * path, char * by)
{
  if (strstr(path, by)) {
    return true;
  }

  return false;
}

static void split_path(char * parent, char * child, char * by)
{
  char fpath[PATH_MAX];
  strcpy(fpath, parent);
  char * substr = strtok(fpath, "/");

  int counter = 0;
  int marker = 0;
  while(substr != NULL) {
    counter++;
    counter += strlen(substr);

    if (strncmp(substr, by, strlen(by)) == 0) {
      marker = counter;
    }

    if (substr != NULL) {
      substr = strtok(NULL, "/");
    }
  }

  strcpy(child, &parent[marker]);
  parent[marker] = '\0';
}

static void pass_path(char * path, char * fpath)
{
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (is_encrypted(path, "/RX_")) {
      char temp_path[PATH_MAX];
      strcpy(temp_path, path);
      char child_path[PATH_MAX];
      split_path(temp_path, child_path, "RX_");

      en_de_crypt_12(child_path, true);
      sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
    } else if (is_encrypted(path, "/AtoZ_")) {
      char temp_path[PATH_MAX];
      strcpy(temp_path, path);
      char child_path[PATH_MAX];
      split_path(temp_path, child_path, "AtoZ_");

      en_de_crypt_12(child_path, false);
      sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

	int res;
	res = lstat(fpath, stbuf);
	if (res == -1) {
		return -errno;
  }

  write_log("INFO", "GETATTR", path, "");
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	printf("access:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

	int res;
	res = access(fpath, mask);
	if (res == -1) {
		return -errno;
  }

  write_log("INFO", "ACCESS", path, "");
	return 0;
}

static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

	printf("readdir:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

  DIR *dp;
  dp = opendir(fpath);
  if (dp == NULL) {
    return -errno;
  }

  struct dirent *de;
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
  
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;

    int res = 0;
    if (is_encrypted(path, "/RX_")) {
      char item_name[PATH_MAX];
      strcpy(item_name, de->d_name);

      en_de_crypt_12(item_name, true);

      res = (filler(buf, item_name, &st, 0));
    } else if (is_encrypted(path, "/AtoZ_")) {
      char item_name[PATH_MAX];
      strcpy(item_name, de->d_name);

      en_de_crypt_12(item_name, false);

      res = (filler(buf, item_name, &st, 0));
    } else {
      res = (filler(buf, de->d_name, &st, 0));
    }

    if (res != 0) {
      break;
    }
  }

  closedir(dp);
  write_log("INFO", "READDIR", path, "");
  return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	printf("readlink:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

	int res;
	res = readlink(fpath, buf, size - 1);
	if (res == -1) {
		return -errno;
  }
	buf[res] = '\0';

  write_log("INFO", "READLINK", path, "");
	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	printf("symlink:\n");
	printf("%s\n", from);

  char fpath[PATH_MAX];
  pass_path(from, fpath);
  
	printf("%s\n", fpath);

  char dpath[PATH_MAX];
  pass_path(to, dpath);

	int res;
	res = symlink(fpath, dpath);
	if (res == -1) {
		return -errno;
  }

  write_log("INFO", "SYMLINK", from, to);
	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	printf("link:\n");
	printf("%s\n", from);

  char fpath[PATH_MAX];
  pass_path(from, fpath);
  
	printf("%s\n", fpath);

  char dpath[PATH_MAX];
  pass_path(to, dpath);

	int res;
	res = link(fpath, dpath);
	if (res == -1) {
		return -errno;
  }

  write_log("INFO", "LINK", from, to);
	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	printf("truncate:\n");
	printf("%s\n", path);
  
  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);
	
	int res;
	res = truncate(fpath, size);
	if (res == -1) {
		return -errno;
  }

  write_log("INFO", "TRUNCATE", path, "");
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	printf("open:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);
	
	int res;
	res = open(fpath, fi->flags);
	if (res == -1) {
		return -errno;
  }

	close(res);
  write_log("INFO", "OPEN", path, "");
	return 0;
}

static int xmp_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
  (void)fi;

	printf("read:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

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
  write_log("INFO", "READ", path, "");
  return res;
}

static int xmp_mkdir(const char * path, mode_t mode)
{
	printf("mkdir:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

  int res;
  res = mkdir(fpath, mode);
  if (res == -1) {
    return -errno;
  }

  write_log("INFO", "MKDIR", path, "");
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("mknod:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

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

  write_log("INFO", "MKNOD", path, "");
	return 0;
}

static int xmp_rename(const char * from, const char * to)
{
	printf("rename:\n");
	printf("%s\n", from);

  char fpath[PATH_MAX];
  pass_path(from, fpath);

	printf("%s\n", fpath);

  char dpath[PATH_MAX];
  pass_path(to, dpath);

  int res;
  res = rename(fpath, dpath);
  if (res == -1) {
    return -errno;
  }

  write_log("INFO", "RENAME", from, to);
	return 0;
}

static int xmp_unlink(const char * path)
{
	printf("unlink:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

	int res;
	res = unlink(fpath);
	if (res == -1) {
		return -errno;
  }

  write_log("WARNING", "UNLINK", path, "");
	return 0;
}

static int xmp_rmdir(const char * path)
{
	printf("rmdir:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  pass_path(path, fpath);

	printf("%s\n", fpath);

	int res;
	res = rmdir(fpath);
	if (res == -1) {
		return -errno;
  }

  write_log("WARNING", "RMDIR", path, "");
	return 0;
}

static struct fuse_operations xmp_oper = {
  .access   = xmp_access,
  .getattr  = xmp_getattr,
  .link     = xmp_link,
  .mkdir    = xmp_mkdir,
  .mknod    = xmp_mknod,
  .open     = xmp_open,
  .read     = xmp_read,
  .readdir  = xmp_readdir,
  .readlink = xmp_readlink,
	.rename		= xmp_rename,
	.rmdir		= xmp_rmdir,
  .symlink  = xmp_symlink,
  .truncate = xmp_truncate,
	.unlink		= xmp_unlink,
};

int main(int argc, char *argv[])
{
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}
