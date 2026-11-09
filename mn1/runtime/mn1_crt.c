/* Biblioteca de C da MN1 para o port do Minish Cap.
   - memoria, mem*, strchr e matematica em double: importados da coredll (comprovados na MN1);
   - texto, formatacao e arquivos: implementados aqui sobre o Windows CE;
   - POSIX que o port so usa em recursos opcionais (mmap, shm, opendir...): falham de forma limpa.
   Caminhos: '/' vira '\'; caminho relativo e resolvido a partir da pasta do .exe.
   stderr/stdout vao para RESULTADOS3\TMC_LOG.txt (via lg do MN1_Probe). */
#include "mn1.h"
#include <stdarg.h>

typedef unsigned int size_t;
typedef long long i64;
typedef unsigned long long u64;

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
void *memmove(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
char *strchr(const char *, int);
double sqrt(double);
double sin(double);
double cos(double);
double tanh(double);
double floor(double);

int mn1_errno;
int *__errno_location(void) { return &mn1_errno; }

/* ---------------------------------------------------------------- texto */
size_t strlen(const char *s) { const char *p = s; while (*p) p++; return (size_t)(p - s); }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (unsigned char)*a - (unsigned char)*b; }
int strncmp(const char *a, const char *b, size_t n)
{
    for (; n; n--, a++, b++) {
        if (*a != *b || !*a) return (unsigned char)*a - (unsigned char)*b;
    }
    return 0;
}
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)) ; return r; }
char *strncpy(char *d, const char *s, size_t n)
{
    size_t i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
    return d;
}
char *strcat(char *d, const char *s) { strcpy(d + strlen(d), s); return d; }
char *strncat(char *d, const char *s, size_t n)
{
    char *p = d + strlen(d);
    while (n-- && *s) *p++ = *s++;
    *p = 0;
    return d;
}
char *strrchr(const char *s, int c)
{
    const char *r = 0;
    for (;; s++) { if (*s == (char)c) r = s; if (!*s) return (char *)r; }
}
char *strstr(const char *h, const char *n)
{
    size_t k = strlen(n);
    if (!k) return (char *)h;
    for (; *h; h++) if (*h == *n && !strncmp(h, n, k)) return (char *)h;
    return 0;
}
char *strdup(const char *s) { size_t n = strlen(s) + 1; char *d = malloc(n); if (d) memcpy(d, s, n); return d; }
size_t strnlen(const char *s, size_t m) { size_t n = 0; while (n < m && s[n]) n++; return n; }
void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    for (; n; n--, p++) if (*p == (unsigned char)c) return (void *)p;
    return 0;
}
int tolower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
int toupper(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isspace(int c) { return c == ' ' || (c >= 9 && c <= 13); }
int isalpha(int c) { return (c | 32) >= 'a' && (c | 32) <= 'z'; }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isxdigit(int c) { return isdigit(c) || ((c | 32) >= 'a' && (c | 32) <= 'f'); }
int isprint(int c) { return c >= 32 && c < 127; }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }
char *strerror(int e) { (void)e; return "erro"; }

static int digval(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    c |= 32;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return 99;
}
unsigned long long strtoull(const char *s, char **end, int base)
{
    u64 v = 0;
    int d;
    while (isspace(*s)) s++;
    if (*s == '+') s++;
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] | 32) == 'x') { s += 2; base = 16; }
    else if (base == 0 && s[0] == '0') base = 8;
    else if (base == 0) base = 10;
    while ((d = digval(*s)) < base) { v = v * (unsigned)base + (unsigned)d; s++; }
    if (end) *end = (char *)s;
    return v;
}
long long strtoll(const char *s, char **end, int base)
{
    int neg = 0;
    while (isspace(*s)) s++;
    if (*s == '-') { neg = 1; s++; }
    {
        u64 v = strtoull(s, end, base);
        return neg ? -(i64)v : (i64)v;
    }
}
long strtol(const char *s, char **end, int base) { return (long)strtoll(s, end, base); }
unsigned long strtoul(const char *s, char **end, int base) { return (unsigned long)strtoull(s, end, base); }
int atoi(const char *s) { return (int)strtol(s, 0, 10); }
long atol(const char *s) { return strtol(s, 0, 10); }

/* qsort: insercao + shellsort simples, sem recursao */
void qsort(void *base, size_t n, size_t sz, int (*cmp)(const void *, const void *))
{
    unsigned char *b = base, tmp[256];
    size_t gap, i, j, k;
    if (sz > sizeof tmp) return;
    for (gap = n / 2; gap; gap /= 2)
        for (i = gap; i < n; i++) {
            memcpy(tmp, b + i * sz, sz);
            for (j = i; j >= gap && cmp(b + (j - gap) * sz, tmp) > 0; j -= gap) memcpy(b + j * sz, b + (j - gap) * sz, sz);
            if (j != i) memcpy(b + j * sz, tmp, sz);
            (void)k;
        }
}
void *bsearch(const void *key, const void *base, size_t n, size_t sz, int (*cmp)(const void *, const void *))
{
    size_t lo = 0, hi = n;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        int c = cmp(key, (const char *)base + mid * sz);
        if (!c) return (char *)base + mid * sz;
        if (c < 0) hi = mid; else lo = mid + 1;
    }
    return 0;
}

/* ---------------------------------------------------------------- formatacao */
typedef struct { char *p; size_t n, k; } Out;
static void put(Out *o, char c) { if (o->k + 1 < o->n) o->p[o->k] = c; o->k++; }

static void put_num(Out *o, u64 v, int base, int upper, int width, int prec, int zero, int left, int neg, int plus)
{
    char t[24];
    int i = 0, len, pad;
    const char *dg = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (!v && prec != 0) t[i++] = '0';
    while (v) { t[i++] = dg[v % (unsigned)base]; v /= (unsigned)base; }
    while (i < prec) t[i++] = '0';
    len = i + ((neg || plus) ? 1 : 0);
    pad = width > len ? width - len : 0;
    if (!left && !zero) while (pad-- > 0) put(o, ' ');
    if (neg) put(o, '-'); else if (plus) put(o, '+');
    if (!left && zero) while (pad-- > 0) put(o, '0');
    while (i) put(o, t[--i]);
    if (left) while (pad-- > 0) put(o, ' ');
}

static void put_double(Out *o, double d, int prec, int width, int left)
{
    char t[48];
    int i = 0, k, neg = 0;
    u64 ip, fp, m = 1;
    if (prec < 0) prec = 6;
    if (prec > 9) prec = 9;
    if (d < 0) { neg = 1; d = -d; }
    for (k = 0; k < prec; k++) m *= 10;
    if (d > 1e18) d = 1e18;
    ip = (u64)d;
    fp = (u64)((d - (double)ip) * (double)m + 0.5);
    if (fp >= m) { ip++; fp -= m; }
    if (neg) t[i++] = '-';
    {
        char r[24];
        int j = 0;
        if (!ip) r[j++] = '0';
        while (ip) { r[j++] = (char)('0' + ip % 10); ip /= 10; }
        while (j) t[i++] = r[--j];
    }
    if (prec) {
        t[i++] = '.';
        for (k = prec - 1; k >= 0; k--) { u64 q = 1; int z; for (z = 0; z < k; z++) q *= 10; t[i++] = (char)('0' + (fp / q) % 10); }
    }
    k = width > i ? width - i : 0;
    if (!left) while (k-- > 0) put(o, ' ');
    for (k = 0; k < i; k++) put(o, t[k]);
    if (left) { k = width > i ? width - i : 0; while (k-- > 0) put(o, ' '); }
}

int vsnprintf(char *buf, size_t n, const char *f, va_list a)
{
    Out o = { buf, n, 0 };
    for (; *f; f++) {
        int left = 0, zero = 0, plus = 0, width = 0, prec = -1, lng = 0;
        if (*f != '%') { put(&o, *f); continue; }
        f++;
        for (;; f++) {
            if (*f == '-') left = 1;
            else if (*f == '0') zero = 1;
            else if (*f == '+') plus = 1;
            else if (*f == ' ' || *f == '#') ;
            else break;
        }
        if (*f == '*') { width = va_arg(a, int); f++; }
        else while (*f >= '0' && *f <= '9') width = width * 10 + *f++ - '0';
        if (*f == '.') {
            f++;
            prec = 0;
            if (*f == '*') { prec = va_arg(a, int); f++; }
            else while (*f >= '0' && *f <= '9') prec = prec * 10 + *f++ - '0';
        }
        while (*f == 'l' || *f == 'h' || *f == 'z' || *f == 'j' || *f == 't' || *f == 'L') { if (*f == 'l' || *f == 'j') lng++; f++; }
        switch (*f) {
        case 'd': case 'i': {
            i64 v = lng >= 2 ? va_arg(a, i64) : va_arg(a, int);
            put_num(&o, (u64)(v < 0 ? -v : v), 10, 0, width, prec, zero && !left, left, v < 0, plus);
            break;
        }
        case 'u': case 'x': case 'X': case 'o': {
            u64 v = lng >= 2 ? va_arg(a, u64) : va_arg(a, unsigned);
            put_num(&o, v, *f == 'o' ? 8 : *f == 'u' ? 10 : 16, *f == 'X', width, prec, zero && !left, left, 0, 0);
            break;
        }
        case 'p':
            put(&o, '0'); put(&o, 'x');
            put_num(&o, (u64)(unsigned)va_arg(a, void *), 16, 0, 0, -1, 0, 0, 0, 0);
            break;
        case 'c': put(&o, (char)va_arg(a, int)); break;
        case 's': {
            const char *s = va_arg(a, const char *);
            int len, pad;
            if (!s) s = "(null)";
            len = (int)strlen(s);
            if (prec >= 0 && len > prec) len = prec;
            pad = width > len ? width - len : 0;
            if (!left) while (pad-- > 0) put(&o, ' ');
            while (len--) put(&o, *s++);
            if (left) while (pad-- > 0) put(&o, ' ');
            break;
        }
        case 'f': case 'F': case 'g': case 'G': case 'e': case 'E':
            put_double(&o, va_arg(a, double), prec, width, left);
            break;
        case '%': put(&o, '%'); break;
        default: put(&o, '%'); if (*f) put(&o, *f); else f--;
        }
    }
    if (n) o.p[o.k < n ? o.k : n - 1] = 0;
    return (int)o.k;
}
int snprintf(char *b, size_t n, const char *f, ...) { va_list a; int r; va_start(a, f); r = vsnprintf(b, n, f, a); va_end(a); return r; }
int vsprintf(char *b, const char *f, va_list a) { return vsnprintf(b, 0x7FFFFFFF, f, a); }
int sprintf(char *b, const char *f, ...) { va_list a; int r; va_start(a, f); r = vsnprintf(b, 0x7FFFFFFF, f, a); va_end(a); return r; }

/* ---------------------------------------------------------------- caminhos */
static wchar exedir[300];
static char exepath_u[300];

void mn1_crt_init(HANDLE inst)
{
    int n = (int)GetModuleFileNameW(inst, exedir, 290), i, cut = 0;
    for (i = 0; i < n; i++) {
        exepath_u[i] = exedir[i] == '\\' ? '/' : (char)exedir[i];
        if (exedir[i] == '\\') cut = i;
    }
    exepath_u[n] = 0;
    exedir[cut] = 0;
}

/* char (UTF-8 so ASCII nos caminhos do port) -> wide do CE, absoluto */
static void wpath(const char *p, wchar *out, int cap)
{
    int k = 0, i;
    if (p[0] != '/' && p[0] != '\\') {
        for (i = 0; exedir[i] && k < cap - 2; i++) out[k++] = exedir[i];
        out[k++] = '\\';
    }
    if (p[0] == '.' && (p[1] == '/' || p[1] == '\\')) p += 2;
    for (; *p && k < cap - 1; p++) out[k++] = (*p == '/') ? '\\' : (wchar)(unsigned char)*p;
    out[k] = 0;
}

long readlink(const char *path, char *buf, unsigned long n)
{
    unsigned long k = strlen(exepath_u);
    if (strcmp(path, "/proc/self/exe")) { mn1_errno = 22; return -1; }
    if (k > n) k = n;
    memcpy(buf, exepath_u, k);
    return (long)k;
}

char *realpath(const char *p, char *out)
{
    static char r[300];
    wchar w[300];
    int i;
    wpath(p, w, 300);
    for (i = 0; w[i] && i < 299; i++) r[i] = w[i] == '\\' ? '/' : (char)w[i];
    r[i] = 0;
    if (!out) return strdup(r);
    strcpy(out, r);
    return out;
}
char *__realpath_chk(const char *p, char *out, size_t n) { (void)n; return realpath(p, out); }

int mkdir(const char *p, unsigned mode) { wchar w[300]; (void)mode; wpath(p, w, 300); return CreateDirectoryW(w, 0) ? 0 : -1; }
int remove(const char *p)
{
    wchar w[300];
    int (*del)(const wchar *) = (int (*)(const wchar *))DYNC(DeleteFileW);
    wpath(p, w, 300);
    return del && del(w) ? 0 : -1;
}
int rename(const char *a, const char *b)
{
    wchar wa[300], wb[300];
    int (*mv)(const wchar *, const wchar *) = (int (*)(const wchar *, const wchar *))DYNC(MoveFileW);
    int (*del)(const wchar *) = (int (*)(const wchar *))DYNC(DeleteFileW);
    wpath(a, wa, 300);
    wpath(b, wb, 300);
    if (del) del(wb);
    return mv && mv(wa, wb) ? 0 : -1;
}
int link(const char *a, const char *b) { (void)a; (void)b; return -1; }

/* ---------------------------------------------------------------- arquivos */
typedef struct MN1_FILE { HANDLE h; int err, eof, ungot, log; } MFILE;
static MFILE f_out = { 0, 0, 0, -1, 1 }, f_err = { 0, 0, 0, -1, 1 };
void *stdout = &f_out;
void *stderr = &f_err;
void *stdin = 0;

static char logbuf[1024];
static int loglen;
static void log_char(char c)
{
    if (c == '\n' || loglen >= (int)sizeof logbuf - 1) {
        logbuf[loglen] = 0;
        lg("%s", logbuf);
        loglen = 0;
        if (c == '\n') return;
    }
    logbuf[loglen++] = c;
}

MFILE *fopen(const char *path, const char *mode)
{
    wchar w[300];
    u32 acc = 0, disp = OPEN_EXISTING;
    int app = 0, plus = strchr(mode, '+') != 0;
    HANDLE h;
    MFILE *f;
    if (mode[0] == 'r') { acc = GENERIC_READ | (plus ? GENERIC_WRITE : 0); disp = OPEN_EXISTING; }
    else if (mode[0] == 'w') { acc = GENERIC_WRITE | (plus ? GENERIC_READ : 0); disp = CREATE_ALWAYS; }
    else if (mode[0] == 'a') { acc = GENERIC_WRITE | (plus ? GENERIC_READ : 0); disp = OPEN_ALWAYS; app = 1; }
    wpath(path, w, 300);
    h = CreateFileW(w, acc, 1, 0, disp, 0x80, 0);
    if (h == INVALID_HANDLE) { mn1_errno = 2; return 0; }
    if (app) SetFilePointer(h, 0, 0, 2);
    f = calloc(1, sizeof *f);
    if (!f) { CloseHandle(h); return 0; }
    f->h = h;
    f->ungot = -1;
    return f;
}
MFILE *fopen64(const char *p, const char *m) { return fopen(p, m); }
int fclose(MFILE *f)
{
    if (!f || f->log) return 0;
    CloseHandle(f->h);
    free(f);
    return 0;
}
size_t fread(void *b, size_t sz, size_t n, MFILE *f)
{
    u32 got = 0, want = sz * n;
    if (!f || f->log || !want) return 0;
    if (f->ungot >= 0) { *(unsigned char *)b = (unsigned char)f->ungot; f->ungot = -1; b = (char *)b + 1; want--; got = 1; if (!want) return n; }
    {
        u32 r = 0;
        if (!ReadFile(f->h, b, want, &r, 0)) { f->err = 1; return got / sz; }
        if (r < want) f->eof = 1;
        got += r;
    }
    return got / sz;
}
size_t fwrite(const void *b, size_t sz, size_t n, MFILE *f)
{
    u32 w = 0;
    if (!f) return 0;
    if (f->log) {
        const char *p = b;
        size_t i;
        for (i = 0; i < sz * n; i++) log_char(p[i]);
        return n;
    }
    if (!WriteFile(f->h, b, sz * n, &w, 0)) { f->err = 1; return 0; }
    return sz ? w / sz : 0;
}
int fseek(MFILE *f, long off, int whence)
{
    if (!f || f->log) return -1;
    f->ungot = -1;
    f->eof = 0;
    return SetFilePointer(f->h, (int)off, 0, (u32)whence) == 0xFFFFFFFF ? -1 : 0;
}
int fseeko64(MFILE *f, i64 off, int w) { return fseek(f, (long)off, w); }
long ftell(MFILE *f) { return (!f || f->log) ? -1 : (long)SetFilePointer(f->h, 0, 0, 1); }
i64 ftello64(MFILE *f) { return ftell(f); }
int fflush(MFILE *f) { if (!f || f->log) lg_commit(); return 0; }
int ferror(MFILE *f) { return f ? f->err : 0; }
int feof(MFILE *f) { return f ? f->eof : 1; }
void clearerr(MFILE *f) { if (f) f->err = f->eof = 0; }
int fgetc(MFILE *f)
{
    unsigned char c;
    if (f && f->ungot >= 0) { c = (unsigned char)f->ungot; f->ungot = -1; return c; }
    return fread(&c, 1, 1, f) == 1 ? c : -1;
}
int getc(MFILE *f) { return fgetc(f); }
int ungetc(int c, MFILE *f) { if (f) f->ungot = c & 255; return c; }
char *fgets(char *s, int n, MFILE *f)
{
    int i = 0, c = 0;
    while (i < n - 1 && (c = fgetc(f)) >= 0) { s[i++] = (char)c; if (c == '\n') break; }
    s[i] = 0;
    return i ? s : 0;
}
int fputc(int c, MFILE *f) { char ch = (char)c; return fwrite(&ch, 1, 1, f) == 1 ? c : -1; }
int putc(int c, MFILE *f) { return fputc(c, f); }
int fputs(const char *s, MFILE *f) { return fwrite(s, 1, strlen(s), f) ? 0 : -1; }
int puts(const char *s) { fputs(s, &f_out); fputc('\n', &f_out); return 0; }
int putchar(int c) { return fputc(c, &f_out); }
int vfprintf(MFILE *f, const char *fmt, va_list a)
{
    static char b[2048];
    int n = vsnprintf(b, sizeof b, fmt, a);
    fwrite(b, 1, n < (int)sizeof b ? n : (int)sizeof b - 1, f);
    return n;
}
int fprintf(MFILE *f, const char *fmt, ...) { va_list a; int r; va_start(a, fmt); r = vfprintf(f, fmt, a); va_end(a); return r; }
int printf(const char *fmt, ...) { va_list a; int r; va_start(a, fmt); r = vfprintf(&f_out, fmt, a); va_end(a); return r; }
int vprintf(const char *fmt, va_list a) { return vfprintf(&f_out, fmt, a); }
int fileno(MFILE *f) { return f ? (f->log ? 2 : 3) : -1; }
int setvbuf(MFILE *f, char *b, int m, size_t n) { (void)f; (void)b; (void)m; (void)n; return 0; }
void perror(const char *s) { fprintf(&f_err, "%s: erro %d\n", s ? s : "", mn1_errno); }
int fsync(int fd) { (void)fd; return 0; }
int close(int fd) { (void)fd; return 0; }
int ftruncate64(int fd, i64 n) { (void)fd; (void)n; return -1; }

/* POSIX usado so por recursos opcionais: falha limpa */
void *opendir(const char *p) { (void)p; mn1_errno = 2; return 0; }
void *readdir64(void *d) { (void)d; return 0; }
void *readdir(void *d) { (void)d; return 0; }
int closedir(void *d) { (void)d; return 0; }
void *mmap64(void *a, size_t n, int p, int fl, int fd, i64 off) { (void)a; (void)n; (void)p; (void)fl; (void)fd; (void)off; return (void *)-1; }
int munmap(void *a, size_t n) { (void)a; (void)n; return 0; }
int shm_open(const char *n, int f, unsigned m) { (void)n; (void)f; (void)m; return -1; }
int shm_unlink(const char *n) { (void)n; return -1; }
int backtrace(void **b, int n) { (void)b; (void)n; return 0; }
char **backtrace_symbols(void *const *b, int n) { (void)b; (void)n; return 0; }

/* ---------------------------------------------------------------- processo e ambiente */
/* ambiente: so o que o proprio programa define (o port usa TMC_AUTOPLAY para pular a tela inicial) */
static char amb_nome[16][40], amb_valor[16][120];
char *getenv(const char *n)
{
    int i;
    for (i = 0; i < 16; i++) if (amb_nome[i][0] && !strcmp(amb_nome[i], n)) return amb_valor[i];
    return 0;
}
int setenv(const char *n, const char *v, int o)
{
    int i, livre = -1;
    for (i = 0; i < 16; i++) {
        if (amb_nome[i][0] && !strcmp(amb_nome[i], n)) { if (o) { strncpy(amb_valor[i], v, 119); } return 0; }
        if (!amb_nome[i][0] && livre < 0) livre = i;
    }
    if (livre < 0 || strlen(n) > 39) return -1;
    strcpy(amb_nome[livre], n);
    strncpy(amb_valor[livre], v, 119);
    return 0;
}
static void (*saida[32])(void);
static int nsaida;
int atexit(void (*fn)(void)) { if (nsaida < 32) saida[nsaida++] = fn; return 0; }
void exit(int code)
{
    while (nsaida) saida[--nsaida]();
    lg("tmc: exit(%d)", code);
    lg_commit();
    ExitThread((u32)code);
    for (;;) ;
}
void _exit(int code) { exit(code); }
void abort(void) { lg("tmc: abort()"); exit(3); }
void __assert_fail(const char *e, const char *f, unsigned l, const char *fn)
{
    lg("tmc: assert %s em %s:%u (%s)", e, f, l, fn);
    exit(4);
}
long time(long *t) { long v = (long)(GetTickCount() / 1000); if (t) *t = v; return v; }
long clock(void) { return (long)GetTickCount(); }

/* ---------------------------------------------------------------- matematica em float (via double) */
float sqrtf(float x) { return (float)sqrt(x); }
float floorf(float x) { return (float)floor(x); }
float tanhf(float x) { return (float)tanh(x); }
float sinf(float x) { return (float)sin(x); }
float cosf(float x) { return (float)cos(x); }
void sincosf(float x, float *s, float *c) { *s = (float)sin(x); *c = (float)cos(x); }
long lrintf(float x) { return (long)(x < 0 ? x - 0.5f : x + 0.5f); }
double atan2(double y, double x)
{
    static double (*f)(double, double);
    if (!f) f = (double (*)(double, double))DYNC(atan2);
    return f ? f(y, x) : 0;
}
float atan2f(float y, float x) { return (float)atan2(y, x); }
float fabsf(float x) { return x < 0 ? -x : x; }
double fabs(double x) { return x < 0 ? -x : x; }

/* ---------------------------------------------------------------- sobras da ligacao */
void _Exit(int code) { exit(code); }
long long __time64(long long *t) { long long v = (long long)(GetTickCount() / 1000); if (t) *t = v; return v; }
/* a MN1 tem um nucleo so; sem ll/sc (MIPS II), basta a troca simples */
unsigned __atomic_exchange_4(volatile void *p, unsigned v, int m) { unsigned o = *(volatile unsigned *)p; (void)m; *(volatile unsigned *)p = v; return o; }

/* sscanf minimo: %d %u %x %X (com largura) e texto literal. O port so usa "%08X.bin". */
int sscanf(const char *s, const char *f, ...)
{
    va_list a;
    int n = 0;
    va_start(a, f);
    for (; *f; f++) {
        if (*f != '%') { if (*s != *f) break; s++; continue; }
        f++;
        {
            int w = 0, base;
            char tmp[24], *e;
            while (*f >= '0' && *f <= '9') w = w * 10 + *f++ - '0';
            base = (*f == 'x' || *f == 'X') ? 16 : 10;
            if (!w || w > 23) w = 23;
            strncpy(tmp, s, (size_t)w);
            tmp[w] = 0;
            if (*f == 'd') *va_arg(a, int *) = (int)strtol(tmp, &e, base);
            else *va_arg(a, unsigned *) = (unsigned)strtoul(tmp, &e, base);
            if (e == tmp) break;
            s += e - tmp;
            n++;
        }
    }
    va_end(a);
    return n;
}
int __isoc99_sscanf(const char *s, const char *f, ...) __attribute__((alias("sscanf")));
