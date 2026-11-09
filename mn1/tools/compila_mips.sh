#!/bin/bash
# Etapa 1: compila os .c do port do Minish Cap para MIPS II (MN1), so para medir o que falta.
# Uso (WSL): bash compila_mips.sh src_files.txt saida
set -u
LIST=$1
OUT=$2
TC=$HOME/mn1tc/root
export PATH=$TC/usr/bin:$PATH
export LD_LIBRARY_PATH=$TC/usr/lib/x86_64-linux-gnu
BASE=$(cd "$(dirname "$0")" && pwd)
REPO=${REPO_OVERRIDE:-$BASE/repo}
CC="mipsel-linux-gnu-gcc-13 -B$TC/usr/lib/gcc-cross/mipsel-linux-gnu/13 -B$TC/usr/mipsel-linux-gnu/bin"
INC="-isystem $TC/usr/mipsel-linux-gnu/include -I$BASE/mn1 -I$REPO/include -I$REPO/libs -I$REPO/port -I$REPO -I$REPO/build/USA \
 -I$REPO/port/ppu/include -I$REPO/libs/VirtuaAPU/include -I$REPO/libs/agbplay_core -I$REPO/tools/src/assets_extractor"
DEF="-DTMC_PC_VERSION=\"0.9.6-mn1\" -DTMC_PORT_VERSION=\"0.9.6-mn1\" -DPC_PORT -DNON_MATCHING -DUSE_HDMA -DUSA -DENGLISH -DREVISION=0 -DMODE1_GBA_WIDTH=240 -include region.h -DTMC_MN1"
CF="-march=mips2 -mtune=mips32 -mabi=32 -EL -msoft-float -mno-abicalls -fno-pic -G0 -mno-gpopt -mno-llsc \
 -mno-branch-likely -O2 -std=gnu11 -fno-stack-protector -fno-asynchronous-unwind-tables -fno-common -fno-strict-aliasing -fwrapv -fno-strict-overflow -funsigned-char -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -w"
rm -rf "$OUT"; mkdir -p "$OUT/obj" "$OUT/err"
cd "$REPO"
compile() {
  f=$1
  o="$OUT/obj/$(echo "$f" | tr '/' '_').o"
  if ! $CC $CF $DEF $INC -c "$f" -o "$o" 2> "$OUT/err/$(echo "$f" | tr '/' '_').txt"; then
    echo "FALHA $f"
  else
    rm -f "$OUT/err/$(echo "$f" | tr '/' '_').txt"
    echo "OK $f"
  fi
}
export -f compile
export CC CF DEF INC OUT
xargs -a "$BASE/$LIST" -P 8 -I{} bash -c 'compile "$@"' _ {} > "$OUT/resultado.txt"
grep -c '^OK' "$OUT/resultado.txt" | sed 's/^/compilados: /'
grep -c '^FALHA' "$OUT/resultado.txt" | sed 's/^/falharam: /'
