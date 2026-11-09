/* Ponto de entrada do tmc.exe na MN1.
   1. Reserva 0x02000000-0x0A000000 antes de qualquer malloc: o port trata qualquer valor nessa
      faixa como endereco do GBA, entao nenhum ponteiro do programa pode cair ali (no PC o
      port_main.c faz o mesmo com VirtualAlloc; testado na MN1 pelo tmcteste em 08/11/2026).
   2. Prepara caminhos, registro (RESULTADOS3\TMC_LOG.txt) e o ponto flutuante, com autoteste.
   3. Chama o main() do port, como se fosse "tmc.exe" sem argumentos. */
#include "mn1.h"

void mn1_crt_init(HANDLE inst);
void mn1_float_init(void);
int mn1_float_autoteste(void);
int main(int argc, char **argv);
int setenv(const char *n, const char *v, int o);
void exit(int);

__attribute__((section(".text.start")))
void _start(HANDLE inst, HANDLE prev, wchar *cmd, int show)
{
    static char *argv[] = { "tmc.exe", 0 };
    void *(*va)(void *, u32, u32, u32);
    void *faixa = 0;
    (void)prev; (void)cmd; (void)show;
    g_inst = inst;
    va = (void *(*)(void *, u32, u32, u32))DYNC(VirtualAlloc);
    if (va) faixa = va((void *)0x02000000, 0x08000000, 0x2000 /* MEM_RESERVE */, 1 /* PAGE_NOACCESS */);
    root_init();
    CreateDirectoryW(respath(L""), 0);
    lg_open(L"TMC_LOG.txt");
    lg("tmc inicio tick=%u reserva_gba=%x", GetTickCount(), (u32)faixa);
    lg_commit();
    if (faixa != (void *)0x02000000) { lg("tmc: reserva da faixa do GBA falhou, abortando"); lg_commit(); ExitThread(2); }
    mn1_crt_init(inst);
    mn1_float_init();
    if (!mn1_float_autoteste()) { lg("tmc: ponto flutuante da coredll nao bate, abortando"); lg_commit(); ExitThread(6); }
    lg_commit();
    /* perfil do proprio port: a cada 120 ticks, tempo de logica e de apresentacao no registro */
    setenv("TMC_PROFILE", "1", 1);
    exit(main(1, argv));
}
