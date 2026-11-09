/* O pedaco do SDL3 que o port usa, sobre o Windows CE da MN1.
   Tempo: relogio de alta resolucao (now_us do MN1_Probe) acumulado em 64 bits.
   Janela/renderizador: a tela de verdade e aberta por mn1_ppu.c; aqui sao objetos de faz-de-conta.
   Eventos: o toque chega pela janela (mn1_ppu.c) direto em mn1_input_set; PollEvent so bombeia
   as mensagens do CE e nunca devolve evento.
   Som: sem audio por enquanto (OpenAudioDeviceStream falha e o port segue mudo). */
#include <SDL3/SDL.h>
#include <stdarg.h>
#include <stdio.h>

unsigned now_us(void);
void Sleep(unsigned);
void ui_pump(void);
void lg(const char *f, ...);
void lg_commit(void);
void mn1_ppu_pump(void);

static unsigned long long relogio_us(void)
{
    static unsigned last;
    static unsigned long long acc;
    unsigned t = now_us();
    acc += (unsigned)(t - last);
    last = t;
    return acc;
}

bool SDL_Init(Uint32 flags) { (void)flags; return true; }
bool SDL_InitSubSystem(Uint32 flags) { (void)flags; return true; }
void SDL_QuitSubSystem(Uint32 flags) { (void)flags; }
void SDL_Quit(void) { lg_commit(); }
const char *SDL_GetError(void) { return "MN1: recurso ausente"; }
bool SDL_SetHint(const char *n, const char *v) { (void)n; (void)v; return true; }
bool SDL_ResetHint(const char *n) { (void)n; return true; }

Uint64 SDL_GetTicks(void) { return relogio_us() / 1000; }
Uint64 SDL_GetTicksNS(void) { return relogio_us() * 1000; }
Uint64 SDL_GetPerformanceCounter(void) { return relogio_us(); }
Uint64 SDL_GetPerformanceFrequency(void) { return 1000000; }
void SDL_Delay(Uint32 ms) { Sleep(ms); }
void SDL_DelayPrecise(Uint64 ns)
{
    unsigned long long fim = relogio_us() + ns / 1000;
    if (ns >= 2000000) Sleep((unsigned)(ns / 1000000) - 1);
    while (relogio_us() < fim) ;
}

void SDL_Log(const char *fmt, ...)
{
    static char b[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(b, sizeof b, fmt, a);
    va_end(a);
    lg("sdl: %s", b);
}
int SDL_snprintf(char *t, size_t n, const char *fmt, ...)
{
    va_list a;
    int r;
    va_start(a, fmt);
    r = vsnprintf(t, n, fmt, a);
    va_end(a);
    return r;
}
size_t SDL_strlcpy(char *d, const char *s, size_t n)
{
    size_t k = strlen(s);
    if (n) {
        size_t c = k < n - 1 ? k : n - 1;
        memcpy(d, s, c);
        d[c] = 0;
    }
    return k;
}
size_t SDL_strlen(const char *s) { return strlen(s); }

bool SDL_PollEvent(SDL_Event *e)
{
    (void)e;
    mn1_ppu_pump();
    return false;
}

/* janela e renderizador de faz-de-conta: ponteiros validos, nada desenhado por aqui */
static int janela_falsa, render_falso;
bool SDL_CreateWindowAndRenderer(const char *title, int w, int h, SDL_WindowFlags flags, SDL_Window **win, SDL_Renderer **ren)
{
    (void)title; (void)w; (void)h; (void)flags;
    if (win) *win = (SDL_Window *)&janela_falsa;
    if (ren) *ren = (SDL_Renderer *)&render_falso;
    return true;
}
void SDL_DestroyWindow(SDL_Window *w) { (void)w; }
bool SDL_ShowWindow(SDL_Window *w) { (void)w; return true; }
bool SDL_RaiseWindow(SDL_Window *w) { (void)w; return true; }
bool SDL_SyncWindow(SDL_Window *w) { (void)w; return true; }
bool SDL_SetWindowIcon(SDL_Window *w, SDL_Surface *s) { (void)w; (void)s; return true; }
void SDL_DestroySurface(SDL_Surface *s) { (void)s; }
bool SDL_SetRenderDrawColor(SDL_Renderer *r, Uint8 a, Uint8 b, Uint8 c, Uint8 d) { (void)r; (void)a; (void)b; (void)c; (void)d; return true; }
bool SDL_SetRenderDrawBlendMode(SDL_Renderer *r, SDL_BlendMode m) { (void)r; (void)m; return true; }
bool SDL_RenderFillRect(SDL_Renderer *r, const SDL_FRect *f) { (void)r; (void)f; return true; }
bool SDL_RenderRect(SDL_Renderer *r, const SDL_FRect *f) { (void)r; (void)f; return true; }
bool SDL_RenderDebugText(SDL_Renderer *r, float x, float y, const char *s) { (void)r; (void)x; (void)y; lg("tela: %s", s); return true; }
int SDL_GetNumVideoDrivers(void) { return 1; }
const char *SDL_GetVideoDriver(int i) { (void)i; return "mn1"; }
const char *SDL_GetCurrentVideoDriver(void) { return "mn1"; }

bool SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *msg, SDL_Window *w)
{
    (void)flags; (void)w;
    lg("aviso: %s: %s", title ? title : "", msg ? msg : "");
    lg_commit();
    return true;
}

SDL_AudioStream *SDL_OpenAudioDeviceStream(SDL_AudioDeviceID d, const SDL_AudioSpec *s, SDL_AudioStreamCallback cb, void *u)
{
    (void)d; (void)s; (void)cb; (void)u;
    return 0;
}
bool SDL_PutAudioStreamData(SDL_AudioStream *s, const void *b, int n) { (void)s; (void)b; (void)n; return false; }
bool SDL_ResumeAudioStreamDevice(SDL_AudioStream *s) { (void)s; return false; }
void SDL_DestroyAudioStream(SDL_AudioStream *s) { (void)s; }
