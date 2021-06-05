#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

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

static void en_de_crypt_21(char * str, bool is_file)
{
  char item_name[PATH_MAX];
  strcpy(item_name, str);
  char ext[PATH_MAX];
  printf("pass 2\n");

  if (is_file) {
    strcpy(ext, strchr(item_name, '.'));
    item_name[strlen(item_name) - (strlen(ext))] = '\0';
  }
  printf("pass 3\n");

  for (int i = strlen(item_name) - 1; i >= 0; i--) {
    if(!isalpha(item_name[i])) {
      continue;
    }

    item_name[i] = atbash_cipher(item_name[i]);
    item_name[i] = rot_13_cipher(item_name[i]);
  }
  printf("pass 4\n");

  if (is_file) {
    strcat(item_name, ext);
  }
  printf("pass 5\n");
  printf("in %s\n", item_name);
  strcpy(str, item_name);
}

void main ()
{
  char path[PATH_MAX];
  scanf("%s", path);
  char * fpath = path;

  char new_path[PATH_MAX];
  strcpy(new_path, path);
  printf("pass 1\n");
  en_de_crypt_21(new_path, true);
  printf("pass 6\n");
  printf("%s\n", new_path);
  en_de_crypt_21(new_path, true);
  printf("pass 7\n");
  printf("%s\n", new_path);
}