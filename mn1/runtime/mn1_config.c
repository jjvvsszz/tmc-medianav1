/* Gerado por gera_config.py: configuracao fixa da MN1. NAO EDITAR A MAO. */
#include <string.h>
#include "port_runtime_config.h"

volatile unsigned char mn1_input[PORT_INPUT_COUNT];
volatile unsigned char mn1_input_edge[PORT_INPUT_COUNT];

/* chamado pelo toque: botao apertado/solto */
void mn1_input_set(int input, int down)
{
    if ((unsigned)input >= PORT_INPUT_COUNT) return;
    if (down && !mn1_input[input]) mn1_input_edge[input] = 1;
    mn1_input[input] = (unsigned char)(down != 0);
}

void Port_Config_Load(const char* path) { (void)path; }
u8 Port_Config_WindowScale(void) {  return 3; }
const char* Port_Config_UpscaleMethod(void) {  return ""; }
u64 Port_Config_FrameTimeNs(void) {  return 16666667ULL; }
u32 Port_Config_TargetFps(void) {  return 60; }
u64 Port_Config_TickTimeNs(void) {  return 16666667ULL; }
bool Port_Config_GetDecoupleRender(void) {  return false; }
void Port_Config_SetDecoupleRender(bool on) { (void)on; }
bool Port_Config_GetShowFps(void) {  return false; }
void Port_Config_SetShowFps(bool on) { (void)on; }
bool Port_Config_GetVoxelView(void) {  return false; }
void Port_Config_SetVoxelView(bool on) { (void)on; }
bool Port_Config_GetVoxelWallFade(void) {  return false; }
void Port_Config_SetVoxelWallFade(bool on) { (void)on; }
int Port_Config_GetVoxelPitch(void) {  return 0; }
void Port_Config_CycleVoxelPitch(int dir) { (void)dir; }
bool Port_Config_PortSettingsMenuEnabled(void) {  return false; }
void Port_Config_SetWindowScale(u8 scale) { (void)scale; }
void Port_Config_SetUpscaleMethod(const char* method) { (void)method; }
void Port_Config_SetTargetFps(u32 fps) { (void)fps; }
void Port_Config_CycleTargetFps(int direction) { (void)direction; }
u8 Port_Config_InternalScale(void) {  return 1; }
void Port_Config_SetInternalScale(u8 scale) { (void)scale; }
void Port_Config_CycleInternalScale(int direction) { (void)direction; }
PortTouchScheme Port_Config_TouchScheme(void) {  return 0; }
void Port_Config_SetTouchScheme(PortTouchScheme scheme) { (void)scheme; }
void Port_Config_CycleTouchScheme(int direction) { (void)direction; }
float Port_Config_TouchScale(void) {  return 1.0f; }
void Port_Config_SetTouchScale(float scale) { (void)scale; }
float Port_Config_TouchOpacity(void) {  return 0.5f; }
void Port_Config_SetTouchOpacity(float opacity) { (void)opacity; }
bool Port_Config_WidescreenEnabled(void) {  return false; }
void Port_Config_SetWidescreenEnabled(bool enabled) { (void)enabled; }
void Port_Config_ToggleWidescreen(void) {  }
bool Port_Config_GetConsoleParity(void) {  return false; }
void Port_Config_SetConsoleParity(bool on) { (void)on; }
bool Port_Config_GetSkipLauncher(void) {  return false; }
void Port_Config_SetSkipLauncher(bool on) { (void)on; }
void Port_Config_ToggleConsoleParity(void) {  }
PortAspectMode Port_Config_AspectMode(void) {  return 0; }
const char* Port_Config_AspectModeName(PortAspectMode mode) { (void)mode; return ""; }
void Port_Config_SetAspectMode(PortAspectMode mode) { (void)mode; }
void Port_Config_CycleAspectMode(int direction) { (void)direction; }
PortBgFill Port_Config_BgFill(void) {  return 0; }
const char* Port_Config_BgFillName(PortBgFill fill) { (void)fill; return ""; }
void Port_Config_SetBgFill(PortBgFill fill) { (void)fill; }
void Port_Config_CycleBgFill(int direction) { (void)direction; }
void Port_Config_BgFillColor(u8* r, u8* g, u8* b) { (void)r; (void)g; (void)b; }
void Port_Config_SetBgFillColor(u8 r, u8 g, u8 b) { (void)r; (void)g; (void)b; }
PortRenderBackend Port_Config_RenderBackend(void) {  return 0; }
const char* Port_Config_RenderBackendName(PortRenderBackend b) { (void)b; return ""; }
void Port_Config_SetRenderBackend(PortRenderBackend b) { (void)b; }
void Port_Config_CycleRenderBackend(int direction) { (void)direction; }
bool Port_Config_GetTtsEnabled(void) {  return false; }
void Port_Config_SetTtsEnabled(bool on) { (void)on; }
float Port_Config_GetTtsRate(void) {  return 1.0f; }
void Port_Config_SetTtsRate(float v) { (void)v; }
float Port_Config_GetTtsPitch(void) {  return 1.0f; }
void Port_Config_SetTtsPitch(float v) { (void)v; }
float Port_Config_GetTtsVolume(void) {  return 1.0f; }
void Port_Config_SetTtsVolume(float v) { (void)v; }
const char* Port_Config_GetTtsVoice(void) {  return ""; }
void Port_Config_SetTtsVoice(const char* v) { (void)v; }
const char* Port_Config_GetTtsLanguage(void) {  return ""; }
void Port_Config_SetTtsLanguage(const char* v) { (void)v; }
bool Port_Config_GetA11yCues(void) {  return false; }
void Port_Config_SetA11yCues(bool on) { (void)on; }
bool Port_Config_GetA11yFootsteps(void) {  return false; }
void Port_Config_SetA11yFootsteps(bool on) { (void)on; }
bool Port_Config_GetA11yHazards(void) {  return false; }
void Port_Config_SetA11yHazards(bool on) { (void)on; }
bool Port_Config_GetA11yRadar(void) {  return false; }
void Port_Config_SetA11yRadar(bool on) { (void)on; }
bool Port_Config_GetA11yWalls(void) {  return false; }
void Port_Config_SetA11yWalls(bool on) { (void)on; }
bool Port_Config_GetPracticeShowTimer(void) {  return false; }
void Port_Config_SetPracticeShowTimer(bool on) { (void)on; }
bool Port_Config_GetPracticeShowInputs(void) {  return false; }
void Port_Config_SetPracticeShowInputs(bool on) { (void)on; }
bool Port_Config_GetPracticeShowHistory(void) {  return false; }
void Port_Config_SetPracticeShowHistory(bool on) { (void)on; }
float Port_Config_GetPracticeSlowmo(void) {  return 1.0f; }
void Port_Config_SetPracticeSlowmo(float v) { (void)v; }
bool Port_Config_GetDiscordRpc(void) {  return false; }
void Port_Config_SetDiscordRpc(bool on) { (void)on; }
bool Port_Config_GetVSync(void) {  return false; }
void Port_Config_SetVSync(bool on) { (void)on; }
bool Port_Config_GetGpuRaster(void) {  return false; }
void Port_Config_SetGpuRaster(bool on) { (void)on; }
bool Port_Config_GetGpuRasterGles(void) {  return false; }
void Port_Config_SetGpuRasterGles(bool on) { (void)on; }
bool Port_Config_GetColorCorrection(void) {  return false; }
void Port_Config_SetColorCorrection(bool on) { (void)on; }
bool Port_Config_GetLcdPersistence(void) {  return false; }
void Port_Config_SetLcdPersistence(bool on) { (void)on; }
float Port_Config_GetLcdPersistenceRho(void) {  return 0.0f; }
void Port_Config_SetLcdPersistenceRho(float v) { (void)v; }
bool Port_Config_GetRibbonEnabled(void) {  return false; }
void Port_Config_SetRibbonEnabled(bool on) { (void)on; }
bool Port_Config_GetMenuHintSeen(void) {  return false; }
void Port_Config_SetMenuHintSeen(bool seen) { (void)seen; }
bool Port_Config_GetHoldToAdvanceText(void) {  return false; }
void Port_Config_SetHoldToAdvanceText(bool on) { (void)on; }
bool Port_Config_GetHdDialogue(void) {  return false; }
void Port_Config_SetHdDialogue(bool on) { (void)on; }
bool Port_Config_GetMenuArtBackground(void) {  return false; }
void Port_Config_SetMenuArtBackground(bool on) { (void)on; }
bool Port_Config_GetRollAttackMacroEnabled(void) {  return false; }
void Port_Config_SetRollAttackMacroEnabled(bool on) { (void)on; }
float Port_Config_GetMasterVolume(void) {  return 1.0f; }
void Port_Config_SetMasterVolume(float v) { (void)v; }
bool Port_Config_GetFullscreen(void) {  return false; }
void Port_Config_SetFullscreen(bool on) { (void)on; }
bool Port_Config_GetFullscreenHideCursor(void) {  return false; }
void Port_Config_SetFullscreenHideCursor(bool on) { (void)on; }
const char* Port_Config_GetShaderPreset(void) {  return ""; }
void Port_Config_SetShaderPreset(const char* path) { (void)path; }
int Port_Config_HasRebornMask(void) {  return 0; }
unsigned Port_Config_GetRebornMask(void) {  return 0; }
void Port_Config_SetRebornMask(unsigned mask) { (void)mask; }
bool Port_Config_GetRandoEnabled(void) {  return false; }
void Port_Config_SetRandoEnabled(bool on) { (void)on; }
bool Port_Config_GetRandoGlitchless(void) {  return false; }
bool Port_Config_GetRandoObscure(void) {  return false; }
bool Port_Config_GetRandoKinstones(void) {  return false; }
bool Port_Config_GetRandoEntrances(void) {  return false; }
bool Port_Config_GetRandoDojos(void) {  return false; }
bool Port_Config_GetRandoOpenWorld(void) {  return false; }
int Port_Config_GetRandoItemPool(void) {  return 0; }
bool Port_Config_GetRandoHomewarp(void) {  return false; }
bool Port_Config_GetRandoStartSword(void) {  return false; }
bool Port_Config_GetRandoEarlyCrests(void) {  return false; }
bool Port_Config_GetRandoInstantText(void) {  return false; }
int Port_Config_GetRandoTunicColor(void) {  return 0; }
int Port_Config_GetRandoHeartColor(void) {  return 0; }
void Port_Config_SetRandoSettings(bool glitchless, bool obscure, bool kinstones, bool entrances, bool dojos,
                                  bool open_world, int item_pool, bool homewarp, bool start_sword, bool early_crests,
                                  bool instant_text, int tunic_color, int heart_color) { (void)glitchless; (void)obscure; (void)kinstones; (void)entrances; (void)dojos; (void)open_world; (void)item_pool; (void)homewarp; (void)start_sword; (void)early_crests; (void)instant_text; (void)tunic_color; (void)heart_color; }
int Port_Config_GetRandoTricks(void) {  return 0; }
void Port_Config_SetRandoTricks(int tricks) { (void)tricks; }
int Port_Config_GetRandoAccessibility(void) {  return 0; }
void Port_Config_SetRandoAccessibility(int accessibility) { (void)accessibility; }
bool Port_Config_GetRandoDungeonItems(void) {  return false; }
void Port_Config_SetRandoDungeonItems(bool dungeon_items) { (void)dungeon_items; }
bool Port_Config_GetRaEnabled(void) {  return false; }
void Port_Config_SetRaEnabled(bool on) { (void)on; }
bool Port_Config_GetRaNotifications(void) {  return false; }
void Port_Config_SetRaNotifications(bool on) { (void)on; }
const char* Port_Config_GetRaUsername(void) {  return ""; }
void Port_Config_SetRaUsername(const char* v) { (void)v; }
const char* Port_Config_GetRaToken(void) {  return ""; }
void Port_Config_SetRaToken(const char* v) { (void)v; }
void Port_Config_OpenGamepads(void) {  }
void Port_Config_HandleEvent(const SDL_Event* e) { (void)e; }
bool Port_Config_EventIsInputDown(const SDL_Event* e, PortInput input) { (void)input; return false; }
bool Port_Config_InputPressed(PortInput input) {  return (unsigned)input < PORT_INPUT_COUNT && mn1_input[input]; }
void Port_Config_CloseGamepads(void) {  }
bool Port_Config_InputEdgePressed(PortInput input) {  return (unsigned)input < PORT_INPUT_COUNT && mn1_input_edge[input]; }
bool Port_Config_SoftSlotPressed(int slot) { (void)slot; return false; }
bool Port_Config_GetLeftStick(float* outX, float* outY) { (void)outX; (void)outY; return false; }
float Port_Config_GetAnalogDeadzone(void) {  return 0.25f; }
void Port_Config_SetAnalogDeadzone(float v) { (void)v; }
void Port_Config_ClearInputEdges(void) {  memset(mn1_input_edge, 0, sizeof mn1_input_edge); }
void Port_Config_TestForceEdge(PortInput input) {  if ((unsigned)input < PORT_INPUT_COUNT) mn1_input_edge[input] = 1; }
void Port_Config_StampInputEdge(PortInput input) {  if ((unsigned)input < PORT_INPUT_COUNT) mn1_input_edge[input] = 1; }
int Port_Config_PreferredRegion(void) {  return -1; }
void Port_Config_SetPreferredRegion(int region) { (void)region; }
int Port_Config_PreferredLanguage(void) {  return -1; }
void Port_Config_SetPreferredLanguage(int lang) { (void)lang; }
void Port_Config_SetPortSettingsMenuEnabled(bool enabled) { (void)enabled; }
const char* Port_Config_InputUiLabel(PortInput input) { (void)input; return ""; }
void Port_Config_FormatBindingsLine(PortInput input, char* out, size_t outCap) { (void)input; (void)out; (void)outCap; }
void Port_Config_SetKeyboardBindExclusive(PortInput input, int sdl_keycode) { (void)input; (void)sdl_keycode; }
void Port_Config_FormatGamepadBindingsLine(PortInput input, char* out, size_t outCap) { (void)input; (void)out; (void)outCap; }
void Port_Config_SetGamepadBindExclusive(PortInput input, int sdl_gamepad_button) { (void)input; (void)sdl_gamepad_button; }
bool Port_Config_GetDebugFlagNotifications(void) {  return false; }
void Port_Config_SetDebugFlagNotifications(bool on) { (void)on; }
