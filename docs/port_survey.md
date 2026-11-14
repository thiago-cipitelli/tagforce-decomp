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
Os gmodules quase não têm imports de firmware próprios (exceções: 23 do `libhtml_view`, rede/HTTP/navegador, e
`sceRtcGetCurrentClockLocalTime` do `libdeck`; achado no passo 4a), mas o BOOT **reexporta** parte deles pela `libehsys`
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
| sceMpeg | 25 | 25 | 0 | 0 | vídeo PMF | feito (passo 7a, ffmpeg) |
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
4. `platform/` em 32 bits: ~~arquivos, thread/timer~~ (feito, seção 5, gate `make host-run`), input, janela, áudio PCM; ~~gmodules linkados juntos~~ (feito, seção 4, gate `make host-link`).
5. ~~Gráficos~~ (feito, seção 6: interpretador da GE), depois fonte (`sceFont*`), save/OSK, som de verdade (SAS,
   Atrac3+ por libavcodec); rede e vídeo como stubs.

Pendência achada na rodada 27: `debug_menu_draw` e `debug_menu_update` (libduel_draw, `dd_debug_menu.c`) não
têm C nenhum, só `@asm` (menu de debug inacessível no jogo de varejo). Se o port precisar dele, escrever do zero;
os protótipos do `#ifdef NON_MATCHING` do arquivo não batem com a tabela de chamadas de `dd_scene_mode.c`.

## 4. Passo 4a: todos os módulos num executável de 32 bits (feito em 2026-11-14)

Gate: `make host-link` (`tools/host_link.py`) liga os 681 `src/*/*.c` dos 23 módulos, os dados de cada módulo e o
`platform/pc` em `build/host/tagforce` (ELF i386, sem PIE) e imprime **`host-link: 0 undefined, 0 duplicate`**.
Nada em `src/`, `include/` ou `symbols/` mudou: o matching não é afetado. Eram 1295 indefinidos e 7 duplicados.

| Etapa | O quê |
|---|---|
| objetos | `gcc -m32 -std=gnu89 -w -O0 -g -fno-common -fno-pie -DNON_MATCHING` por arquivo → `build/host/obj/<m>/` (cache por data); cópias editáveis em `build/host/work/` |
| `inline` de header | `gnu89` emite uma definição externa de `inline` em cada objeto: `cl_entry` (libcharalist), `dmgr_slot` (libduel_mgr), `drec_entry` (libduelrecord) viram `weak` (cópias idênticas) |
| `static` chamado de outro arquivo | `objcopy --globalize-symbol` quando há um só: `snd_stereo_widen`, `snd_box_filter` (`static` só sob `NON_MATCHING` em `BOOT/snd_mix.c`, chamados de `snd_out.c`) e `duel_088A8B94` (libduel_eng; o matching resolve pelo linker script) |
| dados | `tools/host_data.py` → `build/host/data/<m>.s` (x86, `as --32`): `.data` e `.bss` do `build/elf/<m>.elf` em ordem de endereço; cada `R_MIPS_32` (lido das seções de relocação do PRX que o `prx2elf` deixa no ELF) vira `.long alvo`; rótulos globais para os `type:data` de `symbols/<m>.txt` e os `D_XXXXXXXX` pedidos que o C não define; `--selftest MODULE` remonta e compara cada palavra não relocada com o ELF |
| nomes sem `symbols/` | `extern ... nome; /* 0x08XXXXXX */` no C do módulo (`gu_debug_*`, `gu_one`, `crt0_init_hook`...), mais `__fdlib_version` (0x08873880, endereço no comentário de `libm.c`); `func_XXXXXXXX` vira `--defsym` para o nome do endereço em `symbols/` (`func_088047B8` = `_write` etc.) |
| apelidos dentro de tabelas do C | um `type:data` que cai dentro de um objeto que o C define (os `alias` de `symbols/`) vira `--defsym nome=objeto+off` no `ld -r` do módulo, para apontar para a tabela do C e não para a cópia (155 `--defsym` ao todo, contando os `func_`) |
| módulo | `ld -r` de objetos + dados → `build/host/mod/<m>.o` |
| repetidos entre módulos | ficam no BOOT; em cada gmodule viram `<m>_nome` (`objcopy --redefine-sym`; quem importa de um gmodule segue a linha `import:<lib>`): `module_entry` (13), `module_start` (8 + BOOT), `res_get` (libdeck_swap, libpassword, libselect_card), `voice_start` (libduel_draw contra BOOT). Tabela gerada em `platform/pc/modules.c` (`host_mod_syms`: módulo, nome original, endereço) para o carregador de gmodules do PC |
| firmware | `platform/pc/stubs.c` (`tools/host_stubs.py`, `--selftest` compila): 294 `sce*`/NIDs de todos os `symbols/*.txt` (não só do BOOT); 98 com o protótipo do `include/psp.h`, o resto `int nome()`; cada um loga o nome uma vez e retorna 0 |
| resto do `platform/pc` | `libc.c`: `platform_malloc/free/realloc/memalign` (sobre `__libc_*` da glibc), `libc_init/fini` vazios, `libc_thread_reent` → 0; `missing.c`: `debug_menu_draw/update` vazios |

Log por módulo (weak, globalize, estatística dos dados) em `build/host/host_link.log`; saída do `ld` em
`build/host/link.log`.

### 4.1 Achados

- **Ponto de entrada: o jogo tem `main`** (`BOOT/main.c`), então o `_start` da glibc chama direto o `main` do
  jogo; `module_start` (crt0) só cria a thread `user_main`, que chama `main`. Não foi preciso `platform/pc/main.c`.
- **Até onde roda:** `main` → `sys_init` → `gfx_init` → `sceGuStart` → `sceGuSetDither` → `gu_push`:
  SIGSEGV em `BOOT/libgu.c:63`. `sceGuStart` faz `list & 0x1FFFFFFF | 0x40000000` (espelho sem cache da PSP) no
  ponteiro da display list, que no PC é um endereço de pilha (`0xffffc32c` → lixo). Antes disso passam pelos stubs:
  Fpl, UMD, `sceIoOpen/Lseek/Read/Close`, `sceGeEdramGetAddr`, `sceGeListEnQueue`, event flag.
- **Endereços da PSP no C** (sem contar comentários): ~47 linhas com `0x08xxxxxx`/`0x09xxxxxx`, ~21 com VRAM
  `0x04xxxxxx`/`0x44xxxxxx`, 9 com `| 0x40000000` e 3 com `& 0x1FFFFFFF` (GU, `dl.c`, debug da GU, `font.c`,
  `ehp.c`, `gfx.c`; `libduel_eng/effects_ze.c`, `libduel_mgr/dmgr_main.c`). Constantes de VRAM do `gfx_init`
  (`sceGuDispBuffer(..., (void *)0x88000, ...)`) são offsets da EDRAM: o `platform/pc` precisa de uma EDRAM falsa
  e de máscaras de endereço que não destruam ponteiros do host.
- **O jogo intercepta a glibc:** o BOOT define 48 nomes da libc (`malloc`, `free`, `memcpy`, `sprintf`,
  `vfprintf`, `exit`, `_exit`, `qsort`, `strtod`, `setlocale`, `fread`...). Num executável dinâmico eles
  substituem os da glibc também para a própria glibc. O `malloc` do jogo cai em `platform_malloc` →
  `__libc_malloc`, o que funcionou até o crash acima; `exit`/`_exit` também são os do jogo.
- **Tabelas de salto:** 8416 de 17198 `R_MIPS_32` dos dados apontam para o meio de funções (tabelas de `switch` do mwcc,
  6 delas para `strtod_r_tail`, bloco frio sem C): viram `0` (o C do host tem os próprios `switch`). Ponteiros para
  função e imports: 6164 resolvidos por nome.
- **Dados duplicados:** quando o C define um global (`@data`), os bytes continuam na cópia do `.s` sem rótulo;
  ponteiros dos dados para ele vão para o objeto do C só quando `symbols/` tem o nome (4 casos); o resto aponta
  para a cópia (rótulos locais, 2614). Divergência possível se o C escrever num objeto que os dados referenciam.
  *Resolvido no passo 4b* (seção 5).
- **`.bss` somado: 84 MB** (heaps internos de cada gmodule no começo do `.bss`; na PSP os gmodules dividem a
  memória porque só alguns ficam carregados).
- `-fno-pie` é necessário: com PIE o `gcc` gera `__x86.get_pc_thunk.*` em cada módulo e o código do jogo usa
  endereços absolutos em toda parte.

## 5. Passo 4b: o binário passa da inicialização (feito em 2026-11-14)

Gate: `make host-run` (liga e roda `build/host/tagforce` por `HOST_RUN_SECONDS`, padrão 60, com
`TAGFORCE_ROOT=extracted`; passa se o processo ainda estiver vivo no fim; log em `build/host/run.log`). Num worktree,
`extracted/` é um link para o repositório principal: `cd extracted && ../build/host/tagforce` roda o binário **do
repositório principal**; usar `make host-run` ou `TAGFORCE_ROOT`.

**O que roda (180 s sem falha, ~1% de CPU):** `main` → `sys_init` → `efs_init` → `font_init` (carrega `libfont.prx`,
no-op) → `snd_init` (threads de som) → `ehscript_start("libtitle")` → laço principal a 30 fps (`gfx_state.fps` 30).
O `libtitle` abre `msd.ehp`, `cardinfo_eng.ehp`, `title_menu_e.ehp`, tenta o vídeo `opening_a.pmf` (sceMpeg em stub:
falha e segue), fica na tela de título e, sem botões, entra no **duelo de demonstração**: descarrega o `libtitle`,
carrega `libduel_mgr`, `libduel_draw`, `libduel_eng`, lê `duel/start/start_all.ehp`, `basic_ue.ehp`, `cutin_*.ehp`,
`bg/fmagic0000.ehp` e as imagens de carta (`cardh_e.cip`, `cardm_e.cip`, 92 aberturas de cada), e volta ao título.
Nada é desenhado (as display lists são montadas e nunca executadas, passo 5) e não há som.

| Arquivo | O quê |
|---|---|
| `platform/pc/kernel.c` | ThreadManForUser em pthreads, tempo, `sceDisplayWaitVblank*` (60 Hz pelo relógio), `sceCtrl*` (sem botões, `Read` espera o vblank), saída de áudio que só dá o ritmo (44,1 kHz) à thread de som, EDRAM falsa de 2 MB (`sceGeEdramGetAddr`) |
| `platform/pc/io.c` | `sceIo*` sobre POSIX; `disc0:`/`umd0:` → `TAGFORCE_ROOT` (padrão `.`), `ms0:` → `TAGFORCE_SAVE` (padrão `~/.local/share/tagforce/ms0`), caminho sem dispositivo → `PSP_GAME/SYSDIR`; `*Async` síncronos com o resultado guardado para `sceIoWaitAsync`; `sceUmd*` sempre pronto; `TAGFORCE_TRACE_IO` mostra cada tradução |
| `platform/pc/module.c` | `sceKernelLoadModule` guarda o nome; `sceKernelStartModule` chama o `module_start`/`module_entry` do gmodule pela tabela `host_mod_syms` (passo 4a); módulos da Sony viram no-op |
| `tools/host_stubs.py` | não gera stub para nome que outro `platform/pc/*.c` define |
| `tools/host_link.py` | compila todo `platform/pc/*.c` (com `-iquote include`: o `include/stdarg.h` escondia o do sistema), `-mno-sse`, cache de objetos refeito quando as flags mudam |
| `tools/host_data.py` | objetos que o C define não são mais copiados (abaixo) |

**Escalonador (`kernel.c`).** A PSP tem um núcleo e escalonamento estrito por prioridade, e o jogo depende disso:
`ehscript_yield_to` acorda a thread de cena, que roda um quadro e acorda a principal; a principal (prioridade maior)
toma a CPU e suspende a cena no meio da `sceKernelWakeupThread`. Modelo: **uma trava global é a CPU**; uma thread só
roda com ela e só a solta numa chamada que bloqueia; volta de uma espera só quando é a thread pronta, não suspensa,
de prioridade mais alta (número menor). Qualquer chamada que deixa pronta uma thread de prioridade maior (signal, set,
wakeup, start, resume, change priority) cede a CPU na hora. Um `pthread_cond` único com broadcast a cada mudança;
o fim de um delay não preempta a thread que está rodando (é visto no próximo ponto de escalonamento). Threads: 5
(principal 0x20, cena 0x21, ehfile 0x6F, som 0x10 e 0x11). `TAGFORCE_TRACE_SCHED` registra cada troca.

### 5.1 Mudanças em `src/` e `include/` (só o host muda; mwcc compila os mesmos tokens)

- `include/libgu.h`: `PSP_PHYS(p)` (`& 0x1FFFFFFF`) e `PSP_UNCACHED(p)` (`| 0x40000000`), identidade fora do mwcc;
  usados em `sceGuStart` (`libgu.c`) e no `sceGuDebugFlush` (`libgu_ctx.c`). Eram o segfault do passo 4a.
- `include/allegrex.h`: `ALLEGREX_UNCACHED(a)` = `__builtin_allegrex_ins(a, 4, 28, 4)` no mwcc, identidade no host;
  usado em `dl_init` (heap das display lists) e na cópia sem cache de `gfx_ge_defaults` (`gfx.c`).
- **Funções `void` cujo chamador usa o `$v0` da chamada final** (para bater, o C delas é `void`; no MIPS o valor da
  última chamada fica em `$v0`): sob `#ifndef __MWERKS__` passam a devolvê-lo: `dl_alloc_inline` (resultado de
  `dl_push`: os bytes reservados), `gfx_draw_target` (resultado de `dl_select_layer`), `gfx_set_draw_target`,
  `gfx_set_default_target`, `card_draw` (`card_draw_card`/`cardlist_draw_quad`). O `make host-lto` classifica essa
  classe como "benigno: retorno void" (24 casos) e não vê estes cinco (as declarações divergentes estão no mesmo
  arquivo ou não se cruzam); cada um derruba o port no primeiro uso. Das 24 do `host-lto`, só duas têm a
  definição `void` e uma declaração com valor: `se_play_pan` (`snd_play.c`, declarada `int` em `sndapi.c`) e
  `dd_fx2_set_param` (`dd_fx_slot.c`, `int` em `include/dd_duelui.h`); conferir se o chamador usa o valor quando
  o caminho chegar nelas. As outras 22 definem um valor que o chamador ignora (benignas de fato).

Matching: `make MODULE=BOOT verify` → ALL MATCH (inclui `check-all` dos 23 módulos); `tools/check_file.py` dos 5
arquivos de `src/BOOT` mexidos: todas as funções batendo.

### 5.2 Achados

- **Dados do host sem cópia dupla (tarefa 1 do passo):** o endereço de cada global ou `static` de nome único que o C
  define vem de `symbols/` ou da tabela de símbolos do link de matching (`build/<m>.rebuilt.elf`); os bytes e as
  relocações dentro desses objetos saem do `.s` (69527 bytes, 1544 relocações, que eram os ponteiros das tabelas do
  próprio C). Nenhum dado em asm aponta para objeto do C: os 1070 rótulos locais que sobram são asm → asm.
  `--selftest` continua batendo (sem objetos do host nada é tirado).
- **Endereços fixos da PSP no C:** fora de comentários, só sobram palavras de comando da GE: `0x08000000 | addr &
  0xFFFFFF` (JUMP), `0x09000000` (BJUMP), `0x10000000 | (addr >> 24 & 0xF) << 16` (BASE) em `dl.c`, `libgu*.c`,
  `font_tags.c`, `ehf_prim.c`, `gfx.c` (lista-modelo com `0x08000178`) e `font.c` (`glyph_cmds_template`). Elas
  guardam 28 bits do ponteiro: no host os ponteiros vão de `0x08048000` (código/dados) a `0x0Dxxxxxx` (heap) e
  `0xF7xxxxxx` (pilhas), então o interpretador de GE do passo 5 tem de manter a base do lado do host. As constantes de
  VRAM (`0x88000`, `0x110000`, `0x160000`, `0x1F0000`...) são offsets somados a `sceGeEdramGetAddr()` e funcionam com
  a EDRAM falsa. `sndapi.c:152` testa `(int)name >= 0x08000000` para separar nome de arquivo de handle: vale no host
  para strings dos dados (`0x08xxxxxx`); um nome na pilha (`0xFFxxxxxx`, negativo) falharia.
- **31 bits de endereço no heap HOGE** (`HeapEnt.addr : 31`): o `malloc` grande da glibc usa `mmap` (`0xF7xxxxxx`).
  `mallopt(M_MMAP_MAX, 0)` no início deixa tudo no heap do `brk`, abaixo de `0x80000000`.
- **`-mno-sse`:** as `Mat4`/`Vec4` são `aligned(16)` em `vfpu_mat4.c` mas não em todos os chamadores; o gcc copiava
  com `movaps` e caía em matriz de pilha desalinhada. Os floats já eram x87 (`-mfpmath=387`), nada muda nos cálculos.
- **`umd1:`** (disco cru em setores): `ehfile_build_lbn_table` lê o descritor de volume e o diretório raiz para trocar
  nomes por `disc0:/sce_lbn0x..._size0x...`. O host entrega um volume vazio (PVD, tabela de caminhos, raiz sem
  entradas): toda busca falha e o jogo abre os arquivos pelo nome, como faz quando a tabela não tem o arquivo.
- **Bugs do original que a PSP tolera:** `movie_run` restaura a prioridade com `prio` não inicializado quando o vídeo
  não abre (`sceKernelChangeThreadPriority` agora recusa prioridade fora de 1-126, como o firmware);
  `dmgr_load_draw` passa `&status`/`&argp` locais a um `sceKernelStartModule` que a thread do ehfile faz depois do
  retorno: na PSP escreve em pilha morta, aqui escrevia no quadro vivo da thread de cena (o `module.c` não escreve
  `*status`; ninguém o lê).
- Stubs ainda chamados no caminho: GE (`sceGeListEnQueue`, `sceGeListSync`...), `sceFont*` (fonte zerada), `__sceSas*`
  e `sceAtrac*` (som), `sceMpeg*` (vídeo), `sceUtilitySavedata*` (status 0, o jogo segue), `sceDisplaySetMode/
  SetFrameBuf`, `sceKernelDcacheWritebackInvalidateAll`, `sceKernelStdout`.

## 6. Passo 5a: a GE desenhada em OpenGL (feito em 2026-11-14)

Decisão (do brief): interpretar o fluxo de comandos da GE, sem reescrever `sceGu`/`dl.c`. O jogo monta as display
lists como na PSP e as entrega a `sceGeListEnQueue`; o `platform/pc` executa os comandos e desenha com OpenGL 3.3
core. **Nada em `src/` muda o que a GE recebe.**

**O que aparece (quadros despejados com `TAGFORCE_DUMP`, conferidos um a um):** logo da Konami com o fade, tela de
título completa (arte, logo, "PRESS START BUTTON" piscando, copyright), o menu do título com Start (itens sem texto),
a tela "NORMAL DUEL / Versus" com os retratos, as apresentações 3D dos personagens (modelos com skinning, toon,
disco de duelo, fundos de linhas), o campo do duelo (tabuleiro iluminado, fundo 3D com textura DXT1, cartas na mão e
no campo com as imagens DXT dos `.cip`, LP, fases "Draw/Standby/Main Phase", "Next Player's Turn"), cartas em
close (ATK/DEF) e a volta ao título depois de ~3600 quadros de demonstração (o limite do `dmgr`). Oito quadros
exportados em `build/host/frames_5a/*.png` (gitignored). ~30 fps com a CPU de vértices em `-O2`.

| Arquivo | O quê |
|---|---|
| `platform/pc/ge.c` | fila de listas (`EnQueue`/`EnQueueHead`/`UpdateStallAddr`: roda até o stall ou o END, depois a próxima), JUMP/CALL/RET/BASE/ORIGIN/OFFSETADDR, SIGNAL (handler, CALL/RET/JUMP por sinal) e FINISH com os callbacks do `sceGeSetCallback` chamados depois do interpretador (como a interrupção); registradores em `reg[op]`; matrizes (osso/mundo/vista/projeção/texgen), morph, CLUT (`LOADCLUT` copia), transferência de blocos; PRIM: vértices decodificados e transformados na CPU (morph, skinning, mundo/vista/projeção, iluminação da PSP, texgen UV/projeção/shade map, viewport/offset, fog), tiras/leques/sprites viram triângulos, shade flat copia a cor do último vértice; um shader faz a função de textura (modulate/decal/blend/replace/add, TCC, dobro), especular, fog e alpha test; blend/depth/stencil/scissor/máscara de cor/culling no estado do GL; texturas 5650/5551/4444/8888, CLUT4/8/16/32 (shift/mask/start), swizzle, DXT1/3/5 no layout da PSP, cache por endereço+formato+CLUT com hash do conteúdo uma vez por quadro; um FBO RGBA8 + depth/stencil por (offset na EDRAM, largura): é o alvo de desenho e a textura de quem amostra aquele endereço; `sceDisplaySetFrameBuf` apresenta o FBO do endereço e despeja o quadro; EDRAM falsa de 2 MB alinhada em 4 MB (o `& 0x1FFFFF` do endereço dá o offset, como na PSP) |
| `platform/pc/window.c` | janela X11 + contexto GLX 3.3 core (libX11/libGL de 32 bits do sistema), escala inteira (`TAGFORCE_SCALE`, padrão 2), teclado → botões (setas, Z=×, X=○, A=□, S=△, Q/W=L/R, Enter=Start, Backspace=Select), fechar a janela = `exit(0)`, `TAGFORCE_PAD` (botões roteirizados por quadro) |
| `platform/pc/atrac.c` | `sceAtrac*` sem decodificador: silêncio com a duração e os loops do cabeçalho RIFF (`fact`, `smpl`) |
| `platform/pc/kernel.c` | a EDRAM saiu (foi para `ge.c`); `block()` solta o contexto GL; `sceCtrl*` lê `host_buttons()`; `__sceSasGetEndFlag` = todas as vozes acabaram; `TAGFORCE_RTC` fixa o relógio que semeia o `rand()` |
| `tools/host_link.py` | `-O2` no `platform/pc`, `-lX11 -lGL`, version script que não exporta nada |
| `tools/ppm_sheet.py` | quadros PPM → PNG em grade (meia escala) ou inteiros (`--full`), para olhar dezenas de quadros de uma vez |
| `Makefile` | `make host-run` roda sem janela (`TAGFORCE_HEADLESS=1`) por padrão |

Variáveis: `TAGFORCE_HEADLESS=1` (janela não mapeada; o GL desenha e o despejo funciona; sem display nenhum, não há
GL e a GE só segue as listas), `TAGFORCE_DUMP=dir/f%05d.ppm` e `TAGFORCE_DUMP_EVERY=N`, `TAGFORCE_PAD="400:8,500:4000"`
(quadro:máscara hex[:quadros]), `TAGFORCE_RTC=1` (rodadas repetíveis: mesmo duelo de demonstração; o tempo das
threads ainda varia), `TAGFORCE_GE_TRACE=<quadro>` (uma linha por draw daquele quadro), `TAGFORCE_GE_SKIP=<n>`,
`TAGFORCE_GE_SKIP_VT=<hex>` (tira draws para achar quem desenha o quê).

### 6.1 Mudanças em `src/` e `include/` (só o host muda; mwcc compila os mesmos tokens)

- `include/libduel_draw.h`: `DD_OBJ_DT` (vazio no mwcc, `float dt_,` no host) na frente dos parâmetros de
  `lpc_update`, `lps_update` (`dd_lp_digits.c`, `dd_lp_show.c`, `dd_lp_panel.c`), `dd_mark_glide` e
  `dd_arrow_update` (`dd_arrow.c`). `dd_tasks_update` chama `update(dt, task, data)`; no EABI da PSP o float vai em
  `$f12` e esses callbacks do tipo `DdObj`, escritos sem ele, recebem `(task, data)` em `a0/a1`. No x86 o `dt` ocupa
  o primeiro slot: `lps_update` escrevia no próprio nó da lista e o `lps_free` seguinte fazia `dd_free(NULL)`
  (segfault na primeira mudança de LP do duelo). Classe nova, que o `host-lto` não vê: **float antes de inteiros num
  ponteiro de função chamado com outro protótipo** (os outros callbacks da lista declaram `float dt` primeiro).

`make MODULE=libduel_draw verify` → ALL MATCH (com o `check-all` dos 23 módulos); `make host-check` 0 falhas;
`make host-lto` 4 severos (os mesmos de antes); `make host-run` vivo depois de 60 s.

### 6.2 Achados

- **Não há SDL de 32 bits na máquina** (`lib32-sdl2` não existe nos repositórios, `lib32-sdl3` não está instalado; sem
  sudo). A janela é X11/GLX direto, isolada em `window.c` (cinco funções), para trocar por SDL quando houver um.
- **O jogo define `mtx_init`** (uma função de matriz) e mais ~50 nomes da libc/libm (`malloc`, `exit`, `strtod`,
  `sinf`...). Num executável dinâmico eles interpunham os da glibc também para as bibliotecas: o Mesa chamava o
  `mtx_init` do jogo em vez do C11 e caía no primeiro draw. O version script `{ local: *; }` resolve: o jogo usa os
  dele, as bibliotecas os da glibc (o código do `platform/pc` continua chamando os do jogo, ligados estaticamente).
- **Endereços:** as listas da GE guardam 24 bits + 4 (BASE) ou 8 (TBW, CBW, ZBW do `dl.c`) bits altos. Os ponteiros do
  host cabem em 28 bits fora da pilha (`0x08...`-`0x0D...`); para listas na pilha (`gfx_init` monta uma em
  `0xFFxxxxxx`) o nibble alto vem do ponteiro da lista atual quando os bits 24-27 coincidem. Nenhum caso errado visto.
- **Culling:** com a linha 0 do GL = linha 0 da PSP (imagem de cabeça para baixo para o GL), `CULL 0` → `GL_FRONT`;
  ao contrário os olhos e a boca dos personagens saem pretos. Tiras: os triângulos ímpares são invertidos antes.
- **A barra vertical escura no meio da tela** (nos personagens e nas janelas de mensagem) é texto: os glifos vêm do
  `sceFont*` em stub (métricas zero), todos no mesmo x. Some com `TAGFORCE_GE_SKIP_VT=800182`. Não é da GE.
- **O duelo de demonstração parava na primeira apresentação** (também no binário do passo 4b): `dd_chr_anim_done`
  espera `voice_is_playing` → o objeto de som da fala acabar, e com os stubs de Atrac o fluxo nunca termina. O
  `atrac.c` dá a cada fluxo a duração do cabeçalho; `__sceSasGetEndFlag` diz que todas as vozes SAS acabaram.
  Com isso o demo vai do título ao duelo e volta ao título. O `SetLoopNum` de um fluxo sem loop falha como no
  firmware (o mixer lê "0 loops" como "termina"; antes disso um fluxo sem loop prendia a thread de som num laço).
- **Toon (`toon.c`):** as rampas são desenhadas em 0x1F0000 em 5650 e 4444 e amostradas como 4444, com shade map
  (TMAP 2). Nenhum draw com TMAP 2 apareceu nos quadros rastreados do demo; quando aparecer, o FBO é RGBA8 e é
  amostrado direto (`ge: frame buffer sampled with a non-8888 format` no log), não a reinterpretação de bits da PSP.
- **Não conferido contra referência** (PPSSPP não instalado): o fundo cinza com a silhueta vermelha do campo do
  Chazz, as cores do ícone de "loading" e o fator dos blends `2×alfa` (aproximados por `alfa`). Nenhum comando
  desconhecido no caminho título → duelo; nenhum BEZIER/SPLINE, color test ou logic op apareceu (o único aviso
  do log foi DXT, já implementado).
- **Travamento intermitente:** em 3 de ~15 rodadas os quadros pararam de ser apresentados (o processo seguia vivo);
  as rodadas debaixo do gdb e as últimas (depois das correções do Atrac e do `DD_OBJ_DT`) não repetiram. Não
  reproduzido; se voltar, `TAGFORCE_RTC=1` + gdb com `SIGINT`.
- Com o Start no título e × o jogo abre o menu e entra na checagem do save (a janela de mensagem com o texto
  empilhado, à espera do `sceUtilitySavedata*`): o caminho do modo história depende de fonte e save.

## 7. Passo 5b: texto, save e o começo do modo história (feito em 2026-11-14)

Duas peças do firmware que o caminho título → história exigia, as duas só em `platform/pc/` (**nada em `src/` ou
`include/` muda**; `make host-check` 0 falhas, `make host-lto` 4 severos, os mesmos; `make host-run` vivo).

**O que aparece (quadros despejados e conferidos):** o menu do título com "New Game"/"Continue" em texto; a tela
"ENTER NAME" com "Press the × button to change your name." e o pedido de confirmação ("Yes"/"No"); o MAIN MENU
(Story Mode, Free Duel, Deck Menu, ..., Save Data) e o submenu da história ("Start from Part 1"); "1st May (Mon.)
Transference first day."; o mapa do campo (`libfield`, `bg_11_01.ehp`, sprite do jogador) com o HUD ("Chancellor's
Office", "1st May Monday", relógio, "7:30 AM", "0 DP", "92 Days Until Tournament"); o primeiro diálogo da história
(`libstory`, `st01_t01_e.ehp`) com o busto do diretor e a janela "You must be [nome]. Welcome to Duel Academy.";
"Save Data" no menu principal grava `DATA0` (64 KB) e, na rodada seguinte, a checagem do título (GETSIZE) o
encontra (resultado 0). O caminho "Continue" não foi conferido: o Down roteirizado no menu do título não registra
com 1, 2 ou 4 quadros e move duas vezes com 6 (o `pad_update` do BOOT roda no laço principal, não por quadro
apresentado; a escala quadro-apresentado → leitura do pad fica para o próximo passo). Quadros em
`build/host/frames_5b/*.png` (gitignored).

| Arquivo | O quê |
|---|---|
| `platform/pc/font.c` | `sceFont*` sobre a FreeType de 32 bits do sistema (`/usr/lib32/libfreetype.so`, headers em `/usr/include/freetype2`) com um TTF livre: `TAGFORCE_TTF` ou, por ordem, Liberation Sans e DejaVu Sans; tamanho `TAGFORCE_TTF_PX` (padrão 18 px). `NewLib`/`Open` devolvem handles fixos; `GetFontInfo` dá as métricas do tamanho (26.6: ascender, descender, altura = ascender + descender, avanço máximo; bpp 4); `GetCharInfo` dá `bitmap_w/h`, `bitmap_left/top` (do `horiBearing`), `advance_h` (26.6); `GetCharGlyphImage` escreve a cobertura em 4 bpp (formato 0, nibble baixo = x par; formato 1 invertido; 2 = 8 bpp) no topo-esquerdo `(x64, y64) >> 6` do buffer, recortado em `buf_w` × `buf_h`; glifo ausente usa o código de `SetAltCharacterCode` (0x25A1). `TAGFORCE_FONTDIR` (os `.pgf` da PSP) é só um gancho: loga e cai no TTF |
| `platform/pc/utility.c` | `sceUtilitySavedata*`, `sceUtilityMsgDialog*` e `sceUtilityOsk*` sem interface: `InitStart` faz o trabalho na hora e deixa o resultado no cabeçalho comum (+0x1C); `GetStatus` devolve 1, 2, 3 (o jogo chama `ShutdownStart`), 4 e 0; `Update` não faz nada. Save em `TAGFORCE_SAVE/PSP/SAVEDATA/<gameName><saveName>/<fileName>` (`ULUS101360001/DATA0`), os bytes exatamente como o jogo passa (sem cifra, sem PARAM.SFO nem ícones). Modos: AUTOLOAD(0)/LOAD(2) → lê (`0x80110307` sem arquivo); AUTOSAVE(1)/SAVE(3) → grava; DELETE(7)/AUTODELETE(9); SIZES(8) → espaço livre fixo e o uso deste save em `utilityData`; GETSIZE(22) → `0x80110327` (`RW_NO_DATA`) sem arquivo, `neededKB` 0. MsgDialog loga o texto ou o código de erro e responde YES/OK; OSK devolve o texto de entrada (`TAGFORCE_OSK=texto` "digita" outro) |
| `platform/pc/io.c` | `host_save_dir`/`host_make_parents` exportados para o `utility.c` |
| `tools/host_link.py` | `-I/usr/include/freetype2`, `-lfreetype` |
| `tools/ppm_sheet.py` | o cabeçalho do PPM é lido por regex: o `split()` comia o primeiro byte de pixel quando ele valia 9, 10, 13 ou 32 e o quadro inteiro saía com os canais rodados (parecia um bug de cores intermitente do renderizador) |

### 7.1 Achados

- **Como o jogo usa a `sceFont` (`BOOT/font.c`):** abre a fonte 0 da biblioteca (na PSP, `flash0:/font/ltn0.pgf`;
  com a região US `font2 = font`, os glifos de meia largura não usam outra fonte), pede o estilo 20×20 e lê as
  métricas uma vez: `line_h = max_glyph_h`, `baseline = max_ascender + 1 px`. Cada glifo é renderizado num buffer de
  32×32 a 4 bpp (`bytes_per_line` 16) em `(8 + bitmap_left, 8 + baseline − bitmap_top)` px e copiado para um
  atlas de 16×16 células (textura T4 swizzled de 512 px de largura: uma célula = um bloco de 16 bytes × 4 blocos de
  8 linhas); o quad do glifo é a região 28×28 a partir de (1, 1) da célula, desenhada em `(pen − 7, topo − 7)` px e
  escalada por `size/20`. Logo `(x64, y64)` é o topo-esquerdo do bitmap do glifo e cada glifo tem de caber em 28×28
  com a linha de base em `ascender + 1`: 18 px de Liberation Sans (ascender 17, descender 4, linha 21) cabe; o
  tamanho real dos `.pgf` da PSP não foi conferido (não estão no disco).
- **`utilityData` do modo SIZES:** o jogo reserva 0x1C bytes e copia 8 bytes de `+0x14` como o *texto* que imprime
  nas mensagens "precisa de %s" (`msd_code`); o SDK público põe um `usedKB32` inteiro em `+0x14` e `usedString32`
  em `+0x18` (0x20 bytes). O `utility.c` segue o jogo e escreve a string "N KB" em `+0x14`.
- **Resultado 2 do `sceUtilitySavedata`:** `msd.c` trata `result == 2` como "repetir" (`abort_status` no lugar);
  nunca é devolvido aqui.
- **Fluxo roteirizado** (`TAGFORCE_RTC=1`; quadro:máscara): `400:8` Start, `520:4000` × em New Game, `700:8`
  Start na tela do nome, `780:4000` confirma, `900:4000` Story Mode, `1000:4000` Start from Part 1 → mapa em ~1100,
  diálogo em ~1200. Save: `900:10` (Up, vai a Save Data), `980:4000`.
- GE: um aviso novo no campo, `blend with two different fixed colors` (fator fixo diferente para fonte e destino;
  aproximado), além do DXT já implementado.

## 8. Passo 6: som (SAS, Atrac3+ pelo ffmpeg, ALSA) e dois ajustes de entrada (feito em 2026-11-14)

Três peças do firmware, só em `platform/pc/` (**nada em `src/` ou `include/` muda**; `make host-check` 0 falhas,
`make host-lto` 4 severos, os mesmos; `make host-run` vivo), mais um ajuste no gerador de dados do host.

**O que toca (conferido pelo despejo `TAGFORCE_WAV`, RMS por segundo, sem ouvido):** a música do título (Atrac3+,
loop em 245626..1722583 amostras), a música do duelo (loop desde o começo, ~58 s), as falas dos personagens nas
apresentações e cut-ins (clipes Atrac3+ de 1 a 4 s) e os efeitos (vozes SAS, VAG do `psp_snddat.bin`: carta, cursor,
ataque). O trecho do título no WAV tem correlação 1,0000 com o PCM que o ffmpeg decodifica do mesmo arquivo AT3
(`build/host/snd6/`, scripts em `/tmp`): o caminho decodificador → `snd_streams_decode` → `wave_audio_write` →
`sceAudioOutputPannedBlocking` → WAV é só uma escala de volume. Com a janela e a ALSA real (PipeWire): canal aberto em
`default`, 25 s sem erro de escrita nem travamento.

| Arquivo | O quê |
|---|---|
| `platform/pc/audio.c` | `sceAudio*`: cada canal reservado é um fluxo PCM próprio na ALSA de 32 bits (`default`; o PipeWire ou o dmix misturam os fluxos, como o hardware da PSP mistura os 8 canais). `OutputBlocking`/`OutputPannedBlocking` escalam o bloco pelo volume (`× vol >> 15`), somam no WAV e escrevem no dispositivo **fora da CPU** (`host_cpu_leave`/`host_cpu_enter` do `kernel.c`: a thread sai do escalonador como numa espera e volta só quando é a pronta de maior prioridade; a de som é, então preempta no próximo ponto de escalonamento, como o fim de um delay). Sem dispositivo (`TAGFORCE_HEADLESS=1`, `TAGFORCE_AUDIO=0`, ou a abertura falhando) o relógio do host dá o ritmo, como antes. `TAGFORCE_AUDIO=<nome>` escolhe o dispositivo ALSA (também sem janela). `TAGFORCE_WAV=caminho` grava a mistura de todos os canais (cada um somado no próprio cursor, com saturação; cabeçalho reescrito a cada bloco, então o arquivo vale mesmo com `_Exit`). Na prática o jogo escreve um canal só (`wave_hw_ch[2]`, a thread de som; a `SceWaveMain` da libwave só escreve quando um canal de onda toca, e o jogo não usa nenhum) |
| `platform/pc/sas.c` | `__sceSas*`: o mixer de 32 vozes da PSP, seguindo o `Core/HW/SasAudio.cpp` do PPSSPP: VAG ADPCM (blocos de 16 bytes: filtro/shift, flag — 7 termina, 6 marca o começo do loop, 3 volta quando o loop está ligado —, 28 nibbles; os 5 filtros da PS1), reamostragem linear a `pitch/0x1000`, envelope ADSR de 0..0x40000000 andado uma vez por amostra de saída (os modos linear/curvo/exponencial, a decodificação das palavras ADSR1/ADSR2 "simples" que o jogo usa — `snd_voice_set_simple_adsr` em `snd_alloc.c` —, mais `SetADSR`/`SetADSRmode`/`SetSL`), volumes em 1/0x1000, `GetEndFlag` com o bit de cada voz parada, `GetEnvelopeHeight`, pausa por máscara. `__sceSasCore` escreve um grão (256 quadros) em `snd_mix_buf`; `CoreWithMix` soma ao que já está lá. Sem atraso de 32 amostras no key-on, sem vozes de ruído ou PCM, sem reverb (o jogo só usa o dry); `TAGFORCE_TRACE_SAS` registra os key-ons e os primeiros grãos com sinal |
| `platform/pc/atrac.c` | `sceAtrac*` com decodificação real: o jogo entrega a `sceAtracSetDataAndGetID` um arquivo RIFF/AT3 inteiro do banco de som; ele é hasheado (FNV-1a 64), decodificado uma vez pelo `ffmpeg` do sistema (64 bits, subprocesso por `posix_spawnp`: `-i <hash>.at3 -f s16le -ac 2 -ar 44100`) em `$XDG_CACHE_HOME/tagforce/at3/<hash>.pcm` (padrão `~/.cache/tagforce/at3`) e o PCM é `mmap`ado. A amostra 0 do jogo é a amostra `368 + fact[1]` do decodificador (o `FirstSampleOffsetFull` do PPSSPP para Atrac3+: o atraso do codificador, que o ffmpeg também entrega; conferido: `decoded − fact` = exatamente `368 + fact[1]` na música do título); os pontos de loop do `smpl` são amostras do decodificador e `sceAtracGetSoundSample` os devolve a partir da amostra 0, como o firmware — pode dar negativo (a música do duelo começa o loop em `fact[1]`: −368), e o `snd_obj_begin` do jogo trata isso (`if (loop < 0) loop = 0`). `DecodeData` dá 2048 quadros por chamada, enrolando no loop enquanto houver voltas (−1: sempre), o último quadro curto nos sem loop. Sem ffmpeg (ou decodificação falhando) o fluxo é silêncio com a duração do cabeçalho, como no passo 5a. `TAGFORCE_TRACE_ATRAC` registra cada fluxo (tamanho, `fact`, quadros decodificados, salto, loop) e deixa o `.at3` no cache para comparar à mão |
| `platform/pc/kernel.c` | a saída de áudio saiu (foi para o `audio.c`); `host_sleep_until`, `host_cpu_leave`/`host_cpu_enter`; `host_pad_polls` conta as leituras do pad (`sceCtrlPeek/ReadBufferPositive`); `TAGFORCE_TRACE_PAD` imprime a contagem contra o tempo e os quadros a cada 100 leituras |
| `platform/pc/window.c` | `TAGFORCE_PAD="leitura:máscara[:leituras],..."`: **a unidade passou a ser a leitura do pad, não o quadro apresentado**, e a duração padrão é 1 leitura. O `pad_update` do BOOT roda uma vez por volta do laço principal (`p.trig` = borda), então um Down de uma leitura move exatamente uma vez — conferido no menu do título (New Game → Continue → "There is no data from this game"). A sintaxe é a mesma; os números do passo 5b valem (no título há uma leitura por quadro apresentado, 30/s) |
| `platform/pc/utility.c` | OSK sem `TAGFORCE_OSK` e com o campo vazio "digita" `Player` |
| `platform/pc/font.c` | registra (uma vez por código, até 16) cada glifo que a TTF não tem e o substituto usado |
| `tools/host_data.py` | cada bloco `.data`/`.bss` copiado do ELF começa em `.balign 4096` + `.skip (endereço & 0xFFF)` e os objetos que o C define viram preenchimento em vez de buraco: **o endereço de cada objeto da cópia mantém a fase da PSP (mod 4096)**. Era o motivo de o SAS não tocar: `snd_sas_core` recusa um buffer de mistura que não esteja alinhado a 64 bytes (`0x80420005`); `snd_mix_buf` está em `0x08BF6680` na PSP e caía em `...824` no host. `--selftest` acompanha o deslocamento |
| `tools/host_link.py` | `-lasound` |

### 8.1 Achados

- **Como o jogo produz som (`BOOT/snd_out.c`, `libsas.c`, `libwave.c`):** a thread de som (`snd_thread_main`, prioridade
  0x10) a cada bloco de 256 quadros: `__sceSasCore` para `snd_mix_buf` (modo 0x10), efeito global, `snd_streams_decode`
  (decodifica os fluxos Atrac para os anéis de PCM e soma 256 quadros de cada no bloco, com pan), depois
  `wave_audio_write(2, vol, vol, buf)` → `sceAudioOutputPannedBlocking(wave_hw_ch[2], ...)` com o volume mestre
  (`m->cur << 1`). As vozes SAS: `snd_obj_begin` (`snd_alloc.c`) faz `SetVoice` (VAG cru do banco, sem cabeçalho
  `VAGp`), `SetPitch` (afinação com espalhamento aleatório), `SetVolume(0)` e `SetSimpleADSR`; `snd_voices_update`
  aplica o volume/pan e dá o `KeyOn` no bloco seguinte, e libera a voz quando `GetEndFlag` acende o bit. Nenhum
  `sceAudioOutputPanned` não bloqueante, `ChangeChannelConfig` ou `GetChannelRestLength` é chamado no caminho.
- **A música do duelo é masterizada no limite:** o PCM do ffmpeg bate em ±32768 a cada 5 s; no WAV o pico constante
  6543 é só `32767 × volume mestre` (0x1990/0x8000 ≈ 0,2), não saturação nossa. A do título fica em ~13700.
- **Envelope "simples" do jogo:** ataque curvo com taxa 0x1C000000 (2 amostras), decaimento exponencial com taxa
  0x7FFFFFFF (um passo até a metade), sustain linear a 0 (mantém) e release instantâneo: na prática a voz toca o VAG
  a meia altura e termina com o bloco de flag 7 ou o key-off. Mesma aritmética do PPSSPP (`WalkCurve`).
- **O loop da música do duelo falhava** na primeira rodada: o começo do loop (−368) era tratado como "sem loop" e
  `sceAtracSetLoopNum(-1)` era recusado; a música parava em 57,6 s. Agora um fluxo tem loop quando o fim do loop existe.
- **O nome do jogador não era um glifo de substituição:** é vazio num jogo novo (o `SysSave.name` que o título lê
  só é preenchido pelo OSK; `sysmsg 0x17`, "Default Settings", vai para outro campo, `player_name`). O ícone no campo
  é da arte da tela. Com `TAGFORCE_OSK=Jaden` (× na tela do nome abre o OSK) o nome aparece na TTF; sem a variável o
  OSK agora dá `Player`. Nenhum "font: no glyph" em todo o caminho título → história.
- **ffmpeg avisa "non monotonically increasing dts"** a cada pacote do demuxer WAV com Atrac3+; a saída está certa
  (correlação 1,0 com o que o jogo mistura); `-loglevel fatal` esconde.
- **Falta:** reverb do SAS (`RevType/RevParam/RevVON` aceitos e ignorados: só o dry sai; `snd_fx_setup` liga o wet
  quando uma cena pede um tipo de efeito), vozes de ruído e PCM (não usadas), o atraso de 32 amostras no key-on, `sceAudioOutputPanned` não
  bloqueante (tratado como bloqueante), vídeo PMF (feito no passo 7a), o
  `.at3` decodificado de forma síncrona na primeira vez (centenas de ms por clipe novo; o cache resolve nas seguintes).

## 9. Passo 7a: vídeo PMF (sceMpeg pelo ffmpeg) e o travamento "os quadros param" (feito em 2026-11-14)

Só `platform/pc/` muda (**nada em `src/` ou `include/`**; `make host-check` 0 falhas, `make host-lto` 4 severos, os
mesmos; `make host-run` vivo). Duas peças: o `sceMpeg` ao nível da API, e a causa do travamento intermitente do
passo 5a, que foi reproduzida, explicada e corrigida.

**O que toca (quadros despejados e conferidos; WAV conferido por correlação):** a abertura do título
(`movie/opening_a.pmf`, 89,5 s, 480×272 a 29,97 fps, Atrac3+ estéreo) inteira, do logo da Konami à arte final
"TAG FORCE", com a volta ao título e o duelo de demonstração depois; a mesma abertura uma segunda vez no mesmo
processo (depois do demo), e o pulo por botão (`TAGFORCE_PAD="240:4000:3"`: × na leitura 240 encerra o vídeo e o
título segue). O áudio do vídeo no `TAGFORCE_WAV` tem **correlação 1,0000** com o PCM que o ffmpeg decodifica do
fluxo Atrac3+ extraído à mão (ganho 0,80 = volume de BGM 12/15; o WAV começa 2048 amostras adiante: é o buffer
do `init` que o jogo descarta). **Clipe de duelo:** `movie/dm_attack.pmf` (ataque do Dark Magician, 6 s, 133
quadros de áudio) toca inteiro pelo mesmo tocador com `TAGFORCE_PMF=<arquivo>` (gancho de teste do `mpeg.c`: troca
o arquivo de qualquer filme que o jogo abra; aqui o do título), com o som no WAV e a volta ao título. Dentro de um
duelo o clipe sai pelo mesmo `movie_run` (`dd_movie_show`, `libduel_draw/dd_chara06.c`: bloco de `0x17CC00`, anel
de 160 pacotes, que cabe: 160 × 2152 + 2 quadros + som < 0x17CC00), mas não foi visto numa partida: ele só toca
quando o par (personagem, carta) do invocador está em `dmgr_cutin_attack`/`dmgr_cutin_effect`
(`libduel_mgr/dmgr_event.c`: Jaden + Flame Wingman 6344, Zane + Cyber Dragon 6485...) ou no zoom de carta dos
eventos de tutorial (cartas 4027 e 5224, `cfg_get_flag(0)`); nove demos de modo atrativo (`TAGFORCE_RTC=1..9`,
250-330 s) não invocaram nenhum, e o caminho Database → Tutorial mostra a lista `libtutoriallist` vazia (ver
achados). Quadros em `build/host/frames_7a/*.png` (gitignored).

| Arquivo | O quê |
|---|---|
| `platform/pc/mpeg.c` | `sceMpeg*` (todas as 25 que o `movie.c`/`pmf_umd.c` chamam). O anel (`SceMpegRingbuffer` de 0x80 bytes do jogo) guarda só capacidade, pacotes com dados, callback e argumento: `RingbufferPut` chama o callback do jogo (`pmf_read_blocks`, lê N blocos de 2 KB do arquivo) e soma o que ele devolveu; `AvailableSize` = capacidade − pacotes; cada `GetAvcAu`/`GetAtracAu` esvazia o anel até 1 pacote (a thread leitora do jogo segue alimentando até o fim do fluxo) e, quando os dois fluxos acabaram, até 0 (o laço principal termina em "leitor pausado e anel livre"). Nada do anel é demultiplexado: o arquivo por trás é achado pelo fd (`cbArg` é o `MovieFile` do jogo, fd na primeira palavra; `host_io_fd_path` do `io.c`). Vídeo: um `ffmpeg -i x.pmf -an -f rawvideo -pix_fmt rgba -fps_mode passthrough pipe:1` por filme, lido um quadro adiante em `GetAvcAu` (fim do pipe = `0x80618001`, sem dados) e copiado em `AvcDecode` para o buffer 8888 de 512 de largura do jogo (`got` = 1). Áudio: o fluxo privado 1 do MPEG-PS é demultiplexado aqui (andando pelos pacotes a partir do offset do cabeçalho PSMF: pack `BA` de 14 + stuffing, PES com tamanho de 16 bits, sub-cabeçalho privado de 4 bytes), os quadros Atrac3+ (`0F D0 ss ss 00 00 00 00`, tamanho = `(ss & 0x3FF) * 8 + 16`) perdem o cabeçalho de 8 bytes e vão num RIFF/AT3 com o `fmt` dos clipes do próprio jogo (`WAVE_FORMAT_EXTENSIBLE`, GUID do Atrac3+, block align 744 na abertura) para o `host_at3_decode` do `atrac.c` (cache `~/.cache/tagforce/at3/<hash>.pcm`, mmap); `AtracDecode` entrega 2048 quadros estéreo por chamada, zeros no fim. Timestamps como o PPSSPP: primeiro pts do cabeçalho (0x56, 90000 em todos os PMF), +3003 por AU de vídeo, +4180 por AU de áudio; `vid_sync_state` do jogo fica em "sincronizado". `QueryStreamOffset/Size` leem o cabeçalho PSMF (offset em 8, tamanho em 0xC, largura/altura em 0x8E/0x8F × 16); `QueryMemSize` 0x10000, `RingbufferQueryMemSize` pacotes × 2152, `QueryAtracEsSize` 2112/8192, `InitAu` pts/dts −1; `Delete` mata o ffmpeg e desmapeia o PCM. Sem ffmpeg: vídeo vazio e áudio em silêncio com a duração do fluxo (o jogo mostra nada e termina), sem travar |
| `platform/pc/kernel.c` | `sceKernelRegisterSubIntrHandler/Release/Enable/DisableSubIntr` para a interrupção 30 (vblank): uma thread do kernel de prioridade 0 (`vblank_intr`, criada no primeiro registro) chama os handlers a cada vblank; o `movie.c` registra `ctrl_vblank_handler`, que acende o event flag que a thread de display e o laço principal esperam (sem isso o vídeo nunca avançava). `thread_end` e `host_cpu_leave` soltam o contexto GL (`host_gfx_release`): a thread de display do filme desenha (`sceDisplaySetFrameBuf`) e depois morre; morrendo dona do contexto, o próximo `glXMakeCurrent` de outra thread dava `BadAccess` e matava o processo no fim do vídeo. **Correção do travamento** (abaixo) e `SIGUSR1` imprime o estado de cada thread do jogo (nome, prioridade, em que espera, objeto, deadline) |
| `platform/pc/ge.c` | `sceDmacMemcpy` real (memcpy) que anota o último endereço da EDRAM escrito pela CPU; `sceDisplaySetFrameBuf` desse endereço sobe os pixels (8888, 272 linhas, `bufw` de passo) para o FBO do offset antes de apresentar: é o caminho dos quadros do filme (`disp_present`: DMA para 0x88000/0x110000 e `SetFrameBuf`) |
| `platform/pc/io.c` | guarda o caminho do host de cada fd; `host_io_fd_path` |
| `platform/pc/atrac.c` | `decode` virou `host_at3_decode` (exportada) |

### 9.1 Achados

- **O travamento "os quadros param" (passo 5a, 3 em ~15 rodadas) era uma condição de corrida no `block()` do
  escalonador.** Reproduzido em 3 de 3 demos de 330 s (`TAGFORCE_RTC=1,2,3`, parando aos 97, 153 e 193 s, em telas
  diferentes: título, duelo, apresentação). O despejo por `SIGUSR1` mostrou `user_main`, `snd_thread_main` e
  `vblank_intr` todas em "delay" com o deadline 14 s no passado, e o gdb (o jogo precisa rodar *debaixo* do gdb:
  `ptrace_scope` impede o attach) as mostrou em `pthread_cond_wait` **sem timeout**. O `block()` lia o relógio
  duas vezes: `ready(me)` (deadline passou?) e depois `now < deadline` (dorme com timeout?). Quando o deadline
  vencia entre as duas leituras, a thread se achava "não pronta" e ia para a espera sem timeout, contando com um
  broadcast que só vem de quem chama `block()`; as outras acordavam, viam-na como a melhor (prioridade 0) e
  também esperavam por ela. Com a thread do vblank (um deadline a cada 16 ms) a janela era atingida em minutos.
  Correção: uma leitura do relógio por volta, usada nos dois testes (`ready_at`). Depois: 3 de 3 demos de 330 s
  inteiras (título → abertura → demo → título → abertura de novo), nenhuma parada.
- **Como o jogo toca um PMF (`BOOT/movie.c`, código de exemplo PSMF da Sony):** `movie_run` recebe o bloco de memória
  (5 MB no título, `0x17CC00` nos clipes de duelo), anel de `packets << 2` pacotes (256 e 160), `sceMpegCreate` com
  largura 512, modo 0; lê 2 KB do cabeçalho para `QueryStreamOffset/Size`; registra os fluxos AVC e Atrac; dois
  buffers de quadro de 0x88000 (512×272×4, `sceDisplaySetFrameBuf(buf, 0x200, 3, 1)` = 8888) e dois de som de
  `out_size` (8192: 2048 quadros estéreo, `wave_set_sample(0, 0x800)`); três threads (display 0x3E, leitora 0x40,
  som 0x3D; a principal passa a 0x3F) e um handler de vblank. Laço principal, por vblank: `GetAtracAu` →
  `AtracDecode` (o primeiro, `init` = 1, não é comprometido), `GetAvcAu` → `AvcDecode` (`got` = 1 compromete o
  quadro com o pts do AU, `((int *)au)[1]`); `0x80618001` dos `GetAu` é "sem dados" e é tolerado. O display começa
  quando os dois anéis estão cheios e o leitor parado (`movie_check_end`); a thread de display mostra um quadro a
  cada dois vblanks, segurando ou descartando pelo `pts` do som (`vid_sync_state`, ±2 quadros); fim = leitor
  pausado (todo o fluxo alimentado) e anel livre; depois `AvcDecodeStop` (`got` = 0 aqui) e as threads terminam
  com os flags 8/0xFF. `movie_play` (`sndapi.c`) sempre passa `flags | 1`: "mudo" só pula o `sceWaveInit`/`wave_exit`,
  o som sai por `wave_audio_write(0, vol, vol)` com o volume de BGM. Pular: qualquer botão com `flags & 2`
  (abertura), × ou Start.
- **PMF = cabeçalho PSMF de 2048 bytes + MPEG-PS:** todos os 47 `movie/*.pmf` são 480×272, estéreo 44,1 kHz,
  primeiro pts 90000, `offset + tamanho == tamanho do arquivo`. O ffmpeg demultiplexa o H.264 direto do `.pmf`
  (sem `-skip_initial_bytes`), mas **não vê o Atrac3+ do fluxo privado 1** (sub-id 0x00), daí o demux à mão.
  Abertura: 21897 PES de vídeo, 1940 quadros de áudio de 752 bytes (90,09 s) contra 89,5 s de vídeo.
- **Sem cache de quadros**: 90 s de RGBA 480×272 são 1,4 GB; o ffmpeg decodifica esse tamanho a centenas de fps,
  então o pipe (1 MB, ~2 quadros de folga) basta e o custo por `AvcDecode` é um `read` de 522 KB. O áudio inteiro
  é decodificado no primeiro `GetAu` (uma vez por máquina, depois cache), ~150 ms para a abertura.
- O `sceMpegRingbufferPut` real chama o callback com até `packets` pacotes e o jogo lê blocos inteiros; no fim do
  arquivo a leitura cai em 0 e o leitor para só por `total == stream_size`: por isso o tamanho do cabeçalho tem de
  bater com o arquivo (bate nos 47).
- **Roteiro de pad até o menu principal com a abertura tocando** (`TAGFORCE_RTC=1`, leitura:máscara): `240:4000:3`
  pula a abertura, `400:8` Start, `520:4000` New Game, `700:4000` abre o OSK (o nome vazio é recusado pelo Start:
  "A name must be entered"), `850:8` Start, `930:4000` confirma, `1050:40,1060:40` Database (o Free Duel
  desabilitado é pulado pelo cursor), `1100:4000`, `1200:40,1210:40,1250:4000` Tutorial. A lista do tutorial
  (`database/tutoriallist.ehp`) aparece com as abas Duel/Deck Edit e **sem entradas**: × não faz nada. Fica para
  conferir se é o texto da lista que não é desenhado ou se as entradas dependem de algo (save, região).
- **Variáveis:** `TAGFORCE_PMF=<arquivo.pmf>` toca esse arquivo no lugar do filme que o jogo abriu;
  `TAGFORCE_TRACE_ATRAC=1` deixa o `.at3` demultiplexado do filme no cache para comparar à mão.
- **Falta:** o clipe dentro de um duelo real (caminho e condições acima); `sceMpegAvcDecodeMode` (o jogo não chama:
  sempre 8888); `got` = 0 no `AvcDecodeStop` (o firmware pode devolver o último quadro atrasado do decodificador;
  o jogo só faria um `disp_commit` a mais); o cabeçalho de 2 KB e o offset são lidos do PSMF, mas o tamanho e a
  taxa de amostragem do áudio são fixos em 2 canais / 44,1 kHz (os 47 PMF são assim).

## 10. Passo 7b: primeiro teste com teclado de verdade (2026-11-14)

Só `platform/pc/` e `tools/host_link.py` mudam (`make host-check` 0 falhas, `make host-run` vivo). Achados de uma
sessão jogada à mão (título → história → Deck Edit → começo de um duelo no Slifer Red Dorm):

- **A janela num WM de tiling (Hyprland):** o WM ignora o tamanho pedido e o quadro saía num canto. `host_present`
  ajusta o quadro ao tamanho real (`ConfigureNotify`), com a proporção 480:272 e barras pretas.
- **"Cancel load?" no Continue:** o newlib do jogo define `fread` (`BOOT/libc_stdio.c`, com o `FILE` dele) e essa
  definição interpõe a do glibc no executável; o `fread` do `utility.c` sobre um `FILE` do glibc lia 0 bytes e o
  jogo acusava dado corrompido (`0x80110326`). O load usa `read()`; o `host_link.py` recusa objetos de
  `platform/pc` que chamem `fread`/`fwrite`/`fflush`/`fgets`/`fputs`/`fseek`/`ftell`/`getc`/`putc`/`ungetc`/`vfprintf`
  quando o BOOT os define. Com save, o cursor do título já começa em "Continue".
- **Moldura das cartas em blocos (Deck Edit):** sprite (prim 6) invertido em exatamente um eixo é desenhado girado
  (as coordenadas de textura dos dois cantos fora da diagonal trocam, `RotateUV` do PPSSPP). A moldura guarda as
  bordas laterais deitadas (78×15 texels) e as desenha assim; a arte (DXT1 128×64 em três pedaços) estava certa.
- **Nome do jogador:** com janela e sem `TAGFORCE_OSK`, o OSK fica VISIBLE e o teclado digita (texto na barra de
  título; Enter confirma, Esc cancela). Sem janela vale o de antes (`TAGFORCE_OSK` ou "Player").
- **Duelo travado antes do pedra-papel-tesoura:** o jogo pede `duel/cutin_da_faces/DA_boy_R10.gim` e o disco tem
  `da_boy_R10.gim`; os sistemas de arquivos da PSP ignoram a caixa. `host_path` troca cada componente que falta
  pela entrada do diretório que bate sem caixa. (O jogo repete a abertura para sempre: 13,5 milhões de linhas no log.)
- **Ferramentas de teste:** `TAGFORCE_PADFILE=arquivo` (a máscara do pad, em hexa, lida do arquivo a cada leitura:
  dirige uma rodada sem janela ao vivo, junto com `TAGFORCE_DUMP` de nome fixo); o `TAGFORCE_GE_TRACE` também
  imprime posição e texel dos vértices (todos quando o desenho tem até 8).
- **Falta:** as cartas no duelo saem com cores erradas e o jogo morre com SIGSEGV pouco depois do começo do duelo.
