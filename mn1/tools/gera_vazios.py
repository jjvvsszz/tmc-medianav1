#!/usr/bin/env python3
"""Gera mn1/mn1_vazios.c: uma definicao vazia para cada simbolo que ainda falta na ligacao,
exceto os implementados de verdade em mn1/*.c.

Funcao x variavel: decidido pelas realocacoes que referenciam o simbolo nos objetos (R_MIPS_26 =
chamada => funcao). Funcoes vazias devolvem 0 em v0 e v1 (long long), o que serve para int,
ponteiro, bool, u64 e double 0.0 na ABI o32. Variaveis viram um bloco zerado de 64 bytes.

Uso: python gera_vazios.py faltando.txt relocs.txt implementados.txt saida.c
"""
import re
import sys

falta, relocs, impl, saida = sys.argv[1:5]
nomes = [l.strip() for l in open(falta) if l.strip()]
feitos = set(l.strip() for l in open(impl) if l.strip() and not l.startswith('#'))
tipos = {}
for l in open(relocs, errors='ignore'):
    m = re.search(r'\bR_MIPS_(\w+)\s+[0-9a-f]+\s+(\S+)', l)
    if m:
        tipos.setdefault(m.group(2), set()).add(m.group(1))
fun = var = 0
out = ['/* Gerado por gera_vazios.py: definicoes vazias para o que o port usa e a MN1 nao precisa.',
       '   Funcoes devolvem 0 (v0=v1=0); variaveis sao blocos zerados. NAO EDITAR A MAO. */', '']
for n in nomes:
    if n in feitos or n.startswith('__') or n in ('main',):
        continue
    t = tipos.get(n, set())
    if '26' in t or not t:
        out.append('long long %s(void) { return 0; }' % n)
        fun += 1
    else:
        out.append('unsigned char %s[64] __attribute__((aligned(8)));' % n)
        var += 1
open(saida, 'w', newline='\n').write('\n'.join(out) + '\n')
print('vazios: %d funcoes, %d variaveis -> %s' % (fun, var, saida))
