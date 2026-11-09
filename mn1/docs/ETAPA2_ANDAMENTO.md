# Etapa 2: andamento (08/11/2026)

## Feito

- **Jogo:** 618 de 619 arquivos compilam para MIPS.
  - `patch_32bit.py` agora também trata enchimento com comentário de várias linhas e cabeçalhos em `include/`: 89 blocos em 79 arquivos.
  - Falta `fileselect.c`, que inclui `port_runtime_config.h` e, com ele, o SDL.
- **Camada C do port:** 42 de 66 arquivos compilam.
  - 21 incluem o SDL. São os essenciais `port_main.c`, `port_bios.c`, `port_rom.c`, `port_linked_stubs.c`, `port_quicksave.c` e `port_softslots.c`, mais extras dispensáveis (Discord, conquistas online, repro, update).
  - 3 têm funções repetidas (`port_repro_*`, dispensáveis).
- **Pasta `mn1/`:** cabeçalhos da MN1 procurados antes de `port/`. Por enquanto, `port_ppu.h` sem SDL.
- **`mn1_patch.py`:** marca `TMC_MN1`. Segue o alvo N64 do próprio port em "sem TLS" e "quadro/VRAM do tamanho do GBA", mas não na ordem de bytes, porque a MN1 é little-endian.
- **`tmcteste.exe`** (28 KB, em `MN1_Probe`):
  - testa a reserva da faixa do GBA, 0x02000000–0x0A000000, que o port faz no Windows;
  - mede o PPU do port numa cena sintética no pior caso: modo 0, 4 fundos, 64 sprites e transparência;
  - mede a ampliação 3× para 720×480 e a cópia para a tela.
  - No emulador: rodou inteiro e a cena saiu correta. Os tempos do emulador não valem como medida.

## Achados

- O port reserva a faixa do GBA para distinguir endereços do GBA de ponteiros do programa. Na MN1, de 32 bits, isso é **obrigatório**: sem a reserva, um ponteiro do programa poderia cair na faixa.
- O alvo N64 do próprio port (MIPS de 32 bits a 93 MHz) é a exceção oficial à regra de 64 bits. Indica que o código já roda em MIPS 32.
- **Dependências SDL nos essenciais:** 85 identificadores. Plano: um `SDL3/SDL.h` próprio da MN1, implementado sobre o Windows CE. Toque vira evento de controle, que o `port_bios.c` já converte em tecla do GBA.
- **Pacotes de assets:** formato "TMCP" documentado em `port_asset_pak.hpp`. Leitor em C é simples.

## Resultado do `tmcteste.exe` na MN1 (08/11/2026, 21:17)

**Memória: funciona.**
- A faixa do GBA (0x02000000–0x0A000000) foi reservada no endereço exato.
- EWRAM, IWRAM, VRAM e 16 MB de "ROM" foram confirmados, escritos e lidos com 0 erros.
- A RAM livre caiu de 126 MB para 110 MB com tudo confirmado.

**Velocidade:** cena sintética no pior caso, com 4 fundos, transparência e 64 sprites, em 300 quadros.

| Etapa | Média | p95 | Pior |
|---|---|---|---|
| Desenho do quadro GBA (PPU do port) | **31,5 ms** | 44 ms | 47 ms |
| Ampliação 3× para 720×480 | 5,4 ms | 6 ms | 15 ms |
| Cópia para a tela | 9,5 ms | 11 ms | 31 ms |
| **Total** | **46,5 ms (21 fps)** | | |

**Leitura:**
- **Jogável a ~20 fps** do jeito que está, se a lógica do jogo rodar a 60 Hz e só o desenho pular quadros (2 em cada 3). A lógica do GBA é leve, mas o tempo dela na MN1 ainda não foi medido.
- Para **30 fps**, o total precisa cair a ~33 ms. Caminhos:
  - compilar para MIPS32: a sonda da rodada 1 informa núcleo "MIPS32", e hoje compilo como MIPS II por segurança;
  - otimizar o PPU;
  - reduzir a área copiada para a tela.
- A cena é o pior caso. O Minish Cap costuma usar menos camadas ao mesmo tempo.

## Ligação (08/11, depois do teste)

Com `mn1/SDL3/SDL.h`, um SDL próprio da MN1 só com tipos e declarações:
- **jogo: 619/619 compilam**;
- **camada C do port: 56/56** dos que ficam, incluindo o PPU com `TMC_MN1`. Ficaram de fora: conquistas online (`curl`), repro, seletor de rom, Discord e checagem de atualização.

**Tudo junto:** 2,8 MB de código, 3,2 MB zerados, **347 símbolos sem dono** (`faltando_tudo.txt`, por `classifica.py`):

| Grupo | Qtde | Destino |
|---|---|---|
| `port_runtime_config.cpp` | 66 | C com valores fixos |
| Randomizador, menus, editor, voz, depuração, GPU, bugreport, mods... (C++) | ~110 | funções vazias |
| Outros (`Port_*` de extras excluídos) | 51 | funções vazias |
| Biblioteca de C | 39 | `coredll` da MN1 + POSIX vazio (mmap, opendir, shm...) |
| SDL | 37 | `mn1/sdl_mn1.c` |
| Ponto flutuante / 64 bits (libgcc) | 25 | rotinas `__dp*`/`__fp*`/`__ull_*` da `coredll`, com adaptadores nas comparações (GCC: <0/0/>0; Microsoft: 1/0) |
| Som (`port_m4a_backend.cpp`) | 18 | mudo por enquanto |
| Apresentação da tela (`port_ppu.cpp`) | 17 | C: Init + PresentFrame com ampliação 3× e o resto vazio |
| **Carregador de assets** (`port_asset_loader.cpp`) | 14 | **C de verdade**: formato TMCP + dados que hoje vêm de JSON |

**A conferir na MN1:** a convenção de passagem de `double` das rotinas `__dp*` da `coredll`. Vai num autoteste na próxima ida ao carro.

## Próximo

1. ~~Resultado do `tmcteste.exe` na MN1.~~ Viável: seguir.
2. Camada de execução da MN1:
   - libc da `coredll` e mapeamento de ponto flutuante;
   - SDL implementado;
   - funções vazias;
   - `.exe` ligado.
3. Carregador de assets em C.
4. Apresentação da tela e botões de toque.
5. Tela de título no emulador.
2. `mn1/SDL3/SDL.h` e a implementação.
3. Carregador de assets em C.
4. Ligar tudo e chegar à tela de título no emulador.
