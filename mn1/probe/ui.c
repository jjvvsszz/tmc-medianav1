/* Interface minima para tela de toque. Toda espera tem prazo: se o toque nao funcionar,
   a tela fecha sozinha e o motivo fica registrado. */
#include "mn1.h"

volatile int ui_last_x, ui_last_y, ui_tap;
static int cls_done;
STATICW(CLS, L"MN1ProbeWnd");

typedef struct {
    const wchar *title, *text;
    const wchar **opts;
    int n, choice;
    RECT btn[5];
} Ask;

int ui_screen_w(void) { int w = GetSystemMetrics(0); return w > 0 ? w : 800; }
int ui_screen_h(void) { int h = GetSystemMetrics(1); return h > 0 ? h : 480; }

void ui_text(HDC dc, int size, u32 color, RECT *r, const wchar *t, u32 flags)
{
    /* LOGFONTW: lfHeight primeiro, lfWeight no campo 5; FaceName vazio = fonte padrao */
    static u32 lf[23];
    HANDLE f, o = 0;
    memset(lf, 0, sizeof lf);
    lf[0] = (u32)size;
    lf[4] = 700;
    f = CreateFontIndirectW(lf);
    if (f) o = SelectObject(dc, f);
    SetTextColor(dc, color);
    SetBkMode(dc, 1);
    DrawTextW(dc, t, -1, r, flags);
    if (f) { SelectObject(dc, o); DeleteObject(f); }
}

static void fill(u16 *b, int w, int h, int x0, int y0, int x1, int y1, u16 c)
{
    int x, y;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > w) x1 = w;
    if (y1 > h) y1 = h;
    for (y = y0; y < y1; y++) {
        u16 *p = b + y * w;
        for (x = x0; x < x1; x++) p[x] = c;
    }
}

typedef int (*WP)(HWND, u32, u32, u32);
static WP g_proc;
static int ui_proc(HWND w, u32 m, u32 wp, u32 lp)
{
    if (g_proc) return g_proc(w, m, wp, lp);
    return DefWindowProcW(w, m, wp, lp);
}

static const u32 masks565[3] = { 0xF800, 0x07E0, 0x001F };

int scr_open(Screen *s, void *proc)
{
    static u32 bi[10 + 3];
    WNDCLASSW wc;
    if (!cls_done) {
        memset(&wc, 0, sizeof wc);
        wc.wndproc = (void *)ui_proc;
        wc.inst = g_inst;
        wc.name = CLS;
        RegisterClassW(&wc);
        cls_done = 1;
    }
    memset(s, 0, sizeof *s);
    s->w = ui_screen_w();
    s->h = ui_screen_h();
    g_proc = (WP)proc;
    s->wnd = CreateWindowExW(WS_EX_TOPMOST, CLS, L"MN1 Probe", WS_POPUP | WS_VISIBLE, 0, 0, s->w, s->h, 0, 0, g_inst, 0);
    if (!s->wnd) return 0;
    SetForegroundWindow(s->wnd);
    s->dc = GetDC(s->wnd);
    s->mem = CreateCompatibleDC(s->dc);
    memset(bi, 0, sizeof bi);
    bi[0] = 40;
    bi[1] = (u32)s->w;
    bi[2] = (u32)-s->h;      /* negativo: de cima para baixo */
    bi[3] = 1 | (16 << 16);
    bi[4] = 3;               /* BI_BITFIELDS */
    memcpy(&bi[10], masks565, 12);
    s->bmp = CreateDIBSection(s->dc, bi, 0, (void **)&s->bits, 0, 0);
    if (!s->bmp || !s->bits) { scr_close(s); return 0; }
    s->old = SelectObject(s->mem, s->bmp);
    return 1;
}

void scr_close(Screen *s)
{
    if (s->mem && s->old) SelectObject(s->mem, s->old);
    if (s->bmp) DeleteObject(s->bmp);
    if (s->mem) DeleteDC(s->mem);
    if (s->wnd && s->dc) ReleaseDC(s->wnd, s->dc);
    if (s->wnd) DestroyWindow(s->wnd);
    memset(s, 0, sizeof *s);
}

static int ask_proc(HWND w, u32 m, u32 wp, u32 lp)
{
    if (m == WM_LBUTTONDOWN) {
        ui_last_x = (short)(lp & 0xffff);
        ui_last_y = (short)(lp >> 16);
        ui_tap++;
        return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}

void ui_pump(void)
{
    MSG m;
    while (PeekMessageW(&m, 0, 0, 0, 1)) DispatchMessageW(&m);
}

static void ask_paint(Screen *s, Ask *a, int left)
{
    static wchar tmp[16];
    static char n[16];
    RECT r;
    int i, bw;
    fill(s->bits, s->w, s->h, 0, 0, s->w, s->h, 0x0000);
    r.left = 24; r.top = 16; r.right = s->w - 24; r.bottom = 70;
    ui_text(s->mem, 34, 0xFFFFFF, &r, a->title, 0x10);
    r.top = 78; r.bottom = s->h - 150;
    ui_text(s->mem, 26, 0xD0D0D0, &r, a->text, 0x10);
    if (a->n) {
        bw = (s->w - 24 - 16 * (a->n - 1) - 24) / a->n;
        for (i = 0; i < a->n; i++) {
            RECT *b = &a->btn[i];
            b->left = 24 + i * (bw + 16); b->right = b->left + bw;
            b->top = s->h - 130; b->bottom = s->h - 34;
            fill(s->bits, s->w, s->h, b->left, b->top, b->right, b->bottom, i == 0 ? 0x0400 : 0x4208);
            ui_text(s->mem, 28, 0xFFFFFF, b, a->opts[i], 0x25);
        }
    }
    fmt(n, sizeof n, "%ds", left);
    a_wide(tmp, 16, n);
    r.left = s->w - 90; r.right = s->w - 8; r.top = 6; r.bottom = 34;
    ui_text(s->mem, 20, 0x808080, &r, tmp, 0x22);
    BitBlt(s->dc, 0, 0, s->w, s->h, s->mem, 0, 0, 0x00CC0020);
}

/* Devolve o indice escolhido, ou -1 no prazo. Registra a resposta para o relatorio. */
int ui_ask(const char *key, const wchar *title, const wchar *text, const wchar **opts, int n, int timeout_s)
{
    static Screen s;
    Ask a;
    u32 t0 = GetTickCount();
    int choice = -1, last = -1, i, tap0;
    memset(&a, 0, sizeof a);
    a.title = title; a.text = text; a.opts = opts; a.n = n;
    if (!scr_open(&s, ask_proc)) { lg("R %s=SEM_TELA", key ? key : "ui"); lg_commit(); return -1; }
    tap0 = ui_tap;
    while (choice < 0) {
        int left = timeout_s - (int)((GetTickCount() - t0) / 1000);
        if (left < 0) break;
        if (left != last) { ask_paint(&s, &a, left); last = left; }
        ui_pump();
        if (ui_tap != tap0) {
            tap0 = ui_tap;
            for (i = 0; i < n; i++) {
                RECT *b = &a.btn[i];
                if (ui_last_x >= b->left && ui_last_x < b->right && ui_last_y >= b->top && ui_last_y < b->bottom) choice = i;
            }
        }
        if (!n && left == 0) break;
        Sleep(20);
    }
    scr_close(&s);
    if (key) {
        if (choice >= 0) lg("R %s=%S", key, opts[choice]);
        else lg("R %s=SEM_RESPOSTA", key);
        lg_commit();
    }
    return choice;
}

void ui_notice(const wchar *title, const wchar *text, int ms)
{
    ui_ask(0, title, text, 0, 0, ms / 1000);
}
