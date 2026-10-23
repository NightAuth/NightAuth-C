
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>

int strcasecmp(const char *s1, const char *s2) { return _stricmp(s1, s2); }
int strncasecmp(const char *s1, const char *s2, size_t n) { return _strnicmp(s1, s2, n); }
void* __memcpy_chk(void *dest, const void *src, size_t len, size_t destlen) { (void)destlen; return memcpy(dest, src, len); }
void* __memset_chk(void *dest, int c, size_t len, size_t destlen) { (void)destlen; return memset(dest, c, len); }
void* __memmove_chk(void *dest, const void *src, size_t len, size_t destlen) { (void)destlen; return memmove(dest, src, len); }
int fstat64(int fd, struct _stat64 *buf) { return _fstat64(fd, buf); }

int __mingw_vsnprintf(char *s, size_t n, const char *fmt, va_list ap) { return vsnprintf(s, n, fmt, ap); }
int __mingw_snprintf(char *s, size_t n, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = vsnprintf(s, n, fmt, ap); va_end(ap); return r;
}
int __mingw_vasprintf(char **strp, const char *fmt, va_list ap) {
    int len = _vscprintf(fmt, ap);
    if (len < 0) return -1;
    *strp = (char*)malloc(len + 1);
    if (!*strp) return -1;
    return vsnprintf(*strp, len + 1, fmt, ap);
}
int __mingw_asprintf(char **strp, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = __mingw_vasprintf(strp, fmt, ap); va_end(ap); return r;
}
int __mingw_fprintf(FILE *stream, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = vfprintf(stream, fmt, ap); va_end(ap); return r;
}
int __mingw_sscanf(const char *buffer, const char *format, ...) {
    va_list ap; va_start(ap, format); int r = vsscanf(buffer, format, ap); va_end(ap); return r;
}
void ___chkstk_ms(void) {}
