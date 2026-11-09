#!/usr/bin/env python3
"""Correcoes do port para a MN1 (MIPS32 little-endian, Windows CE, sem TLS, sem SDL).

Marca TMC_MN1: segue o alvo N64 onde ele trata de 32 bits/bare-metal, mas NAO onde trata de
big-endian (a MN1 e little-endian como o PC).

Uso: python mn1_patch.py <repo> <saida>   copia port/ppu para <saida>/ppu com as correcoes.
"""
import os
import shutil
import sys

repo, out = sys.argv[1], sys.argv[2]
dst = os.path.join(out, 'ppu')
shutil.rmtree(dst, ignore_errors=True)
shutil.copytree(os.path.join(repo, 'port', 'ppu'), dst)


def troca(rel, antes, depois, vezes=1):
    p = os.path.join(dst, rel)
    t = open(p, encoding='utf8').read()
    n = t.count(antes)
    if n != vezes:
        sys.exit('ERRO %s: esperava %d ocorrencia(s), achei %d de %r' % (rel, vezes, n, antes[:60]))
    open(p, 'w', encoding='utf8', newline='\n').write(t.replace(antes, depois))


N64_OU_MN1 = '#if defined(TMC_N64) || defined(TMC_MN1)'
# 1. Sem TLS: __thread gera RDHWR, que nem o VR4300 nem o MIPS da MN1 (auditoria MIPS II) aceitam.
troca('src/mode1.c', '#ifdef TMC_N64\n/* N64: single-threaded (no OpenMP) and bare-metal (no TLS).',
      N64_OU_MN1 + '\n/* N64: single-threaded (no OpenMP) and bare-metal (no TLS).')
# 2. Quadro nativo 240x160 e VRAM do tamanho do GBA (economiza 5,6 MB de RAM).
troca('include/virtuappu.h', '#ifdef TMC_N64\n    /* N64: only the native 240x160 frame is needed;',
      N64_OU_MN1 + '\n    /* N64: only the native 240x160 frame is needed;')
print('ppu corrigido em', dst)
