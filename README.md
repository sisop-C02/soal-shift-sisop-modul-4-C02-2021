# lapres-sisop-modul-4-C02-2021
Laporan resmi berisi dokumentasi soal shift Sisop Modul 4.
---
Kelompok C-02:
- [Jason Andrew Gunawan](https://github.com/jasandgun): 05111940000085
- [Muchamad Maroqi Abdul Jalil](https://github.com/maroqijalil): 05111940000143
- [Muhammad Zhafran Musyaffa](https://github.com/franszhafran): 05111940000147
---

## Soal 1
### Penjelasan Soal.
Di suatu jurusan, terdapat admin lab baru yang super duper gabut, ia bernama Sin. Sin
baru menjadi admin di lab tersebut selama 1 bulan. Selama sebulan tersebut ia bertemu
orang-orang hebat di lab tersebut, salah satunya yaitu Sei. Sei dan Sin akhirnya
berteman baik. Karena belakangan ini sedang ramai tentang kasus keamanan data,
mereka berniat membuat filesystem dengan metode encode yang mutakhir. Berikut
adalah filesystem rancangan Sin dan Sei:
- (a) Jika sebuah direktori dibuat dengan awalan “AtoZ_”, maka direktori tersebut akan
menjadi direktori ter-encode.
- (b) Jika sebuah direktori di-rename dengan awalan “AtoZ_”, maka direktori tersebut
akan menjadi direktori ter-encode.
- (c) Apabila direktori yang terenkripsi di-rename menjadi tidak ter-encode, maka isi
direktori tersebut akan terdecode.
- (d) Setiap pembuatan direktori ter-encode (mkdir atau rename) akan tercatat ke
sebuah log. Format : /home/[USER]/Downloads/[Nama Direktori] →
/home/[USER]/Downloads/AtoZ_[Nama Direktori]
- (e) Metode encode pada suatu direktori juga berlaku terhadap direktori yang ada di
dalamnya.(rekursif)
### Solusi dan Penjelasannya
Berikut jawaban per poin untuk soal nomor 2:
- Inti dari solusi soal ini adalah melakukan encode pada saat pemanggilan fungsi `readdir`.
```cpp
static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  ...

  return 0;
}
```
- Jadi, dalam prosesnya, ketika sebuah direktori di buka maka fungsi `readdir` pada fuse akan dipanggil. 
Fungsi ini akan membaca satu per-satu file/folder yang ada pada direktori tersebut.
```cpp
static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  ...

  struct dirent *de;
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }

    ...

    if (res != 0) {
      break;
    }
  }

  ...

  return 0;
}
```
- Ketika file/folder tersebut dibaca, akan dicek apakah file/folder tersebut berada pada direktori 
yang ter-encode. Jika iya, maka akan dilakukan encode menggunakan fungsi `en_de_crypt_12_file`.
```cpp
static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  ...

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
    if (!is_special) {
      if (is_encrypted(path, "/RX_") && strncmp(de->d_name, "A_is_a_", 7)) {
        ...
      } else if (is_encrypted(path, "/AtoZ_") && strncmp(de->d_name, "A_is_a_", 7)) {
        char item_name[PATH_MAX];
        strcpy(item_name, de->d_name);

        en_de_crypt_12_file(item_name, false);

        res = (filler(buf, item_name, &st, 0));
      } else {
        res = (filler(buf, de->d_name, &st, 0));
      }
    } else {
      res = (filler(buf, de->d_name, &st, 0));
    }

    if (res != 0) {
      break;
    }
  }

  ...

  return 0;
}
```
- Fungsi `en_de_crypt_12_file` akan melakukan encode pada sebuah string sekaligus dapat melakukan decode. 
Fungsi ini dikhsuskan string yang bukan merupakan berformat direktori.
```cpp
static void en_de_crypt_12_file(char * str, bool with_rot)
{
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

    ...
  }
}
```
- Karena sudah dilakukan sebuah perubahan nama file/folder akibat encode, maka perlu dilakukan decode 
ketika fungsi `getattr` dipanggil.
```cpp
static int xmp_getattr(const char *path, struct stat *stbuf)
{
  char fpath[PATH_MAX];
  pass_path(path, fpath, true);

	int res;
	res = lstat(fpath, stbuf);
	if (res == -1) {
		return -errno;
  }

  ...

	return 0;
}
```
- Pemanggilan fungsi decode dilakukan di dalam fungsi `pass_path`. Fungsi ini akan mengganti direkori 
root '/' dengan direktori sesuai ketentuan soal '/home/[USER]/Downloads'.
```cpp
static void pass_path(char * path, char * fpath, bool with_check)
{
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    ...
  }
}
```
- Dalam fungsi ini akan dilakukan pengecekan terlebih dahulu apakah path tersebut terencode.
```cpp
static void pass_path(char * path, char * fpath, bool with_check)
{
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (with_check) {
      if (is_encrypted(path, "/RX_")) {
        ...
      } else if (is_encrypted(path, "/AtoZ_")) {
        char temp_path[PATH_MAX];
        strcpy(temp_path, path);
        char child_path[PATH_MAX];
        split_path(temp_path, child_path, "AtoZ_");

        en_de_crypt_12_dir(child_path, false);
        sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
      } else {
        sprintf(fpath, "%s%s", dirpath, path);
      }
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }
}
```
- Jika iya, maka akan dilakukan pemanggilan fungsi `en_de_crypt_12_dir` (kurang lebih sama dengan 
fungsi `en_de_crypt_12_file`) untuk melakukan encode pada direktori 
yang sebelumnya telah dipisah menggunakan fungsi `split_path`.
```cpp
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

    if (!strncmp(substr, by, strlen(by))) {
      marker = counter;
    }

    if (substr != NULL) {
      substr = strtok(NULL, "/");
    }
  }

  strcpy(child, &parent[marker]);
  parent[marker] = '\0';
}
```
- Untuk melakukan pengaksesan system call secara tepat, maka semua system call akan memanggil fungsi 
`pass_path`. Sehingga dengan ini semua akses berupa **rename** dan **mkdir** akan dapat dilakukan sesuai dengan 
ketentuan soal yang diberikan. Yaitu akan terencode dengan *atbash cipher* ketika sebuah folder memiliki 
prefix 'AtoZ_' dan akan terdecode jika prefix tersebut dihilangkan.

## Soal 2
### Penjelasan Soal.
Selain itu Sei mengusulkan untuk membuat metode enkripsi tambahan agar data pada
komputer mereka semakin aman. Berikut rancangan metode enkripsi tambahan yang
dirancang oleh Sei:
- (a) Jika sebuah direktori dibuat dengan awalan “RX_[Nama]”, maka direktori
tersebut akan menjadi direktori terencode beserta isinya dengan perubahan
nama isi sesuai kasus nomor 1 dengan algoritma tambahan ROT13 (Atbash +
ROT13).
- (b) Jika sebuah direktori di-rename dengan awalan “RX_[Nama]”, maka direktori
tersebut akan menjadi direktori terencode beserta isinya dengan perubahan
nama isi sesuai dengan kasus nomor 1 dengan algoritma tambahan Vigenere
Cipher dengan key “SISOP” (Case-sensitive, Atbash + Vigenere).
- (c) Apabila direktori yang terencode di-rename (Dihilangkan “RX_” nya), maka folder
menjadi tidak terencode dan isi direktori tersebut akan terdecode berdasar nama
aslinya.
- (d) Setiap pembuatan direktori terencode (mkdir atau rename) akan tercatat ke
sebuah log file beserta methodnya (apakah itu mkdir atau rename).
- (e) Pada metode enkripsi ini, file-file pada direktori asli akan menjadi terpecah
menjadi file-file kecil sebesar 1024 bytes, sementara jika diakses melalui
filesystem rancangan Sin dan Sei akan menjadi normal. Sebagai contoh,
Suatu_File.txt berukuran 3 kiloBytes pada directory asli akan menjadi 3 file kecil
yakni:
> Suatu_File.txt.0000
> Suatu_File.txt.0001
> Suatu_File.txt.0002
Ketika diakses melalui filesystem hanya akan muncul Suatu_File.txt
### Solusi dan Penjelasannya.
- Pada soal ini, kami hanya mengerjakan encode dan decode ketika sebuah folder dibuat dengan prefix 'RX_'.
- Hal yang pertama dilakukan adalah dengan menambahkan kondisi pada iterasi pembacaan file/folder yang ada 
di fungsi `readdir`.
```cpp
static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  char fpath[PATH_MAX];
  pass_path(path, fpath, true);

	printf("%s\n", fpath);

  DIR *dp;
  dp = opendir(fpath);
  if (dp == NULL) {
    return -errno;
  }

  ...

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
    if (!is_special) {
      if (is_encrypted(path, "/RX_") && strncmp(de->d_name, "A_is_a_", 7)) {
        char item_name[PATH_MAX];
        strcpy(item_name, de->d_name);

        en_de_crypt_12_file(item_name, true);

        res = (filler(buf, item_name, &st, 0));
      } else if (is_encrypted(path, "/AtoZ_") && strncmp(de->d_name, "A_is_a_", 7)) {
        char item_name[PATH_MAX];
        strcpy(item_name, de->d_name);

        en_de_crypt_12_file(item_name, false);

        res = (filler(buf, item_name, &st, 0));
      } else {
        res = (filler(buf, de->d_name, &st, 0));
      }
    } else {
      res = (filler(buf, de->d_name, &st, 0));
    }

    if (res != 0) {
      break;
    }
  }

  closedir(dp);
  ...
  return 0;
}
```
- Selain itu, pada fungsi `pass_path` juga ditambahkan kondisi untuk memerikasa apakah folder terencode 
dengan 'RX_';
```cpp
static void pass_path(char * path, char * fpath, bool with_check)
{
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    if (with_check) {
      if (is_encrypted(path, "/RX_")) {
        char temp_path[PATH_MAX];
        strcpy(temp_path, path);
        char child_path[PATH_MAX];
        split_path(temp_path, child_path, "RX_");

        en_de_crypt_12_dir(child_path, true);
        sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
      } else if (is_encrypted(path, "/AtoZ_")) {
        char temp_path[PATH_MAX];
        strcpy(temp_path, path);
        char child_path[PATH_MAX];
        split_path(temp_path, child_path, "AtoZ_");

        en_de_crypt_12_dir(child_path, false);
        sprintf(fpath, "%s%s%s", dirpath, temp_path, child_path);
      } else {
        sprintf(fpath, "%s%s", dirpath, path);
      }
    } else {
      sprintf(fpath, "%s%s", dirpath, path);
    }
  }
}
```

## Soal 3
### Penjelasan Soal.
Karena Sin masih super duper gabut akhirnya dia menambahkan sebuah fitur lagi pada
filesystem mereka.
- (a) Jika sebuah direktori dibuat dengan awalan “A_is_a_”, maka direktori tersebut
akan menjadi sebuah direktori spesial.
- (b) Jika sebuah direktori di-rename dengan memberi awalan “A_is_a_”, maka
direktori tersebut akan menjadi sebuah direktori spesial.
- (c) Apabila direktori yang terenkripsi di-rename dengan menghapus “A_is_a_” pada
bagian awal nama folder maka direktori tersebut menjadi direktori normal.
- (d) Direktori spesial adalah direktori yang mengembalikan enkripsi/encoding pada
direktori “AtoZ_” maupun “RX_” namun masing-masing aturan mereka tetap
berjalan pada direktori di dalamnya (sifat recursive “AtoZ_” dan “RX_” tetap
berjalan pada subdirektori).
- (e) Pada direktori spesial semua nama file (tidak termasuk ekstensi) pada fuse akan
berubah menjadi lowercase insensitive dan diberi ekstensi baru berupa nilai
desimal dari binner perbedaan namanya.
> Contohnya jika pada direktori asli nama filenya adalah “FiLe_CoNtoH.txt” maka
> pada fuse akan menjadi “file_contoh.txt.1321”. 1321 berasal dari biner
> 10100101001.
### Solusi dan Penjelasannya.
- Dari soal ini, kami hanya menghandle pengerjaan pada perintah **mkdir** dan **rename** pada sebuah 
folder dengan diberi prefix 'A_is_a_'.
- Untuk menghandle nya kami membuat tambahan kondisi pada saat melakukan iterasi pada file/folder di fungsi `readdir` 
dengan sebelumnya melakukan pengecekan apakah folder tersebut merupakan sebuah folder spesial.
```cpp
static int xmp_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
  (void)offset;
  (void)fi;

  char fpath[PATH_MAX];
  pass_path(path, fpath, true);

  DIR *dp;
  dp = opendir(fpath);
  if (dp == NULL) {
    return -errno;
  }

  bool is_special = false;
  if (is_encrypted(path, "/A_is_a_")) {
    char temp_path[PATH_MAX];
    strcpy(temp_path, path);
    char child_path[PATH_MAX];
    split_path(temp_path, child_path, "A_is_a_");
    if (!strcmp(child_path, "")) {
      is_special = true;
    }
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
    if (!is_special) {
      if (is_encrypted(path, "/RX_") && strncmp(de->d_name, "A_is_a_", 7)) {
        char item_name[PATH_MAX];
        strcpy(item_name, de->d_name);

        en_de_crypt_12_file(item_name, true);

        res = (filler(buf, item_name, &st, 0));
      } else if (is_encrypted(path, "/AtoZ_") && strncmp(de->d_name, "A_is_a_", 7)) {
        char item_name[PATH_MAX];
        strcpy(item_name, de->d_name);

        en_de_crypt_12_file(item_name, false);

        res = (filler(buf, item_name, &st, 0));
      } else {
        res = (filler(buf, de->d_name, &st, 0));
      }
    } else {
      res = (filler(buf, de->d_name, &st, 0));
    }

    if (res != 0) {
      break;
    }
  }

  closedir(dp);
  ...
  return 0;
}
```
- Selain itu, pada fungsi `en_de_crypt_12_dir` ditambahkan kondisi untuk memeriksa dan menghandle 
apakah terdapat direktori spesial di dalam path tersebut.
```cpp
static void en_de_crypt_12_dir(char * str, bool with_rot)
{
  char fpath[PATH_MAX];
  strcpy(fpath, str);
  char * substr = strtok(fpath, "/");
  char dpath[PATH_MAX];

  int counter = 0;
  int marker = 0;
  bool is_special = false;
  while(substr != NULL) {
    dpath[counter] = '/';
    counter++;

    if (!strncmp(substr, "A_is_a_", 7) || is_special) {
      is_special = true;
      for (int i = 0; i < strlen(substr); i++) {
        dpath[counter + i] = substr[i];
      }
    } else {
      int size = strlen(substr);
      for (int i = size; i >= 0; i--) {
        if (substr[i] == '.') {
          size = i;
          break;
        }
      }

      for (int i = 0; i < size; i++) {
        if(!isalpha(substr[i])) {
          dpath[counter + i] = substr[i];
          continue;
        }

        dpath[counter + i] = atbash_cipher(substr[i]);
        if (with_rot) {
          dpath[counter + i] = rot_13_cipher(substr[i]);
        }
      }

      if (size != strlen(substr)) {
        for (int i = size; i < strlen(substr); i++) {
          dpath[counter + i] = substr[i];
        }
      }
    }

    counter += strlen(substr);

    if (is_special && strncmp(substr, "A_is_a_", 7)) {
      is_special = false;
    }

    if (substr != NULL) {
      substr = strtok(NULL, "/");
    }
  }

  dpath[counter] = '\0';
  strcpy(str, dpath);
}
```

## Soal 4
### Penjelasan Soal.
Untuk memudahkan dalam memonitor kegiatan pada filesystem mereka Sin dan Sei
membuat sebuah log system dengan spesifikasi sebagai berikut.
- (a) Log system yang akan terbentuk bernama “SinSeiFS.log” pada direktori home
pengguna (/home/[user]/SinSeiFS.log). Log system ini akan menyimpan daftar
perintah system call yang telah dijalankan pada filesystem.b. Karena Sin dan Sei suka kerapian maka log yang dibuat akan dibagi menjadi dua
level, yaitu INFO dan WARNING.
- (c) Untuk log level WARNING, digunakan untuk mencatat syscall rmdir dan unlink.
- (d) Sisanya, akan dicatat pada level INFO.
- (e) Format untuk logging yaitu:
> [Level]::[dd][mm][yyyy]-[HH]:[MM]:[SS]:[CMD]::[DESC :: DESC]
### Solusi dan Penjelasannya.
- Untuk mengerjakan soal ini kami membuat sebuah fungsi bernama `write_log` yang akan menghandle setiap system call.
```cpp
static void write_log(char level[], char cmd[], char arg1[], char arg2[])
{
  char message[PATH_MAX];

  time_t t_o = time(NULL);
  struct tm tm_s = * localtime(&t_o);
  sprintf(message, "%s::%02d%02d%d-%02d:%02d:%02d::%s", 
    level, tm_s.tm_mday, tm_s.tm_mon + 1, tm_s.tm_year + 1900,
    tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec, cmd);

  if (strlen(arg1) != 0) {
    strcat(message, "::");
    strcat(message, arg1);
  }

  if (strlen(arg2) != 0) {
    strcat(message, "::");
    strcat(message, arg2);
  }

  char * file_name = "/home/maroqi/SinSeiFS.log";
  FILE * log_file;
  log_file = fopen(file_name, "a");
  fprintf(log_file, "%s\n", message);

  fclose(log_file);
}
```
- Jadi, setiap system call tinggal memanggil fungsi diatas untuk menuliskan log pada file yang sesuai.
```cpp
...
...
...

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
  
  ...

	close(fd);
  write_log("INFO", "WRITE", path, "");
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
  ...

  write_log("INFO", "STATFS", path, "");
	return 0;
}
```
Untuk melihat kode program lengkapnya, [klik disini](SinSeiFS_C02.c).

### Dokumentasi dan Kendala
- Kebanyakan kendala yang dialamai adalah karena tidak tahu secara pasti urutan system call yang akan dipanggil.
