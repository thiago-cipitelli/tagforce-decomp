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

**[?] Em aberto:** qual função de `libehsys` grava `fnPrincipal`/`fnSaída` (+0x44/+0x48) e o próximo nome de cena. Nenhuma escrita direta encontrada no BOOT; provavelmente via ponteiro de struct. A entrada do gmodule (`e_entry = 0` → kernel usa início de `.text`) deve chamá-la. Em investigação em `build/decomp/libtitle/`.

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

## 10. Como os NIDs ajudam [inf]

NID = 4 primeiros bytes de SHA-1 do nome da função. Ao nomear uma função de `libehsys` pelo comportamento, dá para testar candidatos de nome contra o NID exportado (`build/boot_exports.json`) e recuperar o nome original. Ainda não explorado.
