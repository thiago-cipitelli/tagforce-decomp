# Levantamento para o port (fase 5)

Feito em 2026-11-11, depois da rodada 22 de matching (86,1% do código batendo). Responde a duas perguntas:
o quanto o C do decomp está longe de compilar fora do mwcc, e qual API do firmware o `platform/` precisa cobrir.
Os números são *observados* (gcc e `symbols/`), salvo onde está marcado *inferido*.

O port fica neste repositório (`platform/psp`, `platform/pc`). Regra: nenhuma mudança do port pode quebrar
`make verify` (ALL MATCH). O que for só do port vai para `platform/`, para typedefs/macros de `include/` ou para os
ramos `#ifdef NON_MATCHING`/`#else`, nunca para o caminho que o mwcc compila.

## 1. Compilação no host (gcc)

Gate: `make host-check` roda cada `src/*/*.c` em `gcc -m32 -std=gnu89 -fsyntax-only -w -DNON_MATCHING` (32 bits,
como a PSP; os rascunhos `NON_MATCHING` no lugar do asm, que é o C que o port usa).

**681 de 681 compilam** desde 2026-11-11 (eram 670). As declarações que existiam só para o matching ficaram sob
`#ifdef __MWERKS__` (o mwcc vê o mesmo fonte, então o matching não muda); o port usa o protótipo do arquivo.
(`duel_0884F30C` e `dd_slot_xform`, que ficaram com declaração K&R no host no passo 1, têm o protótipo da definição
desde o passo 2, seção 1.1.) Os 11 que falhavam:

| Arquivo | Causa |
|---|---|
| `BOOT/sys.c` | `asm { addi $v0, $ra, 0 }` do mwcc em `ehscript_set_static` (instrução solta do original); a mesma função lê o código de máquina de `fn` para recusar thunks, o que não faz sentido no PC |
| `BOOT/font_tags.c` (`wc_no_line_start`), `libduel_eng/duel_finish.c` (`recipe_get_id`), `libdeck_tutorial/dt_edraw.c` (`dt_draw_count`) | declaração K&R `f();` seguida de definição com parâmetro `char`/`short`/`float`: o gcc recusa (promoção de argumentos) |
| `libdeck/dk_lists.c` (`dk_list_find`), `libduel_draw/dd_event_cmd.c` (`func_08822838`), `libduel_eng/duel_draws.c` (`duel_0884F30C`), `libduel_eng/duel_reqs2.c` (`duel_cmd`), `libduel_eng/effect_live.c` (`duel_act_target`), `libtitle/t_panel.c` (`tframe_tab`), `libduel_draw/dd_field_events.c` (`dd_slot_xform`) | protótipo local e definição no mesmo arquivo com tipos diferentes (`unsigned short` vs `int`, struct por valor) |

`-fsyntax-only` só olha um arquivo por vez. O problema maior está **entre arquivos**: cada `.c` declara os
protótipos que usa, e as declarações divergem. Classificação (`tools/proto_conflicts.py` de todos os módulos,
por função chamada; um nome repetido em vários gmodules conta uma vez):

| Classe | Funções | Efeito no PC |
|---|---|---|
| só ponteiro/inteiro de mesmo tamanho | ~1006 | nenhum em 32 bits; limpeza de tipos |
| só o tipo de retorno difere (`void` vs `int` etc.) | ~219 | lixo lido de um retorno `void`; revisar |
| **posição de `float` diferente** (`f(float*, float*, float)` vs `f(float, float*, float*)`) | ~106 | **bug real no PC**: no EABI da PSP floats vão em registradores próprios e a ordem relativa a inteiros não importa; no x86 importa. Parte é SDK soft-float (`__adddf3` etc.), que o port troca |
| **número de argumentos diferente** | ~97 | **bug real no PC**: wrappers do original que repassam registradores (K&R); `tools/argaudit.py --all` lista as 73 chamadas |
| só K&R + uma assinatura | 15 | trocar o `f();` pela assinatura |
| struct por valor com tipos diferentes | 5 | `dd_cobj_spawn`, `dd_slot_xform`, `duel_0884F30C`, `duel_088AFD5C`, `duel_088CC8EC` |

Atenção: mudar a ordem dos argumentos `float` numa chamada pode mudar a ordem em que o mwcc monta os
argumentos (rodada 18) e quebrar uma função que já bate. Por isso a correção vai pelo protótipo comum num header e
é conferida com `make verify`.

### 1.1 Protótipos entre arquivos (passo 2, feito em 2026-11-11)

Gate: `make host-lto` compila cada `src/*/*.c` com `gcc -m32 -std=gnu89 -O0 -flto -DNON_MATCHING` e liga tudo junto
(`-r`) com `-Wlto-type-mismatch`, que compara cada declaração com a definição, entre arquivos e entre módulos (os
gmodules chamam o BOOT pela `libehsys`, então a definição do BOOT é a verdade). `tools/lto_mismatch.py` relê as
duas declarações de cada aviso no fonte e separa o que quebra a ABI do i386 mesmo em 32 bits (número de argumentos,
`float` fora de posição ou contra inteiro, struct por valor de outro tipo, retorno `float`) do resto. O gcc avisa uma
vez por variante de tipo, então corrigir uma variante pode revelar outra: repetir até zerar.

| Classe (`lto_mismatch.py`) | Antes | Depois |
|---|---|---|
| número de argumentos | 109 | 4 (pontos de entrada, abaixo) |
| posição de `float` / `float` contra inteiro | 25 | 0 |
| struct por valor | 11 | 0 |
| retorno `float` | 0 | 0 |
| benigno: retorno `void` contra valor | 22 | 24 |
| benigno: ponteiro/inteiro/sinal/largura | 303 | 305 |
| benigno: variável (tipo de global) | 48 | 48 |
| **total de avisos** | 518 | 381 |

Os 4 graves que ficam são pontos de entrada, que o port substitui: `main` (o `crt0` chama `main(argc, argv)`,
definido `void main(void)`), `module_start` de libcharalist/libshop e `module_entry` do libfield contra o de outro
gmodule. Junto com eles, 7 símbolos são definidos em mais de um módulo ou arquivo e precisam de nome próprio quando
os gmodules forem ligados num executável só: `cl_entry`, `dmgr_slot`, `drec_entry`, `module_entry`, `module_start`,
`res_get`, `voice_start` (os dois primeiros são globais sem `static` repetidos dentro do mesmo módulo).

Como foi corrigido sem mudar o que o mwcc compila (objeto de cada arquivo tocado conferido idêntico antes e depois, e
`make verify` → ALL MATCH):
- **Posição de `float`**: o mwcc avalia os argumentos na ordem do texto, então reescrever a chamada muda o objeto. A
  declaração local na ordem errada fica sob `#ifdef __MWERKS__` e no `#else` vai a da definição com um `#define` que
  reordena (`#define dl_tex_offset(list, u, v) dl_tex_offset(u, v, list)`). Os `chrNN_motion` passaram a ter a
  assinatura de callback de osso `(void *, void *, void *, int, float)` de `dd_mon_bone`.
- **Argumentos a mais** (o chamado não lê): mesma forma, com `#define` que descarta os excedentes.
- **Argumentos a menos** (o chamado lê o que estiver em a0-a3): lido no asm do original o que o registrador contém no
  `jal` e passado explicitamente no `#else` (ou direto, quando o objeto não muda). Casos: parâmetros do chamador ainda
  em a0/a1 (handlers `(DuelAct *, int)` da tabela de efeitos do `card_desc`, que ganharam o `int b` que não usam;
  exports da `dd_api`, handlers de evento do `dd_event_dispatch`, wrappers `bgm_*`, `snd_load_data(g)`), `a2 = p` em
  `duel_088AE548`, `a2 = link / 10` em `duel_eff_fbcd4`, o owner recém-gravado em `duel_pick_monster_zone`, `cur`
  preservado pela folha `duel_0884F930`, `p`/`z` dos alvos `(a, p, z)` para `duel_tributable`, `i` repassado por
  `duel_field_act`.
- **Struct por valor**: os eventos (`DuelMoveEv`, `DuelLpEv`, `DuelAtkEv`, `DuelActX`) vão como a palavra que a
  definição recebe; `DdLoc`/`DdLoc2`/`short` como `DdPos` (os mesmos 16 bits); `dd_event_dispatch` recebe palavras no
  host (todos os chamadores passam palavras).

Ficaram com valor escolhido (o original lê registrador não inicializado; marcados `port:` no fonte):
`op_wait` (n quando `CMD.b` não é 1-4; 0), `tfd_regulation_select` (`lim` quando o tipo de duelo não é 0 nem 2; 1).
Bugs do C achados: `_svfprintf_r` (0x088116A4) era o `_strtod_r` do newlib (renomeado; o corpo gdtoa ainda falta, o
rascunho não converte nada); o C do port de `libc_sfvwrite` passava `(reent, fp)` a `nl_swsetup(fp)`.

`tools/argaudit.py --all` (chamadas por protótipo K&R com outro número de argumentos) ignora agora o ramo
`#ifdef __MWERKS__` e não lista mais nada além de dois falsos positivos (linhas de definição).

### 1.2 Avisos do gcc contra o asm (passo 3, feito em 2026-11-11)

Gate: `make host-warn` roda `gcc -m32 -std=gnu89 -fsyntax-only -Wall -Wextra -DNON_MATCHING` em cada `src/*/*.c`,
sem as classes que são idioma de matching ou ruído de tipo (`-Wno-parentheses`, `-Wno-sign-compare`, `-Wno-unused`
etc., lista em `HOST_WARN` no Makefile), e imprime a contagem por classe (log em `build/host-warn.log`). Cada aviso foi
classificado lendo o asm do original (`tools/raw_asm.sh`) ou o objeto do mwcc, que é o original quando a função bate:
**fiel** (o original faz o mesmo), **bug do rascunho** (o C faz o que o asm não faz) ou **port** (certo na PSP,
indefinido ou errado no PC).

| Classe | Antes | Depois | Veredito |
|---|---|---|---|
| `-Wimplicit-function-declaration` | 159 | 0 | port: 155 chamadas ganharam o protótipo da definição (84 no `s_vmops`); `strcmp` em `libc.h`, `sceIoWrite`, `sceKernelDcacheWritebackInvalidateAll`, `sceKernelTryAllocateVpl`/`FreeVpl` em `psp.h`; as 4 restantes eram `__builtin_args_info` do `stdarg.h`, que no host usa o `va_list` do gcc |
| `-Wsequence-point` | 7 | 0 | port (RNG, abaixo) |
| `-Wincompatible-pointer-types` | 10 | 0 | fiel; tipos corrigidos (abaixo) |
| `-Wlogical-not-parentheses` | 50 | 50 | fiel: `!(x & 1) == 1` / `!= 1` é idioma de matching e tem a mesma semântica no gcc; os 9 em rascunho (`dd_fx32_s3_update`, `dd_fx_lines_update`, `dd_orbs_update`, `dd_trail_update`, `duel_0887F8E4`) diferem do original só em registradores/ordem |
| `-Wtype-limits` | 8 | 8 | fiel (abaixo) |
| `-Wint-to-pointer-cast` | 6 | 6 | port, não corrigido: `HeapEnt.addr` (`BOOT/heap.c`) tem 31 bits, então a arena do heap precisa ficar abaixo de `0x80000000` no host (anotado no fonte) |
| `-Wint-conversion` | 3 | 3 | fiel: `duel_0880F8C4`/`duel_088807F0` recebem um `DuelAct *` como `b`, que a definição ignora (passo 2) |
| `-Waddress` | 3 | 3 | fiel: `&r != 0` em `ehf_cmd_srt` (bate); `&crt0_init_hook != 0` em `crt0_run_init` é o teste do símbolo fraco, igual ao asm (0x08804364: endereço, depois `lw` e `jalr`) |
| `-Wreturn-type` | 2 | 2 | fiel: `state_noop_update`/`state_noop_exit` (`BOOT/deck.c`) são `jr ra` e `sys_state_update`/`sys_state_set_callbacks` ignoram o retorno |
| `-Woverflow` | 2 | 2 | fiel: `idx = 0xFFFF` num `unsigned char` (`duel_req_088D63FC`, `duel_req_088D9DBC`, ambas batem): o original grava `0xFF` |
| `-Wmemset-elt-size` | 1 | 1 | fiel (bug do original): `duel_net_poll` (bate) zera 4 bytes de `int duel_net_done[4]`, só o jogador 0 |
| `-Wbool-compare` | 1 | 1 | fiel (bug do original): `!duel_count_monsters(p) < 3` em `duel_ai_08820260` (bate; asm `sltu`/`xori`/`slti 3`): a função sempre devolve 0 |
| `-Wmain` | 1 | 1 | ponto de entrada (`void main(void)`), que o port substitui |
| **total** | 253 | 77 | |

Nenhum bug de rascunho apareceu: todos os avisos em código `NON_MATCHING` conferem com o asm. Detalhes:
- **Protótipos que mudam o objeto do mwcc**: com argumento ou retorno `unsigned char`/`unsigned short`, o protótipo faz
  o mwcc estreitar o valor (`andi`, `lbu` no lugar de `lhu`) ou realocar registradores. Esses ficam sob
  `#ifndef __MWERKS__` (`s_vmops` 14, `stt_branch_find` no `s_story`, `recipe_get_id` no `t_deckpick`,
  `duel_088BFFD8` no `ai_08864044`). Os chamados conferidos gravam o byte (`sb`), então estreitar no PC dá o mesmo
  resultado. No `s_vmops` os getters que devolvem ponteiro e o `st_text_at` ficam declarados com `int`, o tipo dos
  campos que recebem o valor (benigno em 32 bits).
- **RNG (`DD_RAND`)**: duas chamadas na mesma expressão não têm ordem definida em C. No asm, o mwcc sorteia o sinal
  (`DD_RAND(r) % 2 * 2 - 1`) antes da magnitude dos dois lados do `*` (`dd_fx32` ×4, `dd_burst_update` e o original de
  `dd_burst2_update`, `dd_fx_shatter_update` em 0x0882DA98), avalia os argumentos de `vec3_set` da direita para a
  esquerda (`dd_fx_shatter`) e sorteia o 6º argumento de `draw_sprite` antes do 5º (`dd_fx_noise_update`, 0x088307D8).
  O ramo `#else` de `__MWERKS__` usa temporários nessa ordem (`dd_rand_sgn` no `dd_fx_shatter`). A busca cobriu toda
  expressão com dois sorteios, não só as que o gcc avisa (`dd_fx_burst` e `dd_fx32` não avisavam).
- **`-Wtype-limits`**: `flight->idx < 0` com `idx` `unsigned short` (`dk_flight_undo`, `ds_input_move`,
  `dt_flight_undo`, `sc_input_move`, rascunhos): o original faz `lhu` + `bgez`, então o teste também nunca falha lá
  (quando a carta não é achada, `idx` fica `0xFFFF` e o fluxo segue); `v >= 0` com `v` sem sinal (`inv_set_limit`,
  `partner_count_set`), `play_counter < 0xFFFFFFFFF` (`frame_tick`) e `a->w[q2] < 0` (`duel_08899D34`) batem.
- **Tipos**: `struct EhfEmitter` declarada antes do `EhfPosFn` (`particle3.c`; antes ficava com escopo do protótipo);
  `DkRecipeCfg.save/load` com a assinatura de `RvParams.on_delete/on_select` no host; `dd_cobj_link_add/del` com o tipo
  do callback de `func_08820D40` no host; cast do callback de `duel_08865C10` (rascunho) para o tipo do parâmetro.

Ponto aberto para o `platform/`: o gcc em `-m32` usa x87 com precisão estendida; para os `float` darem os mesmos bits
que a FPU da PSP, compilar com `-msse2 -mfpmath=sse` (*inferido*, não testado).

**64 bits** (`-m64`, *inferido* como trabalho grande): 398 casts ponteiro→inteiro e 241 inteiro→ponteiro em 124
arquivos, além de structs com ponteiros que são carregadas de arquivo (EHP relocado no lugar). Começar o port em
`-m32` e tratar 64 bits como etapa própria.

## 2. API do firmware que `platform/` precisa cobrir

Só o BOOT importa do firmware: **274 funções em 28 bibliotecas** (`symbols/BOOT.txt`, `type:func import:`).
Os gmodules não têm imports de firmware próprios, mas o BOOT **reexporta** parte deles pela `libehsys`
(o export aponta direto para o stub): 18 funções `sce*` são chamadas de gmodules assim (Fpl, Module start/stop,
OSK, `sceGeEdramGetAddr`, `sceNetInit/Term`, `sceKernelChangeCurrentThreadAttr`).

| Biblioteca | Imports | Usadas no BOOT | Via gmodule | Sem chamada no C | Área do `platform/` | Prioridade |
|---|---|---|---|---|---|---|
| ThreadManForUser | 43 | 41 | 5 | 1 | thread/timer/sync | alta |
| IoFileMgrForUser | 33 | 13 | 0 | 20 | arquivos | alta |
| sceGe_user | 12 | 12 | 1 | 0 | gráficos | alta |
| sceDisplay | 6 | 6 | 0 | 0 | janela/frame | alta |
| sceCtrl | 3 | 3 | 0 | 0 | input | alta |
| sceAudio | 9 | 9 | 0 | 0 | áudio PCM | alta |
| sceSasCore | 21 | 21 | 0 | 0 | áudio (mixer de vozes) | alta |
| SysMemUserForUser, UtilsForUser, Kernel_Library, InterruptManager, sceDmac, StdioForUser | 23 | 19 | 1 | 4 | memória, cache, tempo | alta (quase tudo vira no-op ou stdlib) |
| ModuleMgrForUser, LoadExecForUser | 14 | 10 | 3 | 4 | carregador de gmodules | alta (no PC os gmodules são linkados juntos) |
| sceUtility | 23 | 19 | 6 | 0 | save, diálogos, OSK | média |
| sceLibFont | 7 | 7 | 0 | 0 | fonte do sistema | média |
| sceUmdUser, sceSuspendForUser, sceRtc, sceOpenPSID | 8 | 8 | 0 | 0 | stubs | baixa |
| sceMpeg | 25 | 25 | 0 | 0 | vídeo PMF | fase 7 (ffmpeg) |
| sceAtrac3plus | 9 | 9 | 0 | 0 | música Atrac3+ | fase 7 (ffmpeg) |
| sceNet, sceNetAdhoc, sceNetAdhocctl, sceNetAdhocMatching, sceWlanDrv | 38 | 38 | 2 | 0 | rede ad hoc | stub (sem rede) |

"Sem chamada no C": 29 imports sem chamada direta em nenhum `src/` (20 do `sceIo`, como `sceIoMkdir`,
`sceIoRemove`, os `*Async`). São reexportados pela `libehsys` e chamados de gmodules por endereço, ou não são
usados. Conferir antes de implementar.

**Gráficos: a fronteira não é o `sceGu`.** O `sceGu` (79 funções usadas, em 12 arquivos do BOOT e 7 de gmodules)
está linkado no BOOT e já é C do decomp. Mas `BOOT/dl.c` escreve comandos da GE direto na display list
(`0xD4000000 | ...`, `0xD5000000 | ...`, linha 241). Então o `platform/pc` tem duas opções: interpretar as
display lists da GE (como o PPSSPP; cobre `sceGu` e `dl.c` de uma vez) ou reescrever `dl.c` sobre uma API própria.
*Inferido*: interpretar a GE dá menos mudanças no código do jogo e mantém o build PSP idêntico.

## 3. Ordem sugerida

1. ~~Zerar o `make host-check`~~ (feito).
2. ~~Protótipos consistentes com a definição para as classes que quebram no PC~~ (feito, seção 1.1; gate
   `make host-lto`). Falta: unificar os ~380 avisos benignos num header por módulo (limpeza de tipos), o corpo de
   `_strtod_r` e nomes próprios para os 7 símbolos repetidos quando os gmodules forem ligados juntos.
3. ~~Revisar os avisos do gcc contra o asm~~ (feito, seção 1.2; gate `make host-warn`).
4. `platform/` em 32 bits: arquivos, thread/timer, input, janela, áudio PCM; gmodules linkados juntos.
5. Gráficos (decisão GE vs `dl.c`), depois save/OSK/fonte; rede e vídeo como stubs.
