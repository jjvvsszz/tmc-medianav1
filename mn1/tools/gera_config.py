#!/usr/bin/env python3
"""Gera mn1/mn1_config.c: as ~180 funcoes de port_runtime_config.h com valores fixos da MN1.

O port de PC le um JSON de configuracao (C++/nlohmann). Na MN1 os valores sao fixos:
60 Hz, GBA nativo 240x160, extras desligados. Entrada: vetor de botoes preenchido pelo toque
(mn1_ppu.c), lido por Port_Config_InputPressed / InputEdgePressed.

Uso: python gera_config.py repo/port/port_runtime_config.h saida.c
"""
import re
import sys

hdr, saida = sys.argv[1], sys.argv[2]
t = open(hdr, encoding='utf8').read()
t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
t = re.sub(r'//[^\n]*', '', t)
decls = re.findall(r'^\s*([A-Za-z_][\w\s\*]*?\b(Port_Config_\w+))\s*\(([^)]*)\)\s*;', t, flags=re.M)

ESPECIAIS = {
    'Port_Config_FrameTimeNs': 'return 16666667ULL;',
    'Port_Config_TickTimeNs': 'return 16666667ULL;',
    'Port_Config_TargetFps': 'return 60;',
    'Port_Config_WindowScale': 'return 3;',
    'Port_Config_InternalScale': 'return 1;',
    'Port_Config_GetMasterVolume': 'return 1.0f;',
    'Port_Config_TouchScale': 'return 1.0f;',
    'Port_Config_TouchOpacity': 'return 0.5f;',
    'Port_Config_GetAnalogDeadzone': 'return 0.25f;',
    'Port_Config_GetTtsRate': 'return 1.0f;',
    'Port_Config_GetTtsPitch': 'return 1.0f;',
    'Port_Config_GetTtsVolume': 'return 1.0f;',
    'Port_Config_GetPracticeSlowmo': 'return 1.0f;',
    'Port_Config_GetLcdPersistenceRho': 'return 0.0f;',
    'Port_Config_PreferredRegion': 'return -1;',
    'Port_Config_PreferredLanguage': 'return -1;',
    'Port_Config_InputPressed': 'return (unsigned)input < PORT_INPUT_COUNT && mn1_input[input];',
    'Port_Config_InputEdgePressed': 'return (unsigned)input < PORT_INPUT_COUNT && mn1_input_edge[input];',
    'Port_Config_ClearInputEdges': 'memset(mn1_input_edge, 0, sizeof mn1_input_edge);',
    'Port_Config_StampInputEdge': 'if ((unsigned)input < PORT_INPUT_COUNT) mn1_input_edge[input] = 1;',
    'Port_Config_TestForceEdge': 'if ((unsigned)input < PORT_INPUT_COUNT) mn1_input_edge[input] = 1;',
    'Port_Config_InputUiLabel': 'return "";',
    'Port_Config_GetSkipLauncher': 'return true;',
}

out = ['/* Gerado por gera_config.py: configuracao fixa da MN1. NAO EDITAR A MAO. */',
       '#include <string.h>', '#include "port_runtime_config.h"', '',
       'volatile unsigned char mn1_input[PORT_INPUT_COUNT];',
       'volatile unsigned char mn1_input_edge[PORT_INPUT_COUNT];', '',
       '/* chamado pelo toque: botao apertado/solto */',
       'void mn1_input_set(int input, int down)',
       '{',
       '    if ((unsigned)input >= PORT_INPUT_COUNT) return;',
       '    if (down && !mn1_input[input]) mn1_input_edge[input] = 1;',
       '    mn1_input[input] = (unsigned char)(down != 0);',
       '}', '']
vistos = set()
for ret, nome, args in decls:
    if nome in vistos:
        continue
    vistos.add(nome)
    ret = ' '.join(ret.replace(nome, '').split())
    args_c = args.strip() or 'void'
    if nome in ESPECIAIS:
        corpo = ESPECIAIS[nome]
    elif ret == 'void':
        corpo = ''
    elif ret.startswith('const char'):
        corpo = 'return "";'
    elif ret in ('float', 'double'):
        corpo = 'return 0.0f;'
    elif ret == 'bool':
        corpo = 'return false;'
    else:
        corpo = 'return 0;'
    params = [p.strip() for p in args_c.split(',')] if args_c != 'void' else []
    usados = ' '.join('(void)%s;' % p.split()[-1].lstrip('*') for p in params if p and p != '...' and re.match(r'.*\b\w+$', p) and p.split()[-1].lstrip('*') not in corpo)
    out.append('%s %s(%s) { %s %s}' % (ret, nome, args_c, usados, (corpo + ' ') if corpo else ''))
open(saida, 'w', encoding='utf8', newline='\n').write('\n'.join(out) + '\n')
print('config: %d funcoes -> %s' % (len(vistos), saida))
