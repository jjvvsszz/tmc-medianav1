#!/usr/bin/env python3
"""Liga o tmc.exe (Minish Cap para a MN1).

Pre-requisito: objetos do jogo e do port ja compilados em ~/tmc_build/obj (WSL) por
  REPO_OVERRIDE=~/tmc32 bash compila_mips.sh todos_mn1.txt ~/tmc_build
Passos:
  1. compila a camada da MN1 (mn1/*.c, mn1_setjmp.S) e rt.c/ui.c do MN1_Probe;
  2. lista o que continua sem dono e gera mn1/mn1_vazios.c (funcoes que devolvem 0);
  3. liga com o pe.ld do MN1_Probe e converte para PE do Windows CE (elf2pe, pilha de 1 MB);
  4. confere: importacoes estaticas so comprovadas no aparelho; codigo so MIPS II.
Saida: dist/tmc.exe
"""
import os
import re
import subprocess
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
PROBE = os.path.join(os.path.dirname(AQUI), 'TTCMD from USB for MN1', 'MN1_Probe')
sys.path.insert(0, os.path.join(PROBE, 'tools'))
import build  # noqa: E402
import elf2pe  # noqa: E402

OBJ = '$HOME/tmc_build/obj'
RT = '$HOME/tmc_build/rt'
GEN = os.path.join(AQUI, 'mn1', 'gerado')
DIST = os.path.join(AQUI, 'dist')
w = build.wslpath

TC = '$HOME/mn1tc/root'
CC = 'mipsel-linux-gnu-gcc-13 -B%s/usr/lib/gcc-cross/mipsel-linux-gnu/13 -B%s/usr/mipsel-linux-gnu/bin' % (TC, TC)
REPO = '$HOME/tmc32'
INC_JOGO = ('-isystem %s/usr/mipsel-linux-gnu/include -I%s -I%s/include -I%s/port -I%s -I%s/build/USA -I%s/port/ppu/include'
            % (TC, w(os.path.join(AQUI, 'mn1')), REPO, REPO, REPO, REPO, REPO))
DEF_JOGO = '-DPC_PORT -DNON_MATCHING -DUSE_HDMA -DUSA -DENGLISH -DREVISION=0 -DMODE1_GBA_WIDTH=240 -include region.h -DTMC_MN1'
CF_JOGO = ('-march=mips2 -mtune=mips32 -mabi=32 -EL -msoft-float -mno-abicalls -fno-pic -G0 -mno-gpopt -mno-llsc '
           '-mno-branch-likely -O2 -std=gnu11 -fno-stack-protector -fno-asynchronous-unwind-tables -fno-common '
           '-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -w')
# camada Win32 da MN1: mesmas flags do MN1_Probe (wchar de 16 bits, sem libc do Linux)
INC_MN1 = '-I"%s" -I"%s" -I%s/port/ppu/include' % (w(os.path.join(PROBE, 'src')), w(os.path.join(PROBE, 'build_gen')), REPO)

MN1_WIN = ['mn1/mn1_crt.c', 'mn1/mn1_float.c', 'mn1/mn1_ppu.c', 'mn1/mn1_entry.c']
MN1_JOGO = ['mn1/mn1_config.c', 'mn1/mn1_sdl.c', 'mn1/mn1_assets.c', 'mn1/mn1_rando.c']
PROBE_SRC = ['rt.c', 'ui.c']


def wsl(cmds, ok=True):
    pre = ['set -e', 'export PATH=%s/usr/bin:$PATH' % TC,
           'export LD_LIBRARY_PATH=%s/usr/x86_64-linux-gnu/mipsel-linux-gnu/lib:%s/usr/lib/x86_64-linux-gnu' % (TC, TC)]
    r = subprocess.run(['wsl', '-e', 'bash', '-lc', '\n'.join(pre + cmds)], capture_output=True, text=True, errors='replace')
    if r.returncode and ok:
        sys.stdout.write(r.stdout[-4000:])
        sys.stderr.write(r.stderr[-6000:])
        sys.exit('falhou')
    return r.stdout


def imports():
    """Mesma regra do MN1_Probe: so ordinais/nomes que MortScript.exe ou ttcmd.exe ja importam."""
    import pefile
    pkg = os.path.dirname(PROBE)
    core = pefile.PE(os.path.join(pkg, 'Keyboard', 'COREDL_.DLL'))
    n2o = {x.name.decode(): x.ordinal for x in core.DIRECTORY_ENTRY_EXPORT.symbols if x.name}
    proven, byname = set(), set()
    for f in ('MortScript.exe', os.path.join('ttcmd', 'ttcmd.exe')):
        p = pefile.PE(os.path.join(pkg, f))
        for e in p.DIRECTORY_ENTRY_IMPORT:
            if e.dll.lower() == b'coredll.dll':
                proven |= {i.ordinal for i in e.imports if not i.name}
                byname |= {i.name.decode() for i in e.imports if i.name}
    nomes = [x.split('#')[0].strip() for x in open(os.path.join(AQUI, 'mn1', 'imports_tmc.txt'))]
    nomes = list(dict.fromkeys(x for x in nomes if x))
    ruins = [x for x in nomes if n2o.get(x) not in proven and x not in byname]
    if ruins:
        sys.exit('ERRO: importacao nao comprovada: %s' % ruins)
    os.makedirs(GEN, exist_ok=True)
    with open(os.path.join(GEN, 'imports.S'), 'w', newline='\n') as f:
        f.write('.set noreorder\n.set noat\n')
        for x in nomes:
            f.write('.section .iat,"aw"\n.align 2\n.globl __imp_%s\n__imp_%s: .word 0\n' % (x, x))
            f.write('.text\n.align 2\n.globl %s\n.type %s,@function\n%s:\n lui $t9,%%hi(__imp_%s)\n'
                    ' lw $t9,%%lo(__imp_%s)($t9)\n jr $t9\n nop\n' % (x, x, x, x, x))
    return {x: (n2o[x] if n2o.get(x) in proven else None) for x in nomes}


def main():
    imp = imports()
    a = w(AQUI)
    cmds = ['rm -rf %s; mkdir -p %s' % (RT, RT)]
    for s in MN1_WIN:
        cmds.append('%s %s -ffreestanding -fshort-wchar %s -c "%s/%s" -o %s/%s.o'
                    % (CC, CF_JOGO.replace(' -w', ' -Wall -Wno-unused-function -Wno-pointer-sign'), INC_MN1, a, s, RT, os.path.basename(s)))
    for s in PROBE_SRC:
        cmds.append('%s %s -ffreestanding -fshort-wchar %s -c "%s/src/%s" -o %s/probe_%s.o' % (CC, CF_JOGO, INC_MN1, w(PROBE), s, RT, s))
    for s in MN1_JOGO:
        cmds.append('%s %s %s %s -c "%s/%s" -o %s/%s.o' % (CC, CF_JOGO, DEF_JOGO, INC_JOGO, a, s, RT, os.path.basename(s)))
    cmds.append('%s %s -c "%s/mn1/mn1_setjmp.S" -o %s/setjmp.o' % (CC, CF_JOGO, a, RT))
    cmds.append('%s %s -c "%s/imports.S" -o %s/imports.o' % (CC, CF_JOGO, w(GEN), RT))
    print(wsl(cmds))

    # 2. o que falta: indefinidos menos definidos (jogo + camada MN1)
    out = wsl(['cd ~/tmc_build',
               'mipsel-linux-gnu-nm -g --defined-only obj/*.o rt/*.o 2>/dev/null | awk \'NF==3{print $3}\' | sort -u > def.txt',
               'mipsel-linux-gnu-nm -u obj/*.o rt/*.o 2>/dev/null | awk \'NF==2 && $1=="U"{print $2}\' | sort -u > und.txt',
               'comm -23 und.txt def.txt > falta.txt',
               'mipsel-linux-gnu-objdump -r obj/*.o rt/*.o | grep -wFf falta.txt > relocs.txt || true',
               'wc -l < falta.txt'])
    print('sem dono antes das vazias:', out.strip())
    falta = os.path.join(GEN, 'falta.txt')
    rel = os.path.join(GEN, 'relocs.txt')
    wsl(['cp ~/tmc_build/falta.txt "%s"' % w(falta), 'cp ~/tmc_build/relocs.txt "%s"' % w(rel)])
    feitos = os.path.join(GEN, 'feitos.txt')
    open(feitos, 'w').write('')
    nomes = [l.strip() for l in open(falta) if l.strip()]
    sobra = [n for n in nomes if n.startswith('__') or n == 'main']
    if sobra:
        print('ATENCAO: sem dono e sem vazia possivel:', ' '.join(sobra))
    subprocess.run([sys.executable, os.path.join(AQUI, 'gera_vazios.py'), falta, rel, feitos,
                    os.path.join(GEN, 'mn1_vazios.c')], check=True)
    open(os.path.join(GEN, 'vazios_lista.txt'), 'w').write('\n'.join(n for n in nomes if n not in sobra) + '\n')

    # 3. liga
    elf = '%s/tmc.elf' % w(GEN)
    wsl(['%s %s -fno-builtin -c "%s/mn1_vazios.c" -o %s/vazios.o' % (CC, CF_JOGO, w(GEN), RT),
         'mipsel-linux-gnu-ld -static -T "%s" -Map "%s/tmc.map" -o "%s" ~/tmc_build/rt/probe_*.o $(ls ~/tmc_build/rt/*.o | grep -v probe_) ~/tmc_build/obj/*.o'
         % (w(os.path.join(PROBE, 'tools', 'pe.ld')), w(GEN), elf),
         'mipsel-linux-gnu-size "%s"' % elf])
    os.makedirs(DIST, exist_ok=True)
    exe = os.path.join(DIST, 'tmc.exe')
    n = elf2pe.convert(os.path.join(GEN, 'tmc.elf'), imp, exe, stack=0x100000)
    print('tmc.exe: %d bytes, %d importacoes estaticas' % (n, len(imp)))
    auditoria(exe)


def auditoria(path):
    """Nenhuma instrucao acima de MIPS II (o nucleo da MN1 so foi comprovado com MIPS II)."""
    import pefile
    from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS32, CS_MODE_LITTLE_ENDIAN
    p = pefile.PE(path)
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 + CS_MODE_LITTLE_ENDIAN)
    code = p.sections[0].get_data()[:p.sections[0].Misc_VirtualSize]
    ruins = {'mul', 'madd', 'maddu', 'msub', 'msubu', 'clz', 'clo', 'movz', 'movn', 'seb', 'seh', 'ext', 'ins', 'wsbh',
             'rdhwr', 'pref', 'ssnop', 'ehb', 'll', 'sc', 'ldc1', 'sdc1', 'lwc1', 'swc1', 'mtc1', 'mfc1'}
    n = 0
    for off in range(0, len(code) - 3, 4):
        ins = list(md.disasm(code[off:off + 4], 0x11000 + off))
        if not ins:
            continue  # dados no meio do codigo (tabelas de salto ficam em .rdata; aqui so enchimento)
        n += 1
        if ins[0].mnemonic in ruins:
            sys.exit('ERRO: instrucao acima de MIPS II em %#x: %s %s' % (ins[0].address, ins[0].mnemonic, ins[0].op_str))
    print('auditoria: %d instrucoes, todas MIPS II' % n)


if __name__ == '__main__':
    main()
