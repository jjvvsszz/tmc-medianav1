/* Ponto flutuante e 64 bits para o GCC com -msoft-float, sobre as rotinas da coredll da MN1.

   A libgcc do compilador nao serve: foi feita para MIPS32r2 com FPU (ext, seb, ldc1...).
   O GCC chama __adddf3 & cia.; a coredll do CE tem as mesmas operacoes com nomes da Microsoft
   (__dpadd...). Na ABI o32 sem FPU, double vai e volta como um par de registros, igual a long
   long; por isso tudo aqui e declarado com u64 (double) e u32 (float): este arquivo nao faz
   nenhuma conta em ponto flutuante, so repassa bits.

   Diferenca de contrato nas comparacoes: GCC devolve <0/0/>0, a Microsoft devolve 1/0.
   float: convertido para double, operado e arredondado de volta. Para + - * / o resultado e
   identico ao calculo direto em float (double tem mais que o dobro de bits de mantissa).
   Divisao de 64 bits: feita aqui, em C inteiro.
   A convencao das rotinas da coredll e conferida no aparelho por mn1_float_autoteste(). */
#include "mn1.h"

typedef unsigned long long u64;
typedef long long i64;

/* importadas estaticamente (comprovadas em MortScript.exe/ttcmd.exe) */
u64 __dpadd(u64, u64);
u64 __dpsub(u64, u64);
u64 __dpmul(u64, u64);
u64 __dpdiv(u64, u64);
int __dptoli(u64);
u64 __litodp(int);
u64 __fptodp(u32);
u32 __litofp(int);
int __eqd(u64, u64);
int __ltd(u64, u64);
int __led(u64, u64);
int __gtd(u64, u64);
int __ged(u64, u64);

/* so por GetProcAddress */
static u32 (*dptofp)(u64);

#define D_2_31 0x41E0000000000000ULL
#define D_2_32 0x41F0000000000000ULL

static u32 trunc_df(u64 d)
{
    /* sem __dptofp: corta a mantissa (arredonda para zero), com faixa e sinal tratados */
    u32 s = (u32)(d >> 63) << 31;
    int e = (int)((d >> 52) & 0x7FF);
    u32 m = (u32)(d >> 29) & 0x7FFFFF;
    if (e == 0x7FF) return s | 0x7F800000 | (m ? 0x400000 : 0);
    e = e - 1023 + 127;
    if (e >= 255) return s | 0x7F800000;
    if (e <= 0) return s;
    return s | ((u32)e << 23) | m;
}

void mn1_float_init(void)
{
    dptofp = (u32 (*)(u64))DYNC(__dptofp);
}

/* ------------------------------------------------------------ double */
u64 __adddf3(u64 a, u64 b) { return __dpadd(a, b); }
u64 __subdf3(u64 a, u64 b) { return __dpsub(a, b); }
u64 __muldf3(u64 a, u64 b) { return __dpmul(a, b); }
u64 __divdf3(u64 a, u64 b) { return __dpdiv(a, b); }
u64 __negdf2(u64 a) { return a ^ 0x8000000000000000ULL; }

static int nan_d(u64 a) { return ((a >> 52) & 0x7FF) == 0x7FF && (a << 12) != 0; }
int __unorddf2(u64 a, u64 b) { return nan_d(a) || nan_d(b); }
int __eqdf2(u64 a, u64 b) { return __eqd(a, b) ? 0 : 1; }
int __nedf2(u64 a, u64 b) { return __eqd(a, b) ? 0 : 1; }
int __ltdf2(u64 a, u64 b) { return __ltd(a, b) ? -1 : 1; }
int __ledf2(u64 a, u64 b) { return __led(a, b) ? 0 : 1; }
int __gtdf2(u64 a, u64 b) { return __gtd(a, b) ? 1 : -1; }
int __gedf2(u64 a, u64 b) { return __ged(a, b) ? 0 : -1; }

u64 __floatsidf(int i) { return __litodp(i); }
u64 __floatunsidf(u32 u)
{
    if (!(u & 0x80000000u)) return __litodp((int)u);
    return __dpadd(__litodp((int)(u - 0x80000000u)), D_2_31);
}
u64 __floatdidf(i64 v)
{
    int neg = v < 0;
    u64 u = neg ? (u64)-v : (u64)v;
    u64 r = __dpadd(__dpmul(__floatunsidf((u32)(u >> 32)), D_2_32), __floatunsidf((u32)u));
    return neg ? r ^ 0x8000000000000000ULL : r;
}
u64 __floatundidf(u64 u) { return __dpadd(__dpmul(__floatunsidf((u32)(u >> 32)), D_2_32), __floatunsidf((u32)u)); }

int __fixdfsi(u64 d) { return __dptoli(d); }
u32 __fixunsdfsi(u64 d)
{
    if (__ltd(d, D_2_31)) return (u32)__dptoli(d);
    return (u32)__dptoli(__dpsub(d, D_2_31)) + 0x80000000u;
}
u64 __fixunsdfdi(u64 d)
{
    u32 hi;
    if (d >> 63) return 0;
    hi = __fixunsdfsi(__dpdiv(d, D_2_32));
    return ((u64)hi << 32) | __fixunsdfsi(__dpsub(d, __dpmul(__floatunsidf(hi), D_2_32)));
}
i64 __fixdfdi(u64 d)
{
    if (d >> 63) return -(i64)__fixunsdfdi(d ^ 0x8000000000000000ULL);
    return (i64)__fixunsdfdi(d);
}

u64 __extendsfdf2(u32 f) { return __fptodp(f); }
u32 __truncdfsf2(u64 d) { return dptofp ? dptofp(d) : trunc_df(d); }

/* ------------------------------------------------------------ float (via double) */
#define F2D(x) __fptodp(x)
u32 __addsf3(u32 a, u32 b) { return __truncdfsf2(__dpadd(F2D(a), F2D(b))); }
u32 __subsf3(u32 a, u32 b) { return __truncdfsf2(__dpsub(F2D(a), F2D(b))); }
u32 __mulsf3(u32 a, u32 b) { return __truncdfsf2(__dpmul(F2D(a), F2D(b))); }
u32 __divsf3(u32 a, u32 b) { return __truncdfsf2(__dpdiv(F2D(a), F2D(b))); }
u32 __negsf2(u32 a) { return a ^ 0x80000000u; }
int __eqsf2(u32 a, u32 b) { return __eqdf2(F2D(a), F2D(b)); }
int __nesf2(u32 a, u32 b) { return __nedf2(F2D(a), F2D(b)); }
int __ltsf2(u32 a, u32 b) { return __ltdf2(F2D(a), F2D(b)); }
int __lesf2(u32 a, u32 b) { return __ledf2(F2D(a), F2D(b)); }
int __gtsf2(u32 a, u32 b) { return __gtdf2(F2D(a), F2D(b)); }
int __gesf2(u32 a, u32 b) { return __gedf2(F2D(a), F2D(b)); }
int __unordsf2(u32 a, u32 b) { return ((a & 0x7FFFFFFF) > 0x7F800000) || ((b & 0x7FFFFFFF) > 0x7F800000); }
u32 __floatsisf(int i) { return __litofp(i); }
u32 __floatunsisf(u32 u) { return __truncdfsf2(__floatunsidf(u)); }
u32 __floatdisf(i64 v) { return __truncdfsf2(__floatdidf(v)); }
u32 __floatundisf(u64 v) { return __truncdfsf2(__floatundidf(v)); }
int __fixsfsi(u32 f) { return __dptoli(F2D(f)); }
u32 __fixunssfsi(u32 f) { return __fixunsdfsi(F2D(f)); }
i64 __fixsfdi(u32 f) { return __fixdfdi(F2D(f)); }
u64 __fixunssfdi(u32 f) { return __fixunsdfdi(F2D(f)); }

/* ------------------------------------------------------------ inteiros de 64 bits */
static u64 udivmod(u64 n, u64 d, u64 *rem)
{
    u64 q = 0, r = 0;
    int i;
    if (!d) { if (rem) *rem = 0; return 0; }
    if (!(n >> 32) && !(d >> 32)) {
        u32 a = (u32)n, b = (u32)d;
        if (rem) *rem = a % b;
        return a / b;
    }
    for (i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) { r -= d; q |= 1ULL << i; }
    }
    if (rem) *rem = r;
    return q;
}
u64 __udivdi3(u64 n, u64 d) { return udivmod(n, d, 0); }
u64 __umoddi3(u64 n, u64 d) { u64 r; udivmod(n, d, &r); return r; }
i64 __divdi3(i64 n, i64 d)
{
    int neg = (n < 0) ^ (d < 0);
    u64 q = udivmod(n < 0 ? (u64)-n : (u64)n, d < 0 ? (u64)-d : (u64)d, 0);
    return neg ? -(i64)q : (i64)q;
}
i64 __moddi3(i64 n, i64 d)
{
    u64 r;
    udivmod(n < 0 ? (u64)-n : (u64)n, d < 0 ? (u64)-d : (u64)d, &r);
    return n < 0 ? -(i64)r : (i64)r;
}

/* ------------------------------------------------------------ autoteste no aparelho */
static int chk(const char *nome, u64 got, u64 want)
{
    lg("R float.%s=%s %08x%08x", nome, got == want ? "OK" : "ERRO", (u32)(got >> 32), (u32)got);
    return got == want;
}

/* 1,5 + 2,25 = 3,75; 3 * 0,5 = 1,5; 1 / 4 = 0,25; 7 - 10 = -3; comparacoes; conversoes */
int mn1_float_autoteste(void)
{
    const u64 d1_5 = 0x3FF8000000000000ULL, d2_25 = 0x4002000000000000ULL, d3_75 = 0x400E000000000000ULL;
    const u64 d3 = 0x4008000000000000ULL, d0_5 = 0x3FE0000000000000ULL, d1 = 0x3FF0000000000000ULL;
    const u64 d4 = 0x4010000000000000ULL, d0_25 = 0x3FD0000000000000ULL, dm3 = 0xC008000000000000ULL;
    int ok = 1;
    ok &= chk("add", __adddf3(d1_5, d2_25), d3_75);
    ok &= chk("mul", __muldf3(d3, d0_5), d1_5);
    ok &= chk("div", __divdf3(d1, d4), d0_25);
    ok &= chk("sub", __subdf3(__litodp(7), __litodp(10)), dm3);
    ok &= chk("lt", (u64)(__ltdf2(d1, d3) < 0) * 10 + (__ltdf2(d3, d1) < 0), 10);
    ok &= chk("ge", (u64)(__gedf2(d3, d3) >= 0) * 10 + (__gedf2(d1, d3) >= 0), 10);
    ok &= chk("toi", (u64)(u32)__fixdfsi(d3_75), 3);
    ok &= chk("toi_neg", (u64)(u32)__fixdfsi(dm3), (u32)-3);
    ok &= chk("f2d", __extendsfdf2(0x3FC00000), d1_5);
    ok &= chk("d2f", __truncdfsf2(d3_75), 0x40700000);
    ok &= chk("sf_add", __addsf3(0x3FC00000, 0x40100000), 0x40700000);
    ok &= chk("u64div", __udivdi3(10000000000ULL, 7), 1428571428ULL);
    ok &= chk("u2d", __floatunsidf(0x80000001u), 0x41E0000000200000ULL);
    lg("R float.dptofp=%s", dptofp ? "sim" : "nao");
    lg("R float.total=%s", ok ? "OK" : "ERRO");
    return ok;
}
