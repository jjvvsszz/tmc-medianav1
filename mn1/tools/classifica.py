#!/usr/bin/env python3
"""Classifica os simbolos sem dono da ligacao: SDL, libc, libgcc, C++ do port (e em qual arquivo), resto."""
import os
import re
import sys
from collections import defaultdict

repo, lista = sys.argv[1], sys.argv[2]
nomes = [l.strip() for l in open(lista) if l.strip()]
LIBC = set('''memcpy memmove memset memcmp memchr strlen strnlen strcmp strncmp strcpy strncpy strchr strrchr strstr strcat
strncat strdup strtok strtol strtoul strtoll strtoull strtod strtof atoi atol atof sprintf snprintf vsnprintf vsprintf printf
fprintf vfprintf puts fputs fputc putchar fopen fclose fread fwrite fseek ftell fflush fgets fgetc feof ferror fileno
setvbuf perror remove rename malloc calloc realloc free abort exit atexit getenv qsort bsearch abs labs rand srand time
clock localtime gmtime strftime mktime difftime tolower toupper isdigit isspace isalpha isalnum isupper islower isprint
isxdigit ispunct sscanf __errno_location errno strerror stderr stdout stdin __assert_fail sqrt sqrtf floor floorf ceil
ceilf fabs fabsf pow powf sin sinf cos cosf atan2 atan2f fmod fmodf roundf round lroundf exp expf log logf
__ctype_b_loc __ctype_tolower_loc __ctype_toupper_loc'''.split())
libgcc = re.compile(r'^__(add|sub|mul|div|neg|cmp|unord|eq|ne|lt|le|gt|ge|fix|fixuns|float|floatun|extend|trunc|ashl|ashr|lshr|udiv|umod|mod|pow|bswap|clz|ctz|popcount|ffs)[a-z]*(sf|df|di|si|ti)[0-9]?$')
# definicoes em C++ do port
cpp_def = {}
for dp, _, fs in os.walk(os.path.join(repo, 'port')):
    for f in fs:
        if f.endswith(('.cpp', '.cc', '.hpp')):
            p = os.path.join(dp, f)
            t = open(p, encoding='utf8', errors='ignore').read()
            for m in re.finditer(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{]*\)\s*(const\s*)?\{', t):
                cpp_def.setdefault(m.group(1), os.path.relpath(p, repo).replace('\\', '/'))
for dp, _, fs in os.walk(os.path.join(repo, 'libs')):
    for f in fs:
        if f.endswith(('.cpp', '.cc')):
            p = os.path.join(dp, f)
            t = open(p, encoding='utf8', errors='ignore').read()
            for m in re.finditer(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{]*\)\s*(const\s*)?\{', t):
                cpp_def.setdefault(m.group(1), os.path.relpath(p, repo).replace('\\', '/'))
g = defaultdict(list)
for n in nomes:
    if n.startswith('SDL_'):
        g['SDL (implementar em mn1/)'].append(n)
    elif n in LIBC:
        g['libc'].append(n)
    elif libgcc.match(n):
        g['libgcc (float/64 bits)'].append(n)
    elif n in cpp_def:
        g['C++: ' + cpp_def[n]].append(n)
    else:
        g['outros'].append(n)
for k, v in sorted(g.items(), key=lambda kv: -len(kv[1])):
    print('%-52s %3d  %s' % (k, len(v), ' '.join(v[:9]) + (' ...' if len(v) > 9 else '')))
