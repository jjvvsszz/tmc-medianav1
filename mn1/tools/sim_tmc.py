#!/usr/bin/env python3
"""tmc.exe (Minish Cap para a MN1) no emulador do MN1_Probe.

Acrescenta ao sim.py o que o jogo importa da coredll: malloc & cia., mem*/strchr, matematica e
as rotinas de ponto flutuante da Microsoft (__dpadd...), na convencao que mn1_float.c supoe
(double em par de registros, como long long). Toques roteirizados por tempo simulado.
Para depois de N quadros copiados para a tela e salva alguns como PNG (so para conferencia local).

Uso: python sim_tmc.py [quadros] [rom]
Nada aqui e medida da MN1: tempos do emulador nao valem.
"""
import math
import os
import shutil
import struct
import sys
import time

AQUI = os.path.dirname(os.path.abspath(__file__))
PROBE = os.path.join(os.path.dirname(AQUI), 'TTCMD from USB for MN1', 'MN1_Probe')
sys.path.insert(0, os.path.join(PROBE, 'tools'))
import sim  # noqa: E402
from unicorn.mips_const import UC_MIPS_REG_V1  # noqa: E402

QUADROS = int(sys.argv[1]) if len(sys.argv) > 1 else 120
ROM = sys.argv[2] if len(sys.argv) > 2 else 'baserom.gba'
SALVAR = {10, 30, 60, 90, 120, 160, 200, 260, 320}

# roteiro de toques: (segundo simulado, x, y) aperta por 150 ms
ROTEIRO = [(t, 455, 460) for t in (4, 6, 8, 10, 12)] + [(t, 726, 346) for t in (14, 16, 18, 20)]


def d(lo, hi):
    return struct.unpack('<d', struct.pack('<II', lo, hi))[0]


def f32(b):
    return struct.unpack('<f', struct.pack('<I', b))[0]


def bits_d(x):
    lo, hi = struct.unpack('<II', struct.pack('<d', x))
    return lo, hi


def bits_f(x):
    try:
        return struct.unpack('<I', struct.pack('<f', x))[0]
    except OverflowError:
        return 0x7F800000 if x > 0 else 0xFF800000


class SimTmc(sim.Sim):
    def __init__(self, *a, **k):
        super().__init__(*a, **k)
        self.blocos = {}
        self.inicio = time.time()
        self.toque = []
        for t, x, y in ROTEIRO:
            self.toque.append((t * 1000000, 0x201, x, y))
            self.toque.append((t * 1000000 + 150000, 0x202, x, y))
        self.n_instr_limite = 0

    def ret_d(self, x):
        lo, hi = bits_d(x)
        self.uc.reg_write(UC_MIPS_REG_V1, hi)
        return lo

    # memoria --------------------------------------------------------------
    def a_malloc(self):
        n = self.args()[0]
        a = self.alloc(n + 16) + 16
        self.blocos[a] = n
        return a

    def a_calloc(self):
        a = self.args()
        n = a[0] * a[1]
        p = self.alloc(n + 16) + 16
        self.blocos[p] = n
        self.wr(p, bytes(n))
        return p

    def a_realloc(self):
        p, n = self.args()[:2]
        q = self.alloc(n + 16) + 16
        self.blocos[q] = n
        if p:
            self.wr(q, self.rd(p, min(n, self.blocos.get(p, n))))
        return q

    def a_free(self):
        return 0

    def a_memmove(self):
        a = self.args()
        self.wr(a[0], self.rd(a[1], a[2]))
        return a[0]

    def a_memcmp(self):
        a = self.args()
        x, y = self.rd(a[0], a[2]), self.rd(a[1], a[2])
        for i in range(a[2]):
            if x[i] != y[i]:
                return (x[i] - y[i]) & 0xFFFFFFFF
        return 0

    def a_strchr(self):
        s, c = self.args()[:2]
        c &= 255
        while True:
            b = self.rd(s, 1)[0]
            if b == c:
                return s
            if not b:
                return 0
            s += 1

    # matematica (double em a0:a1, a2:a3) -----------------------------------
    def _m1(self, fn):
        a = self.args()
        try:
            return self.ret_d(fn(d(a[0], a[1])))
        except (ValueError, OverflowError):
            return self.ret_d(float('nan'))

    def a_sqrt(self): return self._m1(math.sqrt)
    def a_floor(self): return self._m1(math.floor)
    def a_sin(self): return self._m1(math.sin)
    def a_cos(self): return self._m1(math.cos)
    def a_asin(self): return self._m1(math.asin)
    def a_tanh(self): return self._m1(math.tanh)
    def a_exp(self): return self._m1(math.exp)
    def a_log(self): return self._m1(math.log)

    def a_pow(self):
        a = self.args()
        return self.ret_d(math.pow(d(a[0], a[1]), d(a[2], a[3])))

    def a_atan2(self):
        a = self.args()
        return self.ret_d(math.atan2(d(a[0], a[1]), d(a[2], a[3])))

    # ponto flutuante da Microsoft ---------------------------------------
    def _dd(self):
        a = self.args()
        return d(a[0], a[1]), d(a[2], a[3])

    def a___dpadd(self): x, y = self._dd(); return self.ret_d(x + y)
    def a___dpsub(self): x, y = self._dd(); return self.ret_d(x - y)
    def a___dpmul(self): x, y = self._dd(); return self.ret_d(x * y)

    def a___dpdiv(self):
        x, y = self._dd()
        if y == 0:
            return self.ret_d(math.copysign(math.inf, x) if x else math.nan)
        return self.ret_d(x / y)

    def a___eqd(self): x, y = self._dd(); return int(x == y)
    def a___ltd(self): x, y = self._dd(); return int(x < y)
    def a___led(self): x, y = self._dd(); return int(x <= y)
    def a___gtd(self): x, y = self._dd(); return int(x > y)
    def a___ged(self): x, y = self._dd(); return int(x >= y)

    def a___dptoli(self):
        a = self.args()
        x = d(a[0], a[1])
        if x != x:
            return 0
        return max(-2**31, min(2**31 - 1, int(x))) & 0xFFFFFFFF

    def a___litodp(self):
        v = self.args()[0]
        return self.ret_d(float(v - (1 << 32) if v & 0x80000000 else v))

    def a___fptodp(self):
        return self.ret_d(f32(self.args()[0]))

    def a___litofp(self):
        v = self.args()[0]
        return bits_f(float(v - (1 << 32) if v & 0x80000000 else v))

    def a___dptofp(self):
        a = self.args()
        return bits_f(d(a[0], a[1]))

    def a_CreateFileW(self):
        n = self.ws(self.args()[0])
        self.abertos = getattr(self, 'abertos', {})
        self.abertos[n] = self.abertos.get(n, 0) + 1
        if self.abertos[n] in (1, 10, 1000):
            print('  [sim] CreateFileW %s (%dx)' % (n, self.abertos[n]), flush=True)
        return super().a_CreateFileW()

    def a_SetFilePointer(self):
        a = self.args()
        o = self.handles[a[0]]
        dist = a[1] - (1 << 32) if a[1] & 0x80000000 else a[1]
        if o[0] == 'mem':
            pos, tam, i = o[2], len(o[1]), 2
        elif o[0] == 'w':
            pos, tam, i = o[3], len(o[2]), 3
        else:
            return 0
        novo = {0: 0, 1: pos, 2: tam}[a[3]] + dist
        if novo < 0:
            return 0xFFFFFFFF
        o[i] = novo
        return novo

    def a_DeleteFileW(self):
        r = self.path(self.ws(self.args()[0]))
        if r and os.path.exists(r):
            os.remove(r)
            return 1
        self.err = 2
        return 0

    def a_MoveFileW(self):
        a = self.args()
        x, y = self.path(self.ws(a[0])), self.path(self.ws(a[1]))
        if x and y and os.path.exists(x) and not os.path.exists(y):
            os.replace(x, y)
            return 1
        self.err = 2
        return 0

    # entrada roteirizada ----------------------------------------------------
    def a_PeekMessageW(self):
        a = self.args(5)
        if self.toque and self.w.t_us >= self.toque[0][0] and self.wnds:
            _, msg, x, y = self.toque.pop(0)
            hw = next(iter(self.wnds))
            self.wr(a[0], struct.pack('<7I', hw, msg, 1 if msg == 0x201 else 0, (y << 16) | x, 0, x, y))
            print('  [sim] t=%.1fs toque %s em (%d,%d)' % (self.w.t_us / 1e6, 'desce' if msg == 0x201 else 'sobe', x, y))
            return 1
        return 0

    def a_BitBlt(self):
        r = super().a_BitBlt()
        n = self.w.frames.get('n', 0)
        if n in SALVAR:
            b, bw, bh = self.w.frames['last']
            sim.write_png(os.path.join(SAIDA, 'quadro_%03d.png' % n), self.w.frames['last'])
            cores = len(set(struct.unpack('<%dH' % (bw * bh), b)[i] for i in range(0, bw * bh, 97)))
            print('  [sim] quadro %d t=%.1fs cores~%d (%.0fs reais)' % (n, self.w.t_us / 1e6, cores, time.time() - self.inicio))
        if n >= QUADROS:
            self.exitcode = 0
            self.uc.emu_stop()
        return r


SAIDA = os.path.join(AQUI, 'sim_saida')


def vigia_simbolos():
    from elftools.elf.elffile import ELFFile
    e = ELFFile(open(os.path.join(AQUI, 'mn1', 'gerado', 'tmc.elf'), 'rb'))
    return [(x['st_value'], x.name) for x in e.get_section_by_name('.symtab').iter_symbols() if x['st_info']['type'] == 'STT_FUNC']
if __name__ == '__main__':
    import json
    import pefile
    core = pefile.PE(os.path.join(os.path.dirname(PROBE), 'Keyboard', 'COREDL_.DLL'))
    os.makedirs(SAIDA, exist_ok=True)
    sim.ORDS = os.path.join(SAIDA, 'ords.json')
    json.dump({str(x.ordinal): x.name.decode() for x in core.DIRECTORY_ENTRY_EXPORT.symbols if x.name}, open(sim.ORDS, 'w'))
    caixa = os.path.join(SAIDA, 'pendrive')
    shutil.rmtree(caixa, ignore_errors=True)
    os.makedirs(os.path.join(caixa, 'MN1'))
    shutil.copy(ROM, os.path.join(caixa, 'MN1', 'baserom.gba'))
    w = sim.World(caixa)
    s = SimTmc(w, os.path.join(AQUI, 'dist', 'tmc.exe'), '')
    import threading
    from unicorn.mips_const import UC_MIPS_REG_PC, UC_MIPS_REG_RA
    simbolos = sorted(vigia_simbolos())

    def onde(a):
        import bisect
        i = bisect.bisect_right(simbolos, (a, '~')) - 1
        return '%s+%#x' % (simbolos[i][1], a - simbolos[i][0]) if i >= 0 else hex(a)

    def vigia():
        ant = {}
        while True:
            time.sleep(20)
            pc, ra = s.uc.reg_read(UC_MIPS_REG_PC), s.uc.reg_read(UC_MIPS_REG_RA)
            novas = {k: v - ant.get(k, 0) for k, v in w.calls.items() if isinstance(v, int) and v != ant.get(k, 0)}
            ant = dict(w.calls)
            print('  [vigia] %.0fs reais t=%.1fs pc=%s ra=%s quadros=%d apis=%s' % (time.time() - s.inicio, w.t_us / 1e6, onde(pc), onde(ra), w.frames.get('n', 0), sorted(novas.items(), key=lambda kv: -kv[1])[:6]), flush=True)
    threading.Thread(target=vigia, daemon=True).start()
    try:
        code = s.run()
    except BaseException as e:
        code = 'excecao: %r' % e
    print('saida:', code, ' quadros:', w.frames.get('n', 0), ' tempo simulado: %.1fs' % (w.t_us / 1e6))
    log = os.path.join(caixa, 'RESULTADOS3', 'TMC_LOG.txt')
    print(open(log, encoding='utf8', errors='replace').read()[-6000:] if os.path.exists(log) else 'SEM LOG')
    print('chamadas mais frequentes:', sorted(((k, v) for k, v in w.calls.items() if isinstance(v, int)), key=lambda kv: -kv[1])[:12])
