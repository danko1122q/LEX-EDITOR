#include "core_utils.h"

#include "core_os.h"
#include "core_terminal.h"

#include <ctype.h>
#include <limits.h>

/**
 * Fungsi panic - menampilkan pesan error fatal dan keluar dari program
 * @param file: nama file tempat error terjadi
 * @param line: nomor baris tempat error terjadi
 * @param s: pesan error yang akan ditampilkan
 */
void panic(const char *file, int line, const char *s)
{
  terminalExit(); // Keluar dari mode terminal
#ifdef _DEBUG
  UNUSED(file);
  UNUSED(line);
  fprintf(stderr, "Fatal error: %s\r\n", s);
#else
  // Pada mode non-debug, tampilkan lokasi error
  fprintf(stderr, "Fatal error at %s:%d: %s\r\n", file, line, s);
#endif
  exit(EXIT_FAILURE); // Keluar dari program dengan status error
}

/**
 * malloc yang aman - memanggil panic jika alokasi gagal
 * @param file: nama file pemanggil (untuk debugging)
 * @param line: nomor baris pemanggil (untuk debugging)
 * @param size: ukuran memori yang dialokasikan
 * @return: pointer ke memori yang dialokasikan
 */
void *_malloc_s(const char *file, int line, size_t size)
{
  void *ptr = malloc(size);
  if (!ptr && size != 0)
    panic(file, line, "malloc"); // Panic jika alokasi gagal

  return ptr;
}

/**
 * calloc yang aman - alokasi memori dan inisialisasi dengan 0
 * @param file: nama file pemanggil
 * @param line: nomor baris pemanggil
 * @param n: jumlah elemen
 * @param size: ukuran per elemen
 * @return: pointer ke memori yang dialokasikan
 */
void *_calloc_s(const char *file, int line, size_t n, size_t size)
{
  void *ptr = calloc(n, size);
  if (!ptr && size != 0)
    panic(file, line, "calloc");

  return ptr;
}

/**
 * realloc yang aman - mengubah ukuran blok memori
 * @param file: nama file pemanggil
 * @param line: nomor baris pemanggil
 * @param ptr: pointer ke memori yang akan diresize
 * @param size: ukuran baru
 * @return: pointer ke memori yang diresize
 */
void *_realloc_s(const char *file, int line, void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr && size != 0)
    panic(file, line, "realloc");
  return ptr;
}

/**
 * Membuat ruang tambahan pada vector dinamis jika sudah penuh
 * @param _vec: pointer ke struktur vector
 * @param item_size: ukuran per item dalam bytes
 */
void _vector_make_room(_Vector *_vec, size_t item_size)
{
  // Jika belum ada kapasitas, inisialisasi dengan kapasitas minimum
  if (!_vec->capacity)
  {
    _vec->data     = malloc_s(item_size * VECTOR_MIN_CAPACITY);
    _vec->capacity = VECTOR_MIN_CAPACITY;
  }
  // Jika ukuran sudah mencapai kapasitas, perluas array
  if (_vec->size >= _vec->capacity)
  {
    _vec->capacity *= VECTOR_EXTEND_RATE; // Gandakan kapasitas
    _vec->data = realloc_s(_vec->data, _vec->capacity * item_size);
  }
}

/**
 * Menambahkan n karakter ke append buffer
 * @param ab: pointer ke append buffer
 * @param s: string yang akan ditambahkan
 * @param n: jumlah karakter yang ditambahkan
 */
void abufAppendN(abuf *ab, const char *s, size_t n)
{
  if (n == 0)
    return;

  // Jika buffer tidak cukup, perluas buffer
  if (ab->len + n > ab->capacity)
  {
    ab->capacity += n;
    ab->capacity *= ABUF_GROWTH_RATE; // Perluas dengan faktor pertumbuhan
    char *new = realloc_s(ab->buf, ab->capacity);
    ab->buf   = new;
  }

  // Salin data ke buffer
  memcpy(&ab->buf[ab->len], s, n);
  ab->len += n;
}

/**
 * Membebaskan memori append buffer
 * @param ab: pointer ke append buffer
 */
void abufFree(abuf *ab)
{
  free(ab->buf);
  ab->buf      = NULL;
  ab->len      = 0;
  ab->capacity = 0;
}

/**
 * Validasi format warna hexadecimal (6 karakter, 0-9 A-F)
 * @param color: string warna hex
 * @return: true jika valid, false jika tidak
 */
static inline bool isValidColor(const char *color)
{
  if (strlen(color) != 6)
    return false;
  // Periksa setiap karakter harus 0-9 atau A-F (case insensitive)
  for (int i = 0; i < 6; i++)
  {
    if (!((color[i] >= '0' && color[i] <= '9') || (color[i] >= 'A' && color[i] <= 'F') ||
          (color[i] >= 'a' && color[i] <= 'f')))
      return false;
  }
  return true;
}

/**
 * Konversi string hex ke struktur Color RGB
 * @param color: string warna dalam format hex (contoh: "FF5733")
 * @param out: pointer output untuk menyimpan nilai RGB
 * @return: true jika berhasil, false jika format tidak valid
 */
bool strToColor(const char *color, Color *out)
{
  if (!isValidColor(color))
    return false;

  int          shift = 16;
  unsigned int hex   = strtoul(color, NULL, 16); // Konversi string hex ke integer
  // Ekstrak komponen R, G, B dari nilai hex
  out->r             = (hex >> shift) & 0xFF; // Red (8 bit tertinggi)
  shift -= 8;
  out->g = (hex >> shift) & 0xFF; // Green (8 bit tengah)
  shift -= 8;
  out->b = (hex >> shift) & 0xFF; // Blue (8 bit terendah)
  return true;
}

/**
 * Menambahkan escape sequence warna ANSI ke buffer
 * @param ab: append buffer
 * @param color: struktur warna RGB
 * @param is_bg: 1 untuk background, 0 untuk foreground
 */
void setColor(abuf *ab, Color color, int is_bg)
{
  char buf[32];
  int  len;
  // Warna hitam (0,0,0) pada background diubah ke default background
  if (color.r == 0 && color.g == 0 && color.b == 0 && is_bg)
  {
    len = snprintf(buf, sizeof(buf), "%s", ANSI_DEFAULT_BG);
  }
  else
  {
    // Format ANSI: ESC[38;2;R;G;Bm untuk foreground, ESC[48;2;R;G;Bm untuk background
    len = snprintf(buf, sizeof(buf), "\x1b[%d;2;%d;%d;%dm", is_bg ? 48 : 38, color.r, color.g,
                   color.b);
  }
  abufAppendN(ab, buf, len);
}

/**
 * Pindahkan kursor ke posisi (x, y) menggunakan ANSI escape sequence
 * @param ab: append buffer
 * @param x: baris (1-based)
 * @param y: kolom (1-based)
 */
void gotoXY(abuf *ab, int x, int y)
{
  char buf[32];
  int  len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", x, y);
  abufAppendN(ab, buf, len);
}

/**
 * Konversi struktur Color ke string hexadecimal
 * @param color: struktur warna RGB
 * @param buf: buffer output minimal 8 bytes
 * @return: jumlah karakter yang ditulis
 */
int colorToStr(Color color, char buf[8])
{
  return snprintf(buf, 8, "%02x%02x%02x", color.r, color.g, color.b);
}

/**
 * Cek apakah karakter adalah separator/pembatas kata
 * @param c: karakter yang dicek
 * @return: 1 jika separator, 0 jika bukan
 */
int isSeparator(int c)
{
  return strchr("`~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", c) != NULL;
}

/**
 * Cek apakah karakter bukan separator
 */
int isNonSeparator(int c)
{
  return !isSeparator(c);
}

/**
 * Cek apakah karakter adalah whitespace
 * @param c: karakter yang dicek
 * @return: 1 jika whitespace, 0 jika bukan
 */
int isSpace(int c)
{
  switch (c)
  {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
    case '\f':
      return 1;
    default:
      return 0;
  }
}

/**
 * Cek apakah karakter bukan whitespace
 */
int isNonSpace(int c)
{
  return !isSpace(c);
}

/**
 * Cek apakah karakter bukan bagian dari identifier
 * (identifier adalah nama variabel/fungsi dalam pemrograman)
 * @param c: karakter yang dicek
 * @return: 1 jika bukan identifier char, 0 jika identifier char
 */
int isNonIdentifierChar(int c)
{
  return isSpace(c) || c == '\0' || isSeparator(c);
}

/**
 * Cek apakah karakter adalah bagian dari identifier
 */
int isIdentifierChar(int c)
{
  return !isNonIdentifierChar(c);
}

/**
 * Hitung jumlah digit dalam bilangan bulat
 * @param n: bilangan yang dihitung digitnya
 * @return: jumlah digit
 */
int getDigit(int n)
{
  if (n < 10)
    return 1;
  if (n < 100)
    return 2;
  if (n < 1000)
    return 3;
  if (n < 10000000)
  {
    if (n < 1000000)
    {
      if (n < 10000)
        return 4;
      return 5 + (n >= 100000); // 5 atau 6 digit
    }
    return 7;
  }
  if (n < 1000000000)
    return 8 + (n >= 100000000); // 8 atau 9 digit
  return 10;
}

/**
 * Mendapatkan nama file dari path lengkap
 * Contoh: "/home/user/file.txt" -> "file.txt"
 * @param path: path lengkap
 * @return: pointer ke nama file dalam string path
 */
char *getBaseName(char *path)
{
  char *file = path + strlen(path);
  // Mundur dari akhir string hingga menemukan separator path
  for (; file > path; file--)
  {
    if (*file == '/'
#ifdef _WIN32
        || *file == '\\' // Windows menggunakan backslash
#endif
    )
    {
      file++; // Skip separator
      break;
    }
  }
  return file;
}

/**
 * Mendapatkan nama direktori dari path
 * Contoh: "/home/user/file.txt" -> "/home/user"
 * @param path: path lengkap (akan dimodifikasi)
 * @return: pointer ke string direktori
 */
char *getDirName(char *path)
{
  char *name = getBaseName(path);
  if (name == path)
  {
    // Jika tidak ada separator, gunakan direktori saat ini "."
    name  = path;
    *name = '.';
    name++;
  }
  else
  {
    path--; // Mundur ke separator
  }
  *name = '\0'; // Terminasi string
  return path;
}

/**
 * Menambahkan ekstensi default ke path jika belum ada ekstensi
 * Contoh: "file" + ".txt" -> "file.txt", "file.c" tetap "file.c"
 * @param path: path file (akan dimodifikasi)
 * @param extension: ekstensi yang akan ditambahkan (dengan titik)
 * @param path_length: panjang maksimal path
 */
void addDefaultExtension(char *path, const char *extension, int path_length)
{
  char *src = path + strlen(path) - 1;

  // Cari dari belakang hingga menemukan '/' atau '.'
  while (!(*src == '/'
#ifdef _WIN32
           || *src == '\\'
#endif
           ) &&
         src > path)
  {
    if (*src == '.')
    {
      return; // Sudah ada ekstensi, tidak perlu menambahkan
    }
    src--;
  }

  // Tambahkan ekstensi
  strncat(path, extension, path_length);
}

/**
 * Membaca satu baris dari file stream (mirip getline di POSIX)
 * @param lineptr: pointer ke buffer (akan dialokasikan otomatis jika NULL)
 * @param n: pointer ke ukuran buffer
 * @param stream: file stream untuk dibaca
 * @return: jumlah karakter yang dibaca, atau -1 jika EOF/error
 */
int64_t getLine(char **lineptr, size_t *n, FILE *stream)
{
  char        *buf = NULL;
  size_t       capacity;
  int64_t      size = 0;
  int          c;
  const size_t buf_size = 128;

  if (!lineptr || !stream || !n)
    return -1;

  buf      = *lineptr;
  capacity = *n;

  c = fgetc(stream);
  if (c == EOF)
    return -1;

  // Alokasi buffer awal jika belum ada
  if (!buf)
  {
    buf      = malloc_s(buf_size);
    capacity = buf_size;
  }

  // Baca karakter hingga newline atau EOF
  while (c != EOF)
  {
    // Perluas buffer jika penuh
    if ((size_t) size > (capacity - 1))
    {
      capacity += buf_size;
      buf = realloc_s(buf, capacity);
    }
    buf[size++] = c;

    if (c == '\n')
      break;

    c = fgetc(stream);
  }

  buf[size] = '\0'; // Terminasi string
  *lineptr  = buf;
  *n        = capacity;

  return size;
}

/**
 * Perbandingan string case-insensitive (tidak peduli huruf besar/kecil)
 * @param s1: string pertama
 * @param s2: string kedua
 * @return: 0 jika sama, <0 jika s1<s2, >0 jika s1>s2
 */
int strCaseCmp(const char *s1, const char *s2)
{
  if (s1 == s2)
    return 0;

  int result;
  // Bandingkan karakter demi karakter dalam lowercase
  while ((result = tolower(*s1) - tolower(*s2)) == 0)
  {
    if (*s1 == '\0')
      break;
    s1++;
    s2++;
  }
  return result;
}

/**
 * Mencari substring case-insensitive (mirip strstr tapi ignore case)
 * @param str: string utama
 * @param sub_str: substring yang dicari
 * @return: pointer ke posisi substring ditemukan, atau NULL jika tidak ada
 */
char *strCaseStr(const char *str, const char *sub_str)
{
  // Algoritma naive O(n*m), tapi cukup untuk kebanyakan kasus
  if (*sub_str == '\0')
    return (char *) str;

  while (*str != '\0')
  {
    const char *s   = str;
    const char *sub = sub_str;
    // Cocokkan karakter demi karakter
    while (tolower(*s) == tolower(*sub))
    {
      s++;
      sub++;
      if (*sub == '\0')
      {
        return (char *) str; // Substring ditemukan
      }
    }
    str++;
  }

  return NULL;
}

/**
 * Mencari posisi substring dalam string
 * @param haystack: string yang dicari
 * @param haystack_len: panjang haystack
 * @param needle: substring yang dicari
 * @param needle_len: panjang needle
 * @param start: posisi mulai pencarian
 * @param ignore_case: true untuk case-insensitive
 * @return: indeks posisi ditemukan, atau -1 jika tidak ada
 */
int findSubstring(const char *haystack, size_t haystack_len, const char *needle, size_t needle_len,
                  size_t start, bool ignore_case)
{
  if (needle_len == 0)
  {
    return (start <= haystack_len) ? (int) start : -1;
  }

  if (haystack_len < needle_len)
    return -1;

  size_t limit = haystack_len - needle_len;
  if (start > limit)
    return -1;

  // Cari dari posisi start hingga limit
  for (size_t i = start; i <= limit; ++i)
  {
    size_t j = 0;
    // Cocokkan setiap karakter
    for (; j < needle_len; ++j)
    {
      uint8_t hay = (uint8_t) haystack[i + j];
      uint8_t nee = (uint8_t) needle[j];
      if (ignore_case)
      {
        if (tolower(hay) != tolower(nee))
          break;
      }
      else if (hay != nee)
      {
        break;
      }
    }
    if (j == needle_len)
      return (int) i; // Substring ditemukan di posisi i
  }

  return -1;
}

/**
 * Konversi string ke integer dengan validasi
 * @param str: string yang akan dikonversi
 * @return: nilai integer, atau 0 jika invalid
 */
int strToInt(const char *str)
{
  if (!str)
  {
    return 0;
  }

  // Skip spasi di awal
  while (*str == ' ' || *str == '\t')
  {
    str++;
  }

  // Deteksi tanda positif/negatif
  int sign = 1;
  if (*str == '+' || *str == '-')
  {
    sign = (*str++ == '-') ? -1 : 1;
  }

  int result = 0;
  // Konversi digit demi digit
  while (*str >= '0' && *str <= '9')
  {
    // Cek overflow sebelum melakukan perkalian
    if (result > INT_MAX / 10 || (result == INT_MAX / 10 && (*str - '0') > INT_MAX % 10))
    {
      // Overflow: kembalikan INT_MIN atau INT_MAX
      return (sign == -1) ? INT_MIN : INT_MAX;
    }

    result = result * 10 + (*str - '0');
    str++;
  }

  result = sign * result;

  // Validasi: pastikan tidak ada karakter non-spasi setelah angka
  while (*str != '\0')
  {
    if (*str != ' ' && *str != '\t')
    {
      return 0; // Invalid: ada karakter non-digit dan non-spasi
    }
    str++;
  }

  return result;
}


// Tabel karakter untuk encoding Base64
static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * Encode data ke format Base64
 * @param string: data input yang akan diencode
 * @param len: panjang data input
 * @param output: buffer output untuk menyimpan hasil encoding
 * @return: panjang string Base64 hasil encoding
 */
int base64Encode(const char *string, int len, char *output)
{
  int   i;
  char *p = output;

  // Proses setiap 3 bytes input menjadi 4 bytes Base64
  for (i = 0; i < len - 2; i += 3)
  {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) | ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) | ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
  }

  // Handle sisa bytes (padding dengan '=')
  if (i < len)
  {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1))
    {
      // 1 byte tersisa
      *p++ = basis_64[((string[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else
    {
      // 2 bytes tersisa
      *p++ = basis_64[((string[i] & 0x3) << 4) | ((int) (string[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  *p++ = '\0'; // Terminasi string
  return p - output;
}

/**
 * Menulis semua data ke console (retry jika perlu)
 * @param buf: buffer data yang akan ditulis
 * @param len: panjang data
 * @return: true jika berhasil, false jika error
 */
bool writeConsoleAll(const void *buf, size_t len)
{
  const uint8_t *p = (const uint8_t *) buf;
  while (len)
  {
    int n = writeConsole(p, len);
    if (n <= 0)
      return false; // Error saat menulis
    p += (size_t) n;
    len -= (size_t) n;
  }
  return true;
}
