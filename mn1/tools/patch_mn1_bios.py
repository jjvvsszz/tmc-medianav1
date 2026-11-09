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
