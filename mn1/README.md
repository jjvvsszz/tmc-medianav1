# The Legend of Zelda: The Minish Cap for Renault MediaNav 1

Fork do port de PC [999sian/tmc](https://github.com/999sian/tmc) para a central multimídia **Renault MediaNav 1 (MN1)**:
Windows CE 6, processador RMI Au13xx (MIPS32, little-endian), tela de toque de 800×480 e 16 bits.

**A ROM não está neste repositório.** O jogo precisa da sua própria `baserom.gba` (Minish Cap USA), copiada para a pasta do `.exe` no pendrive.

## Situação (08/11/2026)

- **Compila e liga:** jogo (619 arquivos C) e camada C do port (55 arquivos) para MIPS II, sem SDL nem C++. Resultado: `tmc.exe` de 2,8 MB.
- **Medido na MN1 real** com `tmcteste.exe`:
  - a reserva da faixa do GBA funciona;
  - o PPU em software desenha a ~21 fps no pior caso.
- **Ainda não rodou** nem no emulador nem no aparelho. Próximo passo: tela de título no emulador (`tools/sim_tmc.py`).
- **Sem som** por enquanto.
- **Controles:** botões de toque desenhados por cima da imagem. A tela da MN1 só lê um toque por vez.

## O que mudou no código do port

- **`patch_32bit.py`:** os enchimentos `#ifdef PC_PORT` que só existem para ponteiros de 8 bytes passam a valer só em 64 bits. Assim, em 32 bits, as structs voltam ao layout do GBA (89 blocos em 79 arquivos).
- **`port/port_types.h`:** sai a trava "só 64 bits".
- **`port/ppu`:** com `TMC_MN1`, segue o alvo N64 do próprio port (sem TLS, quadro do tamanho do GBA). A ordem de bytes não muda.
- **`port/port_bios.c`:** com `TMC_MN1`, o laço recupera até 8 ticks atrasados. Desenhar custa ~46 ms na MN1, e sem isso o jogo andaria em câmera lenta.

## Camada da MN1 (`mn1/runtime`)

| Arquivo | Papel |
|---|---|
| `mn1_entry.c` | Entrada: reserva 0x02000000–0x0A000000, faz o autoteste de ponto flutuante e chama `main()` |
| `mn1_crt.c` | libc: arquivos sobre Win32, `printf`, texto. O POSIX opcional falha de forma limpa |
| `mn1_float.c` | Ponto flutuante e 64 bits do GCC (`-msoft-float`) sobre as rotinas `__dp*` da `coredll` |
| `mn1_setjmp.S` | `setjmp`/`longjmp` (soft reset do jogo) |
| `mn1_sdl.c`, `SDL3/` | O pedaço do SDL3 que o port usa |
| `mn1_ppu.c` | Ampliação 3× para 720×480, controle do ritmo de desenho e botões de toque |
| `mn1_assets.c` | Modo "só ROM": o port já cai na ROM quando não há `assets/` |
| `mn1_config.c` | Configuração fixa (gerada por `gera_config.py`) |

`mn1/probe` traz o runtime Win32 mínimo do projeto MN1_Probe: `rt.c`, `ui.c`, o conversor ELF→PE e o mapa de ordinais da `coredll`.

## Compilar

Compilador: GCC 13 `mipsel-linux-gnu` no WSL. Flags: `-march=mips2 -msoft-float -mno-abicalls -fno-pic -G0`.

1. Os cabeçalhos `build/USA/assets/*_offsets.h` vêm da compilação normal do port de PC (estão no `.gitignore` do original).
2. Compile os objetos:

   ```bash
   REPO_OVERRIDE=<este repo> bash mn1/tools/compila_mips.sh todos_mn1.txt ~/tmc_build
   ```

3. Ligue: `python mn1/tools/build_tmc.py` gera `dist/tmc.exe`. Ele confere duas coisas:
   - só importa funções da `coredll` comprovadas no aparelho;
   - o código usa só instruções MIPS II.

`build_tmc.py` ainda procura o MN1_Probe na pasta irmã. Deixar a compilação autocontida neste repositório é a próxima limpeza.

Licença: GPL-3.0, a mesma do port original.
