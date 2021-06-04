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

static char * vignere_cipher(char * str, bool decode)
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

    char ch;
    if (isupper(str[i])) {
      ch = 'A';
    } else {
      ch = 'a';
    }

    if (!decode) {
      str[i] = ((str[i] + new_key[i]) % 26) + ch;
    } else {
      str[i] = (((str[i] - new_key[i]) + 26) % 26) + ch;
    }
  }

  return str;
}

void main ()
{
  char path[PATH_MAX];
  scanf("%s", path);

  char first[PATH_MAX];
  strcpy(first, path);
  char second[PATH_MAX];
  strcpy(second, path);
  for (int i = strlen(first) - 1; i >= 0; i--) {
    if (first[i] == '/') {
      break;
    }

    if(!isalpha(first[i])) { 
      continue;
    }

    first[i] = atbash_cipher(first[i]);
    first[i] = rot_13_cipher(first[i]);
  }
  printf("%s\n", first);
  printf("%s\n", vignere_cipher(path, false));
  printf("%s\n", second);

  for (int i = strlen(first) - 1; i >= 0; i--) {
    if (first[i] == '/') {
      break;
    }

    if(!isalpha(first[i])) { 
      continue;
    }

    first[i] = atbash_cipher(first[i]);
    first[i] = rot_13_cipher(first[i]);
  }
  printf("%s\n", first);
  printf("%s\n", vignere_cipher(vignere_cipher(second, false), true));
}