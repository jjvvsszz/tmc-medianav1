#!/bin/bash
# Lista simbolos usados e nao definidos pelos objetos MIPS do jogo, e o tamanho do codigo.
# Uso (WSL): bash simbolos.sh <pasta de objetos> <saida.txt>
set -u
OBJ=$1
OUT=$2
TC=$HOME/mn1tc/root
export PATH=$TC/usr/bin:$PATH
export LD_LIBRARY_PATH=$TC/usr/lib/x86_64-linux-gnu
NM=mipsel-linux-gnu-nm
SZ=mipsel-linux-gnu-size
$NM -g --defined-only "$OBJ"/*.o 2>/dev/null | awk 'NF==3{print $3}' | sort -u > /tmp/def.$$
$NM -u "$OBJ"/*.o 2>/dev/null | awk 'NF==2 && $1=="U"{print $2}' | sort -u > /tmp/und.$$
comm -23 /tmp/und.$$ /tmp/def.$$ > "$OUT"
echo "simbolos definidos: $(wc -l < /tmp/def.$$)"
echo "simbolos sem dono:  $(wc -l < "$OUT")"
$SZ -t "$OBJ"/*.o | tail -1
rm -f /tmp/def.$$ /tmp/und.$$
