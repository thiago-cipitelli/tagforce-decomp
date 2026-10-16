# Arquitetura do jogo (fase 1, em andamento)

Fonte das observações: `build/elf/BOOT.elf` (SHA-256 `fd01f7ac…82f03`), analisado com Ghidra 12.1.3 + ghidra-allegrex via REA (inventário: Evidence `ev_813a27b3…`) e via exportação headless (`tools/ghidra/ExportDecomp.java`, saída em `build/decomp/BOOT/`). Endereços são do ELF relocado na base `0x08804000`. Legenda: **[obs]** observado no binário, **[inf]** inferido, **[?]** desconhecido.

## 1. Visão geral

```
BOOT.BIN (módulo "modehsys", exporta a biblioteca libehsys com 1245 funções)
  │  runtime: threads, VFS por LBN, carregador de módulos, libgu estática, som, vídeo, fontes
  │
  ├─ ehscript thread ──► carrega 1 gmodule de "cena" por vez (libtitle, libfield, libshop, ...)
  │                       chama a função principal da cena; quando ela retorna, descarrega e carrega a próxima
  │
  └─ modmgr ──────────► módulos auxiliares carregados sob demanda por id:
                         1 libfont, 2 videocodec, 3 audiocodec, 4 mpegbase, 5 mpeg, 6 libatrac3plus (Sony)
                         8 libduel_eng, 9 libduel_draw, 10 libdeck_swap, 11 libtutorial (jogo)

gmodules: importam libehsys inteira (1245 NIDs) + libduel_draw importa libduel_eng (143 funções)
```

**Consequência para o port [inf]:** `libehsys` é a API entre engine e jogo. Reimplementar o BOOT em C portável e manter a assinatura dessas 1245 funções é o que permite que os 22 gmodules, decompilados, rodem em PC sem alteração de lógica.

## 2. Inicialização [obs]

| Endereço | Nome | O que faz |
|---|---|---|
| `0x08804000` | `module_start` | `sceKernelSetCompiledSdkVersion(0x02080010)`, cria thread `user_main` (`0x088040d4`, prioridade/stack vindos de `DAT_088eb9d4..dc`) e a inicia com argc/argv. |
| `0x088040d4` | `user_main` | Divide argv, obtém info do módulo, chama `main` e depois `exit`. É o crt0 da Metrowerks. |
| `0x0883dde4` | `main` | Sequência abaixo. |

Sequência de `main`:
1. `malloc(0x10)`/`free` (aquecimento do heap), `FUN_0886a0b4` [?].
2. `sceUmdActivate(1, "disc0:")`, `sceUmdWaitDriveStat(0x20)`.
3. `ehfile_build_lbn_table(0x10000)` (`0x0882f774`): abre `umd1:` cru, lê o diretório ISO9660 e monta uma tabela nome→(LBN, tamanho) ordenada por `qsort`. Ver §3.
4. `sys_init(&cfg)` (`0x0883d33c`) com `cfg = 0x088ee288` = `{dirTable=0x088ee1cc, 1, 0, heap=0x08906a80, heapSize=0x15c000, 0, fps=30}`: `gfx_init(heap, 30|60)`, `ehfile_init`, `modmgr_init(1,0)`, pool de memória, `FUN_08839a8c`, `FUN_08835f48` (VRAM).
5. `FUN_0882923c` [?], `font_init(0)` (`0x08845ac0`: carrega `libfont.prx` via modmgr e abre a fonte do sistema), `FUN_08845df0(4)`, `FUN_08839f50(1)`, `FUN_088363cc(0x8905780,4,4,0x40,4)` [? pool de texturas], `modmgr_register_table(0x088ee238)` (registra os 4 gmodules de id 8..11), `snd_init`, `loading_screen_init` (`loading.ehf`), `FUN_0883e3d4(1)`.
6. `ehscript_start("libtitle", 2, 0)`: cena inicial. Dir index 2 = `disc0:/PSP_GAME/USRDIR/gmodule/`.
7. Registra `EXIT_CALLBACK` (`FUN_0883dd58`).
8. Loop infinito: `ehscript_yield_to()` → `FUN_0883e010` (contador de frames, watchdog de 0x20f57f frames ≈ 20 h, `FUN_08865bc4`, `FUN_0885eddc` [? som]) → `sys_state_update` (tabela de estados em `0x08905720`, 12 bytes/entrada) → `FUN_088694f0` [? diálogos do sistema, checa UMD] → `gfx_frame_end`-like `FUN_0883d3b0` (= `FUN_08834658` swap + `pad_update` + `FUN_08839fa8`) → `sys_check_reboot` (`sceKernelLoadExec("disc0:/PSP_GAME/SYSDIR/BOOT.BIN")` se flags `0x309`).

## 3. VFS "ehfile" [obs]

- Tabela de diretórios em `0x088ee1cc`: índice 0 `USRDIR/module/`, 1 `kmodule/`, 2 `gmodule/`, 3 `USRDIR/`, 4 `duel/`, 5 `duelsys/`, 6 `main/`, 7 `snd/`.
- `ehfile_make_path(buf, dirIdx, name, flag)` (`0x0882f02c`): concatena dir+nome; se o caminho começa com `disc` e a tabela LBN existe, substitui por `disc0:/sce_lbn0x%04x_size0x%04x`. **O jogo lê arquivos do UMD por número de setor, não por nome.** Para o port basta `fopen` no caminho lógico.
- `ehfile_build_lbn_table` (`0x0882f774`) + `ehfile_scan_iso_dir` (`0x0882f4a8`): parser recursivo de registros de diretório ISO9660 (campo flags em +0x19, nome em +0x21, LBN em +2, tamanho em +10) direto de `umd1:`. Entradas de 16 bytes `{namePtr, lbn, size, ?}`; busca por `bsearch` com comparador `0x0882f6f4`.
- `ehfile_init` (`0x0882ecac`): thread `ehfile` (prio 0x6f) + semáforos `ehfile_sema`, `ehfile_queue`. Fila de 16 pedidos de 0x40 bytes em `0x08903418`: `{busy, tipo(0=ler arquivo, 1=carregar módulo), dirIdx, buf, offset, len, callback, cbArg, nome[0x1f]}`. `FUN_0882f12c` enfileira; `ehfile_thread` (`0x0882f2c8`) consome e chama o callback.
- `ehfile_read_file` (`0x0882edf8`): `sceIoOpen`/`sceIoLseek`/`sceIoRead` do caminho LBN. `ehfile_load_module` (`0x0882f0f4`): `sceKernelLoadModule` do caminho LBN.

## 4. Cenas: thread "ehscript" [obs]

Contexto em `0x089056b8` (100 bytes): `+0 dirIdx`, `+4 nomeDoMódulo[32]`, `+0x24 nomeAnterior`, `+0x44 fnPrincipal`, `+0x48 fnSaída`, `+0x50 moduleId`, `+0x54 fnEstática`, `+0x58 estado`, `+0x5c threadId`, `+0x60 threadPrincipal`.

`ehscript_thread` (`0x0883d1cc`), estados:
1. pedir carga: se nome == `"staticmodule"` chama a função estática; senão enfileira no ehfile (tipo 1) com callback `0x0883d320`.
2. aguardando módulo: quando `moduleId != 0`, `sceKernelStartModule`.
3. carregado: copia nome para `nomeAnterior`, estado 5, limpa o nome, **chama `fnPrincipal()`** — a cena inteira executa dentro dessa chamada.
5. ao retornar: se um novo nome foi definido, chama `fnSaída()`, `sceKernelStopModule` + `sceKernelUnloadModule`, volta ao estado 1.

Escalonamento cooperativo: `main` chama `ehscript_yield_to` (`0x0883d0a4`) que dá `sceKernelResumeThread(ehscript)` e dorme; a cena, a cada frame, faz `sceKernelWakeupThread(main)` e dorme. Um frame = uma volta do loop de `main`.

**Registro da cena [obs]:** o gmodule não tem `module_start`; com `e_entry = 0` o kernel chama o início de `.text`. Em `libtitle`, `0x08804000` é `module_entry`: chama `ehscript_register(fnPrincipal, fnSaída, dados)` = `libehsys` `0x0883d0f8` (NID `…`; o Ghidra não a cria porque nada no BOOT a chama, só a tabela de exports). O próximo nome de cena é gravado por `ehscript_set_module(nome)` (`0x0883d118`, exportada).

**Frame da cena [obs]:** o `main` de uma cena (`libtitle` `0x08804034`) é `init(); decidir sub-tela pela transição anterior (scene_get_transition_info, 0x08853c60); sys_state_set_callbacks(0xd, cb1, cb2) (0x0883d464); while (estado != 0x28) sys_frame_wait();` onde `sys_frame_wait` (`0x08859044`) devolve o controle ao `main` do BOOT até o próximo frame. A lógica da tela roda nos callbacks registrados e na máquina de estados do gmodule (`libtitle` `0x08806ef0`, 40 estados).

**Memória do gmodule [obs]:** o PRX declara `.bss` com tamanho 0 no cabeçalho de seção, mas `PT_LOAD.memsz` é 12,7 MB: `libtitle` cria um heap de `0xc00000` no início do `.bss` (`ehsys_08837B28(0x088306f0, 0xc00000)`) e seus globais ficam depois, em `0x094306f0+`. `prx2elf.py` agora grava o tamanho real do `.bss` para o Ghidra mapear esses endereços.

## 5. Gerenciador de módulos "modmgr" [obs]

Registro em `0x08be9f78`, entradas de 0x4c bytes `{id, arg, moduleId(+8), estado(+0xc: 0 livre,1 carregando,2 carregado), refcount(+0x10), nome[0x20](+0x14), tipo(+0x34: 0 PRX, 1 net, 2 av, 3 usb via sceUtility), handle(+0x38), args de start(+0x3c..)}`, ordenado por `qsort`, achado por `bsearch` (`modmgr_find`). Tabela estática em `0x088f0064` (ids 1..6, módulos Sony) e tabela de jogo em `0x088ee238` (ids 8..11). `modmgr_load_sync` (`0x088591bc`) enfileira no ehfile e espera girando o escalonador; `modmgr_update` (`0x08859518`) descarrega quando refcount ≤ 0.

## 6. Gráficos [obs/inf]

O BOOT **não importa `sceGu*`**: importa só `sceGe_user` e `sceDisplay`. A libgu da Sony está linkada estaticamente em `.text` (`0x08804a0c` `sceGuInit`, `0x08804b48` `sceGuStart`, `0x08805838` `sceGuFinish`, `0x08805a60` `sceGuSync`, `0x0880523c` `sceGuSwapBuffers`, `0x08805ae8` `sceGuDisplay`, …; demais em `symbols/BOOT.txt` marcadas `libgu`). `gfx_init` (`0x0883451c`) é a sequência canônica: draw buffer 8888 em 0, disp buffer 480×272 em `0x88000`, depth em `0x110000`, stride 512, offset 2048−240/2048−136, viewport 480×272, depth range 50000..10000, scissor, depth func 7, front face CW. Chamada de `sys_init` com 30 ou 60 fps conforme `cfg+0x18`.

**Port [inf]:** a camada de plataforma substitui a libgu inteira; o resto do engine fala só com a API `sceGu*`.

## 7. Threads do runtime [obs]

| Nome | Entrada | Criada por | Papel |
|---|---|---|---|
| `user_main` | `0x088040d4` | `module_start` | `main` do jogo |
| `ehfile` | `0x0882f2c8` | `ehfile_init` | I/O assíncrono de arquivos e módulos |
| `ehscript` | `0x0883d1cc` | `ehscript_start` | executa a cena atual |
| `snd_thread_main`, `snd_thread_seq` | [?] | `snd_thread_init` (`0x0885e634`) | mixer SAS + Atrac3+ + sequenciador |
| `SceWaveMain` | `0x088642f8` | `wave_init` (`0x08864254`) | libwave (Sony, estática) |
| `displayThread`, `readThread`, `soundThread` | `0x0885380c`, `0x08850bd4`, `0x088525e0` | `movie_init` (`0x088513e0`) | player PMF (sceMpeg) |

## 8. libc estática (Metrowerks MSL) [obs]

`memset 0x0880e140`, `memcpy 0x08811458`, `strcpy 0x0881128c`, `strcat 0x0881106c`, `strlen 0x08811324`, `strcmp 0x08811218`, `sprintf 0x08810c94`, `qsort 0x0880f69c`, `bsearch 0x0880a694`, `malloc 0x0880d2cc`, `free 0x0880d2fc`, `exit 0x0880bf40` (roda atexit, fecha FILEs). Strings `libc:%s: no reent structure` confirmam MSL.

## 9. Imports do BOOT (API da Sony que o port precisa cobrir) [obs]

274 funções em 28 bibliotecas (lista completa em `symbols/BOOT.txt`): ThreadManForUser 43, IoFileMgrForUser 33, sceMpeg 25, sceUtility 23, sceSasCore 21, sceNetAdhoc 14, sceGe_user 12, ModuleMgrForUser 11, sceNetAdhocctl 10, sceAudio 9, sceAtrac3plus 9, sceNetAdhocMatching 9, SysMemUserForUser 8, sceLibFont 7, sceDisplay 6, UtilsForUser 6, sceUmdUser 5, sceNet 4, InterruptManager 3, LoadExecForUser 3, StdioForUser 3, sceCtrl 3, Kernel_Library 2, sceDmac 1, sceOpenPSID 1, sceRtc 1, sceSuspendForUser 1, sceWlanDrv 1. Seis NIDs sem nome na tabela do pspsdk.

## 10. Formato EHP [obs, validado]

Recuperado de `ehp_find` (`0x08833bc8`), `ehp_find_entry` (`0x08833bb8`), `ehp_entry_size` (`0x08833f48`) e `ehp_is_valid` (`0x08833c88`); `tools/ehp.py` lista e extrai, validado nos 303 `.ehp` do disco sem nenhuma entrada fora dos limites.

```
+0  "EHP" u8 versão (3 no TF1)     +4  u32 tamanho total
+8  "NOT "                          +12 u32 count
+16 count × { u32 name_off, u32 data_off }   (offsets do início do arquivo; ordenado por nome, busca binária com strcmp 0x08810fd0)
nome NUL-terminado; na versão ≥ 3 o nome é seguido de u32 tamanho. Versão < 3: tamanho = data_off seguinte − data_off.
```

`ehp_find(pack, nome, &tamanho)` devolve `pack + data_off`. Os packs ficam inteiros em memória. `table_get_entry` (`0x088433a4`) é um acessor para outro formato de tabela `{count, tableOff, base}` usado pelos `.bin` internos (ex.: `CARD_*.bin` em `duelsys/cardinfo_*.ehp`).

## 11. gmodules [obs]

| Módulo | Papel | Entrada | Exports |
|---|---|---|---|
| libtitle, libfield, libstory, libshop, libdeck, libdeck_swap, libdeck_tutorial, libselect_card, libcardalbum, libcharalist, libconv_machine, libduelrecord, libhtml_view, liblabo, liblimitlist, libpassword, librecipe_viewer, libtutorial, libtutoriallist | cena (carregada pela thread ehscript) | `.text+0` chama `ehscript_register(main, exit, dados)` | 0–7 funções (callbacks de UI) |
| libduel_eng | biblioteca do motor de duelo (id 8 no modmgr) | `.text+0` retorna 0 | 143 funções, importadas pela libduel_draw |
| libduel_draw | renderização do duelo (id 9) | [?] | 38 |
| libduel_mgr | orquestra o duelo | [?] | 0 |

Todos importam a `libehsys` inteira (1245 NIDs). Uso real: 680 funções distintas. Ranking por pontos de chamada em `build/ehsys_usage.tsv`; as 10 mais chamadas: `draw_sprite` (997), `ehsys_0883AF84` (800), `se_play` (702), `memset` (700), `table_get_entry` (470), `dl_ambient_color` (453), `gim_set_texture` (398), `font_set_size` (370), `ehsys_0883A4E4` (354), `dl_set_tex_filter` (346).

## 12. Engine: subsistemas identificados na libehsys [obs/inf]

- **Display lists (`dl_*`)**: a engine monta comandos GE diretamente (`dl_push1 0x0883529c`, `dl_push 0x08835308`) em listas por camada (`dl_select_layer 0x0883a29c`, `dl_set_list 0x0883a3b4`), com `dl_color` (AMC/AMA/DMC/SMC), `dl_tex_mode` (TMODE/TPSM), `dl_tex_image` (TSIZE/TBP/TBW), `dl_tex_filter`, `dl_draw_array` (VTYPE/BASE/VADDR). `draw_sprite` (`0x0883ad8c`, 8 parâmetros) é a primitiva 2D.
- **Matrizes VFPU (`mtx_*`)**: pilha de matrizes M300 em `0x08905100` (`mtx_push/pop/identity/store`), `mtx_flush` (`0x0883636c`) despeja as sujas na display list.
- **Texturas GIM**: `gim_set_texture` (`0x08835704`) lê o cabeçalho `MIG.00.1PSP` e emite TPSM/TSIZE.
- **Fontes**: contexto em `PTR_DAT_088ee900+0x50c00..`; `font_set_size`, `font_set_color`, `font_draw_text`, `font_print`.
- **Som**: `se_play(id)` (`0x0886568c`) → `se_play_pan`; sistema SAS/Atrac3+ em `snd_*`.
- **Heap**: `heap_create` (magic `"HOGE"` `0x45474f48`), `heap_alloc`, `heap_malloc_cur` + `malloc/free` da MSL.
- **UI**: `menu_update` (`0x0884b6cc`, 1,5 KB) lê o pad e toca SE; `menu_reset`.

## 13. Como os NIDs ajudam [inf]

NID = 4 primeiros bytes de SHA-1 do nome da função. Ao nomear uma função de `libehsys` pelo comportamento, dá para testar candidatos de nome contra o NID exportado (`build/boot_exports.json`) e recuperar o nome original. Ainda não explorado.
