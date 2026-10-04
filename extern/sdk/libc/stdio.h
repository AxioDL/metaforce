#ifndef _STDIO
#define _STDIO

#include "types.h"
#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define __ungetc_buffer_size 2

typedef unsigned long __file_handle;
typedef unsigned long fpos_t;
#ifndef __cplusplus
typedef unsigned short wchar_t;
#endif

enum __io_modes {
  __read = 1,
  __write = 2,
  __read_write = 3,
  __append = 4,
};
enum __file_kinds {
  __closed_file,
  __disk_file,
  __console_file,
  __unavailable_file,
};
enum __file_orientation {
  __unoriented,
  __char_oriented,
  __wide_oriented,
};

enum __io_results {
  __no_io_error,
  __io_error,
  __io_EOF,
};

typedef struct {
  unsigned int open_mode : 2;
  unsigned int io_mode : 3;
  unsigned int buffer_mode : 2;
  unsigned int file_kind : 3;
  unsigned int file_orientation : 2;
  unsigned int binary_io : 1;
} __file_modes;

enum __io_states {
  __neutral,
  __writing,
  __reading,
  __rereading,
};

typedef struct {
  unsigned int io_state : 3;
  unsigned int free_buffer : 1;
  unsigned char eof;
  unsigned char error;
} __file_state;

typedef void (*__idle_proc)(void);
typedef int (*__pos_proc)(__file_handle file, fpos_t* position, int mode, __idle_proc idle_proc);
typedef int (*__io_proc)(__file_handle file, unsigned char* buff, size_t* count,
                         __idle_proc idle_proc);
typedef int (*__close_proc)(__file_handle file);

typedef struct _IO_FILE {
  __file_handle handle;
  __file_modes mode;
  __file_state state;
  unsigned char is_dynamically_allocated;
  unsigned char char_buffer;
  unsigned char char_buffer_overflow;
  unsigned char ungetc_buffer[__ungetc_buffer_size];
  wchar_t ungetwc_buffer[__ungetc_buffer_size];
  unsigned long position;
  unsigned char* buffer;
  unsigned long buffer_size;
  unsigned char* buffer_ptr;
  unsigned long buffer_len;
  unsigned long buffer_alignment;
  unsigned long saved_buffer_len;
  unsigned long buffer_pos;
  __pos_proc position_proc;
  __io_proc read_proc;
  __io_proc write_proc;
  __close_proc close_proc;
  __idle_proc idle_proc;
  struct _IO_FILE* next_file_struct;
} FILE;

typedef struct {
  char* CharStr;
  size_t MaxCharCount;
  size_t CharsWritten;
} __OutStrCtrl;

typedef struct {
  char* NextChar;
  int NullCharDetected;
} __InStrCtrl;

#define EOF	-1L

enum __ReadProcActions { __GetChar, __UngetChar, __CheckForError };

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

int puts(const char* s);
int printf(const char*, ...);
int snprintf(char* s, size_t n, const char* format, ...);
size_t sprintf(char* s, const char* format, ...);
size_t vprintf(const char* format, va_list arg);
size_t vsprintf(char* s, const char* format, va_list arg);
size_t fread(const void*, size_t memb_size, size_t num_memb, FILE*);
size_t fwrite(const void*, size_t memb_size, size_t num_memb, FILE*);
int fseek(FILE* file, long offset, int mode);
size_t __fwrite(const void*, size_t, size_t, FILE*);

int __StringRead(void* isc, int ch, int Action);

#ifdef __cplusplus
}
#endif

#endif // _STDIO
