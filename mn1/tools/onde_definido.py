#!/usr/bin/env python3
"""Para cada simbolo sem dono, acha em que arquivo do repo ele e definido (funcao ou variavel global)."""
import os
import re
import sys
from collections import Counter, defaultdict

repo, lista = sys.argv[1], sys.argv[2]
falhou = set(sys.argv[3].split(',')) if len(sys.argv) > 3 else set()
nomes = [l.strip() for l in open(lista) if l.strip()]
alvo = set(nomes)
achado = {}
tok = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*(\(|\[|=|;)')
for dp, _, fs in os.walk(repo):
    if '/.git' in dp.replace('\\', '/'):
        continue
    for f in fs:
        if not f.endswith(('.c', '.cpp', '.cc', '.s', '.inc', '.h')):
            continue
        p = os.path.join(dp, f)
        rel = os.path.relpath(p, repo).replace('\\', '/')
        try:
            t = open(p, encoding='utf8', errors='ignore').read()
        except OSError:
            continue
        for line in t.split('\n'):
            if not line or line[0] in ' \t#/*}' or line.startswith('extern') or line.rstrip().endswith(';') and '(' in line:
                continue
            for m in tok.finditer(line):
                n = m.group(1)
                if n in alvo and n not in achado:
                    achado[n] = rel
                    break
grupo = defaultdict(list)
for n in nomes:
    f = achado.get(n)
    if f is None:
        g = 'NAO ACHADO'
    elif f.startswith('src/') and f in falhou:
        g = 'src (arquivo que ainda nao compila)'
    elif f.startswith('src/'):
        g = 'src (outro)'
    elif f.startswith('port/ppu'):
        g = 'port/ppu (C)'
    elif f.startswith('port/') and f.endswith('.c'):
        g = 'port C'
    elif f.startswith('port/'):
        g = 'port C++'
    elif f.startswith('asm/') or f.endswith('.s') or f.endswith('.inc'):
        g = 'asm/dados'
    elif f.startswith('libs/'):
        g = 'libs'
    elif f.startswith(('data/', 'build/')):
        g = 'data'
    else:
        g = 'outro: ' + f.split('/')[0]
    grupo[g].append((n, f))
for g, v in sorted(grupo.items(), key=lambda kv: -len(kv[1])):
    arqs = Counter(f for _, f in v if f)
    print('%-38s %4d   %s' % (g, len(v), ', '.join('%s(%d)' % (a, c) for a, c in arqs.most_common(6)) or ' '.join(n for n, _ in v[:12])))
