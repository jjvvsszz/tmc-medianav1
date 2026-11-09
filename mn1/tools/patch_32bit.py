#!/usr/bin/env python3
"""Porta o tmc (Picori) para ponteiros de 32 bits, como a MN1.

O port de PC compensa o crescimento de ponteiros em 64 bits com blocos:
    #ifdef PC_PORT
        u8 unk_68[0x10 + 4];   /* +4 for Enemy::child PC growth */
    #else
        u8 unk_68[0x10];
    #endif
Em 32 bits o ponteiro volta a ter 4 bytes e o layout correto e o do GBA. Este script troca
`#ifdef PC_PORT` por `#if defined(PC_PORT) && __SIZEOF_POINTER__ == 8` SOMENTE quando o bloco e de
layout: tem #else e as duas versoes so declaram campos de struct (nenhum codigo executavel).
Uso: python3 patch_32bit.py <raiz do repo> [arquivos...]   (padrao: todo src/)
"""
import os
import re
import sys

COND = '#if defined(PC_PORT) && __SIZEOF_POINTER__ == 8'
CAMPO = re.compile(r'^\s*(/\*.*?\*/\s*)?(const\s+)?(struct\s+)?[A-Za-z_][A-Za-z0-9_]*(\s*\*)*\s+\**[A-Za-z_][A-Za-z0-9_]*(\s*\[[^\]]+\])*\s*;\s*(/\*.*\*/|//.*)?\s*$')


def sem_comentarios(linhas):
    t = re.sub(r'/\*.*?\*/', '', '\n'.join(linhas), flags=re.S)
    return t.split('\n')


def bloco_de_layout(linhas):
    """True se todas as linhas nao vazias sao declaracoes de campo (comentarios ignorados)."""
    n = 0
    for l in sem_comentarios(linhas):
        s = l.strip()
        if not s or s.startswith('//') or (s.startswith('/*') and s.endswith('*/')):
            continue
        if not CAMPO.match(l):
            return False
        n += 1
    return n > 0


ENCHIMENTO = re.compile(r'(pad|filler|_pc\b|_pc\[|unused)', re.I)
ABRE = re.compile(r'^\s*(/\*[^*]*\*/\s*)?#ifdef PC_PORT\s*$')


def so_enchimento(linhas):
    """Bloco sem #else: so troca se todos os campos tem nome de enchimento."""
    return bloco_de_layout(linhas) and all(ENCHIMENTO.search(l) for l in sem_comentarios(linhas) if l.strip() and not l.strip().startswith('//'))


def patch(path):
    t = open(path, encoding='utf8', errors='surrogateescape').read().split('\n')
    out, i, mudou = [], 0, 0
    while i < len(t):
        m = ABRE.match(t[i])
        if m:
            # acha #else e #endif do mesmo nivel
            dep, j, els, end = 0, i + 1, None, None
            while j < len(t):
                s = t[j].strip()
                if s.startswith('#if'):
                    dep += 1
                elif s.startswith('#endif'):
                    if dep == 0:
                        end = j
                        break
                    dep -= 1
                elif s.startswith('#else') and dep == 0:
                    els = j
                j += 1
            ok = (els and end and bloco_de_layout(t[i + 1:els]) and bloco_de_layout(t[els + 1:end])) or \
                 (els is None and end and so_enchimento(t[i + 1:end]))
            if ok:
                if m.group(1):                       # comentario antes do #ifdef: preserva em linha propria
                    out.append(m.group(1).rstrip())
                out.append(COND)
                mudou += 1
                i += 1
                continue
        out.append(t[i])
        i += 1
    if mudou:
        open(path, 'w', encoding='utf8', errors='surrogateescape', newline='\n').write('\n'.join(out))
    return mudou


def main():
    root = sys.argv[1]
    files = sys.argv[2:] or [os.path.join(dp, f) for d in ('src', 'include') for dp, _, fs in os.walk(os.path.join(root, d)) for f in fs if f.endswith(('.c', '.h'))]
    total = arq = 0
    for f in files:
        n = patch(f if os.path.isabs(f) else os.path.join(root, f))
        if n:
            total += n
            arq += 1
    print('blocos de layout trocados: %d em %d arquivos' % (total, arq))


if __name__ == '__main__':
    main()
