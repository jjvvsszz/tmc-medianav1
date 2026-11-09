#!/usr/bin/env python3
"""Ajuste de ritmo do port para a MN1 (so com TMC_MN1).

O port desiste de recuperar ticks atrasados quando passa de 1 tick (16,7 ms) de atraso.
Na MN1 cada desenho custa ~46 ms, entao todo desenho atrasaria o jogo de vez (camera lenta).
Com TMC_MN1 o limite vira 8 ticks: depois de um desenho, os ticks atrasados rodam em seguida
sem desenhar, e o jogo fica na velocidade certa enquanto a logica couber no tempo.

Tambem: com TMC_MN1 o port nao grava as paginas da ROM em rom_data/ (11 MB a cada inicio,
lento no pendrive e desnecessario: a ROM inteira ja esta na memoria).

Uso: python patch_mn1_bios.py <repo>
"""
import sys
p = sys.argv[1] + '/port/port_bios.c'
t = open(p, encoding='utf8').read()
velho = '            if (now > deadline + tickPeriodNs) {'
novo = ('#ifdef TMC_MN1\n            if (now > deadline + 8 * tickPeriodNs) {\n#else\n'
        '            if (now > deadline + tickPeriodNs) {\n#endif')
if novo in t:
    print('ja aplicado')
else:
    assert t.count(velho) == 1, 'trecho nao encontrado'
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
    print('aplicado')

p = sys.argv[1] + '/port/port_rom.c'
t = open(p, encoding='utf8').read()
velho = 'static void ExtractPage(u32 page) {\n'
novo = velho + '#ifdef TMC_MN1\n    (void)page;\n    return;\n#endif\n'
if novo in t:
    print('rom_data: ja aplicado')
else:
    assert t.count(velho) == 1, 'ExtractPage nao encontrado'
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
    print('rom_data: aplicado')

# Font em 32 bits: o detector olha os bytes 4-7 da struct. Em 64 bits e a metade alta do ponteiro
# dest (nunca um endereco do GBA); em 32 bits e o campo gfx_dest, que costuma guardar 0x06xxxxxx.
# Uma Font montada em tempo de execucao (CreateDialogBox) era tomada por blob do GBA, o dest nativo
# virava NULL e ShowTextBox escrevia no endereco 0 (travamento ao criar o save na MN1, 08/11/2026).
p = sys.argv[1] + '/port/port_rom.h'
t = open(p, encoding='utf8').read()
velho = ('    const u8* raw = (const u8*)data;\n'
         '    u32 word1 = raw[4] | (raw[5] << 8) | (raw[6] << 16) | (raw[7] << 24);\n'
         '    return (word1 >= 0x02000000u && word1 < 0x08000000u);\n')
novo = ('    const u8* raw = (const u8*)data;\n'
        '#if __SIZEOF_POINTER__ == 4\n'
        '    /* 32 bits: layouts iguais; blob do GBA tem dest no formato do GBA, Font nativa nao */\n'
        '    u32 word0 = raw[0] | (raw[1] << 8) | (raw[2] << 16) | ((u32)raw[3] << 24);\n'
        '    return (word0 >= 0x02000000u && word0 < 0x08000000u);\n'
        '#else\n'
        '    u32 word1 = raw[4] | (raw[5] << 8) | (raw[6] << 16) | (raw[7] << 24);\n'
        '    return (word1 >= 0x02000000u && word1 < 0x08000000u);\n'
        '#endif\n')
if novo in t:
    print('font: ja aplicado')
else:
    assert t.count(velho) == 1, 'Port_IsFontGBAEncoded nao encontrado'
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
    print('font: aplicado')

# MapLayer (gMapTop/gMapBottom): Port_ResolveEwramPtr soma 4 fixo aos campos depois de bgSettings
# ("8 bytes no PC"). Em 32 bits o ponteiro tem 4 bytes e nao ha deslocamento: com o +4 os subTiles
# e tileTypes das salas caiam 4 bytes adiante e parte dos metatiles saia preta/errada.
p = sys.argv[1] + '/port/port_rom.c'
t = open(p, encoding='utf8').read()
velho = 'u32 pc_off = (gba_off < 4) ? gba_off : gba_off + 4;'
novo = 'u32 pc_off = (gba_off < 4) ? gba_off : gba_off + (u32)(sizeof(void*) - 4);'
if t.count(novo) == 2:
    print('maplayer: ja aplicado')
else:
    assert t.count(velho) == 2, 'pc_off de MapLayer nao encontrado'
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
    print('maplayer: aplicado')

# Validacao de ponteiro "host": fora do Windows o port so aceita ponteiros entre 4 GB e 2^48
# (Linux/macOS de 64 bits). Em 32 bits todo ponteiro fica abaixo de 4 GB: IsColliding rejeitaria
# todas as colisoes e Port_IsValidHostPtr diria que nenhum ponteiro e valido. Na MN1 vale o
# criterio do Windows (acima de 64 KB), que funciona com ponteiros de 4 bytes.
for arq, velho in (('/src/collision.c', '            uintptr_t pb = (uintptr_t)bb_that;\n#if defined(_WIN32)\n'),
                   ('/port/port_rom.h', '    uintptr_t a = (uintptr_t)p;\n#if defined(_WIN32)\n')):
    p = sys.argv[1] + arq
    t = open(p, encoding='utf8').read()
    novo = velho.replace('#if defined(_WIN32)\n', '#if defined(_WIN32) || defined(TMC_MN1)\n')
    if novo in t:
        print('ponteiro %s: ja aplicado' % arq)
    else:
        assert t.count(velho) == 1, 'validacao de ponteiro nao encontrada em ' + arq
        open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
        print('ponteiro %s: aplicado' % arq)

# GetNextScriptCommandHalfword: (addr >> 48) com uintptr_t de 32 bits e deslocamento maior que o
# tipo (comportamento indefinido). Faz o deslocamento em 64 bits.
p = sys.argv[1] + '/port/port_stubs.c'
t = open(p, encoding='utf8').read()
velho = 'if ((addr >> 48) != 0) {'
novo = 'if (((unsigned long long)addr >> 48) != 0) {'
if novo in t:
    print('script >>48: ja aplicado')
else:
    assert t.count(velho) == 1, 'addr >> 48 nao encontrado'
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(velho, novo))
    print('script >>48: aplicado')
