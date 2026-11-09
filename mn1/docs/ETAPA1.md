# Etapa 1: o jogo (Minish Cap, port Picori) compila para o MIPS da MN1?

08/11/2026. Repositório `github.com/999sian/tmc` (v0.9.6), clonado em `repo/` só para análise. Nada foi alterado nele: as mudanças foram feitas numa cópia no WSL (`~/tmc32`).

## Resposta

**Sim, quase todo.** Com uma correção mecânica de layout, **601 dos 619 arquivos** de `src/` compilam para MIPS II com o mesmo compilador da sonda e do visor.

| Medida | Valor |
|---|---|
| Arquivos do jogo que o port usa | 619 (187 mil linhas, C) |
| Compilam para MIPS | **601** |
| Ainda não compilam | 17 inimigos (mesma causa, variação de escrita) + `fileselect.c` (inclui o SDL direto) |
| Tamanho do código gerado | **1,36 MB** de código + 16 KB de dados zerados |
| Funções de biblioteca de C usadas | **10** (`memcpy`, `memmove`, `memset`, `strlen`, `fwrite`, `fputc`, `fflush`, `getenv`, `abort`, `stderr`) |
| Ponto flutuante | **1 operação** (`__divsf3`, divisão de `float`) |

## O obstáculo encontrado: o port exige 64 bits

O port de PC trava a compilação fora de 64 bits (`sizeof(void*) == 8`). Isso acontece porque várias estruturas do jogo ganharam **4 bytes de enchimento** para compensar o ponteiro `Enemy::child`, que no PC tem 8 bytes. Na MN1, como no GBA original, o ponteiro tem 4 bytes e o enchimento sobra.

O próprio código já prevê os dois layouts: as verificações `PORT_STATIC_ASSERT_OFFSET(tipo, campo, deslocamento_GBA, deslocamento_PC)` escolhem o valor do GBA quando o ponteiro tem 4 bytes.

**Correção:** `patch_32bit.py` troca `#ifdef PC_PORT` por `#if defined(PC_PORT) && __SIZEOF_POINTER__ == 8` **somente** em blocos que são enchimento de estrutura:
- blocos com `#else` cujas duas versões só declaram campos;
- blocos sem `#else` cujos campos têm nome de enchimento (`pad`, `filler`, `_pc`, `unused`).

Blocos de comportamento do port não são tocados. Resultado: 71 blocos em 61 arquivos. **As verificações de layout do próprio código confirmam** que essas estruturas agora batem com o GBA. Os 17 inimigos restantes escrevem o enchimento de outro jeito e precisam de ajuste à mão.

## O que falta para ligar o programa

1.349 símbolos usados e não definidos pelos 601 objetos:

| Origem | Qtde | Leitura |
|---|---|---|
| Camada C do port (`port/*.c`) | 791 | 463 são `data_stubs_autogen.c`: dados do jogo **lidos da rom em tempo de execução**, pelo endereço. Entram na etapa 2 |
| Carregador de assets (C++) e dados gerados | ~521 | Funções `Port_*` do carregador em C++ (a reescrever em C) e rótulos de dados de `data/*.s`, também resolvidos pela camada do port |
| Arquivos de `src/` que ainda não compilam | 21 | Somem quando os 17 inimigos forem ajustados |
| Som (C++) | ~6 | Fica para depois |

## Conclusão

Nenhum impeditivo apareceu:
- o código do jogo é C portável;
- quase não usa biblioteca nem ponto flutuante;
- o problema de 64 bits foi entendido e é mecânico.

**Etapa 2:**
1. ajustar os 17 inimigos e tirar o SDL de `fileselect.c`;
2. compilar a camada C do port (`port/*.c`) e medir o que dela depende de SDL e de C++;
3. reescrever em C o carregador de assets, ~20 funções `extern "C"`;
4. criar a camada da MN1: tela, toque, tempo e arquivos;
5. ligar tudo num `.exe` e chegar à tela de título no emulador do PC.

Arquivos desta etapa:
- `compila_mips.sh`: compila uma lista de arquivos para MIPS;
- `patch_32bit.py`: correção de layout;
- `simbolos.sh` e `onde_definido.py`: levantamento dos símbolos que faltam;
- `faltando_src.txt`: os 1.349 símbolos.
