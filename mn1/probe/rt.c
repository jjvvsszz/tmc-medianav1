/* Runtime minimo: strings, formato, caminhos e registro que sobrevive a travamento.
   Toda escrita em disco do programa passa por respath(): so existe caminho para dentro de
   <pendrive>\RESULTADOS. Cada confirmacao abre, grava, descarrega e FECHA o arquivo, porque
   o fechamento e o que atualiza o tamanho no diretorio FAT se a energia cair logo depois. */
#include "mn1.h"

HANDLE g_inst;
wchar g_root[280];
HWND g_prev_fg;

typedef int (*QPC_t)(void *);
typedef int (*FLUSH_t)(HANDLE);
static QPC_t qpc;
static u32 qfreq;
static FLUSH_t flushf;
static int rt_ready;
static wchar lgpath[300];
static char lgbuf[8192];
static int lglen;

void *dyn(const wchar *dll, const wchar *name, int ord)
{
    HANDLE h = LoadLibraryW(dll);
    void *p;
    if (!h) return 0;
    p = GetProcAddressW(h, name);
    if (!p && ord) p = GetProcAddressW(h, (const wchar *)(unsigned)ord); /* ordinal do mapa CE6 */
    return p;
}

static void rt_init(void)
{
    u32 q[2];
    int (*fr)(void *);
    if (rt_ready) return;
    rt_ready = 1;
    flushf = (FLUSH_t)DYNC(FlushFileBuffers);
    qpc = (QPC_t)DYNC(QueryPerformanceCounter);
    fr = (int (*)(void *))DYNC(QueryPerformanceFrequency);
    q[0] = q[1] = 0;
    if (!qpc || !fr || !fr(q) || q[1] || q[0] < 1000) { qpc = 0; qfreq = 0; }
    else qfreq = q[0];
}

int have_qpc(void) { rt_init(); return qpc != 0; }
u32 qpc_freq(void) { rt_init(); return qfreq; }

u32 now_us(void)
{
    u32 q[2];
    rt_init();
    if (!qpc || !qpc(q)) return GetTickCount() * 1000;
    /* Acumula diferencas, em vez de dividir o contador: a 660 MHz os 32 bits baixos dao a volta
       em 6,5 s. A diferenca modular so e valida entre chamadas proximas; acima disso vale o tick. */
    {
        static u32 lastq, lastt, acc, rem;
        u32 t = GetTickCount(), d = q[0] - lastq, div = qfreq >= 1000000 ? qfreq / 1000000 : 1;
        if (t - lastt > 4000) { acc += (t - lastt) * 1000; rem = 0; }
        else if (qfreq >= 1000000) { d += rem; acc += d / div; rem = d % div; }
        else acc += d * (1000000 / qfreq);
        lastq = q[0];
        lastt = t;
        return acc;
    }
}

int wlen(const wchar *s) { int n = 0; while (s[n]) n++; return n; }
void wcpy(wchar *d, const wchar *s) { while ((*d++ = *s++)) ; }
void strcpy_a(char *d, const char *s) { while ((*d++ = *s++)) ; }
void wcat(wchar *d, const wchar *s) { wcpy(d + wlen(d), s); }
static wchar low(wchar c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
int wcmpi(const wchar *a, const wchar *b)
{
    while (*a && low(*a) == low(*b)) { a++; b++; }
    return low(*a) - low(*b);
}
int wfind(const wchar *h, const char *n)
{
    int i, j;
    for (i = 0; h[i]; i++) {
        for (j = 0; n[j] && low(h[i + j]) == (wchar)n[j]; j++) ;
        if (!n[j]) return i;
    }
    return -1;
}

void w_utf8(char *d, int n, const wchar *s)
{
    int k = 0;
    for (; *s && k < n - 4; s++) {
        wchar c = *s;
        if (c < 0x20) d[k++] = '?';
        else if (c < 0x80) d[k++] = (char)c;
        else if (c < 0x800) { d[k++] = 0xC0 | (c >> 6); d[k++] = 0x80 | (c & 63); }
        else { d[k++] = 0xE0 | (c >> 12); d[k++] = 0x80 | ((c >> 6) & 63); d[k++] = 0x80 | (c & 63); }
    }
    d[k] = 0;
}

void a_wide(wchar *d, int n, const char *s)
{
    int k = 0;
    while (*s && k < n - 1) d[k++] = (u8)*s++;
    d[k] = 0;
}

int fmtv(char *b, int n, const char *f, __builtin_va_list a)
{
    int k = 0;
    static char wtmp[600];
#define PUT(c) do { if (k < n - 1) b[k++] = (c); } while (0)
    for (; *f; f++) {
        char t[12];
        int i = 0, zero = 0, width = 0;
        u32 v;
        const char *s;
        if (*f != '%') { PUT(*f); continue; }
        f++;
        if (*f == '0') { zero = 1; f++; }
        while (*f >= '0' && *f <= '9') width = width * 10 + *f++ - '0';
        switch (*f) {
        case 'd': {
            int x = __builtin_va_arg(a, int);
            v = x < 0 ? 0u - (u32)x : (u32)x;
            if (x < 0) PUT('-');
            goto dec;
        case 'u':
            v = __builtin_va_arg(a, u32);
        dec:
            do { t[i++] = '0' + v % 10; v /= 10; } while (v);
            goto out;
        }
        case 'x':
            v = __builtin_va_arg(a, u32);
            do { t[i++] = "0123456789abcdef"[v & 15]; v >>= 4; } while (v);
        out:
            while (i < width) t[i++] = zero ? '0' : ' ';
            while (i) PUT(t[--i]);
            break;
        case 's':
            s = __builtin_va_arg(a, const char *);
            if (!s) s = "(null)";
            while (*s) PUT(*s++);
            break;
        case 'S':
            w_utf8(wtmp, sizeof wtmp, __builtin_va_arg(a, const wchar *));
            for (s = wtmp; *s; s++) PUT(*s);
            break;
        case 'c':
            PUT(__builtin_va_arg(a, int));
            break;
        default:
            PUT(*f);
        }
    }
    b[k] = 0;
    return k;
#undef PUT
}

int fmt(char *b, int n, const char *f, ...)
{
    __builtin_va_list a;
    int r;
    __builtin_va_start(a, f);
    r = fmtv(b, n, f, a);
    __builtin_va_end(a);
    return r;
}

void root_init(void)
{
    int n, i, last = -1, sep = 0;
    n = GetModuleFileNameW(g_inst, g_root, 270);
    for (i = 0; i < n; i++) if (g_root[i] == '\\') { sep = last; last = i; }
    if (sep > 0) g_root[sep] = 0;  /* <raiz>\MN1\mn1probe.exe -> <raiz> */
    else wcpy(g_root, L"\\MD");
}

wchar *respath(const wchar *name)
{
    static wchar p[4][320];
    static int i;
    wchar *r = p[i = (i + 1) & 3];
    wcpy(r, g_root);
    wcat(r, L"\\RESULTADOS3");
    if (name[0]) { wcat(r, L"\\"); wcat(r, name); }
    return r;
}

void lg_commit(void)
{
    HANDLE h;
    u32 w;
    if (!lglen || !lgpath[0]) return;
    rt_init();
    h = CreateFileW(lgpath, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH | 0x80, 0);
    if (h == INVALID_HANDLE) return;
    SetFilePointer(h, 0, 0, 2);
    WriteFile(h, lgbuf, lglen, &w, 0);
    if (flushf) flushf(h);
    CloseHandle(h);
    lglen = 0;
}

void lg_open(const wchar *name)
{
    lg_commit();
    wcpy(lgpath, respath(name));
}

void lg(const char *f, ...)
{
    static char line[1200];
    __builtin_va_list a;
    int n;
    __builtin_va_start(a, f);
    n = fmtv(line, sizeof line - 1, f, a);
    __builtin_va_end(a);
    line[n++] = '\n';
    if (lglen + n > (int)sizeof lgbuf) lg_commit();
    memcpy(lgbuf + lglen, line, n);
    lglen += n;
}

void hexdump(const char *tag, const void *p, int n)
{
    static char b[140];
    const u8 *d = p;
    int i, k;
    for (i = 0; i < n; i += 32) {
        k = fmt(b, sizeof b, "%s %04x:", tag, i);
        int j;
        for (j = i; j < i + 32 && j < n; j++) k += fmt(b + k, sizeof b - k, " %02x", d[j]);
        lg("%s", b);
    }
}
