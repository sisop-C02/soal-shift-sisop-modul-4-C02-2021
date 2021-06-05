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

static void en_de_crypt_21(char * str)
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
    str[i] = rot_13_cipher(str[i]);
  }
}

static bool is_encrypted(char * path, char * by)
{
  if (strstr(path, by)) {
    return true;
  }

  return false;
}

static void path_after(char * parent, char * child, char * by)
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

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (is_encrypted(path, "/RX_")) {
      char temp_path[PATH_MAX];
      strcpy(temp_path, path);
      char child_path[PATH_MAX];
      path_after(temp_path, child_path, "RX_");

      en_de_crypt_21(child_path);
      sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }

	printf("%s\n", fpath);

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

	printf("readdir:\n");
	printf("%s\n", path);

  bool encrypted = false;
  char * ret_parent_path = strstr(path, "/RX_");
  if (ret_parent_path) {
    encrypted = true;
  }

  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (encrypted) {
      char temp_path[PATH_MAX];
      strcpy(temp_path, path);
      char child_path[PATH_MAX];
      path_after(temp_path, child_path, "RX_");

      // if (strlen(child_path) == 0) {
      //   encrypted = false;
      // }

      en_de_crypt_21(child_path);
      sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }

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
    if (encrypted) {
      char item_name[PATH_MAX];
      strcpy(item_name, de->d_name);

      en_de_crypt_21(item_name);

	    printf("%s\n", item_name);

      res = (filler(buf, item_name, &st, 0));
    } else {
      res = (filler(buf, de->d_name, &st, 0));
    }

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

	printf("read:\n");
	printf("%s\n", path);

  char fpath[PATH_MAX];
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (is_encrypted(path, "/RX_")) {
      char temp_path[PATH_MAX];
      strcpy(temp_path, path);
      char child_path[PATH_MAX];
      path_after(temp_path, child_path, "RX_");

      en_de_crypt_21(child_path);
      sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }

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
  return res;
}

static int xmp_mkdir(const char * path, mode_t mode)
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

  write_log("INFO", "MKDIR", path, "");

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
