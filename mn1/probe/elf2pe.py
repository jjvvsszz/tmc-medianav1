#!/usr/bin/env python3
"""Converte ELF mipsel estatico em PE Windows CE, com cabecalhos iguais aos de
Special_Tool.exe/MortScript.exe (Machine 0x166, subsistema 9, base 0x10000, relocs stripped)."""
import struct
import json
from elftools.elf.elffile import ELFFile

ALIGN, FILEAL, BASE = 0x1000, 0x200, 0x10000


def up(v, a):
    return (v + a - 1) & ~(a - 1)


def convert(src, imports, dst, stack=0x10000):
    e = ELFFile(open(src, 'rb'))
    sym = {s.name: s['st_value'] for s in e.get_section_by_name('.symtab').iter_symbols()}

    def sec(n):
        s = e.get_section_by_name(n)
        return s if s is not None and s['sh_size'] else None

    txt, rd, dat, bss = sec('.text'), sec('.rdata'), sec('.data'), sec('.bss')
    secs = [(b'.text', txt['sh_addr'] - BASE, txt.data(), txt['sh_size'], 0x60000020)]
    if rd:
        secs.append((b'.rdata', rd['sh_addr'] - BASE, rd.data(), rd['sh_size'], 0x40000040))
    dv = (bss['sh_addr'] + bss['sh_size'] - dat['sh_addr']) if bss else dat['sh_size']
    # .data: inclui IAT; ordinais gravados tambem no IAT, como o linker da MS faz
    items = sorted(imports.items(), key=lambda kv: sym['__imp_' + kv[0]])
    iat0 = sym['__iat_start'] - BASE
    assert [sym['__imp_' + k] - BASE - iat0 for k, _ in items] == [4 * i for i in range(len(items))], 'IAT nao contiguo'
    assert sym['__iat_end'] - sym['__iat_start'] == 4 * len(items)
    # .idata: descritor + INT + nome da DLL + tabela hint/nome
    irva = up(dat['sh_addr'] - BASE + dv, ALIGN)
    int_rva = irva + 40
    name_rva = int_rva + 4 * (len(items) + 1)
    hn_rva = name_rva + 12
    hn = b''
    thunks = []
    for k, o in items:
        if o is not None:
            thunks.append(0x80000000 | o)
        else:
            thunks.append(hn_rva + len(hn))
            s = b'\0\0' + k.encode() + b'\0'
            hn += s + (b'\0' if len(s) & 1 else b'')
    d = bytearray(dat.data())
    for i, t in enumerate(thunks):
        d[4 * i:4 * i + 4] = struct.pack('<I', t)
    secs.append((b'.data', dat['sh_addr'] - BASE, bytes(d), dv, 0xC0000040))
    id_ = struct.pack('<5I', int_rva, 0, 0, name_rva, iat0) + b'\0' * 20
    id_ += b''.join(struct.pack('<I', t) for t in thunks) + b'\0\0\0\0' + b'COREDLL.dll\0' + hn
    secs.append((b'.idata', irva, id_, len(id_), 0x40000040))

    hdr_sz = 0x400
    out = bytearray(hdr_sz)
    out[:2] = b'MZ'
    out[0x3c:0x40] = struct.pack('<I', 0x80)
    fp = hdr_sz
    tbl = b''
    bodies = []
    for n, rva, data, vs, ch in secs:
        raw = up(len(data), FILEAL)
        tbl += struct.pack('<8sIIIIIIHHI', n, vs, rva, raw, fp, 0, 0, 0, 0, ch)
        bodies.append((fp, data))
        fp += raw
    img = up(secs[-1][1] + secs[-1][3], ALIGN)
    code_sz = up(len(secs[0][2]), FILEAL)
    init_sz = sum(up(len(s[2]), FILEAL) for s in secs[1:])
    coff = struct.pack('<4sHHIIIHH', b'PE\0\0', 0x166, len(secs), 0x5F000000, 0, 0, 0xE0, 0x10F)
    opt = struct.pack('<HBBIIIII', 0x10B, 6, 24, code_sz, init_sz, 0, sym['_start'] - BASE, secs[0][1])
    opt += struct.pack('<IIIIHHHHHHIIIIHHIIIIII', secs[1][1], BASE, ALIGN, FILEAL, 4, 0, 0, 0, 5, 0, 0, img, hdr_sz,
                       0, 9, 0, stack, 0x1000, 0x100000, 0x1000, 0, 16)
    dirs = [(0, 0)] * 16
    dirs[1] = (irva, 40)
    dirs[12] = (iat0, 4 * len(items))
    opt += b''.join(struct.pack('<II', *x) for x in dirs)
    assert len(opt) == 0xE0
    b = coff + opt + tbl
    assert 0x80 + len(b) <= hdr_sz
    out[0x80:0x80 + len(b)] = b
    out = out.ljust(fp, b'\0')
    for p, data in bodies:
        out[p:p + len(data)] = data
    open(dst, 'wb').write(out)
    return len(out)
