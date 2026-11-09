#!/bin/bash
# Diagnostico: avisos de deslocamento maior que o tipo e comparacao sempre verdadeira/falsa,
# que denunciam suposicoes de ponteiro de 64 bits no port. Uso (WSL): bash avisos_32bit.sh <lista> <saida.txt>
TC=$HOME/mn1tc/root
export PATH=$TC/usr/bin:$PATH
export LD_LIBRARY_PATH=$TC/usr/lib/x86_64-linux-gnu
BASE=$(cd "$(dirname "$0")" && pwd)
REPO=${REPO_OVERRIDE:-$BASE/repo}
CC="mipsel-linux-gnu-gcc-13 -B$TC/usr/lib/gcc-cross/mipsel-linux-gnu/13 -B$TC/usr/mipsel-linux-gnu/bin"
INC="-isystem $TC/usr/mipsel-linux-gnu/include -I$BASE/mn1 -I$REPO/include -I$REPO/libs -I$REPO/port -I$REPO -I$REPO/build/USA -I$REPO/port/ppu/include"
DEF="-DPC_PORT -DNON_MATCHING -DUSE_HDMA -DUSA -DENGLISH -DREVISION=0 -DMODE1_GBA_WIDTH=240 -include region.h -DTMC_MN1"
CF="-march=mips2 -mabi=32 -EL -msoft-float -mno-abicalls -fno-pic -G0 -O2 -std=gnu11 -funsigned-char -fsyntax-only -Wno-everything -Wshift-count-overflow -Wtype-limits"
cd "$REPO"
xargs -a "$BASE/$1" -P 8 -I{} sh -c "$CC $CF $DEF $INC -c {} 2>&1 | grep -E 'warning: (right|left) shift count|warning: comparison is always' || true" > "$2"
wc -l < "$2"
