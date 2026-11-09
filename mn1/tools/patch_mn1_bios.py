#!/usr/bin/env python3
"""Ajuste de ritmo do port para a MN1 (so com TMC_MN1).

O port desiste de recuperar ticks atrasados quando passa de 1 tick (16,7 ms) de atraso.
Na MN1 cada desenho custa ~46 ms, entao todo desenho atrasaria o jogo de vez (camera lenta).
Com TMC_MN1 o limite vira 8 ticks: depois de um desenho, os ticks atrasados rodam em seguida
sem desenhar, e o jogo fica na velocidade certa enquanto a logica couber no tempo.

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
