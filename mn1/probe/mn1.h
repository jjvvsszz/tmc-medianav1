/* Declaracoes minimas do Win32/CE, sem SDK. ABI MIPS CE = o32: a0-a3 + home space de 16 bytes. */
#ifndef MN1_H
#define MN1_H
#include "ords.h"
typedef unsigned short wchar;
typedef void *HANDLE;
typedef void *HWND;
typedef void *HDC;
typedef unsigned u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef struct { int left, top, right, bottom; } RECT;
typedef struct { HWND hwnd; u32 message, wParam, lParam, time; int x, y; } MSG;
typedef struct { u32 style; void *wndproc; int cls, wnd; HANDLE inst, icon, cursor, brush; const wchar *menu, *name; } WNDCLASSW;
typedef struct { u16 y, mo, dow, d, h, mi, s, ms; } SYSTEMTIME;

#define INVALID_HANDLE ((HANDLE)-1)
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define CREATE_ALWAYS 2
#define FILE_FLAG_WRITE_THROUGH 0x80000000
#define WM_DESTROY 2
#define WM_PAINT 15
#define WM_CLOSE 16
#define WM_KEYDOWN 0x100
#define WM_KEYUP 0x101
#define WM_CHAR 0x102
#define WM_SYSKEYDOWN 0x104
#define WM_TIMER 0x113
#define WM_MOUSEMOVE 0x200
#define WM_LBUTTONDOWN 0x201
#define WM_LBUTTONUP 0x202
#define WS_POPUP 0x80000000
#define WS_VISIBLE 0x10000000
#define WS_EX_TOPMOST 8
#define HKLM ((HANDLE)0x80000002)
#define HKCU ((HANDLE)0x80000001)

/* Importacoes estaticas: todas comprovadas no aparelho (ver tools/build.py) */
HANDLE CreateFileW(const wchar *, u32, u32, void *, u32, u32, HANDLE);
int WriteFile(HANDLE, const void *, u32, u32 *, void *);
int ReadFile(HANDLE, void *, u32, u32 *, void *);
int CloseHandle(HANDLE);
u32 SetFilePointer(HANDLE, int, int *, u32);
u32 GetFileSize(HANDLE, u32 *);
u32 GetFileAttributesW(const wchar *);
int CreateDirectoryW(const wchar *, void *);
HANDLE FindFirstFileW(const wchar *, void *);
int FindNextFileW(HANDLE, void *);
int FindClose(HANDLE);
u32 GetLastError(void);
void Sleep(u32);
u32 GetTickCount(void);
u32 GetModuleFileNameW(HANDLE, wchar *, u32);
HANDLE LoadLibraryW(const wchar *);
void *GetProcAddressW(HANDLE, const wchar *);
int FreeLibrary(HANDLE);
int CreateProcessW(const wchar *, const wchar *, void *, void *, int, u32, void *, void *, void *, void *);
u32 WaitForSingleObject(HANDLE, u32);
int GetExitCodeProcess(HANDLE, u32 *);
int TerminateProcess(HANDLE, u32);
void ExitThread(u32);
void *memset(void *, int, unsigned);
void *memcpy(void *, const void *, unsigned);
void *LocalAlloc(u32, u32);
void *LocalFree(void *);
void GetLocalTime(SYSTEMTIME *);
void GlobalMemoryStatus(void *);
int GetVersionExW(void *);
int GetDiskFreeSpaceExW(const wchar *, void *, void *, void *);
int RegOpenKeyExW(HANDLE, const wchar *, u32, u32, HANDLE *);
int RegEnumKeyExW(HANDLE, u32, wchar *, u32 *, u32 *, wchar *, u32 *, void *);
int RegEnumValueW(HANDLE, u32, wchar *, u32 *, u32 *, u32 *, void *, u32 *);
int RegCloseKey(HANDLE);
u16 RegisterClassW(const WNDCLASSW *);
HWND CreateWindowExW(u32, const wchar *, const wchar *, u32, int, int, int, int, HWND, void *, HANDLE, void *);
int DefWindowProcW(HWND, u32, u32, u32);
int ShowWindow(HWND, int);
int PeekMessageW(MSG *, HWND, u32, u32, u32);
int GetMessageW(MSG *, HWND, u32, u32);
int DispatchMessageW(const MSG *);
int TranslateMessage(const MSG *);
void PostQuitMessage(int);
int PostMessageW(HWND, u32, u32, u32);
HDC GetDC(HWND);
int ReleaseDC(HWND, HDC);
int GetSystemMetrics(int);
int GetDeviceCaps(HDC, int);
int DrawTextW(HDC, const wchar *, int, RECT *, u32);
int SetTextColor(HDC, u32);
int SetBkMode(HDC, int);
HANDLE CreateFontIndirectW(const void *);
HANDLE SelectObject(HDC, HANDLE);
int DeleteObject(HANDLE);
HDC CreateCompatibleDC(HDC);
int DeleteDC(HDC);
HANDLE CreateDIBSection(HDC, const void *, u32, void **, HANDLE, u32);
int BitBlt(HDC, int, int, int, int, HDC, int, int, u32);
HWND GetForegroundWindow(void);
int SetForegroundWindow(HWND);
int SetWindowPos(HWND, HWND, int, int, int, int, u32);
int EnumWindows(void *, u32);
int GetWindowTextW(HWND, wchar *, int);
int GetClassNameW(HWND, wchar *, int);
int GetWindowRect(HWND, RECT *);
u32 GetWindowThreadProcessId(HWND, u32 *);
int IsWindowVisible(HWND);
int DestroyWindow(HWND);
HWND FindWindowW(const wchar *, const wchar *);
int RegQueryValueExW(HANDLE, const wchar *, u32 *, u32 *, void *, u32 *);

/* rt.c: runtime e registro em disco */
extern HANDLE g_inst;
extern wchar g_root[];               /* raiz do pendrive, ex. \MD */
extern HWND g_prev_fg;
void *dyn(const wchar *dll, const wchar *name, int ord);   /* NULL se nao existir */
#define DYNC(n) dyn(L"coredll.dll", L###n, ORD_##n)
int fmtv(char *b, int n, const char *f, __builtin_va_list a);
int fmt(char *b, int n, const char *f, ...);
int wlen(const wchar *s);
void wcpy(wchar *d, const wchar *s);
void wcat(wchar *d, const wchar *s);
int wcmpi(const wchar *a, const wchar *b);
int wfind(const wchar *hay, const char *needle_lower);
void w_utf8(char *d, int n, const wchar *s);
void strcpy_a(char *d, const char *s);
void a_wide(wchar *d, int n, const char *s);
wchar *respath(const wchar *name);   /* raiz\RESULTADOS\name */
void root_init(void);
void lg_open(const wchar *name);
void lg(const char *f, ...);
void lg_commit(void);
u32 now_us(void);                    /* relogio de alta resolucao se houver; senao ms*1000 */
int have_qpc(void);
u32 qpc_freq(void);
void hexdump(const char *tag, const void *p, int n);

/* ui.c */
typedef struct { HWND wnd; HDC dc, mem; HANDLE bmp, old; u16 *bits; int w, h; } Screen;
int ui_ask(const char *key, const wchar *title, const wchar *text, const wchar **opts, int n, int timeout_s);
void ui_notice(const wchar *title, const wchar *text, int ms);
int scr_open(Screen *s, void *proc);
void scr_close(Screen *s);
void ui_pump(void);
void ui_text(HDC dc, int size, u32 color, RECT *r, const wchar *t, u32 flags);
int ui_screen_w(void);
int ui_screen_h(void);
extern volatile int ui_last_x, ui_last_y, ui_tap;

/* testes */
void t_sysinfo(void); void t_files(void); void t_procs(void); void t_registry(void);
void t_presence(void); void t_samples(void);
void t_gfxinfo(void); void t_bench(void); void t_dash(void); void t_tear(void); void t_rawfb(void);
void t_touch(void); void t_keys(void); void t_audio(void); void t_return(void);
void t_net(void); void t_com(void); void t_gps(void);
void t_bt_local(void); void t_bt_inquiry(void); void t_bt_spp(void);
void t_collect(void); void t_collect_nav(void); void t_ports(void); void t_usbread(void); void t_keys2(void);
void t_audio2(void); void t_gl(void); void t_long(void); void t_mort(void);
void t_live(void); void t_visor(void);
HANDLE live_open(int nobuf);
int live_read(HANDLE h, u32 *seq, u32 *ms, u32 *n);

#define STATICW(n, s) static const wchar n[] = s
#endif
