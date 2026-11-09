/* Randomizador do port: desligado na MN1 (rando.cpp e C++).
   Rando_GetSettings/Rando_DefaultSettings devolvem struct por valor: na ABI o32 quem chama passa
   um ponteiro escondido e espera a struct preenchida. Uma funcao vazia deixava lixo da pilha como
   configuracao (suspeita do travamento ao criar o save na MN1, 08/11/2026). Aqui: os padroes do
   rando.cpp, sem randomizador ativo (Rando_IsActive continua 0). */
#include "global.h"
#include "rando/rando.h"

RandomizerSettings Rando_DefaultSettings(void)
{
    RandomizerSettings s;
    memset(&s, 0, sizeof s);
    s.glitchless_logic = true;
    s.obscure_locations = false;
    s.shuffle_kinstones = true;
    s.shuffle_entrances = false;
    s.shuffle_dojos = true;
    s.open_world = false;
    s.item_difficulty = RANDO_ITEM_POOL_NORMAL;
    s.tricks = 0u;
    s.accessibility = RANDO_ACCESS_GOAL;
    s.shuffle_dungeon_items = false;
    s.homewarp = true;
    s.start_sword = true;
    s.early_crests = true;
    s.instant_text = true;
    s.tunic_color = 0;
    s.heart_color = 0;
    return s;
}

RandomizerSettings Rando_GetSettings(void) { return Rando_DefaultSettings(); }
