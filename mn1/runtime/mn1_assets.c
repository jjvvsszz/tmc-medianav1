/* Carregador de assets da MN1: modo "so ROM".
   No PC, port_asset_loader.cpp (C++, JSON) pode trocar textos, sprites, paletas e areas pelos
   arquivos extraidos em assets/. Quando nao acha, o proprio port cai na ROM, que ele trata como
   a fonte com autoridade (e o caminho usado para ROMs JP). Na MN1 so existe esse caminho:
   todas as funcoes "FromAssets" dizem que nao ha asset, e as consultas de ponteiro respondem
   pela ROM, como o .cpp faz no fim de cada funcao. */
#include "global.h"
#include "port_gba_mem.h"
#include "port_rom.h"
#include <stdio.h>

static bool32 rom_ptr(const void *p, u32 size)
{
    uintptr_t a = (uintptr_t)p, s = (uintptr_t)gRomData;
    return p && gRomData && gRomSize >= size && a >= s && a <= s + gRomSize - size;
}

/* espelha o fim de Port_GetSpriteAnimationData do .cpp (o caminho da ROM) */
const u8 *Port_GetSpriteAnimationData(u16 spriteIndex, u32 animIndex)
{
    const SpritePtr *spr = Port_GetSpritePtr(spriteIndex);
    const u8 *tab;
    u32 addr;
    if (!spr || !spr->animations) return NULL;
    tab = (const u8 *)spr->animations;
    if (!rom_ptr(tab, (animIndex + 1) * 4)) return NULL;
    addr = Port_ReadU32(tab + animIndex * 4);
    return addr ? (const u8 *)Port_ResolveRomData(addr) : NULL;
}

bool32 Port_IsRoomHeaderPtrReadable(const void *p) { return rom_ptr(p, 10); }
bool32 Port_IsAreaTablePtrFromAssets(u32 area, const void *p) { (void)area; (void)p; return FALSE; }
bool32 Port_IsLoadedAssetBytes(const void *p, u32 n) { (void)p; (void)n; return FALSE; }
const u8 *Port_LoadedAssetBytesEnd(const void *p) { (void)p; return NULL; }
const u8 *Port_GetMapAssetDataByIndex(u32 i, u32 *size) { (void)i; if (size) *size = 0; return NULL; }
bool32 Port_AreSpritePtrsLoadedFromAssets(void) { return FALSE; }
bool32 Port_RefreshAreaDataFromAssets(u32 area) { (void)area; return FALSE; }
bool32 Port_LoadTextsFromAssets(void) { return FALSE; }
bool32 Port_LoadSpritePtrsFromAssets(void) { return FALSE; }
bool32 Port_LoadAreaTablesFromAssets(void) { return FALSE; }
bool32 Port_LoadGfxGroupFromAssets(u32 g) { (void)g; return FALSE; }
bool32 Port_LoadPaletteGroupFromAssets(u32 g) { (void)g; return FALSE; }
void Port_EnsureAssetsReadyWithDisplay(void *w, const u8 *rom, u32 n) { (void)w; (void)rom; (void)n; }
void Port_PaintBootSplash(void *w, const char *m) { (void)w; fprintf(stderr, "splash: %s\n", m ? m : ""); }
void Port_LogAssetLoaderStatus(void) { fprintf(stderr, "assets: modo so ROM (MN1)\n"); }
