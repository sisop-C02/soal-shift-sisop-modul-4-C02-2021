#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

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

void main ()
{
  char path[PATH_MAX];
  scanf("%s", path);
  char fpath[PATH_MAX];
  strcpy(fpath, path);
  char new_path[PATH_MAX];
  
  write_log("info", "mkdir", path, "ss");

  path_after(fpath, new_path, "RX_");
  printf("%s ss %s\n", new_path, fpath);

  char parent_path[PATH_MAX];
  strcpy(parent_path, path);
  char * current_path = strchr(path, '/');
  printf("%s\n", current_path);
  if (strlen(path) != strlen(current_path)) {
    parent_path[sizeof(parent_path) - sizeof(current_path)] = '\0';
  }

  bool encrypted = false;
  char * ret_parent_path = strstr(parent_path, "RX_");
  if (ret_parent_path) {
    encrypted = true;
  }

  char * end_fpath = strchr(path, '/');
  printf("strchr\n");
  char * ret_fpath = strstr(path, "RX_");
  printf("strstr\n");

  printf("%s\n", parent_path);
  fpath[0] = 'a';
  printf("%s\n", parent_path);
  printf("strcpy\n");

  if (ret_fpath) {
    printf("ada broo\n");
  }

  printf("%s\n", end_fpath);
}