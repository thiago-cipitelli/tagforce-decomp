# Progresso da decompilação

Gerado a partir dos cabeçalhos `@range` de `src/` e das marcas `matched`/`asm` em `symbols/`;
atualizar ao fechar um arquivo. Números do relatório ByteMatch de 2026-10-28 (`python3 tools/bytematch.py`).

| Módulo | Código batendo | Fuzzy | Funções batendo |
|---|---|---|---|
| total | 51,6% | 68,9% | 7359 / 11039 |
| BOOT | 63,0% | 79,9% | 1815 / 2208 |
| libduel_eng | 61,7% | 81,2% | 2987 / 3429 |
| libhtml_view | 100% | 100% | 16 / 16 |
| liblabo | 100% | 100% | 13 / 13 |
| libpassword | 100% | 100% | 33 / 33 |
| libtutoriallist | 89,5% | 99,8% | 25 / 26 |
| libduelrecord | 92,2% | 99,9% | 26 / 27 |
| libduel_mgr | 37,9% | 87,9% | 97 / 158 |
| libselect_card | 37,2% | 79,5% | 123 / 207 |
| libcardalbum | 79,2% | 99,7% | 62 / 68 |
| libcharalist | 60,2% | 90,7% | 41 / 50 |
| libconv_machine | 79,9% | 97,1% | 44 / 47 |
| liblimitlist | 82,1% | 97,3% | 56 / 62 |
| librecipe_viewer | 56,3% | 91,7% | 51 / 69 |
| libdeck | 65,7% | 89,4% | 275 / 328 |
| libdeck_swap | 61,4% | 84,8% | 156 / 204 |
| libdeck_tutorial | 57,7% | 85,9% | 224 / 293 |
| libshop | 55,9% | 78,2% | 186 / 223 |
| libtitle | 75,4% | 89,7% | 382 / 434 |
| libtutorial | 86,2% | 98,4% | 196 / 207 |
| libfield | 56,9% | 73,4% | 551 / 652 |
| libduel_draw, libstory | 0% | 0% | 0 / 2285 |

Funções "em asm" ficam no `.c` como `// @asm A-B` com o C equivalente sob `#ifdef NON_MATCHING`
(rascunho funcional, ainda não idêntico byte a byte).

## Maiores trechos ainda sem C

Saída de `MODULE=<m> python3 tools/list_funcs.py --gaps` (início-fim, funções, bytes). Bom lugar para começar:

**libduel_eng**

```
0x08839360-0x0883F0DC     7 functions    23932 bytes
0x088CC8EC-0x088D13CC     7 functions    19168 bytes
0x088F2D70-0x088F5D54     1 functions    12260 bytes
0x088482F8-0x0884ACD0     1 functions    10712 bytes
0x08850F44-0x08853340     1 functions     9212 bytes
0x08811BDC-0x088137BC     2 functions     7136 bytes
0x08846584-0x088480D0     1 functions     6988 bytes
0x0884B094-0x0884C718    11 functions     5764 bytes
0x088D9080-0x088DA5EC     4 functions     5484 bytes
0x088451E8-0x08846498     1 functions     4784 bytes
```

**libtitle**: todo o `.text` em C (restam 53 funções em asm).

**libfield**: `0x0881C240-0x0882BE80` (204 funções: sombras, lista de desenho, menus, efeitos, sprites); `0x08804000-0x0881C240` está em C.

**BOOT**: nenhum trecho sem C. Todo o `.text` está em arquivos `.c`; o que falta são as funções em `@asm` (C sob `NON_MATCHING`). Na parte do SDK compilada com GCC (libgu, newlib, libgcc, libvfpu, libccc, libfpu, libdeflt, libsas/libwave) as funções são `asm` do mwcc com C portável sob `#else`; ficam em `@asm` as que carregam dados por `lui`/`%lo`, que o assembler embutido não aceita (ver `docs/build.md`).

## BOOT (107 arquivos, 1471 funções batendo, 369 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`crt0.c`](../src/BOOT/crt0.c) | 0x08804000-0x08804A0C | 23 | 6 | crt0 e camada de syscalls do newlib (GCC, funções `asm` com C portável sob `#else`): module_start, threads de entrada, `_exit`, sbrk, stubs de arquivo |
| [`libgu.c`](../src/BOOT/libgu.c) | 0x08804A0C-0x08805EC4 | 3 | 40 | libgu do SDK (sceGu*, nomes pelos NIDs): listas de exibição, sinais, chamadas, estado, buffers de tela e desenho |
| [`libgu_state.c`](../src/BOOT/libgu_state.c) | 0x08805EC4-0x08807070 | 0 | 57 | libgu: comandos de estado do GE (desenho de arrays/bezier/spline, sprites, cópia de imagem, clear, luzes, material, textura, CLUT, névoa, testes, blend, matrizes de osso) |
| [`libgu_ctx.c`](../src/BOOT/libgu_ctx.c) | 0x08807070-0x088087F8 | 17 | 10 | libgu: contextos de lista, inicialização, sceGuDebugPrint/Flush, vblank_handler_set |
| [`libm.c`](../src/BOOT/libm.c) | 0x088087F8-0x0880A5C0 | 9 | 13 | libm float do newlib (fdlibm): kernels de sin/cos/tan, atan2f, sqrtf e wrappers SVID, abs |
| [`libc_stdlib.c`](../src/BOOT/libc_stdlib.c) | 0x0880A5C0-0x0880C040 | 5 | 3 | newlib: atexit, bsearch, `_calloc_r`, quorem/`_dtoa_r`, `__errno`, exit |
| [`libc_stdio.c`](../src/BOOT/libc_stdio.c) | 0x0880C040-0x0880CDD4 | 3 | 10 | newlib stdio: fflush, `__sinit`, findfp, fvwrite |
| [`libc_malloc.c`](../src/BOOT/libc_malloc.c) | 0x0880CDD4-0x0880DC98 | 12 | 7 | newlib dlmalloc: `_malloc_r`, `_free_r`, smakebuf, trim, memalign |
| [`libc_mem.c`](../src/BOOT/libc_mem.c) | 0x0880DC98-0x0880E700 | 5 | 5 | newlib: memchr/memcpy/memmove/memset e afins |
| [`libc_mprec.c`](../src/BOOT/libc_mprec.c) | 0x0880E700-0x0880F60C | 16 | 1 | newlib mprec: Balloc, multadd, hi0bits/lo0bits, i2b, mult, pow5mult, lshift, cmp, diff, ulp, b2d/d2b, ratio |
| [`libc_misc.c`](../src/BOOT/libc_misc.c) | 0x0880F60C-0x08810BA8 | 10 | 5 | newlib: printf, qsort, rand/srand, `__srefill`, `_snprintf_r`, funções de reent.c |
| [`libc_string.c`](../src/BOOT/libc_string.c) | 0x08810BA8-0x088126E8 | 18 | 3 | newlib: strcat/strchr/strcmp/strcpy/strlen/strncmp/strncpy/strstr e strtok |
| [`strtol.c`](../src/BOOT/strtol.c) | 0x088126E8-0x08813108 | 4 | 5 | newlib strtod/strtof/strtol/strtoul e os `_r` para long, long long e unsigned |
| [`ungetc.c`](../src/BOOT/ungetc.c) | 0x08813108-0x08813390 | 2 | 0 | newlib ungetc, `__submore` |
| [`vfprintf.c`](../src/BOOT/vfprintf.c) | 0x08813390-0x088151B8 | 5 | 1 | newlib `__sprint`, `__sbprintf`, vfprintf, cvt, exponent; `_vfprintf_r` em asm |
| [`vfscanf.c`](../src/BOOT/vfscanf.c) | 0x088151B8-0x0881635C | 4 | 1 | newlib vfscanf e `__sccl`; `__svfscanf_r` transcrito do asm |
| [`wcrtomb.c`](../src/BOOT/wcrtomb.c) | 0x0881635C-0x08816B1C | 6 | 2 | newlib vsprintf, wcrtomb/wcsrtombs, `_wctomb_r` (UTF-8/SJIS/EUC-JP/JIS), `_write_r` |
| [`wsetup.c`](../src/BOOT/wsetup.c) | 0x08816B1C-0x08816C3C | 1 | 0 | newlib `__swsetup` |
| [`libgcc.c`](../src/BOOT/libgcc.c) | 0x08816C3C-0x08817784 | 1 | 2 | libgcc `__muldi3`, `__udivdi3`/`__umoddi3` |
| [`fpbit.c`](../src/BOOT/fpbit.c) | 0x08817784-0x08818E64 | 19 | 4 | libgcc fp-bit: pack/unpack, soma/subtração, comparações, conversões, negação |
| [`ccc.c`](../src/BOOT/ccc.c) | 0x08818E64-0x08819BEC | 13 | 16 | libccc da Sony (sceCcc*): codificação UTF-8/UTF-16/SJIS, validação, strlen, caracteres de erro, tabelas |
| [`fpu.c`](../src/BOOT/fpu.c) | 0x08819BEC-0x08819F0C | 35 | 1 | libfpu da Sony (sceFpu*): arredondamento, max/min, fmod/frac, testes de sinal/zero/denormal/inf, NaN, FCR31 |
| [`vfpu_math.c`](../src/BOOT/vfpu_math.c) | 0x08819F0C-0x0881AB1C | 33 | 8 | libvfpu (funções `asm` com C portável): isinf_sign, sceFpuSin/Cos/Atan/Asin/Acos, Vector2 |
| [`vfpu_vec3.c`](../src/BOOT/vfpu_vec3.c) | 0x0881AB1C-0x0881B244 | 35 | 0 | libvfpu: sceVfpuVector3* |
| [`vfpu_vec4.c`](../src/BOOT/vfpu_vec4.c) | 0x0881B244-0x0881B974 | 47 | 0 | libvfpu: sceVfpuVector4* |
| [`vfpu_mat.c`](../src/BOOT/vfpu_mat.c) | 0x0881B974-0x0881C4E4 | 36 | 0 | libvfpu: Matrix2, Matrix3 e as primeiras de Matrix4 |
| [`vfpu_mat4.c`](../src/BOOT/vfpu_mat4.c) | 0x0881C4E4-0x0881CD50 | 17 | 0 | libvfpu: produto de matrizes 4x4, transformações, rotações, traço, determinante, adjunta, inversa, sombra, ortonormalização |
| [`vfpu_scalar.c`](../src/BOOT/vfpu_scalar.c) | 0x0881CD50-0x0881D214 | 22 | 0 | libvfpu: sceVfpuScalar* (trig, exp/log, pow, arredondamento) |
| [`vfpu_color.c`](../src/BOOT/vfpu_color.c) | 0x0881D214-0x0881D8A8 | 34 | 4 | libvfpu: sceVfpuColor* (conversões 8888/4444/5551/565) e sceVfpuRand* |
| [`vfpu_proj.c`](../src/BOOT/vfpu_proj.c) | 0x0881D8A8-0x0881DE88 | 7 | 2 | libvfpu: view, perspectiva, ortho, frustum, look_at, projeção de pontos |
| [`vfpu_quat.c`](../src/BOOT/vfpu_quat.c) | 0x0881DE88-0x0881EB0C | 18 | 5 | libvfpu: quatérnios, slerp, log/exp, Euler zyx/xyz/yxz |
| [`vfpu_mem.c`](../src/BOOT/vfpu_mem.c) | 0x0881EB0C-0x0881F6E4 | 10 | 0 | libvfpu: memcpy/memmove/memset pela VFPU, contexto de registradores, estado do gerador aleatório |
| [`gzip.c`](../src/BOOT/gzip.c) | 0x0881F6E4-0x0881FFB0 | 7 | 0 | libdeflt: cabeçalho gzip e o decodificador deflate escrito à mão (C portável: inflate completo) |
| [`postfx.c`](../src/BOOT/postfx.c) | 0x0881FFB0-0x08820ED4 | 8 | 3 | efeitos de tela cheia por texturas de trabalho na VRAM: redução, cópia escurecida, blends aditivos de cópias deslocadas e limiar por CLUT |
| [`ehf_args.c`](../src/BOOT/ehf_args.c) | 0x08820ED4-0x08822F84 | 1 | 0 | `ehf_cmd_args`: conversão dos argumentos dos comandos EHF por letra de destino (n o p q / O P Q N inteiros, f g h i / G H I F floats, r palavra crua) e de origem (F N f n V I v i), por macros |
| [`ehf_cmd.c`](../src/BOOT/ehf_cmd.c) | 0x08822F84-0x08823BD0 | 18 | 1 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd2.c`](../src/BOOT/ehf_cmd2.c) | 0x08823BD0-0x08824040 | 6 | 1 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd3.c`](../src/BOOT/ehf_cmd3.c) | 0x08824040-0x088243E4 | 3 | 0 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd4.c`](../src/BOOT/ehf_cmd4.c) | 0x088243E4-0x088253FC | 13 | 2 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_prim.c`](../src/BOOT/ehf_prim.c) | 0x088253FC-0x0882644C | 9 | 1 | gizmo de eixos e primitivas cujos vértices vêm de comandos seguintes: início com espaço na lista ou vértices externos, coordenadas de textura, cores, posições, fita voltada para a câmera |
| [`particle.c`](../src/BOOT/particle.c) | 0x0882644C-0x08827030 | 8 | 0 | partículas EHF: pool com envelhecimento, emissor periódico com callbacks de posição/velocidade, geradores em caixa, caixa móvel e disco com o gerador rand3 |
| [`particle2.c`](../src/BOOT/particle2.c) | 0x08827030-0x08827B8C | 6 | 0 | geradores em anel, tetraédrico, casca esférica e cone, posições levadas ao espaço do modelo, velocidades constante e radial, movimento, aceleração e arrasto |
| [`particle3.c`](../src/BOOT/particle3.c) | 0x08827B8C-0x08828590 | 10 | 1 | geradores em anel, tetraédrico, casca esférica e cone, posições levadas ao espaço do modelo, velocidades constante e radial, movimento, aceleração e arrasto |
| [`ptcl_draw.c`](../src/BOOT/ptcl_draw.c) | 0x08828590-0x0882923C | 7 | 1 | partículas desenhadas como quads de faixa de triângulos no espaço da câmera, cor fixa ou por chaves de idade, textura, tamanho e uv do sprite |
| [`efs.c`](../src/BOOT/efs.c) | 0x0882923C-0x0882AD90 | 21 | 5 | efeitos .efs: carga com relocação por bytecode, raízes nomeadas com busca binária, 0x300 instâncias em lista circular no heap de efeitos, interpretador de árvores de comandos por fase, argumentos por formato, comandos de instância aninhada |
| [`camera.c`](../src/BOOT/camera.c) | 0x0882AD90-0x0882B65C | 18 | 0 | câmera com look-at e as quatro luzes do GE com bits de ativação reemitidos na lista |
| [`tms.c`](../src/BOOT/tms.c) | 0x0882B65C-0x0882C428 | 11 | 1 | geometria TGMS: relocação, materiais com quatro camadas de textura, hooks de textura e desenho, callbacks de mapa de ambiente, prims com cache de oito matrizes de osso, culling pela caixa |
| [`model.c`](../src/BOOT/model.c) | 0x0882C428-0x0882D208 | 19 | 1 | modelos: geometria TGMS com matrizes de osso e de skin, animações TGMT com cross-fade, buffers de animação descomprimida, animação de material TGMA, cópias de materiais e texturas, duas passadas de desenho com stencil e contorno |
| [`toon.c`](../src/BOOT/toon.c) | 0x0882D208-0x0882D9F0 | 3 | 1 | rampas de toon shading desenhadas na VRAM a partir das tabelas do contexto de desenho, hook de textura das camadas iluminadas pela luz 0, relocação TGMT |
| [`anim.c`](../src/BOOT/anim.c) | 0x0882D9F0-0x0882ECAC | 7 | 0 | animação esquelética TGMT: percurso em profundidade da hierarquia, translação/rotação/escala por chaves Bezier e quatérnios com slerp, cross-fade entre duas animações, quatérnios compactados em 16 bits |
| [`ehfile.c`](../src/BOOT/ehfile.c) | 0x0882ECAC-0x0882FC3C | 0 | 0 | VFS, 29 funções |
| [`ehf.c`](../src/BOOT/ehf.c) | 0x0882FC3C-0x08833420 | 15 | 8 | player de animações EHF, formato tipo Flash em twips: acessores de arquivo/instância, timeline com comandos de colocar/remover, pool de clips em listas circulares por profundidade, avanço por tick, desenho com máscara por stencil, transformação de cor e matrizes 2D |
| [`ehp.c`](../src/BOOT/ehp.c) | 0x08833420-0x0883451C | 10 | 1 | pacotes EHP: busca binária por nome ou ponteiro, relocação por extensão com ptrs.txt, TGMA/TGMT, TMS, EHF e GIM, handlers registrados, carga síncrona e assíncrona, tabela de pacotes montados, projeção 2D |
| [`gfx.c`](../src/BOOT/gfx.c) | 0x0883451C-0x08835650 | 6 | 2 | inicialização do GE/libgu, fim de quadro com limite de 30 fps e callback de flip, alvos de desenho, capturas BMP/TGA para host0:, rastreador do estado do GE com a lista de valores padrão |
| [`gim.c`](../src/BOOT/gim.c) | 0x08835650-0x08835F48 | 0 | 3 | texturas GIM: busca de blocos na árvore, modo e imagem de um quadro ou célula, CLUT da paleta, imagem residente na VRAM no lugar da assinatura PSP, troca de textura fora da lista de desenho |
| [`vram.c`](../src/BOOT/vram.c) | 0x08835F48-0x088362E8 | 1 | 1 | área de texturas na VRAM depois dos framebuffers: alocador linear, envio de um quadro de GIM para a VRAM, percurso dos blocos do GIM |
| [`mtx.c`](../src/BOOT/mtx.c) | 0x088362E8-0x088370A4 | 30 | 0 | pilhas de matrizes por tipo do GE com a matriz corrente em M300 da VFPU: init, emissão/flush das sujas para a lista, salvar/restaurar estado, identidade, rotações, translação, escala, perspectiva, ortho, look-at, push/pop; blocos `asm` VFPU no C com equivalente em C para o port |
| [`draw.c`](../src/BOOT/draw.c) | 0x088370A4-0x08837B28 | 0 | 1 | primitivas dentro da lista ou imediatas fechando e reabrindo a sublista, teste de esfera contra o frustum, flags de matriz suja |
| [`heap.c`](../src/BOOT/heap.c) | 0x08837B28-0x088390C4 | 7 | 1 | heap HOGE: tabela ordenada de fronteiras com bit de livre, guardada num bloco do próprio heap e dobrada quando enche, alocação best fit com divisão pelo topo, liberação com fusão, compactação |
| [`transform.c`](../src/BOOT/transform.c) | 0x088390C4-0x08839A8C | 2 | 2 | splines cúbicas pelas chaves: termos de segunda derivada por sistema tridiagonal, aberto ou em anel, e avaliação num índice fracionário |
| [`pad.c`](../src/BOOT/pad.c) | 0x08839A8C-0x08839F50 | 2 | 1 | controle: amostra por quadro com reaproveitamento, máscaras de borda e auto-repeat por botão, analógico dobrado em direcional com repetição própria, tempos de repetição pela taxa de quadros, ângulo do analógico ou do direcional |
| [`dl.c`](../src/BOOT/dl.c) | 0x08839F50-0x0883CEB0 | 73 | 6 | listas de comandos do GE: heap em duas metades com tabela de 16 camadas encadeadas por JUMP, troca de camada, e uma função por estado ou primitiva do GE montando as palavras no stack e empurrando com dl_push: enable/disable, matrizes, luzes, material, textura, CLUT, névoa, scissor, testes, blend, dither, sprite, clear, cópia de imagem; SysConf com os campos reais |
| [`sys.c`](../src/BOOT/sys.c) | 0x0883CEB0-0x0883D708 | 11 | 0 | System glue: display defaults, the "ehscript" scene thread (one gmodule at a time, cooperative with BOOT's main loop), init order and the per-scene state stack |
| [`ehf_cam.c`](../src/BOOT/ehf_cam.c) | 0x0883D708-0x0883DD58 | 5 | 0 | comandos EHF de câmera perspectiva/orto/look-at e animação de material TGMA |
| [`main.c`](../src/BOOT/main.c) | 0x0883DD58-0x0883E670 | 21 | 0 | BOOT entry after the C runtime: init order, the frame loop, soft reset, the loading overlay and small accessors of the game globals |
| [`save.c`](../src/BOOT/save.c) | 0x0883E670-0x0883F340 | 14 | 0 | Save state ("sysstate"): new game defaults, checksum, player name, day counter, money and the duel record counters |
| [`inv.c`](../src/BOOT/inv.c) | 0x0883F340-0x08840478 | 16 | 0 | Card inventory records (one 4-byte bitfield per card index), the current deck, tag partner slots and the player name inside the save state |
| [`cfg.c`](../src/BOOT/cfg.c) | 0x08840478-0x08840DC4 | 29 | 0 | Save-state flag records, options bitfields and the session game state (player slots filled from recipes / saved decks) |
| [`deck.c`](../src/BOOT/deck.c) | 0x08840DC4-0x08841BD0 | 23 | 0 | Stage progress, per-character deck slots, deck/recipe comparison, request flags, region and language, and the language-dependent name/message packs |
| [`wstr.c`](../src/BOOT/wstr.c) | 0x08841BD0-0x088420B4 | 1 | 0 | charmap, wcs* |
| [`wfmt.c`](../src/BOOT/wfmt.c) | 0x088420B4-0x088433F0 | 6 | 1 | formatação UCS-2 |
| [`menu.c`](../src/BOOT/menu.c) | 0x088433F0-0x08843D78 | 1 | 1 | registro de itens de janela |
| [`menu_draw.c`](../src/BOOT/menu_draw.c) | 0x08843D78-0x08845AC0 | 8 | 3 | moldura 9-slice |
| [`font.c`](../src/BOOT/font.c) | 0x08845AC0-0x08847008 | 9 | 2 | cache de glifos sceFont |
| [`font_text.c`](../src/BOOT/font_text.c) | 0x08847008-0x08848C68 | 0 | 0 | layout de texto UCS-2 com tags $R/$C/$f/$F, só C sob `NON_MATCHING` |
| [`font_flags.c`](../src/BOOT/font_flags.c) | 0x08848C68-0x0884900C | 0 | 0 | Font layer state accessors: layout flags, extra line spacing, and the display list used while a menu item's text is being built |
| [`font_tags.c`](../src/BOOT/font_tags.c) | 0x0884900C-0x0884A9B4 | 11 | 6 | Text tags and line emission: $R ruby (furigana over the base text), $C color, $f/$F small capitals, word-break measurement, and font_layout_end which turns FontCtx.cmds into glyph quads |
| [`menu_widget.c`](../src/BOOT/menu_widget.c) | 0x0884A9B4-0x0884C8D0 | 4 | 2 | janela com lista rolável e botões |
| [`card_db.c`](../src/BOOT/card_db.c) | 0x0884C8D0-0x0884EA38 | 5 | 2 | receitas RCP*.ydc, cardinfo_<lang>.ehp, acessores de carta, 64 + 2 callbacks de cena |
| [`card_groups.c`](../src/BOOT/card_groups.c) | 0x0884EA38-0x08850A70 | 1 | 1 | listas de ids hard-coded como `switch`, classes especiais e ids alternativos |
| [`pmf_umd.c`](../src/BOOT/pmf_umd.c) | 0x08850A70-0x0885133C | 18 | 0 | thread do ring buffer sceMpeg, wrappers DvdUmdRetry* da Sony, carga dos módulos de codec |
| [`movie.c`](../src/BOOT/movie.c) | 0x0885133C-0x08853A2C | 64 | 4 | player PSMF da Sony: movie_run, threads display/read/sound, filas de pts e sincronia A/V |
| [`cip.c`](../src/BOOT/cip.c) | 0x08853A2C-0x08854920 | 22 | 2 | pilha de cenas scene_push/pop_to/reset_to + leitor CIP/CPM das imagens de carta com anel de slots e prefetch |
| [`cardlist.c`](../src/BOOT/cardlist.c) | 0x08854920-0x08859044 | 52 | 6 | desenho de carta e painel: imagem com cross-fade, moldura cil_0000, estrelas, atributo, ATK/DEF, caixa de descrição com barra de rolagem, coluna de comparação do editor de deck, tabelas de vértices de 16 bits |
| [`modmgr.c`](../src/BOOT/modmgr.c) | 0x08859044-0x08859824 | 14 | 0 | Frame wait, a debug text menu, synchronous file/module loads and the module manager |
| [`fade.c`](../src/BOOT/fade.c) | 0x08859824-0x08859DF0 | 9 | 0 | fades de tela linear/quadrático/wipe |
| [`cardinfo.c`](../src/BOOT/cardinfo.c) | 0x08859DF0-0x0885C688 | 24 | 5 | visualizador de detalhes da carta: estado em globais 0x08BEA44C, abas descrição/gêneros/listas, scroll, dicas de botão |
| [`snd_mix.c`](../src/BOOT/snd_mix.c) | 0x0885C688-0x0885D948 | 8 | 2 | mixer: faders por quadro, parâmetros e key on/fim das vozes SAS, buffer de eco de 0x3000 quadros, ordenação dos streams, nível de bloco, decodificador Atrac que enche o anel PCM de cada stream e o mistura no bloco de saída |
| [`snd_out.c`](../src/BOOT/snd_out.c) | 0x0885D948-0x0885DE90 | 1 | 1 | thread de som que mistura vozes SAS e streams em dois buffers e entrega ao canal de áudio; 1 + efeito global em asm |
| [`snd_bank.c`](../src/BOOT/snd_bank.c) | 0x0885DE90-0x0885E634 | 31 | 2 | cabeçalho SHDS de psp_snddat.bin, grupos, cache de handles, listas intrusivas |
| [`snd_engine.c`](../src/BOOT/snd_engine.c) | 0x0885E634-0x0885F05C | 4 | 1 | inicialização: estado, listas, callbacks ou buffer de grupos, semáforos, SAS, pool de objetos e threads; início da thread, abertura direta do banco, passo de pausa/retomada com fader, atualização por quadro de todos os sons ativos |
| [`snd_alloc.c`](../src/BOOT/snd_alloc.c) | 0x0885F05C-0x0885FAA8 | 1 | 2 | histórico de variantes sorteadas, escolha do slot de objeto de um som novo, início das vozes SAS com pitch aleatório e envelope ou do stream Atrac; tabelas de pitch em C, as 3 funções em asm |
| [`snd_play.c`](../src/BOOT/snd_play.c) | 0x0885FAA8-0x08860DA4 | 16 | 2 | reset/anexação de objetos, início de som com variantes aleatórias e sequenciais, slots de BGM com suspensão, voz, parada com fade e callback, ducking, parada por grupo ou classe |
| [`snd_core.c`](../src/BOOT/snd_core.c) | 0x08860DA4-0x088615D0 | 16 | 3 | máscara de vozes SAS, volumes por canal, medidor de nível para lip sync, reverb |
| [`snd_obj.c`](../src/BOOT/snd_obj.c) | 0x088615D0-0x08862C5C | 24 | 3 | handles do pool de objetos de som, busca por id, ordem de início, slots do som atual, carga de grupos pelos callbacks ou VPL, curva de distância e modelo 3D com pan e Doppler, faders, volumes SAS por objeto |
| [`snd_rand.c`](../src/BOOT/snd_rand.c) | 0x08862C5C-0x08862D28 | 3 | 0 | gerador aleatório da biblioteca de som |
| [`libsas.c`](../src/BOOT/libsas.c) | 0x08862D28-0x088634EC | 1 | 23 | libsas: wrappers sceSas* das syscalls `__sceSas*` |
| [`libwave.c`](../src/BOOT/libwave.c) | 0x088634EC-0x08865260 | 1 | 30 | libwave: 8 canais VAG/PCM em software misturados pela thread SceWaveMain, 4 canais de hardware, fades |
| [`sndapi.c`](../src/BOOT/sndapi.c) | 0x08865260-0x08866068 | 57 | 2 | API de som exportada aos gmodules: BGM/SE, callbacks de leitura/heap da biblioteca de som, tabelas de voz por personagem, filas de voz por jogador, movie_play, caminhos de save-data |
| [`savedata.c`](../src/BOOT/savedata.c) | 0x08866068-0x088668C0 | 10 | 0 | bloco de parâmetros do sceUtilitySavedata, textos SFO, ícones de msd.ehp, chave, inícios por modo |
| [`msd.c`](../src/BOOT/msd.c) | 0x088668C0-0x08868DC8 | 10 | 0 | máquinas de estado de save-data por operação: novo jogo, verificação, autoload, carregar, salvar, apagar; carga de msd.ehp e despacho por quadro |
| [`net_main.c`](../src/BOOT/net_main.c) | 0x08868DC8-0x0886A6EC | 48 | 1 | entrada de estados, listas do lobby, driver por quadro com watchdogs de pad/UMD/link, exports para os gmodules, diálogos netconf/mensagem, carga dos módulos de rede |
| [`net_match.c`](../src/BOOT/net_match.c) | 0x0886A6EC-0x0886B654 | 17 | 0 | sceNetAdhocMatching host/convidado, callback que enfileira eventos num anel de pacotes Mbx, tabela de pares |
| [`net_ctl.c`](../src/BOOT/net_ctl.c) | 0x0886B654-0x0886C904 | 22 | 1 | sceNetAdhocctl, máquina de estados net_update, handshake PTP com a lista de MACs, 22 + 1 em asm; `include/net.h` com NetSession/NetPeer |
| [`msgbox.c`](../src/BOOT/msgbox.c) | 0x0886C904-0x0886CF50 | 7 | 1 | caixa de mensagem do jogo, simples ou com botões OK/SIM/NÃO via menu widget |
| [`extdata.c`](../src/BOOT/extdata.c) | 0x0886CF50-0x0886E0A4 | 9 | 2 | arquivos YGTF .YGC/.YGR/.YGL da pasta do save: listagem, leitura, verificação e desembaralhamento |
| [`netsync.c`](../src/BOOT/netsync.c) | 0x0886E0A4-0x0886ED2C | 26 | 3 | troca de registros de 0x100 bytes por jogador via PDP ad hoc, fila de pacotes, transferência em blocos de 0x80, lado do jogador local |

## libduel_eng (183 arquivos, 2984 funções batendo, 388 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`ai_08804000.c`](../src/libduel_eng/ai_08804000.c) | 0x08804000-0x088070A8 | 18 | 3 | IA: passos da fase principal do computador (invocação, posição de ataque/defesa, tabela duel_ai_phases) |
| [`ai_088070A8.c`](../src/libduel_eng/ai_088070A8.c) | 0x088070A8-0x08809B50 | 19 | 2 | IA: escolha de alvos e de cartas a jogar na fase principal |
| [`ai_util.c`](../src/libduel_eng/ai_util.c) | 0x08809B50-0x0880B448 | 22 | 5 | ajudantes da IA: escolher cartas para um diálogo, comparar os monstros mais fortes dos dois jogadores, melhor alvo de ataque, cartas que a IA prefere, contagens por tipo na mão e no cemitério |
| [`ai_0880B448.c`](../src/libduel_eng/ai_0880B448.c) | 0x0880B448-0x0880FDF4 | 22 | 7 | IA: avaliação de ataques e salvamento/restauração de duel_special, ordenação por prioridade |
| [`ai_cond3.c`](../src/libduel_eng/ai_cond3.c) | 0x0880FDF4-0x08810370 | 6 | 5 | mais condições da IA por carta: ATK do mais forte contra os LP do oponente, fase e turno, monstros por posição |
| [`ai_cond4.c`](../src/libduel_eng/ai_cond4.c) | 0x08810370-0x08811BDC | 36 | 3 | condições da IA: LP contra o ATK mais forte, monstros e cartas especiais no campo, cartas da corrente, mão e cemitério, fase e turno |
| [`ai_cond5.c`](../src/libduel_eng/ai_cond5.c) | 0x088137BC-0x08814EC8 | 26 | 6 | condições da IA contra a carta sendo ativada: alvos no campo, tipo e dono da carta, monstros e cartas especiais, LP |
| [`ai_cond6.c`](../src/libduel_eng/ai_cond6.c) | 0x08814EC8-0x08815E90 | 8 | 4 | escolha da carta a ativar pelas tabelas de cartas e condições da IA, zonas embaralhadas por máscara com a de maior ATK/DEF, carta da mão por pontuação |
| [`ai_08815E90.c`](../src/libduel_eng/ai_08815E90.c) | 0x08815E90-0x08819F94 | 16 | 11 | IA: escolha de alvos por carta (seletor de 5,8 KB) e contagens por posição |
| [`ai_cond2.c`](../src/libduel_eng/ai_cond2.c) | 0x08819F94-0x0881C118 | 57 | 6 | condições da IA: alvos do descritor, cartas com o mesmo nome, contadores por zona, bits de posição dos monstros, PV contra o dano previsto, magia de campo, monstros da mão invocáveis por tributo |
| [`ai_cond1.c`](../src/libduel_eng/ai_cond1.c) | 0x0881C118-0x0881D0D8 | 12 | 6 | condições da IA: número de monstros, alvos com ATK alto, ATK/DEF do monstro escolhido contra o maior do oponente, tabela de cartas que a IA invoca do cemitério |
| [`ai_cond0.c`](../src/libduel_eng/ai_cond0.c) | 0x0881D0D8-0x0881E0A0 | 22 | 3 | mais condições da IA por carta: mão e cemitério sem cartas proibidas, PV, nível e ATK do monstro escolhido |
| [`ai_cond.c`](../src/libduel_eng/ai_cond.c) | 0x0881E0A0-0x08820618 | 74 | 4 | condições que o jogador do computador verifica antes de ativar cada carta: PV do oponente, contagens no campo, na mão e no cemitério, alvos aceitos pelo descritor |
| [`ai_08820618.c`](../src/libduel_eng/ai_08820618.c) | 0x08820618-0x08827618 | 34 | 6 | IA: escolha da carta a ativar (tabelas de cartas no campo e na mão), passos de ataque, registro de ataque/invocação (DuelAtkRec) |
| [`ai_08827618.c`](../src/libduel_eng/ai_08827618.c) | 0x08827618-0x08827DA4 | 9 | 0 | IA: preparação do registro de invocação |
| [`ai_08827DA4.c`](../src/libduel_eng/ai_08827DA4.c) | 0x08827DA4-0x0882926C | 0 | 2 | IA: opções de invocação da mão e verificação de invocação especial por carta |
| [`ai_0882926C.c`](../src/libduel_eng/ai_0882926C.c) | 0x0882926C-0x0882A23C | 6 | 0 | IA: verificações de oferendas (tributáveis por zona) |
| [`ai_0882A23C.c`](../src/libduel_eng/ai_0882A23C.c) | 0x0882A23C-0x0882AFA4 | 7 | 3 | filtros de zona das oferendas e duel_summon_init; tabela duel_ritual_ids |
| [`ai_0882AFA4.c`](../src/libduel_eng/ai_0882AFA4.c) | 0x0882AFA4-0x0882CAC0 | 11 | 0 | duel_summon_tributes e máquinas de passos de invocação por carta |
| [`ai_0882CAC0.c`](../src/libduel_eng/ai_0882CAC0.c) | 0x0882CAC0-0x0882D398 | 7 | 0 | mais máquinas de passos de invocação; tabela duel_ritual_ids2 |
| [`ai_0882D398.c`](../src/libduel_eng/ai_0882D398.c) | 0x0882D398-0x0882EE74 | 7 | 3 | invocações por atributo, invocações sem oferendas, materiais nomeados |
| [`ai_0882EE74.c`](../src/libduel_eng/ai_0882EE74.c) | 0x0882EE74-0x0882FF4C | 5 | 1 | filtros de 0x19A3/0x19A4/0x19C8 e duel_summon_run (despacho por id de carta) |
| [`ai_0882FF4C.c`](../src/libduel_eng/ai_0882FF4C.c) | 0x0882FF4C-0x08830E94 | 11 | 3 | perguntas de evento e seus filtros de zona |
| [`ai_08830E94.c`](../src/libduel_eng/ai_08830E94.c) | 0x08830E94-0x088324F0 | 5 | 3 | mais perguntas de evento e duel_ev_run |
| [`duel_event.c`](../src/libduel_eng/duel_event.c) | 0x088324F0-0x08832B54 | 32 | 0 | eventos do duelo para a apresentação: um registro pendente de jogador, tipo e duas meias-palavras ou uma lista curta, enviado ao outro console como mensagem 0x1E; 32 wrappers por tipo |
| [`ai_08832B54.c`](../src/libduel_eng/ai_08832B54.c) | 0x08832B54-0x08834734 | 19 | 1 | IA: escolha de uma carta da mão para um efeito, passos das mensagens da corrente |
| [`ai_08834734.c`](../src/libduel_eng/ai_08834734.c) | 0x08834734-0x08835AEC | 17 | 1 | IA: passos de ação do computador, quais cartas da mão e do campo podem agir, escolha por zona |
| [`ai_08835AEC.c`](../src/libduel_eng/ai_08835AEC.c) | 0x08835AEC-0x08836868 | 15 | 0 | IA: despachante de escolhas por `duel_w_08919630`, setters de escolha, leitura da tabela `duel_ai_tab` (23 entradas id/zona/check/apply) |
| [`ai_08836868.c`](../src/libduel_eng/ai_08836868.c) | 0x08836868-0x08837500 | 8 | 1 | IA: travas da fase de batalha, checagens de quem pode atacar, retrato do ataque |
| [`ai_08837500.c`](../src/libduel_eng/ai_08837500.c) | 0x08837500-0x08838548 | 3 | 0 | IA: máquinas de passos da batalha (atacante, depois alvo) |
| [`ai_08838548.c`](../src/libduel_eng/ai_08838548.c) | 0x08838548-0x08839360 | 3 | 1 | IA: efeitos na declaração de ataque |
| [`ai_0883F0DC.c`](../src/libduel_eng/ai_0883F0DC.c) | 0x0883F0DC-0x088400B0 | 21 | 1 | IA: avaliação de jogadas por zona e contador de passos em registrador |
| [`ai_088400B0.c`](../src/libduel_eng/ai_088400B0.c) | 0x088400B0-0x08840B5C | 13 | 0 | IA: checagens de cartas do campo e da mão para a jogada do computador |
| [`ai_08840B5C.c`](../src/libduel_eng/ai_08840B5C.c) | 0x08840B5C-0x088415C4 | 29 | 0 | IA: condições por carta e stubs de chamada final |
| [`ai_088415C4.c`](../src/libduel_eng/ai_088415C4.c) | 0x088415C4-0x088420F4 | 21 | 0 | IA: escolha de alvos e máscaras de zonas |
| [`ai_08842C24.c`](../src/libduel_eng/ai_08842C24.c) | 0x08842C24-0x08843858 | 6 | 1 | IA: máquinas de passos com tabela de saltos (duel_0884356C) |
| [`ai_08843858.c`](../src/libduel_eng/ai_08843858.c) | 0x08843858-0x08844024 | 16 | 1 | IA: condições e escolhas do computador |
| [`ai_08844024.c`](../src/libduel_eng/ai_08844024.c) | 0x08844024-0x0884451C | 26 | 1 | IA: condições curtas por carta |
| [`ai_0884451C.c`](../src/libduel_eng/ai_0884451C.c) | 0x0884451C-0x08844C2C | 21 | 0 | IA: condições curtas por carta |
| [`ai_08844C2C.c`](../src/libduel_eng/ai_08844C2C.c) | 0x08844C2C-0x088451E8 | 13 | 1 | IA: máquinas de passos de escolha |
| [`ai_08846498.c`](../src/libduel_eng/ai_08846498.c) | 0x08846498-0x08846584 | 1 | 0 | IA: ajudante isolado entre duas máquinas de passos grandes |
| [`ai_088480D0.c`](../src/libduel_eng/ai_088480D0.c) | 0x088480D0-0x088482F8 | 2 | 0 | registros dos alvos respondidos pelas cartas especiais (`DuelSpecial.recs` em +0x9C) |
| [`duel_dialog.c`](../src/libduel_eng/duel_dialog.c) | 0x0884ACD0-0x0884B094 | 18 | 1 | pedidos de diálogo ao host, duel_host_call 0x31 por subcomando: mensagens, pergunta com cartas ou texto, sim/não, só para o jogador local |
| [`duel_pick.c`](../src/libduel_eng/duel_pick.c) | 0x0884C718-0x0884D050 | 21 | 1 | lista de cartas que um diálogo oferece, `DuelPick` em 0x0891524C com zona e número de cada carta em bitfield, flags de exibição, gerador aleatório LCG do duelo, tamanho das pilhas, Exodia e cartas de vitória especial |
| [`duel_end.c`](../src/libduel_eng/duel_end.c) | 0x0884D050-0x0884E228 | 14 | 5 | fim do duelo: motivo de vitória de cada jogador em +0x20..+0x2C do `DuelPlayer`, apresentação do fim, reprodução do log de entradas, passos finais por tabela, contadores e flags do registro do duelo em 0x08913C1C, pedido de desistência |
| [`card_desc.c`](../src/libduel_eng/card_desc.c) | 0x0884E228-0x0884F038 | 24 | 0 | descritores de efeito das cartas: quatro tabelas de 0x18 bytes ordenadas por id conforme o tipo da ação, busca binária `static` cujos chamadores guardam argumentos em t-regs, chamada dos handlers de condição, alvo e resolução, máscara e contagens das zonas aceitas pelo alvo |
| [`ai_0884F038.c`](../src/libduel_eng/ai_0884F038.c) | 0x0884F038-0x08850164 | 11 | 0 | IA: resolução de mensagens e escolhas da corrente |
| [`ai_08850164.c`](../src/libduel_eng/ai_08850164.c) | 0x08850164-0x08850F44 | 0 | 1 | máquina de passos da resolução da corrente (em asm) |
| [`ai_08853340.c`](../src/libduel_eng/ai_08853340.c) | 0x08853340-0x08853CE8 | 11 | 0 | IA: ações pendentes da corrente (`DuelChain.pending`) |
| [`ai_08853CE8.c`](../src/libduel_eng/ai_08853CE8.c) | 0x08853CE8-0x08854D4C | 1 | 0 | `duel_act_check_08853CE8`: checagem de 4 KB de ações por id |
| [`ai_08854D4C.c`](../src/libduel_eng/ai_08854D4C.c) | 0x08854D4C-0x08855214 | 6 | 0 | IA: ajudantes de checagem de ação |
| [`ai_08855214.c`](../src/libduel_eng/ai_08855214.c) | 0x08855214-0x08855908 | 1 | 0 | IA: checagem de ação isolada |
| [`ai_08855908.c`](../src/libduel_eng/ai_08855908.c) | 0x08855908-0x08856860 | 14 | 0 | IA: filtros de alvo e condições por carta |
| [`ai_08856860.c`](../src/libduel_eng/ai_08856860.c) | 0x08856860-0x08856EE0 | 0 | 1 | IA: condição por carta (em asm) |
| [`ai_08856EE0.c`](../src/libduel_eng/ai_08856EE0.c) | 0x08856EE0-0x08857B34 | 13 | 0 | IA: filtros de alvo e condições por carta |
| [`ai_08857B34.c`](../src/libduel_eng/ai_08857B34.c) | 0x08857B34-0x08858A30 | 14 | 1 | IA: filtros de alvo e condições por carta |
| [`ai_08858A30.c`](../src/libduel_eng/ai_08858A30.c) | 0x08858A30-0x088594D0 | 5 | 1 | IA: filtros de alvo e condições por carta |
| [`ai_088594D0.c`](../src/libduel_eng/ai_088594D0.c) | 0x088594D0-0x08859AC8 | 6 | 0 | IA: filtros de alvo e condições por carta |
| [`ai_08859AC8.c`](../src/libduel_eng/ai_08859AC8.c) | 0x08859AC8-0x0885B4B8 | 21 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_0885B4B8.c`](../src/libduel_eng/ai_0885B4B8.c) | 0x0885B4B8-0x0885C594 | 17 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_0885C594.c`](../src/libduel_eng/ai_0885C594.c) | 0x0885C594-0x0885D4E4 | 19 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_0885D4E4.c`](../src/libduel_eng/ai_0885D4E4.c) | 0x0885D4E4-0x0885E1DC | 16 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_0885E1DC.c`](../src/libduel_eng/ai_0885E1DC.c) | 0x0885E1DC-0x0885F2E4 | 18 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_0885F2E4.c`](../src/libduel_eng/ai_0885F2E4.c) | 0x0885F2E4-0x088601AC | 17 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_088601AC.c`](../src/libduel_eng/ai_088601AC.c) | 0x088601AC-0x08860A88 | 7 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_08860A88.c`](../src/libduel_eng/ai_08860A88.c) | 0x08860A88-0x0886191C | 5 | 0 | IA: tabelas id→constante em switch (inclui switch aninhado por id e opção) |
| [`ai_0886191C.c`](../src/libduel_eng/ai_0886191C.c) | 0x0886191C-0x08861E7C | 10 | 1 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_08861E7C.c`](../src/libduel_eng/ai_08861E7C.c) | 0x08861E7C-0x08862E24 | 7 | 2 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_08862E24.c`](../src/libduel_eng/ai_08862E24.c) | 0x08862E24-0x08863284 | 7 | 1 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_08863284.c`](../src/libduel_eng/ai_08863284.c) | 0x08863284-0x08864044 | 11 | 1 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`ai_08864044.c`](../src/libduel_eng/ai_08864044.c) | 0x08864044-0x08864DF0 | 8 | 0 | IA: filtros de alvo `(DuelAct *a, unsigned p, int z)` e handlers do computador pelo contador `duel_w_08914904` |
| [`effects_q.c`](../src/libduel_eng/effects_q.c) | 0x08864DF0-0x08865F2C | 24 | 5 | callbacks de descritores de efeito em 0x08864DF0: passos com contador, máscaras de zonas, valores da zona, escolhas na mão |
| [`effects_08865F2C.c`](../src/libduel_eng/effects_08865F2C.c) | 0x08865F2C-0x08867F54 | 27 | 2 | handlers com passos em duel_w_08914904 (carta da mão, ATK/DEF, marcação de cartas do campo, escolha por atributo) |
| [`effects_08867F54.c`](../src/libduel_eng/effects_08867F54.c) | 0x08867F54-0x08869190 | 13 | 2 | handlers de compra, escolha de cartas do campo e da mão, pagamento de PV |
| [`effects_08869190.c`](../src/libduel_eng/effects_08869190.c) | 0x08869190-0x0886AAB4 | 11 | 1 | handlers de efeito e duel_act_resolve_6a230 |
| [`effects_0886AAB4.c`](../src/libduel_eng/effects_0886AAB4.c) | 0x0886AAB4-0x0886CB44 | 9 | 3 | handlers de efeito com listas de ids finais |
| [`act_cond.c`](../src/libduel_eng/act_cond.c) | 0x0886CB44-0x0886E784 | 47 | 3 | condições de ativação das cartas sobre o `DuelAct`: descartar da mão, zonas livres, fase e turno, tributos, cartas nas pilhas, carta especial como alvo, armadilhas como alvo da corrente, contadores e posição na zona, carta especial e mão do oponente, cartas no deck |
| [`act_cond2.c`](../src/libduel_eng/act_cond2.c) | 0x0886F23C-0x0887DB24 | 397 | 32 | mais condições: cemitério e mão, monstros na mão, alvos de uma corrente, cartas que voltam ao cemitério ou ao deck, monstros virados, tributos, cartas especiais, alvos virados para cima de um grupo, carta especial do oponente, descartes de custo, mão e campo cheios, LP baixo, buscas na mão, nível do monstro especial, cartas do oponente mandadas ao cemitério, zonas de magia livres, contadores exigidos na zona, entrada na fase de batalha, somas de contadores, cinco cartas no cemitério, diferença de LP, cartas exigidas no campo, zona F, alvos no cemitério, magia de campo, diferença de 7000 LP, DEF do monstro especial, alvos de ataque do monstro especial, alvos da corrente na zona, maior ATK na mão, bits de estado da zona, monstros de defesa do tipo 0x12, atributos 3 a 6 no campo, cópias no cemitério, atributos dos monstros oferecíveis, cartas na zona F, atributos da carta que causou a ação, nomes no cemitério e na mão, tipo da carta que causou a ação, cartas nomeadas no cemitério, bits 16 e 17 do jogador, cartas 0x19A3/0x19A4, zona de origem pelo valor da zona, nível das cartas da corrente, diferença de 8000 LP, imunidade da zona a 0x1A77, zonas de magia livres do oponente |
| [`effects_p.c`](../src/libduel_eng/effects_p.c) | 0x0887DB24-0x0888146C | 16 | 12 | Card effect handlers (DuelAct *a): what a's card does when it resolves |
| [`effects_r.c`](../src/libduel_eng/effects_r.c) | 0x0888146C-0x08883114 | 24 | 4 | handlers de efeito depois de effects_p: passos por duel_step, alvos e máscaras de zonas, trocas de controle, contadores de turno, compras e descartes |
| [`effects_s.c`](../src/libduel_eng/effects_s.c) | 0x08883114-0x08884118 | 11 | 1 | handlers: virar e destruir pela DEF, cartas viradas para baixo reveladas, trocas de zona de magia, metade do ATK como dano |
| [`effects_t.c`](../src/libduel_eng/effects_t.c) | 0x08884118-0x08884F18 | 8 | 5 | handlers: invocação especial da mão, comandos de virar, ATK/DEF originais guardados na zona, descarte de cartas da mão |
| [`effects_u.c`](../src/libduel_eng/effects_u.c) | 0x08884F18-0x08885B70 | 7 | 2 | handlers: cartas de mesmo nome mandadas de todas as pilhas, fichas invocadas por tabela, mãos devolvidas e recompradas, metade do ATK como dano, ataque forçado |
| [`effects_v.c`](../src/libduel_eng/effects_v.c) | 0x08885B70-0x08886984 | 6 | 5 | handlers: maior ou menor ATK/DEF entre os alvos, três alvos de uma vez, cartas do cemitério devolvidas, carta escolhida na mão do oponente, máscaras de zonas do oponente |
| [`effects_w.c`](../src/libduel_eng/effects_w.c) | 0x08886984-0x08887618 | 16 | 2 | handlers: zonas com um efeito destruídas, cartas viradas para cima todas de uma vez, invocação pela carta especial do turno, descarte e recompra |
| [`effects_x.c`](../src/libduel_eng/effects_x.c) | 0x08887618-0x08889BE4 | 13 | 7 | handlers: troca de controle e de alvo, cartas reveladas da mão do oponente, invocações especiais do deck, da mão e do cemitério com zonas livres por jogador, dano e PV pelo número de cartas no cemitério ou viradas; tabelas de salto no `@data` |
| [`effects_y.c`](../src/libduel_eng/effects_y.c) | 0x08889BE4-0x0888B904 | 17 | 5 | handlers: invocações do deck por jogador alternado, cartas do cemitério e da zona F de volta ao campo, troca de zona com a carta especial, dano igual ao ATK do alvo, máscaras de zonas por tipo de carta |
| [`effects_z.c`](../src/libduel_eng/effects_z.c) | 0x0888B904-0x0888C1A8 | 4 | 6 | handlers que chamam duel_088C87A8 sem protótipo: marcas em cartas do campo pelo jogador da vez, compras dos dois jogadores, carta do cemitério para a zona de magia, carta 0x13EA do deck |
| [`effects_za.c`](../src/libduel_eng/effects_za.c) | 0x0888C1A8-0x0888E5D4 | 21 | 2 | filtro de escolha na mão, troca de três cartas da mão com o oponente, ATK fixo e marcas no alvo, fichas por id da carta, invocação na fase 5, monstros do cemitério de volta pela escolha da lista, troca com a carta especial, marcas com ATK/5 e contadores, invocação pela carta especial do turno, cartas viradas para baixo na zona, troca de monstros entre jogadores, dano fixo, compras da mão |
| [`effects_zb.c`](../src/libduel_eng/effects_zb.c) | 0x0888E5D4-0x0888F420 | 7 | 3 | handlers: máscara de zonas pelos alvos, cartas no cemitério dos dois jogadores, soma de ATK dos monstros controlados, LP dos dois jogadores, marcas por cartas no cemitério, cartas da mão ou do deck para o campo |
| [`effects_zc.c`](../src/libduel_eng/effects_zc.c) | 0x0888F420-0x08890094 | 2 | 3 | handlers: troca de cartas da mão entre os jogadores, carta do cemitério para a zona de magia, contador de níveis 0x1497-0x149A, ataque forçado contra a carta especial |
| [`effects_zd.c`](../src/libduel_eng/effects_zd.c) | 0x08890094-0x08890820 | 4 | 2 | handlers: invocação do deck com pedido ao oponente, dano pelo ATK do monstro escolhido, carta do campo de volta, máscara de monstros com DEF até o ATK do alvo |
| [`effects_ze.c`](../src/libduel_eng/effects_ze.c) | 0x08890820-0x088917D4 | 14 | 0 | handlers: invocação por tributo do próprio monstro, invocação pelo oponente, alvo em S/T com máscara por jogador, carta virada com flip especial |
| [`effects_zf.c`](../src/libduel_eng/effects_zf.c) | 0x088917D4-0x08892554 | 10 | 0 | handlers: dano a todos os monstros do oponente, dano/ganho por contagem de alvos (switch de ids), swap de ATK entre dois alvos, fila de uids em duel_chain, callback de efeito 0x14C0 |
| [`effects_zg.c`](../src/libduel_eng/effects_zg.c) | 0x08892554-0x08894158 | 27 | 0 | handlers: tributo por nível vindo do cemitério, efeitos 0x14C4/0x17C2, alvos duplos, troca de mão entre jogadores, cartas de unk3832 de volta, dano por máscara, fim de turno com tabela de saltos, despacho por id (0x14FD-0x1694), invocação alternada pelos dois jogadores |
| [`effects_zh.c`](../src/libduel_eng/effects_zh.c) | 0x08894158-0x08895C8C | 28 | 1 | handlers: escolha de tipo de carta da mão (dano 700 / destruir a carta especial), descarte do topo do cemitério, contadores por zona, máquinas com tabela de saltos (0x7B-0x80), aposta entre jogadores, recarga da mão até 4, invocação de token por id (0x12A0-0x1A37), adivinhação 0/1, carta da zona D por nome |
| [`effects_zi.c`](../src/libduel_eng/effects_zi.c) | 0x08895C8C-0x088966D8 | 8 | 0 | handlers: carta virada para o dono com troca de alvo, invocação múltipla pelo nível (0x157A/0x1978), dano/destruição por w[2], carta do topo do deck revelada (0x159D) |
| [`effects_088966D8.c`](../src/libduel_eng/effects_088966D8.c) | 0x088966D8-0x08898B10 | 22 | 2 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_08898B10.c`](../src/libduel_eng/effects_08898B10.c) | 0x08898B10-0x0889A75C | 22 | 3 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_0889A75C.c`](../src/libduel_eng/effects_0889A75C.c) | 0x0889A75C-0x0889C150 | 22 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_0889C150.c`](../src/libduel_eng/effects_0889C150.c) | 0x0889C150-0x0889CD38 | 10 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_0889CD38.c`](../src/libduel_eng/effects_0889CD38.c) | 0x0889CD38-0x088A1904 | 64 | 0 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`effects_088A1904.c`](../src/libduel_eng/effects_088A1904.c) | 0x088A1904-0x088A3430 | 25 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088A3430.c`](../src/libduel_eng/effects_088A3430.c) | 0x088A3430-0x088A49F8 | 17 | 2 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088A49F8.c`](../src/libduel_eng/effects_088A49F8.c) | 0x088A49F8-0x088A6464 | 26 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088A6464.c`](../src/libduel_eng/effects_088A6464.c) | 0x088A6464-0x088A67D8 | 2 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088A67D8.c`](../src/libduel_eng/effects_088A67D8.c) | 0x088A67D8-0x088A8B94 | 11 | 3 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`effects_088A8B94.c`](../src/libduel_eng/effects_088A8B94.c) | 0x088A8B94-0x088AF390 | 77 | 7 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`effects_088AF390.c`](../src/libduel_eng/effects_088AF390.c) | 0x088AF390-0x088AFD5C | 11 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088AFD5C.c`](../src/libduel_eng/effects_088AFD5C.c) | 0x088AFD5C-0x088B08B8 | 4 | 2 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B153C.c`](../src/libduel_eng/effects_088B153C.c) | 0x088B153C-0x088B1854 | 4 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B1EAC.c`](../src/libduel_eng/effects_088B1EAC.c) | 0x088B1EAC-0x088B2460 | 5 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B2D0C.c`](../src/libduel_eng/effects_088B2D0C.c) | 0x088B2D0C-0x088B2F34 | 3 | 0 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B3360.c`](../src/libduel_eng/effects_088B3360.c) | 0x088B3360-0x088B4560 | 20 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B4560.c`](../src/libduel_eng/effects_088B4560.c) | 0x088B4560-0x088B7D94 | 60 | 3 | handlers de efeito (63 funções) |
| [`effects_088B7D94.c`](../src/libduel_eng/effects_088B7D94.c) | 0x088B7D94-0x088B9CC8 | 35 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088B9CC8.c`](../src/libduel_eng/effects_088B9CC8.c) | 0x088B9CC8-0x088BB8C0 | 28 | 2 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088BB8C0.c`](../src/libduel_eng/effects_088BB8C0.c) | 0x088BB8C0-0x088BC9E0 | 17 | 1 | handlers de efeito / callbacks de candidatos (trabalho paralelo de 4 agentes, 2026-10-21) |
| [`effects_088BC9E0.c`](../src/libduel_eng/effects_088BC9E0.c) | 0x088BC9E0-0x088C004C | 49 | 4 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`effects_088C004C.c`](../src/libduel_eng/effects_088C004C.c) | 0x088C004C-0x088C134C | 10 | 2 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`effects_088C134C.c`](../src/libduel_eng/effects_088C134C.c) | 0x088C134C-0x088C2F80 | 18 | 3 | handlers de efeito / callbacks de candidatos (agentes paralelos, 2026-10-21) |
| [`duel_checks.c`](../src/libduel_eng/duel_checks.c) | 0x088C2F80-0x088C4448 | 13 | 4 | verificações de estado entre os passos: cartas contínuas cuja condição acabou são destruídas ou perdem o efeito |
| [`duel_lp.c`](../src/libduel_eng/duel_lp.c) | 0x088C4448-0x088C4864 | 9 | 0 | mudanças de PV: dano e recuperação com as cartas que os impedem, comandos 0x25-0x30, evento `DuelLpEv` |
| [`duel_draws.c`](../src/libduel_eng/duel_draws.c) | 0x088C4864-0x088C77D4 | 11 | 7 | pagamento de PV, compra e envio de cartas do deck e da zona D com seus comandos; os passos longos em asm |
| [`duel_cmds.c`](../src/libduel_eng/duel_cmds.c) | 0x088C77D4-0x088C7F18 | 14 | 1 | comandos à apresentação via duel_cmd com 0x8000 para o jogador 1, contador de aninhamento do efeito exibido |
| [`duel_reqs.c`](../src/libduel_eng/duel_reqs.c) | 0x088C87A8-0x088C936C | 13 | 4 | comandos à apresentação sobre uma zona: 0x36 carta chega, 0x37 efeito de uma carta, 0x38 contador, 0x42 movimento com seus efeitos |
| [`duel_reqs2.c`](../src/libduel_eng/duel_reqs2.c) | 0x088C936C-0x088CA4E8 | 5 | 7 | mais comandos à apresentação: virar/mudar posição 0x35, mover entre zonas 0x3D/0x3E, da mão ou do cemitério ao campo 0x3F |
| [`duel_hand.c`](../src/libduel_eng/duel_hand.c) | 0x088CA4E8-0x088CAF24 | 7 | 2 | cartas que saem da mão para o cemitério, a zona D ou o campo, comando 0x33 e evento com o bitfield `DuelMoveEv` |
| [`duel_moves.c`](../src/libduel_eng/duel_moves.c) | 0x088CB400-0x088CBB64 | 13 | 2 | comandos de movimento entre pilhas e campo 0x33/0x3F/0x41, ataques 0x29, embaralhar zonas 0x49, carta saindo de uma zona 0x43 |
| [`duel_counters.c`](../src/libduel_eng/duel_counters.c) | 0x088CC178-0x088CC8EC | 8 | 0 | comandos de valores e contadores numa carta 0x3A-0x3C e as cartas que reagem a eles |
| [`act_apply.c`](../src/libduel_eng/act_apply.c) | 0x088D13CC-0x088D1DF4 | 29 | 1 | wrappers de duel_act_apply 0x088D1094: um efeito do tipo 0xB, 0xD, 0xE ou 0xF numa máscara de zonas, teste de uma zona, carta de ação montada na pilha com `= {0}` |
| [`battle_088D1DF4.c`](../src/libduel_eng/battle_088D1DF4.c) | 0x088D1DF4-0x088D28F8 | 1 | 4 | montagem de eventos de batalha (DuelActX campo a campo) |
| [`req_088D28F8.c`](../src/libduel_eng/req_088D28F8.c) | 0x088D28F8-0x088D6DE8 | 20 | 18 | fila de pedidos do duelo: duel_cmd (despacho com jtbl), duel_req_dispatch, ganho/perda de PV |
| [`req_effect.c`](../src/libduel_eng/req_effect.c) | 0x088D6DE8-0x088D9080 | 3 | 9 | pedidos que mexem nas entradas de efeito e nos valores de uma zona: adicionar, mudar, liberar, ordem de ativação, valor, contadores, virar uma carta, mudar a posição, trocar duas zonas, mover para outra zona ou lugar, mover de qualquer lugar a uma pilha |
| [`req_step.c`](../src/libduel_eng/req_step.c) | 0x088DA5EC-0x088DEB2C | 47 | 10 | pedidos que o motor executa um passo por quadro, `duel_req` 0x08914A30: log de entradas, mensagens, cemitério e mão, invocar e baixar da mão, pedidos que só repassam ao host, mudança de fase, fim de turno, próximo turno, comprar, mandar do deck ao cemitério, efeito e contador numa zona, pôr uma carta num lugar, anunciar, ativar e apontar cartas, banir, mandar a uma pilha, deck (embaralhar, inverter, devolver uma carta), bits de modo, de jogador e de slot; localizações de 16 bits nas mensagens ao host |
| [`rules_check.c`](../src/libduel_eng/rules_check.c) | 0x088DEB2C-0x088DFFC0 | 21 | 2 | carta, id e uid, posição, face e contagem por zona vistas pelas regras, bits de modo, de jogador e de slot, pagar custo, cartas que proíbem ativar, invocar ou baixar, bloqueios dos dois jogadores, quem deve responder |
| [`effect_mod.c`](../src/libduel_eng/effect_mod.c) | 0x088DFFC0-0x088E0890 | 1 | 10 | alocar/liberar entradas de efeito, adicionar e remover por fonte, tipo ou carta |
| [`effect_scan.c`](../src/libduel_eng/effect_scan.c) | 0x088E0890-0x088E1FE0 | 20 | 6 | contagem regressiva das entradas temporárias, limpeza no fim do turno e ao sair de uma zona, troca da zona de origem, contagens e buscas das entradas que uma carta pôs numa zona, zona que uma carta afeta |
| [`effect_list.c`](../src/libduel_eng/effect_list.c) | 0x088E1FE0-0x088E2964 | 26 | 0 | listas de efeitos das zonas, das cartas especiais e da lista de cartas: buscar, contar, somar, percorrer |
| [`effect_live.c`](../src/libduel_eng/effect_live.c) | 0x088E2964-0x088E5800 | 9 | 2 | entrada de efeito ainda em vigor, contagem de bits, cartas usáveis por zona, `switch` por id da carta ativada: se tem alvo, o que espera antes de resolver, quantas escolhas, quem pode responder, de que lado vão as cartas ao cemitério |
| [`card_place.c`](../src/libduel_eng/card_place.c) | 0x088E5800-0x088E5F84 | 11 | 3 | alvos de uma ativação, cópia e troca de cartas, colocar uma carta numa zona de monstro ou de magia/armadilha |
| [`card_find.c`](../src/libduel_eng/card_find.c) | 0x088E5F84-0x088E6998 | 18 | 1 | localização de um uid no campo, nas pilhas e nas cartas especiais, índice de um uid em cada pilha, valores guardados na zona |
| [`piles.c`](../src/libduel_eng/piles.c) | 0x088E6998-0x088E7518 | 16 | 1 | carta de uma zona vista pelas regras, deck e zona C: empilhar, embaralhar, buscar, tirar, comprar, contagens |
| [`deck_setup.c`](../src/libduel_eng/deck_setup.c) | 0x088E7518-0x088E80CC | 5 | 2 | deck pela receita com uids, lista de cartas do duelo, troca das pilhas salvas, tirar e pôr uma carta numa zona |
| [`field_active.c`](../src/libduel_eng/field_active.c) | 0x088E80CC-0x088E8CF4 | 6 | 4 | cópias ativas de uma carta, zonas desativadas, cartas especiais fora das zonas |
| [`field_find.c`](../src/libduel_eng/field_find.c) | 0x088E8CF4-0x088E94E4 | 8 | 2 | cópias de uma carta no campo por id ou nome, ordem de ativação mínima |
| [`field_count.c`](../src/libduel_eng/field_count.c) | 0x088E94E4-0x088EA23C | 19 | 1 | contagens no campo por tipo, atributo, nome, zonas livres |
| [`tribute.c`](../src/libduel_eng/tribute.c) | 0x088EA23C-0x088EAC0C | 8 | 1 | zonas livres na ordem de colocação, monstros que podem ser oferecidos em tributo, tributo duplo, contagens de tributos |
| [`field_st.c`](../src/libduel_eng/field_st.c) | 0x088EAC0C-0x088EB710 | 17 | 0 | cartas no campo por face, zonas de magia/armadilha livres e escolha de zona, contagens no cemitério por tipo, mesmo nome no campo |
| [`protect.c`](../src/libduel_eng/protect.c) | 0x088EB710-0x088EBF44 | 4 | 0 | proteção por 0x1368 com o mesmo nome, verificações antes de agir com um monstro e código de motivo em duel_err |
| [`attack.c`](../src/libduel_eng/attack.c) | 0x088EBF44-0x088EC5D8 | 3 | 0 | restrições de ataque por carta, estado e efeitos, pode atacar e tem algum alvo |
| [`attack_check.c`](../src/libduel_eng/attack_check.c) | 0x088EC5D8-0x088ED2E0 | 1 | 0 | condições de ataque do monstro: proteções, efeitos que impedem ataques, condições por carta, estatísticas e pontos de vida; 1 função de 3,3 KB |
| [`attack_target.c`](../src/libduel_eng/attack_target.c) | 0x088ED2E0-0x088EDE6C | 2 | 0 | alvos de ataque: restrições do atacante, cartas que protegem ou desviam, condições na declaração |
| [`direct_attack.c`](../src/libduel_eng/direct_attack.c) | 0x088EDE6C-0x088EE93C | 4 | 0 | ataque direto: permitido, bloqueado enquanto o oponente tem alvo, cartas que atacam direto |
| [`control.c`](../src/libduel_eng/control.c) | 0x088EE93C-0x088EF170 | 3 | 1 | marca de ataque, controlador do monstro pelos efeitos de troca de controle, duas verificações de colocação |
| [`immunity.c`](../src/libduel_eng/immunity.c) | 0x088EF170-0x088EF5EC | 5 | 0 | imunidade a efeitos e a alvos por carta, por 0x1693, 0x1667 e 0x17A1, imunidade a uma carta ativada |
| [`act_target.c`](../src/libduel_eng/act_target.c) | 0x088EF5EC-0x088EFEE4 | 6 | 1 | alvo e efeito de uma carta ativada sobre o monstro, contagens no campo |
| [`grave.c`](../src/libduel_eng/grave.c) | 0x088EFEE4-0x088F1190 | 21 | 1 | alvos da corrente, contadores no campo, entrada e saída do cemitério e da zona F, carta ao topo, buscas e contagens no cemitério e na zona F, troca da zona D com o cemitério |
| [`hand.c`](../src/libduel_eng/hand.c) | 0x088F1190-0x088F1E5C | 14 | 1 | cartas usáveis do cemitério e da zona F, mão: entrada, saída, buscas, contagens, embaralhamento, nível de carta da lista de trabalho |
| [`field_bonus.c`](../src/libduel_eng/field_bonus.c) | 0x088F1E5C-0x088F2D70 | 2 | 1 | bônus de ATK/DEF das magias de campo por tipo e de cartas nomeadas, bônus de efeitos de monstros e contínuas, ATK/DEF base |
| [`stat_reason.c`](../src/libduel_eng/stat_reason.c) | 0x088F5D54-0x088F6380 | 6 | 0 | motivo de cada carta que altera um monstro, marcadores de todos os slots, acessores de ATK/DEF; 6, com tabela de salto no `@data` |
| [`field_props.c`](../src/libduel_eng/field_props.c) | 0x088F6380-0x088F71D8 | 7 | 0 | nível, tipo e espécie efetivos do monstro, máscara de espécies |
| [`msg.c`](../src/libduel_eng/msg.c) | 0x088F71D8-0x088F7418 | 3 | 0 | mensagens do motor para a apresentação com contagem das pendentes |
| [`effects_a.c`](../src/libduel_eng/effects_a.c) | 0x088F7EB4-0x088F85D4 | 11 | 0 | estado das mensagens, classe de cada comando, primeiros handlers de efeito de carta |
| [`effects_b.c`](../src/libduel_eng/effects_b.c) | 0x088F85D4-0x088F9024 | 4 | 1 | handlers |
| [`effects_c.c`](../src/libduel_eng/effects_c.c) | 0x088F9024-0x088F9668 | 2 | 1 | handlers |
| [`effects_d.c`](../src/libduel_eng/effects_d.c) | 0x088F9668-0x088F9C00 | 5 | 1 | handlers |
| [`effects_e.c`](../src/libduel_eng/effects_e.c) | 0x088F9C00-0x088FA288 | 1 | 2 | handlers |
| [`effects_f.c`](../src/libduel_eng/effects_f.c) | 0x088FA288-0x088FA920 | 2 | 3 | handlers |
| [`effects_g.c`](../src/libduel_eng/effects_g.c) | 0x088FA920-0x088FAEFC | 4 | 0 | handlers |
| [`effects_h.c`](../src/libduel_eng/effects_h.c) | 0x088FAEFC-0x088FB5E4 | 4 | 1 | handlers |
| [`effects_i.c`](../src/libduel_eng/effects_i.c) | 0x088FB5E4-0x088FBAC8 | 5 | 0 | handlers |
| [`effects_j.c`](../src/libduel_eng/effects_j.c) | 0x088FBAC8-0x088FC114 | 6 | 1 | handlers |
| [`effects_k.c`](../src/libduel_eng/effects_k.c) | 0x088FC114-0x088FCA90 | 10 | 0 | handlers; 10; `DuelSrc` em +0x14 do `DuelAct` |
| [`effects_l.c`](../src/libduel_eng/effects_l.c) | 0x088FCA90-0x088FD2DC | 6 | 0 | handlers |
| [`effects_m.c`](../src/libduel_eng/effects_m.c) | 0x088FD2DC-0x088FD690 | 3 | 0 | handlers |
| [`effects_n.c`](../src/libduel_eng/effects_n.c) | 0x088FD690-0x088FDE68 | 6 | 1 | handlers |
| [`effects_o.c`](../src/libduel_eng/effects_o.c) | 0x088FDE68-0x088FE160 | 1 | 1 | handlers |
| [`duel_net.c`](../src/libduel_eng/duel_net.c) | 0x088FE160-0x088FE638 | 10 | 1 | duelo on-line: mensagens de duelo pelos registros do netsync do BOOT, máscara de destino por classe, espera de todos os jogadores, acessores do estado do duelo |
| [`duel_api.c`](../src/libduel_eng/duel_api.c) | 0x088FE638-0x088FF0DC | 71 | 2 | acessores exportados à libduel_draw: LP, contagens e uids por zona, campos dos slots, jogador da vez, fase, tipo de jogador, `DuelSetup` em 0x089139F0 com os decks; jogador de cada lado no duelo em dupla, log de entradas, callback do host |
| [`duel_text.c`](../src/libduel_eng/duel_text.c) | 0x088FF0DC-0x088FF3FC | 5 | 2 | memset/memcpy do módulo, wcscpy/wcscat aceitando id de texto do diálogo, número decimal em UCS-2, substituição de %s/%d |
| [`duel_result.c`](../src/libduel_eng/duel_result.c) | 0x088FF3FC-0x08900C94 | 5 | 2 | resultado de uma série de duelos em dupla, estatísticas do deck para as recompensas, marcas de uma vez no save, atualização dos 40 recordes de duelo, cálculo dos 82 bônus do duelo |
| [`duel_finish.c`](../src/libduel_eng/duel_finish.c) | 0x08900C94-0x08901BA0 | 3 | 2 | fim do duelo: resultados, 118 títulos, tabela de personagens com busca binária |

## libhtml_view (5 arquivos, 16 funções batendo, 0 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`html_net.c`](../src/libhtml_view/html_net.c) | 0x08804000-0x0880439C | 9 | 0 | entrada da cena (ehscript_register), laço por quadro e subida/descida da pilha de rede do visor: módulos de rede, sceNet/Inet/Resolver/Apctl, SSL, HTTP/HTTPS e cookies |
| [`lang_remap.c`](../src/libhtml_view/lang_remap.c) | 0x0880439C-0x08804414 | 1 | 0 | 0x0880439c: maps the config language id to the download-site language index |
| [`html_url.c`](../src/libhtml_view/html_url.c) | 0x08804414-0x088044B8 | 1 | 0 | página inicial do visor: página de download do Tag Force no site da Konami conforme a região (jp, us, eu) |
| [`save_path.c`](../src/libhtml_view/save_path.c) | 0x088044B8-0x08804508 | 1 | 0 | 0x088044b8: builds "/PSP/SAVEDATA/<GAMEID>" |
| [`html_viewer.c`](../src/libhtml_view/html_viewer.c) | 0x08804508-0x088047F4 | 4 | 0 | sessão do sceUtilityHtmlViewer: parâmetros (pasta de download = pasta do save, 3 abas, 6 MB de memória no .bss) e a máquina de estados rede → visor → fim |

## liblabo (2 arquivos, 12 funções batendo, 0 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`labo_menu.c`](../src/liblabo/labo_menu.c) | 0x08804000-0x08804AD0 | 6 | 0 | entrada da cena 9 "Labo Debug Menu" (menu de desenvolvimento que ficou no jogo): máquina de estados com 12 estados, carga sob demanda dos gmodules de sub-tela, tabela de desenho por estado |
| [`voicetest.c`](../src/liblabo/voicetest.c) | 0x08804AD0-0x088054F8 | 6 | 0 | teste de vozes do menu Labo ("Japanese Only"): personagem/sub/pack/voz, BGM e voz, fala lida de labo/voicetest.ehp; tabelas de BGM, formatos e rótulos SJIS |

## libtutoriallist (4 arquivos, 25 funções batendo, 1 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`tlist_main.c`](../src/libtutoriallist/tlist_main.c) | 0x08804000-0x088042F4 | 5 | 0 | entrada da lista de tutoriais (init/update/result/draw, importada por libtitle, libstory e liblabo): heap do chamador ou FPL de 2 MB, lição pré-selecionada, fades, resultado |
| [`tlist_draw.c`](../src/libtutoriallist/tlist_draw.c) | 0x088042F4-0x08805B0C | 9 | 1 | desenho: fundo bg.ehf, título, janela com duas abas, linhas com número e título em moldura 9-slice, cursor pulsante, barra de rolagem |
| [`tlist_res.c`](../src/libtutoriallist/tlist_res.c) | 0x08805B0C-0x08805F10 | 9 | 0 | leitura assíncrona de database/tutoriallist.ehp e tabela de recursos por id (qsort, busca binária, cache), letra do idioma |
| [`tlist_list.c`](../src/libtutoriallist/tlist_list.c) | 0x08805F10-0x088065B0 | 2 | 0 | montagem das duas listas pelas lições concluídas (8-23 na aba 0, 0-7 em "básico"), altura dos títulos, rolagem até a pré-selecionada, entrada do pad |

## libduelrecord (3 arquivos, 26 funções batendo, 1 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`drec_main.c`](../src/libduelrecord/drec_main.c) | 0x08804000-0x08804690 | 13 | 0 | entrada do registro de duelos (init/update/draw, importada por libtitle, libstory e liblabo): máquina de estados da tela, carga de database/duelrecord.ehp e busca dos arquivos por id |
| [`drec_draw.c`](../src/libduelrecord/drec_draw.c) | 0x08804690-0x088060A4 | 9 | 1 | desenho: cabeçalho com porcentagem, abas, linhas (número, texto, valor, tempo mm:ss, marca de novo piscando), barra de rolagem |
| [`drec_build.c`](../src/libduelrecord/drec_build.c) | 0x088060A4-0x08806A2C | 4 | 0 | montagem das três abas a partir do save (40 contadores, 83 itens, 118 itens; aba 2 só com sysflag 7), porcentagem de conclusão, entrada do pad, tabelas de itens |

## libpassword (4 arquivos, 33 funções batendo, 0 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`password_main.c`](../src/libpassword/password_main.c) | 0x08804000-0x08804844 | 11 | 0 | entrada da Password Machine, despacho dos estados, recursos de labo/password.ehp (leitura assíncrona, tabela id→nome com qsort e busca binária, letra do idioma) |
| [`draw.c`](../src/libpassword/draw.c) | 0x08804844-0x0880597C | 8 | 0 | desenho: fundo, monitor, janela de mensagens com texto rolando e cursor piscante, imagem da carta em 154 linhas com ondulação senoidal, teclado e dígitos (EHF) |
| [`states.c`](../src/libpassword/states.c) | 0x0880597C-0x08807460 | 8 | 0 | cenas: introdução, teclado de 8 dígitos, conferência da senha (card_id_from_password, faixa 0xFA7-0x1B2D, 6 ids excluídos), carta encontrada e adicionada, não encontrada, sair |
| [`keypad.c`](../src/libpassword/keypad.c) | 0x08807460-0x088078D0 | 6 | 0 | layout do teclado 3x4 e cursor, faixas de animação de key.ehf/nixi.ehf, ids de carta excluídos |

## libduel_mgr (5 arquivos, 97 funções batendo, 61 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`dmgr_main.c`](../src/libduel_mgr/dmgr_main.c) | 0x08804000-0x08805FF8 | 22 | 1 | entrada da cena 6, thread da cena (carga de libduel_draw e codecs, troca de deck, tutorial, duelo, resultado, rede), callbacks de saída e desenho, preparação do duelo (jogadores, decks, LP, música) |
| [`dmgr_event.c`](../src/libduel_mgr/dmgr_event.c) | 0x08805FF8-0x08808A20 | 27 | 4 | fila de eventos de apresentação do duelo (cut-ins de personagem por carta-assinatura, música pelo humor dos LP, troca de turno e de parceiro, fim do duelo), despacho do callback do motor e reprodução quadro a quadro |
| [`dmgr_janken.c`](../src/libduel_mgr/dmgr_janken.c) | 0x08808A20-0x0880C8E8 | 18 | 8 | menu de início do duelo: pedra-papel-tesoura para o primeiro turno (local ou ad hoc), jogar primeiro ou segundo, troca de decks dos parceiros pela rede, desenho |
| [`dmgr_partner.c`](../src/libduel_mgr/dmgr_partner.c) | 0x0880C8E8-0x0880F9C0 | 21 | 17 | painel de parceiros e adversários (rostos, nomes, decks, animações EHF), tabela de rostos por personagem, utilitários de texto e sprite |
| [`dmgr_result.c`](../src/libduel_mgr/dmgr_result.c) | 0x0880F9C0-0x08815888 | 9 | 31 | tela de resultado (bônus, recordes e títulos em páginas com rolagem, pagamento em dinheiro, placar da série, diálogos de revanche e de espera da rede) |

## libselect_card (15 arquivos, 123 funções batendo, 84 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`sc_main.c`](../src/libselect_card/sc_main.c) | 0x08804000-0x08804D0C | 18 | 0 | entrada, init/update/unload, packs EHP e recursos por id, buscas por id e tipo→ícone |
| [`sc_parts.c`](../src/libselect_card/sc_parts.c) | 0x08804D0C-0x08807630 | 21 | 3 | peças de desenho (dígitos, molduras, ícones, ATK/DEF, figura da carta, marcas, linha do tipo, contadores, barra de rolagem com setas, janela de ajuda) |
| [`sc_draw.c`](../src/libselect_card/sc_draw.c) | 0x08807630-0x08808040 | 7 | 1 | desenho por quadro (export), camada de popups, fundo bg.ehf, janela do tipo, linhas separadoras e título |
| [`sc_panes.c`](../src/libselect_card/sc_panes.c) | 0x08808040-0x088099CC | 3 | 6 | cabeçalhos dos painéis, contagens das abas, grade de figuras, linhas largas/estreitas |
| [`sc_lists.c`](../src/libselect_card/sc_lists.c) | 0x088099CC-0x0880AD2C | 15 | 1 | listas dos dois painéis (ids do chamador, troca/conversão, aluguel, parceiro), adicionar/remover, totais, gravação do resultado |
| [`sc_input.c`](../src/libselect_card/sc_input.c) | 0x0880AD2C-0x0880C940 | 6 | 12 | layouts/modos/abas por tipo e idioma, despacho da entrada, cursores grade/lista, paginação, saltos por grupo, moldura de foco |
| [`sc_state.c`](../src/libselect_card/sc_state.c) | 0x0880C940-0x0880D54C | 7 | 4 | carta sob o cursor, tela de informação, preparação após a carga, troca de aba, ajuste de rolagem, histórico de desfazer |
| [`sc_info.c`](../src/libselect_card/sc_info.c) | 0x0880D54C-0x0880E64C | 8 | 4 | faixa de informação da carta, ícones, botão com legenda, sombra do painel, valor da ordenação, abertura do menu de ações |
| [`sc_action.c`](../src/libselect_card/sc_action.c) | 0x0880E64C-0x0880FFCC | 10 | 12 | menu de ações e submenus (marca, informação, limpar/restaurar, layout, marca de novidade) |
| [`sc_sort.c`](../src/libselect_card/sc_sort.c) | 0x0880FFCC-0x08810BF4 | 6 | 3 | menu de ordenação ao vivo, ícones de ordem/direção |
| [`sc_filter.c`](../src/libselect_card/sc_filter.c) | 0x08810BF4-0x088119C8 | 3 | 10 | ordenação estável pelas nove chaves, valores das chaves, filtro da marca, busca incremental com histórico, filtros por tipo/atributo/raça |
| [`sc_match.c`](../src/libselect_card/sc_match.c) | 0x088119C8-0x088121D0 | 3 | 4 | filtros restantes e busca de texto (Horspool) em nome/texto normalizados |
| [`sc_fmenu.c`](../src/libselect_card/sc_fmenu.c) | 0x088121D0-0x08813CF4 | 7 | 8 | menu de filtro (grade de valores por página), ícones/legendas dos valores, abertura da janela de palavra |
| [`sc_word.c`](../src/libselect_card/sc_word.c) | 0x08813CF4-0x08815168 | 5 | 6 | busca por palavra (teclado do sistema, histórico, barra de progresso) |
| [`sc_move.c`](../src/libselect_card/sc_move.c) | 0x08815168-0x08816DD4 | 4 | 10 | atualização do teclado do sistema, mover carta entre painéis com voo em curva e desfazer, popup de mensagens e confirmação de saída |

## libcardalbum (7 arquivos, 62 funções batendo, 6 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`ca_main.c`](../src/libcardalbum/ca_main.c) | 0x08804000-0x08804408 | 4 | 0 | entrada, init/update exportados e liberação do álbum |
| [`ca_res.c`](../src/libcardalbum/ca_res.c) | 0x08804408-0x08804D08 | 13 | 0 | recursos (database/cardalbum.ehp, texturas e modelos por id, leitores CIP, fila de pré-carga de figuras) |
| [`ca_gfx.c`](../src/libcardalbum/ca_gfx.c) | 0x08804D08-0x08805CA4 | 10 | 1 | parte 3D: estado do GE, luz, câmera, capa, modelos das páginas com as figuras como textura, cursor 3D |
| [`ca_draw.c`](../src/libcardalbum/ca_draw.c) | 0x08805CA4-0x0880681C | 4 | 2 | parte 2D: fundo, barra de título com percentual, menu de saltos, mão do cursor, dígitos |
| [`ca_input.c`](../src/libcardalbum/ca_input.c) | 0x0880681C-0x08807CD0 | 12 | 1 | entrada (células, cantos, menu), pré-carga dos lados visíveis, carta sob o cursor, virada de página e salto animado |
| [`ca_list.c`](../src/libcardalbum/ca_list.c) | 0x08807CD0-0x08808E5C | 14 | 1 | lista do álbum em ordem de sort, salto para índice/grupo, tela de detalhe, aceleração de virada, objetos de página |
| [`ca_index.c`](../src/libcardalbum/ca_index.c) | 0x08808E5C-0x08809B30 | 5 | 1 | janela de índice por letra inicial (kana/alfabeto) e páginas extras |

## libcharalist (4 arquivos, 41 funções batendo, 9 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`cl_res.c`](../src/libcharalist/cl_res.c) | 0x08804000-0x08804810 | 12 | 0 | database/charalist.ehp, recursos por id, bust-ups compactados de field/bustup/ e tabelas de voz |
| [`cl_main.c`](../src/libcharalist/cl_main.c) | 0x08804810-0x08804F48 | 7 | 0 | entrada, init/update/resultado exportados, sorteio de duelista para a história e liberação |
| [`cl_draw.c`](../src/libcharalist/cl_draw.c) | 0x08804F48-0x08807F68 | 18 | 3 | janela com três abas, linhas de duelista (rosto, número, nome, nível, estrelas, retrospecto), cursor, barras de rolagem e página de detalhe |
| [`cl_list.c`](../src/libcharalist/cl_list.c) | 0x08807F68-0x0880A34C | 4 | 6 | abas montadas a partir dos encontros, entrada da lista e do detalhe (roupas, perfil, teste de voz), tabela de personagens e retrato |

## libconv_machine (4 arquivos, 44 funções batendo, 3 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`cm_main.c`](../src/libconv_machine/cm_main.c) | 0x08804000-0x08804B50 | 15 | 0 | entrada, init/update exportados, liberação, carga dos pacotes e arquivos por idioma, busca de recursos e modelos |
| [`cm_draw.c`](../src/libconv_machine/cm_draw.c) | 0x08804B50-0x08805FF8 | 8 | 2 | desenho exportado, fundo, painel da máquina, animações EHF, janela de mensagem, cartas, clarão, escurecimento, nome e dica |
| [`cm_gfx.c`](../src/libconv_machine/cm_gfx.c) | 0x08805FF8-0x08806D4C | 9 | 1 | estado do GE, luzes, câmeras, tampa da máquina, cartas entrando e carta saindo com textura renderizada |
| [`cm_states.c`](../src/libconv_machine/cm_states.c) | 0x08806D4C-0x08808A38 | 12 | 0 | estados (introdução, escolha das cartas, inserção, conversão, resultado com nome e ficha, pergunta de repetir), sorteio da carta pelo grupo, medidor e slots de animação |

## liblimitlist (8 arquivos, 55 funções batendo, 7 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`ll_util.c`](../src/liblimitlist/ll_util.c) | 0x08804000-0x08804128 | 3 | 0 | índice no fileTbl.dat, busca de par (carta, valor), ícone do tipo e a lista oficial de proibidas/limitadas |
| [`ll_draw.c`](../src/liblimitlist/ll_draw.c) | 0x08804128-0x08805948 | 14 | 0 | números, moldura 9-slice, ícones, ATK/DEF, figura da carta (arte alternativa), faixa do tipo, contadores, setas, barra de rolagem |
| [`ll_scroll.c`](../src/liblimitlist/ll_scroll.c) | 0x08805948-0x08805A00 | 0 | 2 | rolagem do texto pelo analógico |
| [`ll_res.c`](../src/liblimitlist/ll_res.c) | 0x08805A00-0x088060C4 | 9 | 0 | packs deck/all e limitlist.ehp assíncronos, tabelas id->recurso (qsort/bsearch), leitor CIP, lista de cartas |
| [`ll_main.c`](../src/liblimitlist/ll_main.c) | 0x088060C4-0x08806490 | 4 | 0 | entrada, init (heap de 5 MB), máquina de estados, liberação em passos |
| [`ll_screen.c`](../src/liblimitlist/ll_screen.c) | 0x08806490-0x0880740C | 6 | 2 | desenho da tela: fundo, abas com tamanhos, cabeçalho, linhas ou grade, contadores deck/parceiro/posse |
| [`ll_list.c`](../src/liblimitlist/ll_list.c) | 0x0880740C-0x08808934 | 10 | 3 | montagem das listas, cursor lista/grade com paginação, abas, ficha da carta, verificação dos decks |
| [`ll_menu.c`](../src/liblimitlist/ll_menu.c) | 0x08808934-0x08809538 | 9 | 0 | menus de ajuda, aviso de carta ilegal, listas (oficial/limpar/dados extras) |

## librecipe_viewer (7 arquivos, 50 funções batendo, 19 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`rv_res.c`](../src/librecipe_viewer/rv_res.c) | 0x08804000-0x08804778 | 9 | 0 | packs deck/all (ou o do chamador) e deck/recipe_view, tabelas id->recurso, CIP, lista de cartas |
| [`rv_util.c`](../src/librecipe_viewer/rv_util.c) | 0x08804778-0x08804918 | 4 | 0 | índice no fileTbl.dat, busca de par, ícone do tipo, parceiros que têm a carta |
| [`rv_draw.c`](../src/librecipe_viewer/rv_draw.c) | 0x08804918-0x08805874 | 8 | 3 | dígitos em sprite, números, moldura, figura e marcas da carta, botão com texto, setas, rolagem analógica |
| [`rv_screen.c`](../src/librecipe_viewer/rv_screen.c) | 0x08805874-0x08807A44 | 11 | 5 | fundo, título por modo, contagem/barras por tipo, grade da receita, listas com barra de rolagem, cursor |
| [`rv_list.c`](../src/librecipe_viewer/rv_list.c) | 0x08807A44-0x08809274 | 5 | 7 | listas dos dados extras, entrada da lista e da grade, ficha da carta, piscar do menu, animações, montagem da grade |
| [`rv_menu.c`](../src/librecipe_viewer/rv_menu.c) | 0x08809274-0x0880A248 | 9 | 4 | cartas por linha, entrada sob o cursor, revelação, menus de confirmação/opções, teclado na tela |
| [`rv_main.c`](../src/librecipe_viewer/rv_main.c) | 0x0880A248-0x0880A99C | 4 | 0 | entrada, init (listas, heap de 4 MB), update com resultado, liberação |

## libtitle (30 arquivos, 381 funções batendo, 53 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`t_main.c`](../src/libtitle/t_main.c) | 0x08804000-0x08804348 | 6 | 0 | entrada do módulo, thread da cena 13, init e callbacks |
| [`t_menu.c`](../src/libtitle/t_menu.c) | 0x08804348-0x088056EC | 16 | 0 | menu principal (checagem do save, lista principal e sublistas) |
| [`t_sub.c`](../src/libtitle/t_sub.c) | 0x088056EC-0x088062B4 | 27 | 0 | sub-telas carregadas pelo título (PRX, callbacks, init/update/draw) |
| [`t_state.c`](../src/libtitle/t_state.c) | 0x088062B4-0x08806EF0 | 13 | 0 | troca de estado após o fade, avisos de save, fluxo de novo jogo |
| [`t_update.c`](../src/libtitle/t_update.c) | 0x08806EF0-0x08807C98 | 10 | 0 | update/draw por quadro (despacho por estado, boot, logos, filme) |
| [`t_gfx.c`](../src/libtitle/t_gfx.c) | 0x08807C98-0x08808D48 | 32 | 1 | recursos e desenho (logos, pacotes por idioma, mensagens, sprites, fade) |
| [`t_tmenu.c`](../src/libtitle/t_tmenu.c) | 0x08808D48-0x08809DC4 | 8 | 2 | menus de painéis (fade escalonado, cursor, aceitar/cancelar) |
| [`t_scene.c`](../src/libtitle/t_scene.c) | 0x08809DC4-0x0880A41C | 5 | 1 | saída para o duelo (GameState de duelo aleatório, lições, duelo livre, tutoriais) |
| [`t_res.c`](../src/libtitle/t_res.c) | 0x0880A41C-0x0880AFA0 | 15 | 1 | camadas EHF, placa da tela, módulos de sub-tela, reset do som |
| [`t_screen.c`](../src/libtitle/t_screen.c) | 0x0880AFA0-0x0880B7D0 | 7 | 0 | tela de título, novo jogo, continuar |
| [`t_screen2.c`](../src/libtitle/t_screen2.c) | 0x0880B7D0-0x0880C0F0 | 10 | 1 | tela de opções do menu |
| [`t_msg.c`](../src/libtitle/t_msg.c) | 0x0880C0F0-0x0880C998 | 10 | 2 | janela de mensagem (até três linhas, paginação) |
| [`t_cfg.c`](../src/libtitle/t_cfg.c) | 0x0880C998-0x0880D454 | 8 | 0 | tela de opções (estado 17) e cópia das opções do sistema |
| [`t_cfgdraw.c`](../src/libtitle/t_cfgdraw.c) | 0x0880D454-0x0880EB20 | 2 | 6 | desenho das opções (grupos, medidor de volume, setas, ON/OFF) |
| [`t_lang.c`](../src/libtitle/t_lang.c) | 0x0880EB20-0x0880F438 | 6 | 0 | seleção de idioma (estado 19), recarga do pacote e do banco de cartas |
| [`t_osk.c`](../src/libtitle/t_osk.c) | 0x0880F438-0x0880F900 | 0 | 2 | teclado na tela (SceUtility OSK), mesmo código da librecipe_viewer |
| [`t_name.c`](../src/libtitle/t_name.c) | 0x0880F900-0x088106D8 | 14 | 0 | novo jogo (estado 8: nome, confirmação, gravação) e title_prompt |
| [`t_panel.c`](../src/libtitle/t_panel.c) | 0x088106D8-0x08813088 | 20 | 7 | caixa de células com rolagem, abas, janela com abas, quadros de nove peças, cursor piscante, caixas prontas e ajudas |
| [`t_ext.c`](../src/libtitle/t_ext.c) | 0x08813088-0x08813AC4 | 8 | 0 | cartas de dados extras (estado 6): .YGC do Memory Stick, carta nova recebida em três cópias |
| [`t_freeduel.c`](../src/libtitle/t_freeduel.c) | 0x08813AC4-0x0881494C | 8 | 0 | free duel: listas de jogadores e decks, painel de opções, despacho das sub-telas, eventos da rede ad hoc |
| [`t_fdnet.c`](../src/libtitle/t_fdnet.c) | 0x0881494C-0x088162CC | 9 | 1 | free duel ad hoc: pacotes de jogador, máquina de troca de decks, GameState do duelo em rede, prontidão e assentos |
| [`t_fdvscpu.c`](../src/libtitle/t_fdvscpu.c) | 0x088162CC-0x08818A88 | 23 | 2 | free duel: despacho do desenho, duelo vs CPU, configuração do duelo (cursor), regulamento e grade de jogadores |
| [`t_fdduelist.c`](../src/libtitle/t_fdduelist.c) | 0x08818A88-0x088195B4 | 6 | 0 | free duel: escolha de duelista |
| [`t_fdadhoc.c`](../src/libtitle/t_fdadhoc.c) | 0x088195B4-0x0881BA64 | 9 | 1 | free duel: topo do modo ad hoc (painéis, handlers, desenho) |
| [`t_fdlobby.c`](../src/libtitle/t_fdlobby.c) | 0x0881BA64-0x0881E0CC | 28 | 4 | free duel: lobby da senha (cartas via libselect_card), info do duelo, entrada vs CPU, lista de opções |
| [`t_fddraw.c`](../src/libtitle/t_fddraw.c) | 0x0881E0CC-0x0881F02C | 9 | 1 | free duel: títulos das caixas, ícones, escolha de personagem (libcharalist), mensagem de entrada |
| [`t_partner.c`](../src/libtitle/t_partner.c) | 0x0881F02C-0x08821D3C | 22 | 17 | painel de parceiros/adversários (cópia do dmgr_partner.c com quatro funções a mais) |
| [`t_duel.c`](../src/libtitle/t_duel.c) | 0x08821D3C-0x08824DA8 | 38 | 1 | duelo contra o computador (estado 12, item 1 do menu): regras (tag, troca de deck, LP, tempo), parceiro, adversários, personagem pela libcharalist |
| [`t_deckpick.c`](../src/libtitle/t_deckpick.c) | 0x08824DA8-0x0882647C | 11 | 2 | escolha de deck das configurações de duelo (estado 12 e ad hoc): deck próprio e salvos, receitas, arquivos dos dados extras, linhas com dono e registro, repetição acelerada do cursor |
| [`t_chr.c`](../src/libtitle/t_chr.c) | 0x0882647C-0x08826B8C | 4 | 1 | números com a fonte e com sprites de dígitos, retrato do personagem, tabela de rostos |

## libdeck (21 arquivos, 275 funções batendo, 53 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`dk_lists.c`](../src/libdeck/dk_lists.c) | 0x08804000-0x088060B0 | 21 | 2 | listas de cartas do editor (principal, side, extra, baú, view do extra, alugadas ao parceiro): montagem a partir do sysstate, inclusão/remoção, ordem visível, totais, gravar/carregar deck |
| [`dk_move.c`](../src/libdeck/dk_move.c) | 0x088060B0-0x0880931C | 10 | 7 | mover carta entre listas: janela de destino, voo em curva, desfazer, limites de cópias/proibidas, posição dos espaços |
| [`dk_input.c`](../src/libdeck/dk_input.c) | 0x0880931C-0x0880A694 | 4 | 8 | entrada da tela de listas: despacho pelo estado, teclas de mover/ação/aba/layout, L/R, cursores, rolagem analógica, moldura de foco |
| [`dk_mode.c`](../src/libdeck/dk_mode.c) | 0x0880A694-0x0880B874 | 7 | 3 | verificação do deck antes de gravar, popup de mensagens, troca de layout dos painéis, abas, animação do fundo |
| [`dk_main.c`](../src/libdeck/dk_main.c) | 0x0880B874-0x0880BD30 | 7 | 0 | entrada do módulo, thread da cena (init, laço, fade, gravação), saída, callbacks de desenho |
| [`dk_pane.c`](../src/libdeck/dk_pane.c) | 0x0880BD30-0x0880C728 | 6 | 4 | sprites da lista, entrada do cursor, texto da carta sob o cursor, troca de lista no painel, limites de rolagem, desfazer |
| [`dk_scene.c`](../src/libdeck/dk_scene.c) | 0x0880C728-0x0880D6C0 | 9 | 0 | recompensas ao entrar pela história, nome do deck, atualização por quadro e sub-telas (info, ajuda, librecipe_viewer, libdeck_tutorial) |
| [`dk_res.c`](../src/libdeck/dk_res.c) | 0x0880D6C0-0x0880DBD0 | 9 | 0 | pacote deck/all_%c.ehp assíncrono, leitor de imagens de carta, tabela de recursos e busca por id |
| [`dk_draw.c`](../src/libdeck/dk_draw.c) | 0x0880DBD0-0x0880E4D0 | 8 | 0 | callback de desenho, camada de popups, bg.ehf, d_window0.ehf, nome do deck, linhas dos painéis |
| [`dk_panes.c`](../src/libdeck/dk_panes.c) | 0x0880E4D0-0x0880F1E4 | 5 | 1 | cabeçalhos dos painéis, cópias nas abas, cartas do deck por atributo |
| [`dk_cards.c`](../src/libdeck/dk_cards.c) | 0x0880F1E4-0x088103A0 | 3 | 4 | cartas em grade ou linhas e faixa da carta sob o cursor com descrição rolando |
| [`dk_icons.c`](../src/libdeck/dk_icons.c) | 0x088103A0-0x08810CD0 | 7 | 1 | ícones de carta, botão com rótulo, sombra de painel/aba, balão do valor de ordenação |
| [`dk_sortmenu.c`](../src/libdeck/dk_sortmenu.c) | 0x08810CD0-0x08812510 | 13 | 2 | menu de ordenação (chaves e direção, reordenação ao vivo) e lista de tipos do menu de filtro |
| [`dk_fmenu.c`](../src/libdeck/dk_fmenu.c) | 0x08812510-0x08813268 | 6 | 2 | janela de valores do filtro, ícones dos valores, marca e rótulo |
| [`dk_action.c`](../src/libdeck/dk_action.c) | 0x08813268-0x08815280 | 25 | 1 | menu de ações da carta e submenus (info, favorito, ordenar/filtrar/buscar, operações do deck, layout, novidade) |
| [`dk_parts.c`](../src/libdeck/dk_parts.c) | 0x08815280-0x08817D18 | 25 | 3 | peças de desenho comuns (dígitos, molduras, ícones, ATK/DEF, imagem, contadores, barra de rolagem, ajuda dos botões) |
| [`dk_filter.c`](../src/libdeck/dk_filter.c) | 0x08817D18-0x088192C8 | 10 | 10 | filtros e ordenação das listas (busca incremental, filtros por tipo/raça/nível/limite/gênero, Horspool, ordenação estável por 9 chaves) |
| [`dk_word.c`](../src/libdeck/dk_word.c) | 0x088192C8-0x0881A69C | 9 | 2 | busca por palavra (janela com histórico, teclado do sistema, barra de progresso) |
| [`dk_osk.c`](../src/libdeck/dk_osk.c) | 0x0881A69C-0x0881AB64 | 2 | 0 | teclado do sistema (SceUtilityOsk) para o nome do deck |
| [`dk_help_ui.c`](../src/libdeck/dk_help_ui.c) | 0x0881AB64-0x0881D918 | 72 | 2 | widgets da ajuda: barra de botões, lista rolável por callbacks, tópicos, páginas, painel de texto com imagem, sprites em nove/três partes |
| [`dk_help.c`](../src/libdeck/dk_help.c) | 0x0881D918-0x0881E870 | 17 | 1 | tela de ajuda (duelo e editor): abertura por id, estados com fade, imagem da página lida de help/ |

## libshop (17 arquivos, 187 funções batendo, 36 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`shop_main.c`](../src/libshop/shop_main.c) | 0x08804000-0x088052F8 | 27 | 1 | entradas da cena (init/update/draw/term), recursos por id (bsearch), número, 2D, prateleira |
| [`shop_gfx.c`](../src/libshop/shop_gfx.c) | 0x088052F8-0x08805AB4 | 8 | 0 | camadas 3D da loja e das cartas, estado do GE, seleção, espera do pack do deck, compra gravada |
| [`shop_state.c`](../src/libshop/shop_state.c) | 0x08805AB4-0x08807CAC | 6 | 5 | máquina de estados da loja, código secreto, entrada principal e da prateleira, passos da compra |
| [`shop_clerk.c`](../src/libshop/shop_clerk.c) | 0x08807CAC-0x08808B44 | 13 | 0 | modelos, a vendedora (carga, deslize, piscar, fala, voz) |
| [`shop_msgwin.c`](../src/libshop/shop_msgwin.c) | 0x08808B44-0x08809D2C | 17 | 3 | janela de mensagens com texto digitado |
| [`shop_points.c`](../src/libshop/shop_points.c) | 0x08809D2C-0x0880A960 | 16 | 0 | contador de DP |
| [`shop_packs.c`](../src/libshop/shop_packs.c) | 0x0880A960-0x0880C238 | 11 | 3 | estado, preço, nome e desbloqueio dos packs, estoque, porcentagem de coleção |
| [`shop_shelf.c`](../src/libshop/shop_shelf.c) | 0x0880C238-0x0880D298 | 9 | 3 | contador, gasto, prateleira e camadas |
| [`shop_list.c`](../src/libshop/shop_list.c) | 0x0880D298-0x0880EF74 | 9 | 4 | lista das cartas compradas (linhas, contadores, cursor, balão da chave) |
| [`shop_parts.c`](../src/libshop/shop_parts.c) | 0x0880EF74-0x088117B8 | 23 | 3 | peças de UI (mesma fonte de sc_parts da libselect_card) |
| [`shop_open.c`](../src/libshop/shop_open.c) | 0x088117B8-0x08812C60 | 6 | 6 | abertura dos packs comprados, cartas somadas à lista |
| [`shop_anim.c`](../src/libshop/shop_anim.c) | 0x08812C60-0x08813808 | 3 | 3 | animação do pack aberto e curvas das cartas |
| [`shop_detail.c`](../src/libshop/shop_detail.c) | 0x08813808-0x08813FF4 | 9 | 0 | menu de uma carta comprada |
| [`shop_sort.c`](../src/libshop/shop_sort.c) | 0x08813FF4-0x08814BE8 | 8 | 1 | diálogo de ordenação (chaves e direção, reordenação ao vivo) |
| [`shop_dialog.c`](../src/libshop/shop_dialog.c) | 0x08814BE8-0x08814F58 | 3 | 0 | diálogos sim/não (sair, comprar, abrir a lista) |
| [`shop_glow.c`](../src/libshop/shop_glow.c) | 0x08814F58-0x08817DD4 | 15 | 3 | brilho das cartas raras (barra de luz com scissor, efeitos super/ultra), ajudantes de GE e matriz |
| [`shop_lsort.c`](../src/libshop/shop_lsort.c) | 0x08817DD4-0x08818554 | 4 | 1 | ordenação da lista (13 chaves, qsort estável), valor mostrado na linha |

## libdeck_swap (20 arquivos, 156 funções batendo, 48 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`ds_res.c`](../src/libdeck_swap/ds_res.c) | 0x08804000-0x08804608 | 10 | 0 | packs deck/all_%c.ehp e deck/deckswap.ehp, recursos por id, passos da carga e montagem da tela |
| [`ds_state.c`](../src/libdeck_swap/ds_state.c) | 0x08804608-0x08804E2C | 4 | 4 | entrada do cursor, texto de informação, troca de aba, rolagem, histórico de desfazer |
| [`ds_lists.c`](../src/libdeck_swap/ds_lists.c) | 0x08804E2C-0x08805E58 | 13 | 1 | listas principal/side/extra montadas do RecipeData com as cartas dos parceiros fixas, totais, gravação do deck |
| [`ds_mode.c`](../src/libdeck_swap/ds_mode.c) | 0x08805E58-0x08806654 | 3 | 4 | layouts dos painéis, abas, animação da janela e do fluxo de troca |
| [`ds_draw.c`](../src/libdeck_swap/ds_draw.c) | 0x08806654-0x08806F0C | 1 | 2 | export de desenho por quadro, camada de popups, faixa de informação |
| [`ds_icons.c`](../src/libdeck_swap/ds_icons.c) | 0x08806F0C-0x08807828 | 6 | 2 | ícones, botão com rótulo, sombra do painel, detalhe |
| [`ds_bg.c`](../src/libdeck_swap/ds_bg.c) | 0x08807828-0x08808A54 | 12 | 1 | fundo, janela do fluxo com a diferença para o tamanho do deck, cabeçalhos e totais, atributos, tempo restante |
| [`ds_cards.c`](../src/libdeck_swap/ds_cards.c) | 0x08808A54-0x08809804 | 3 | 3 | grade e linhas das cartas, linhas pequenas |
| [`ds_input.c`](../src/libdeck_swap/ds_input.c) | 0x08809804-0x0880AB28 | 4 | 8 | despacho da entrada, estados de ajuda, cursores, páginas, grupos, rolagem, foco |
| [`ds_sortmenu.c`](../src/libdeck_swap/ds_sortmenu.c) | 0x0880AB28-0x0880B750 | 8 | 1 | menu de ordenação |
| [`ds_sort.c`](../src/libdeck_swap/ds_sort.c) | 0x0880B750-0x0880BE1C | 4 | 1 | ordenação estável, valor da chave |
| [`ds_move.c`](../src/libdeck_swap/ds_move.c) | 0x0880BE1C-0x0880D23C | 3 | 8 | mover carta entre principal e extra (voo em curva, desfazer), posição das vagas |
| [`ds_filter.c`](../src/libdeck_swap/ds_filter.c) | 0x0880D23C-0x0880DFFC | 9 | 5 | busca incremental, filtros, busca Horspool |
| [`ds_word.c`](../src/libdeck_swap/ds_word.c) | 0x0880DFFC-0x0880F3E0 | 9 | 2 | janela da busca por palavra |
| [`ds_fmenu.c`](../src/libdeck_swap/ds_fmenu.c) | 0x0880F3E0-0x08810D64 | 11 | 3 | menu de filtro e valores |
| [`ds_action.c`](../src/libdeck_swap/ds_action.c) | 0x08810D64-0x08812510 | 20 | 0 | menu de ações e submenus |
| [`ds_anim.c`](../src/libdeck_swap/ds_anim.c) | 0x08812510-0x08812880 | 2 | 0 | limite de tempo (popup de tempo esgotado), temporizador do cursor, tela de informação |
| [`ds_parts.c`](../src/libdeck_swap/ds_parts.c) | 0x08812880-0x08815318 | 25 | 3 | peças de desenho |
| [`ds_osk.c`](../src/libdeck_swap/ds_osk.c) | 0x08815318-0x088157E0 | 2 | 0 | teclado do sistema |
| [`ds_scene.c`](../src/libdeck_swap/ds_scene.c) | 0x088157E0-0x08815ED4 | 7 | 0 | exports (init/update) e popups |

## libtutorial (9 arquivos, 196 funções batendo, 11 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`tut_main.c`](../src/libtutorial/tut_main.c) | 0x08804000-0x088046B4 | 15 | 0 | exports, máquina de estados por quadro, trava |
| [`tut_cmd.c`](../src/libtutorial/tut_cmd.c) | 0x088046B4-0x08805034 | 30 | 0 | comandos vindos da libduel_mgr |
| [`tut_res.c`](../src/libtutorial/tut_res.c) | 0x08805034-0x088056E0 | 24 | 0 | pacote e recursos, ajudantes de sprintf |
| [`tut_window.c`](../src/libtutorial/tut_window.c) | 0x088056E0-0x08806BC0 | 25 | 0 | janelas de mensagem |
| [`tut_sprite.c`](../src/libtutorial/tut_sprite.c) | 0x08806BC0-0x08807A10 | 26 | 0 | sprites, destaques, pausa |
| [`tut_arrow.c`](../src/libtutorial/tut_arrow.c) | 0x08807A10-0x08807E24 | 8 | 0 | setas |
| [`tut_script.c`](../src/libtutorial/tut_script.c) | 0x08807E24-0x08809A5C | 48 | 2 | script da lição, montagem do duelo (%02d_field_set.txt), filtro de ações, gancho de eventos, sim/não |
| [`tut_text.c`](../src/libtutorial/tut_text.c) | 0x08809A5C-0x0880AC6C | 14 | 9 | caixa de fala (texto digitado, páginas, ícone piscando) |
| [`tut_face.c`](../src/libtutorial/tut_face.c) | 0x0880AC6C-0x0880B284 | 6 | 0 | animação dos rostos (bust-up) |

## libdeck_tutorial (17 arquivos, 224 funções batendo, 69 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`dt_main.c`](../src/libdeck_tutorial/dt_main.c) | 0x08804000-0x088058E0 | 13 | 1 | init/carga/liberação, quadro, formas de destaque, dicas de botão, quadro de avisos |
| [`dt_res.c`](../src/libdeck_tutorial/dt_res.c) | 0x088058E0-0x08805BE4 | 7 | 0 | pacote do tutorial |
| [`dt_edraw.c`](../src/libdeck_tutorial/dt_edraw.c) | 0x08805BE4-0x08808C6C | 15 | 14 | desenho do editor (linhas, grade, contadores, informação) |
| [`dt_parts.c`](../src/libdeck_tutorial/dt_parts.c) | 0x08808C6C-0x0880B704 | 25 | 3 | cópia de sc_parts.c (ícones, dígitos, figura da carta) |
| [`dt_lists.c`](../src/libdeck_tutorial/dt_lists.c) | 0x0880B704-0x0880CDC8 | 18 | 1 | listas de cartas (cópia de dk_lists.c) |
| [`dt_move.c`](../src/libdeck_tutorial/dt_move.c) | 0x0880CDC8-0x088100AC | 10 | 7 | mover/desfazer carta (dk_move.c) mais o contador de metas da lição |
| [`dt_input.c`](../src/libdeck_tutorial/dt_input.c) | 0x088100AC-0x08811BB8 | 8 | 8 | entrada do editor, cursores, verificação do deck, popup |
| [`dt_fmenu.c`](../src/libdeck_tutorial/dt_fmenu.c) | 0x08811BB8-0x088135A8 | 11 | 3 | menu de filtro, com entrada da lição e trava de itens |
| [`dt_filter.c`](../src/libdeck_tutorial/dt_filter.c) | 0x088135A8-0x08814460 | 10 | 5 | filtros, busca de texto Horspool |
| [`dt_mode.c`](../src/libdeck_tutorial/dt_mode.c) | 0x08814460-0x08814D44 | 4 | 2 | layouts dos painéis, abas, animação do fundo (sem lista do parceiro) |
| [`dt_action.c`](../src/libdeck_tutorial/dt_action.c) | 0x08814D44-0x08816C34 | 24 | 1 | menu de ações da carta (sem renomear, sem páginas de ajuda) |
| [`dt_pane.c`](../src/libdeck_tutorial/dt_pane.c) | 0x08816C34-0x088175FC | 7 | 4 | painéis, troca de aba, histórico de desfazer |
| [`dt_eres.c`](../src/libdeck_tutorial/dt_eres.c) | 0x088175FC-0x08817B78 | 8 | 0 | atualização do cursor, recursos de deck/all_%c.ehp |
| [`dt_sort.c`](../src/libdeck_tutorial/dt_sort.c) | 0x08817B78-0x08818E94 | 8 | 6 | menu de ordenação e chaves |
| [`dt_word.c`](../src/libdeck_tutorial/dt_word.c) | 0x08818E94-0x0881A20C | 9 | 2 | busca por palavra (a lição define a palavra com dt_word_set) |
| [`dt_lesson.c`](../src/libdeck_tutorial/dt_lesson.c) | 0x0881A20C-0x0881C0E8 | 34 | 3 | cartas da lição, início do editor, interpretador do script (24 opcodes), janelas de mensagem/saída |
| [`dt_text.c`](../src/libdeck_tutorial/dt_text.c) | 0x0881C0E8-0x0881D318 | 13 | 9 | cópia de tut_text.c |

## libfield (44 arquivos, 556 funções batendo, 91 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`f_main.c`](../src/libfield/f_main.c) | 0x08804000-0x088044D8 | 10 | 0 | entrada do módulo, thread da cena 5 (sala escolhida por menu, sequência de comandos de campo), exit, init, sala → bg |
| [`f_bustup.c`](../src/libfield/f_bustup.c) | 0x088044D8-0x08805220 | 23 | 0 | retratos bust-up (até 3, corpo e boca EHF): fades, piscar, fala, comandos load/open/anim/close/unload |
| [`f_chr.c`](../src/libfield/f_chr.c) | 0x08805220-0x08805DF4 | 13 | 3 | personagens SD do mapa: setup por tipo, contadores de animação, registro de desenho, oito direções, colisão (asm) |
| [`f_chrdraw.c`](../src/libfield/f_chrdraw.c) | 0x08805DF4-0x08806FC4 | 3 | 9 | desenho dos personagens SD (modelo, folha de sprites, sprite plano em asm), projeção ortográfica, tipo de desenho por chara |
| [`f_bg.c`](../src/libfield/f_bg.c) | 0x08806FC4-0x08807844 | 12 | 1 | sala: modelos, portas, células marcadas, área do jogador com crossfade, mapa de células |
| [`f_map.c`](../src/libfield/f_map.c) | 0x08807844-0x08808214 | 4 | 4 | mapa de células: posição/célula, células marcadas, portas, hall especial (andável e altura em asm) |
| [`f_bgdraw.c`](../src/libfield/f_bgdraw.c) | 0x08808214-0x08809C40 | 11 | 9 | carga e desenho da sala: modelos por área, sombras por stencil, objetos, máscara de malhas, gancho das portas, som das portas |
| [`f_npc.c`](../src/libfield/f_npc.c) | 0x08809C40-0x0880AC74 | 19 | 4 | NPCs (32 com caminho) e figurantes (80): reset, desenho, próximos, NPC à frente, posição, virar para o jogador |
| [`f_player.c`](../src/libfield/f_player.c) | 0x0880AC74-0x0880BB98 | 15 | 4 | jogador: setup, desenho, trava de entrada, efeito, marcação das células (update/entrada/movimento em asm) |
| [`f_res.c`](../src/libfield/f_res.c) | 0x0880BB98-0x0880C1C4 | 12 | 1 | carregador: heap de 7,5 MB, leitor de cartas, cache dos GIM compartilhados |
| [`f_pool.c`](../src/libfield/f_pool.c) | 0x0880C1C4-0x0880CA9C | 15 | 1 | pools de arquivos (SD, sala, sprites, vozes): descarga, tamanho, pedidos tipados, cancelamento, buscas |
| [`f_req.c`](../src/libfield/f_req.c) | 0x0880CA9C-0x0880D4F4 | 18 | 2 | fila de pedidos e de callbacks, leituras assíncronas, contexto, hora do dia, comprimento, lerp de cor, sprites |
| [`f_tab.c`](../src/libfield/f_tab.c) | 0x0880D4F4-0x0880F228 | 4 | 1 | tabelas de índice (bust-up, sala, grupo, voz) geradas das jump tables; chara → modelo em asm |
| [`f_item.c`](../src/libfield/f_item.c) | 0x0880F228-0x0880FBB4 | 21 | 0 | efeitos sonoros do campo e objetos nas células (colocar, sombra, pegar, desenho) |
| [`f_sys.c`](../src/libfield/f_sys.c) | 0x0880FBB4-0x08810814 | 25 | 2 | sistema do campo (update, desenho, init/free/term dos subsistemas) e comandos de script pela tabela de comandos |
| [`f_scene.c`](../src/libfield/f_scene.c) | 0x08810814-0x08811BE0 | 20 | 4 | comandos de cena: história, posição do jogador, alvo da câmera, salvar posições, portas, esperas, GIM da cena |
| [`f_hud.c`](../src/libfield/f_hud.c) | 0x08811BE0-0x08812FBC | 18 | 2 | HUD: janela do nome da sala, ícone do evento da célula, piscar, deslizar, textos dos menus |
| [`f_hud2.c`](../src/libfield/f_hud2.c) | 0x08812FBC-0x08813308 | 6 | 0 | nome da sala e relógio deslizando, despacho do desenho do relógio |
| [`f_hud3.c`](../src/libfield/f_hud3.c) | 0x08813308-0x08814D94 | 13 | 8 | partes do HUD deslizando (dia, data, ícone que voa, painel, caixa de mensagem); desenhos em asm |
| [`f_digits.c`](../src/libfield/f_digits.c) | 0x08814D94-0x08815FD8 | 4 | 3 | dígitos em sprite, ícones das partes do dia, ponteiros do relógio |
| [`f_msg.c`](../src/libfield/f_msg.c) | 0x08815FD8-0x08817218 | 25 | 2 | janela de mensagens (field_msg; a tmsg da libtitle é uma cópia reduzida): moldura que entra e sai, até três linhas na velocidade das opções, páginas, cursor de "próximo", avanço e pulo pelo jogador |
| [`f_talk.c`](../src/libfield/f_talk.c) | 0x08817218-0x08817AA4 | 13 | 1 | comandos de fala e escolha (field_talk): fala de um slot na janela de mensagens (fechando no fim), lista numerada de escolhas centrada, R esconde a janela |
| [`f_money.c`](../src/libfield/f_money.c) | 0x08817AA4-0x08818724 | 17 | 1 | contador de dinheiro do HUD (field_money): ícone e valor entrando, quantia ganha ou gasta que aparece e voa até o contador, rolagem do valor com dígitos baixos aleatórios, sprites dos dígitos |
| [`f_cam.c`](../src/libfield/f_cam.c) | 0x08818724-0x08818FFC | 18 | 0 | câmera, luz e estado do GE do campo (field_cam): câmera seguindo o jogador, luz da hora do dia, comando de tremor, quad de tela cheia |
| [`f_bmap.c`](../src/libfield/f_bmap.c) | 0x08818FFC-0x0881BA90 | 25 | 6 | mapa do campus (field_bm): ilha com 14 lugares, cursor pelo d-pad ou pelo analógico (lugar mais próximo num cone de 45/60/75 graus), marcas das pessoas a encontrar, lista das salas da academia, salvar no mapa, fades; setup, abertura do comando, entrada, marcas, ícone e cursor em asm |
| [`f_partner.c`](../src/libfield/f_partner.c) | 0x0881BA90-0x0881C240 | 9 | 2 | parceiro que anda atrás do jogador (field_partner): segue a posição do quadro anterior (velocidade dobrada correndo), desvia dos NPCs próximos, marca as células, começa na primeira célula andável em volta do jogador; update e posicionamento em asm |
| [`f_shadow.c`](../src/libfield/f_shadow.c) | 0x0881C240-0x0881CE68 | 14 | 1 | lista de desenho do campo (field_draw) ordenada por profundidade, sombras em stencil (cilindro de 12 lados iluminado por field_shadow) e atributos das células da sala (fld_map_attr/fld_cell_mark) |
| [`f_bag.c`](../src/libfield/f_bag.c) | 0x0881CE68-0x0881EDC8 | 26 | 2 | janela da bolsa (field_bag): abas de itens e cartas trocadas com L/R, ajuda da entrada sob o cursor na janela de mensagem, menu de escolher/descartar carta |
| [`f_cal.c`](../src/libfield/f_cal.c) | 0x0881EDC8-0x0881FFEC | 10 | 1 | PDA (field_pda, no heap): abertura, deslize da página, estados, moldura, linhas, título, caixa de nota e abas |
| [`f_pdapage.c`](../src/libfield/f_pdapage.c) | 0x0881FFEC-0x08822898 | 8 | 5 | páginas do PDA: status do jogador (dias, duelos V/D/E, aproveitamento, cartas), opções, títulos das páginas, calendário do mês, lista de e-mails e e-mail aberto; moldura 9-slice, cursor, medidor e número alinhado à direita |
| [`f_pdaparts.c`](../src/libfield/f_pdaparts.c) | 0x08822898-0x08824028 | 8 | 2 | peças das páginas do PDA: célula e marca do calendário, texto da data (ja/en), seta e barra de rolagem, item e cabeçalho de e-mail, barra de progresso, máscara do texto selecionado, entrada da barra de páginas |
| [`f_pdamenu.c`](../src/libfield/f_pdamenu.c) | 0x08824028-0x08825F7C | 19 | 3 | lógica das páginas do PDA: opções (cfg/volumes), escolha de duas opções, registro de eventos e leitor com rolagem e voz, calendário, arquivos do pdapack (capa, event_name, option_msg_%c) e capa 3D |
| [`f_get.c`](../src/libfield/f_get.c) | 0x08825F7C-0x08826BB0 | 15 | 0 | mensagem 'você obteve': item com ícone, ou carta girando em 3D (face renderizada na VRAM por hook do modelo) que abre o visualizador de cartas |
| [`f_fx.c`](../src/libfield/f_fx.c) | 0x08826BB0-0x088274B4 | 10 | 0 | balões de emoção sobre os personagens (pool de 32), ícone animado ou sprite do chamador, de frente para a câmera |
| [`f_date.c`](../src/libfield/f_date.c) | 0x088274B4-0x08827C44 | 9 | 1 | faixa da data do novo dia (JP: mês/dia (sem); US: 3rd mês (sem)) com contorno, segunda linha opcional e fade |
| [`f_clock.c`](../src/libfield/f_clock.c) | 0x08827C44-0x08827FE4 | 3 | 1 | início do menu da hora (field_hour_menu): abertura, update, desenho, resultado |
| [`f_hour.c`](../src/libfield/f_hour.c) | 0x08827FE4-0x088290BC | 14 | 3 | menu da hora até a qual esperar: janela de título, lista com texto de ajuda e seletor de hora em quartos de hora até 23:45 |
| [`f_egg.c`](../src/libfield/f_egg.c) | 0x088290BC-0x08829F5C | 14 | 0 | sorteio do ovo dourado: itens girando sobre o jogador até apertar um botão; o ovo dourado mostra o modelo goldegg_pan e o efeito Effect_04 |
| [`f_pict.c`](../src/libfield/f_pict.c) | 0x08829F5C-0x0882A3A8 | 9 | 0 | imagem em tela cheia dos comandos de cena, com fade de 15 frames; o id -2 faz fade de tela preta |
| [`f_choice.c`](../src/libfield/f_choice.c) | 0x0882A3A8-0x0882A918 | 7 | 1 | menu de cinco escolhas (textos 0x2B..0x2F); devolve 1..5, ou 0 se cancelado |
| [`f_amb.c`](../src/libfield/f_amb.c) | 0x0882A918-0x0882ABD4 | 3 | 0 | som ambiente por sala, mais o volume da fonte da sala 0x1D pela distância do jogador |
| [`f_face.c`](../src/libfield/f_face.c) | 0x0882ABD4-0x0882B238 | 6 | 1 | rostos EHF animados do tutorial (body/mouth de tutorial/face/bu_xxx.ehp) com piscar e boca falando |
| [`f_mark.c`](../src/libfield/f_mark.c) | 0x0882B238-0x0882B7C8 | 3 | 0 | marcador no stencil: prisma sobre o ponto alvo e cor pulsante na tela onde o stencil está marcado |
| [`f_dlg.c`](../src/libfield/f_dlg.c) | 0x0882B7C8-0x0882BE80 | 5 | 0 | diálogo sim/não (fld_dlg_*), escolha do botão de face e desenho do painel de botões |

