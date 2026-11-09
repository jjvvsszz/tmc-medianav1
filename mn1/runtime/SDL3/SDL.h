/* SDL3/SDL.h da MN1: implementacao propria, so com o que o port do Minish Cap usa.
   Nao e o SDL: os nomes batem para o codigo do port compilar sem mudanca; os valores das
   constantes so precisam ser coerentes com a implementacao em mn1/sdl_mn1.c.
   Janela, renderizador, GPU e dialogos viram operacoes vazias; tempo, eventos (toque ->
   botoes de controle), arquivos, threads e audio sao implementados sobre o Windows CE. */
#ifndef MN1_SDL_H
#define MN1_SDL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef int32_t Sint32;
typedef uint64_t Uint64;
typedef int64_t Sint64;

/* ---------------------------------------------------------------- inicializacao e erros */
#define SDL_INIT_AUDIO 0x00000010u
#define SDL_INIT_VIDEO 0x00000020u
bool SDL_Init(Uint32 flags);
bool SDL_InitSubSystem(Uint32 flags);
void SDL_QuitSubSystem(Uint32 flags);
void SDL_Quit(void);
const char *SDL_GetError(void);
#define SDL_main main

/* ---------------------------------------------------------------- dicas */
#define SDL_HINT_VIDEO_DRIVER "SDL_VIDEO_DRIVER"
#define SDL_HINT_RENDER_DRIVER "SDL_RENDER_DRIVER"
#define SDL_HINT_AUDIO_DRIVER "SDL_AUDIO_DRIVER"
#define SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES "SDL_AUDIO_DEVICE_SAMPLE_FRAMES"
#define SDL_HINT_THREAD_PRIORITY_POLICY "SDL_THREAD_PRIORITY_POLICY"
#define SDL_HINT_ANDROID_LOW_LATENCY_AUDIO "SDL_ANDROID_LOW_LATENCY_AUDIO"
#define SDL_VIDEODRIVER "SDL_VIDEODRIVER"
#define SDL_AUDIODRIVER "SDL_AUDIODRIVER"
bool SDL_SetHint(const char *name, const char *value);
bool SDL_ResetHint(const char *name);

/* ---------------------------------------------------------------- tempo */
Uint64 SDL_GetTicks(void);
Uint64 SDL_GetTicksNS(void);
void SDL_Delay(Uint32 ms);
void SDL_DelayPrecise(Uint64 ns);

/* ---------------------------------------------------------------- texto e memoria */
void SDL_Log(const char *fmt, ...);
int SDL_snprintf(char *text, size_t maxlen, const char *fmt, ...);
size_t SDL_strlcpy(char *dst, const char *src, size_t maxlen);
size_t SDL_strlen(const char *str);
void SDL_free(void *mem);
#define SDL_zero(x) memset(&(x), 0, sizeof((x)))

/* ---------------------------------------------------------------- eventos */
#define SDL_EVENT_QUIT 0x100u
#define SDL_EVENT_KEY_DOWN 0x300u
#define SDL_EVENT_KEY_UP 0x301u
#define SDL_EVENT_MOUSE_MOTION 0x400u
#define SDL_EVENT_MOUSE_BUTTON_DOWN 0x401u
#define SDL_EVENT_MOUSE_BUTTON_UP 0x402u
#define SDL_EVENT_GAMEPAD_BUTTON_DOWN 0x651u
#define SDL_EVENT_GAMEPAD_BUTTON_UP 0x652u

typedef enum {
    SDL_GAMEPAD_BUTTON_SOUTH,
    SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST,
    SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_BACK,
    SDL_GAMEPAD_BUTTON_GUIDE,
    SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK,
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_DPAD_UP,
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT
} SDL_GamepadButton;

typedef Uint32 SDL_Keycode;
typedef Uint32 SDL_Scancode;
typedef Uint16 SDL_Keymod;
#define SDL_KMOD_SHIFT 0x0003u
#define SDL_KMOD_CTRL 0x00C0u
#define SDL_KMOD_ALT 0x0300u
/* teclas: so as que o port compara; a MN1 nao tem teclado, nenhuma chega */
#define SDLK_RETURN 0x0Du
#define SDLK_ESCAPE 0x1Bu
#define SDLK_BACKSPACE 0x08u
#define SDLK_TAB 0x09u
#define SDLK_APOSTROPHE 0x27u
#define SDLK_PERIOD 0x2Eu
#define SDLK_SEMICOLON 0x3Bu
#define SDLK_LEFTBRACKET 0x5Bu
#define SDLK_BACKSLASH 0x5Cu
#define SDLK_RIGHTBRACKET 0x5Du
#define SDLK_P 0x70u
#define SDLK_F1 0x4000003Au
#define SDLK_F4 0x4000003Du
#define SDLK_F5 0x4000003Eu
#define SDLK_F6 0x4000003Fu
#define SDLK_F7 0x40000040u
#define SDLK_F8 0x40000041u
#define SDLK_F9 0x40000042u
#define SDLK_F10 0x40000043u
#define SDLK_F11 0x40000044u
#define SDLK_F12 0x40000045u
#define SDLK_RIGHT 0x4000004Fu
#define SDLK_LEFT 0x40000050u
#define SDLK_DOWN 0x40000051u
#define SDLK_UP 0x40000052u

typedef struct { Uint32 type; Uint64 timestamp; SDL_Scancode scancode; SDL_Keycode key; SDL_Keymod mod; bool down, repeat; } SDL_KeyboardEvent;
typedef struct { Uint32 type; Uint64 timestamp; Uint8 button; bool down; } SDL_GamepadButtonEvent;
typedef struct { Uint32 type; Uint64 timestamp; float x, y; } SDL_MouseMotionEvent;
typedef struct { Uint32 type; Uint64 timestamp; Uint8 button; bool down; float x, y; } SDL_MouseButtonEvent;
typedef union SDL_Event {
    Uint32 type;
    SDL_KeyboardEvent key;
    SDL_GamepadButtonEvent gbutton;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    Uint8 padding[64];
} SDL_Event;
bool SDL_PollEvent(SDL_Event *event);
void SDL_PumpEvents(void);

/* ---------------------------------------------------------------- janela e renderizador (vazios) */
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Texture SDL_Texture;
typedef Uint64 SDL_WindowFlags;
#define SDL_WINDOW_RESIZABLE 0x20u
#define SDL_WINDOW_OPENGL 0x2u
#define SDL_WINDOW_VULKAN 0x10000000u
typedef enum { SDL_BLENDMODE_NONE = 0, SDL_BLENDMODE_BLEND = 1 } SDL_BlendMode;
typedef enum { SDL_SCALEMODE_NEAREST, SDL_SCALEMODE_LINEAR } SDL_ScaleMode;
#define SDL_PIXELFORMAT_ABGR8888 0x16762004u
#define SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE 8
typedef struct { float x, y, w, h; } SDL_FRect;
SDL_Window *SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags);
bool SDL_CreateWindowAndRenderer(const char *title, int w, int h, SDL_WindowFlags flags, SDL_Window **window,
                                 SDL_Renderer **renderer);
void SDL_DestroyWindow(SDL_Window *window);
SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, const char *name);
SDL_Renderer *SDL_GetRenderer(SDL_Window *window);
bool SDL_ShowWindow(SDL_Window *window);
bool SDL_RaiseWindow(SDL_Window *window);
bool SDL_SyncWindow(SDL_Window *window);
bool SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon);
void SDL_DestroySurface(SDL_Surface *surface);
bool SDL_SetRenderDrawColor(SDL_Renderer *r, Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca);
bool SDL_SetRenderDrawBlendMode(SDL_Renderer *r, SDL_BlendMode mode);
bool SDL_RenderFillRect(SDL_Renderer *r, const SDL_FRect *rect);
bool SDL_RenderRect(SDL_Renderer *r, const SDL_FRect *rect);
bool SDL_RenderDebugText(SDL_Renderer *r, float x, float y, const char *str);
bool SDL_RenderPresent(SDL_Renderer *r);
int SDL_GetNumVideoDrivers(void);
const char *SDL_GetVideoDriver(int index);
const char *SDL_GetCurrentVideoDriver(void);

/* GPU: so tipos opacos; o port na MN1 usa o PPU em software */
typedef struct SDL_GPUDevice SDL_GPUDevice;
typedef struct SDL_GPUTexture SDL_GPUTexture;
typedef struct SDL_GPUCommandBuffer SDL_GPUCommandBuffer;
typedef struct SDL_GPURenderPass SDL_GPURenderPass;
typedef Uint32 SDL_GPUShaderFormat;
typedef int SDL_GPUTextureFormat;
#define SDL_GPU_SHADERFORMAT_INVALID 0u
#define SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM 4

/* ---------------------------------------------------------------- arquivos e caminhos */
typedef struct SDL_IOStream SDL_IOStream;
typedef enum { SDL_PATHTYPE_NONE, SDL_PATHTYPE_FILE, SDL_PATHTYPE_DIRECTORY, SDL_PATHTYPE_OTHER } SDL_PathType;
typedef struct { SDL_PathType type; Uint64 size; Sint64 create_time, modify_time, access_time; } SDL_PathInfo;
void *SDL_LoadFile(const char *file, size_t *datasize);
bool SDL_SaveFile(const char *file, const void *data, size_t datasize);
SDL_IOStream *SDL_IOFromFile(const char *file, const char *mode);
char *SDL_GetPrefPath(const char *org, const char *app);
bool SDL_CreateDirectory(const char *path);
bool SDL_GetPathInfo(const char *path, SDL_PathInfo *info);
bool SDL_RemovePath(const char *path);
#define SDL_ANDROID_EXTERNAL_STORAGE_WRITE 0x02
const char *SDL_GetAndroidInternalStoragePath(void);
const char *SDL_GetAndroidExternalStoragePath(void);
Uint32 SDL_GetAndroidExternalStorageState(void);

/* ---------------------------------------------------------------- caixas de mensagem e dialogos */
#define SDL_MESSAGEBOX_ERROR 0x10u
#define SDL_MESSAGEBOX_WARNING 0x20u
#define SDL_MESSAGEBOX_INFORMATION 0x40u
#define SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT 0x1u
#define SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT 0x2u
typedef struct { Uint32 flags; int buttonID; const char *text; } SDL_MessageBoxButtonData;
typedef struct {
    Uint32 flags;
    SDL_Window *window;
    const char *title;
    const char *message;
    int numbuttons;
    const SDL_MessageBoxButtonData *buttons;
    const void *colorScheme;
} SDL_MessageBoxData;
bool SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, SDL_Window *window);
bool SDL_ShowMessageBox(const SDL_MessageBoxData *data, int *buttonid);
typedef struct { const char *name; const char *pattern; } SDL_DialogFileFilter;
typedef void (*SDL_DialogFileCallback)(void *userdata, const char *const *filelist, int filter);
void SDL_ShowOpenFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window,
                            const SDL_DialogFileFilter *filters, int nfilters, const char *default_location,
                            bool allow_many);
bool SDL_OpenURL(const char *url);

/* ---------------------------------------------------------------- threads */
typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_Mutex SDL_Mutex;
typedef struct SDL_Semaphore SDL_Semaphore;
typedef int (*SDL_ThreadFunction)(void *data);
typedef struct { int value; } SDL_AtomicInt;
SDL_Thread *SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data);
void SDL_WaitThread(SDL_Thread *thread, int *status);
SDL_Mutex *SDL_CreateMutex(void);
void SDL_LockMutex(SDL_Mutex *mutex);
void SDL_UnlockMutex(SDL_Mutex *mutex);
void SDL_DestroyMutex(SDL_Mutex *mutex);
SDL_Semaphore *SDL_CreateSemaphore(Uint32 initial_value);
void SDL_SignalSemaphore(SDL_Semaphore *sem);
void SDL_WaitSemaphore(SDL_Semaphore *sem);
void SDL_DestroySemaphore(SDL_Semaphore *sem);
int SDL_SetAtomicInt(SDL_AtomicInt *a, int v);
int SDL_GetAtomicInt(SDL_AtomicInt *a);

/* ---------------------------------------------------------------- audio (por enquanto mudo) */
typedef Uint32 SDL_AudioDeviceID;
typedef enum { SDL_AUDIO_UNKNOWN = 0, SDL_AUDIO_S16LE = 0x8010 } SDL_AudioFormat;
#define SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK ((SDL_AudioDeviceID)0xFFFFFFFFu)
typedef struct { SDL_AudioFormat format; int channels; int freq; } SDL_AudioSpec;
typedef struct SDL_AudioStream SDL_AudioStream;
typedef void (*SDL_AudioStreamCallback)(void *userdata, SDL_AudioStream *stream, int additional_amount,
                                        int total_amount);
SDL_AudioStream *SDL_OpenAudioDeviceStream(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec,
                                           SDL_AudioStreamCallback callback, void *userdata);
bool SDL_PutAudioStreamData(SDL_AudioStream *stream, const void *buf, int len);
bool SDL_ResumeAudioStreamDevice(SDL_AudioStream *stream);
void SDL_DestroyAudioStream(SDL_AudioStream *stream);

#ifdef __cplusplus
}
#endif
#endif
