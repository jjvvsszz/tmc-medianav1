/* Tela e toque da MN1 para o port do Minish Cap (substitui port_ppu.cpp, que e C++/SDL/GPU).

   Quadro: o PPU em software do port (VirtuaPPU, o mesmo medido no tmcteste) desenha 240x160;
   aqui ele e ampliado 3x para 720x480 no centro da tela de 800x480 e copiado com BitBlt.
   Ritmo: desenhar custa ~46 ms na MN1. PresentFrame so desenha quando ja passou o intervalo
   minimo desde o ultimo desenho; nas outras chamadas volta na hora, e o port gasta o tempo
   rodando a logica do jogo (ver patch_mn1_bios.py: o port recupera ticks atrasados).
   Toque: botoes desenhados por cima do jogo (so contorno, para nao esconder a cena). A tela da
   MN1 e de um toque so: segurar direcao e apertar A ao mesmo tempo nao da. Deslizar o dedo
   de um botao para outro troca o botao.
   SAIR (alto, no meio): fecha o jogo e devolve a MediaNav. */
#include "mn1.h"
#include "virtuappu.h"
#include "cpu/mode1.h"

/* entrada (mn1_config.c) e ordem do enum PortInput do port */
void mn1_input_set(int input, int down);
enum { IN_A, IN_B, IN_SELECT, IN_START, IN_RIGHT, IN_LEFT, IN_UP, IN_DOWN, IN_R, IN_L, IN_N };

/* memoria do GBA emulada pelo port */
extern u8 gIoMem[0x400];
extern u16 gBgPltt[256];
extern u16 gObjPltt[256];
extern u16 gOamMem[0x200];
extern u8 gVram[0x18000];
int port_hdma_has_active_channels(void);
int port_hdma_dest_overlaps(const void *lo, const void *hi);
void port_hdma_step_line(int line);
void exit(int);

static Screen tela;
static int pronta;
static VirtuaPPUMode1GbaMemory mem;

/* ---------------------------------------------------------------- botoes */
typedef struct { int x0, y0, x1, y1, in; const wchar *rotulo; int redondo; } Botao;
#define DX 110
#define DY 340
static const Botao BOT[] = {
    { 0, 0, 130, 64, IN_L, L"L", 0 },
    { 670, 0, 800, 64, IN_R, L"R", 0 },
    { 300, 440, 390, 480, IN_SELECT, L"SELECT", 0 },
    { 410, 440, 500, 480, IN_START, L"START", 0 },
    { 676, 296, 776, 396, IN_A, L"A", 1 },
    { 566, 366, 656, 456, IN_B, L"B", 1 },
};
#define NBOT (int)(sizeof BOT / sizeof BOT[0])
static const RECT SAIR = { 360, 0, 440, 34 };

/* contorno pre-calculado: deslocamento no quadro e cor */
static u32 *ov_pos;
static u16 *ov_cor;
static int ov_n;

static void px(u16 *b, int w, int x, int y, u16 c)
{
    if (x >= 0 && y >= 0 && x < w && y < tela.h) b[y * w + x] = c;
}
static void retangulo(u16 *b, int w, int x0, int y0, int x1, int y1, u16 c, int esp)
{
    int x, y, k;
    for (k = 0; k < esp; k++) {
        for (x = x0; x < x1; x++) { px(b, w, x, y0 + k, c); px(b, w, x, y1 - 1 - k, c); }
        for (y = y0; y < y1; y++) { px(b, w, x0 + k, y, c); px(b, w, x1 - 1 - k, y, c); }
    }
}
static void anel(u16 *b, int w, int cx, int cy, int r, u16 c, int esp)
{
    int x, y;
    for (y = -r; y <= r; y++)
        for (x = -r; x <= r; x++) {
            int d = x * x + y * y;
            if (d <= r * r && d > (r - esp) * (r - esp)) px(b, w, cx + x, cy + y, c);
        }
}
static void seta(u16 *b, int w, int cx, int cy, int dx, int dy, u16 c)
{
    int i, j;
    for (i = 0; i < 16; i++)
        for (j = -i; j <= i; j++)
            px(b, w, cx + dx * (16 - i) + dy * j, cy + dy * (16 - i) + dx * j, c);
}

static void monta_contornos(void)
{
    u16 *b = tela.bits;
    int w = tela.w, i, n = 0;
    const u16 claro = 0xFFFF, escuro = 0x2104;
    RECT r;
    memset(b, 0, (u32)w * tela.h * 2);
    /* direcional: cruz de 3 quadrados por eixo, com setas */
    retangulo(b, w, DX - 30, DY - 100, DX + 30, DY + 100, escuro, 4);
    retangulo(b, w, DX - 100, DY - 30, DX + 100, DY + 30, escuro, 4);
    retangulo(b, w, DX - 29, DY - 99, DX + 29, DY + 99, claro, 2);
    retangulo(b, w, DX - 99, DY - 29, DX + 99, DY + 29, claro, 2);
    seta(b, w, DX, DY - 64, 0, -1, claro);
    seta(b, w, DX, DY + 64, 0, 1, claro);
    seta(b, w, DX - 64, DY, -1, 0, claro);
    seta(b, w, DX + 64, DY, 1, 0, claro);
    for (i = 0; i < NBOT; i++) {
        const Botao *o = &BOT[i];
        if (o->redondo) {
            int cx = (o->x0 + o->x1) / 2, cy = (o->y0 + o->y1) / 2, rr = (o->x1 - o->x0) / 2;
            anel(b, w, cx, cy, rr, escuro, 5);
            anel(b, w, cx, cy, rr - 1, claro, 3);
        } else {
            retangulo(b, w, o->x0 + 2, o->y0 + 2, o->x1 - 2, o->y1 - 2, escuro, 4);
            retangulo(b, w, o->x0 + 3, o->y0 + 3, o->x1 - 3, o->y1 - 3, claro, 2);
        }
    }
    retangulo(b, w, SAIR.left, SAIR.top, SAIR.right, SAIR.bottom, 0xF800, 2);
    /* rotulos: o GDI escreve no mesmo bitmap */
    SetBkMode(tela.mem, 1);
    for (i = 0; i < NBOT; i++) {
        r.left = BOT[i].x0; r.top = BOT[i].y0; r.right = BOT[i].x1; r.bottom = BOT[i].y1;
        ui_text(tela.mem, BOT[i].redondo ? 40 : 22, 0xFFFFFF, &r, BOT[i].rotulo, 0x25);
    }
    r = SAIR;
    ui_text(tela.mem, 18, 0x0000FF, &r, L"SAIR", 0x25);
    /* guarda so os pixels acesos */
    for (i = 0; i < w * tela.h; i++) if (b[i]) n++;
    ov_pos = LocalAlloc(0, (u32)n * 4);
    ov_cor = LocalAlloc(0, (u32)n * 2);
    if (!ov_pos || !ov_cor) { ov_n = 0; return; }
    for (i = 0, ov_n = 0; i < w * tela.h; i++) if (b[i]) { ov_pos[ov_n] = (u32)i; ov_cor[ov_n++] = b[i]; }
    memset(b, 0, (u32)w * tela.h * 2);
    lg("tela: contornos=%d pixels", ov_n);
}

/* botoes sob o dedo -> mascara de entradas */
static u32 botoes_em(int x, int y)
{
    u32 m = 0;
    int i, dx = x - DX, dy = y - DY;
    if (dx > -130 && dx < 130 && dy > -130 && dy < 130 && (dx * dx + dy * dy) > 15 * 15) {
        int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
        /* diagonal quando os dois eixos passam de metade do outro */
        if (ax * 2 > ay) m |= 1u << (dx > 0 ? IN_RIGHT : IN_LEFT);
        if (ay * 2 > ax) m |= 1u << (dy > 0 ? IN_DOWN : IN_UP);
        return m;
    }
    for (i = 0; i < NBOT; i++)
        if (x >= BOT[i].x0 - 10 && x < BOT[i].x1 + 10 && y >= BOT[i].y0 - 10 && y < BOT[i].y1 + 10) m |= 1u << BOT[i].in;
    return m;
}

static volatile int pedir_saida;
static u32 apertados;

static void aplica(u32 m)
{
    int i;
    for (i = 0; i < IN_N; i++)
        if (((apertados ^ m) >> i) & 1) mn1_input_set(i, (m >> i) & 1);
    apertados = m;
}

static int proc_tela(HWND w, u32 msg, u32 wp, u32 lp)
{
    int x = (short)(lp & 0xFFFF), y = (short)(lp >> 16);
    switch (msg) {
    case WM_LBUTTONDOWN:
        if (x >= SAIR.left && x < SAIR.right && y >= SAIR.top && y < SAIR.bottom + 10) { pedir_saida = 1; return 0; }
        aplica(botoes_em(x, y));
        return 0;
    case WM_MOUSEMOVE:
        if (wp & 1) aplica(botoes_em(x, y));
        return 0;
    case WM_LBUTTONUP:
        aplica(0);
        return 0;
    case WM_PAINT:
    case 0x14: /* WM_ERASEBKGND */
        break;
    }
    return DefWindowProcW(w, msg, wp, lp);
}

void mn1_ppu_pump(void)
{
    ui_pump();
    if (pedir_saida) {
        lg("tmc: SAIR tocado");
        scr_close(&tela);
        exit(0);
    }
}

/* ---------------------------------------------------------------- quadro */
static u32 ultimo_desenho, custo_ema, n_desenhos, n_chamadas, t_relatorio, soma_render, soma_amplia, soma_copia;

static void amplia(const u32 *src, u16 *dst, int dw)
{
    int x, y, x0 = (dw - 720) / 2;
    for (y = 0; y < 160; y++) {
        u16 *l0 = dst + (y * 3) * dw + x0;
        const u32 *s = src + y * 240;
        for (x = 0; x < 240; x++) {
            u32 c = s[x];
            u16 p = (u16)(((c & 0xF8) << 8) | ((c >> 5) & 0x7E0) | ((c >> 19) & 0x1F));
            l0[x * 3] = l0[x * 3 + 1] = l0[x * 3 + 2] = p;
        }
        memcpy(l0 + dw, l0, 720 * 2);
        memcpy(l0 + 2 * dw, l0, 720 * 2);
    }
}

void Port_PPU_Init(void *window)
{
    (void)window;
    if (pronta) return;
    if (!scr_open(&tela, (void *)proc_tela) || tela.w < 720 || tela.h < 480) {
        lg("tela: nao abriu (%dx%d)", tela.w, tela.h);
        lg_commit();
        exit(5);
    }
    monta_contornos();
    mem.io_mem = gIoMem;
    mem.vram = gVram;
    mem.bg_palette = gBgPltt;
    mem.obj_palette = gObjPltt;
    mem.oam_mem = gOamMem;
    virtuappu_reset();
    virtuappu_mode1_bind_gba_memory(&mem);
    pronta = 1;
    t_relatorio = GetTickCount();
    lg("tela: %dx%d pronta", tela.w, tela.h);
    lg_commit();
}

void Port_PPU_PresentFrame(void)
{
    u32 a, b, c, d, agora, intervalo;
    u16 dispcnt;
    int i;
    if (!pronta) return;
    n_chamadas++;
    agora = now_us();
    /* desenha no maximo ~20 vezes por segundo, e deixa pelo menos metade do tempo para a logica */
    intervalo = custo_ema * 2;
    if (intervalo < 50000) intervalo = 50000;
    if (n_desenhos && agora - ultimo_desenho < intervalo) return;
    ultimo_desenho = agora;

    dispcnt = (u16)(gIoMem[0] | (gIoMem[1] << 8));
    virtuappu_registers.frame_width = 240;
    virtuappu_registers.frame_pitch = 240;
    virtuappu_registers.mode = (u8)(((dispcnt & 7) == 1 || (dispcnt & 7) == 2) ? 2 : 1);
    virtuappu_mode1_pre_line_callback = port_hdma_has_active_channels() ? port_hdma_step_line : 0;
    virtuappu_mode1_bg2x_hdma_strobe = port_hdma_dest_overlaps(gIoMem + 0x28, gIoMem + 0x2C) != 0;
    virtuappu_mode1_bg2y_hdma_strobe = port_hdma_dest_overlaps(gIoMem + 0x2C, gIoMem + 0x30) != 0;
    a = now_us();
    virtuappu_render_frame();
    b = now_us();
    amplia(virtuappu_get_frame_buffer(), tela.bits, tela.w);
    for (i = 0; i < ov_n; i++) tela.bits[ov_pos[i]] = ov_cor[i];
    c = now_us();
    BitBlt(tela.dc, 0, 0, tela.w, tela.h, tela.mem, 0, 0, 0x00CC0020);
    d = now_us();
    custo_ema = n_desenhos ? (custo_ema * 7 + (d - a)) / 8 : d - a;
    soma_render += b - a; soma_amplia += c - b; soma_copia += d - c;
    n_desenhos++;
    if (GetTickCount() - t_relatorio >= 10000) {
        u32 ms = GetTickCount() - t_relatorio;
        lg("R quadro: desenhos=%u chamadas=%u em %u ms; media ms: ppu=%u ampliar=%u copiar=%u",
           n_desenhos, n_chamadas, ms, soma_render / n_desenhos / 1000, soma_amplia / n_desenhos / 1000,
           soma_copia / n_desenhos / 1000);
        lg_commit();
        t_relatorio = GetTickCount();
        n_desenhos = n_chamadas = soma_render = soma_amplia = soma_copia = 0;
    }
}

/* o resto da interface de port_ppu.h: sem efeito na MN1 */
void Port_PPU_Shutdown(void) { if (pronta) scr_close(&tela); pronta = 0; }
void Port_PPU_SetPresentIsFirstOfTick(int first) { (void)first; }
void Port_PPU_SetWindowTitle(const char *t) { (void)t; }
void Port_PPU_ToggleFullscreen(void) { }
int Port_PPU_IsFullscreen(void) { return 1; }
void Port_PPU_CycleWindowScale(int d) { (void)d; }
unsigned char Port_PPU_WindowScale(void) { return 3; }
void Port_PPU_ToggleSmoothing(void) { }
void Port_PPU_CyclePresentationMode(int d) { (void)d; }
const char *Port_PPU_PresentationModeName(void) { return "MN1 3x"; }
void Port_PPU_SetVSync(int on) { (void)on; }
int Port_PPU_VSyncEnabled(void) { return 0; }
unsigned Port_PPU_DisplayRefreshRate(void) { return 60; }
