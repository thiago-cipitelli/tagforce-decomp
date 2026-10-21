# Progresso da decompilação

Gerado a partir dos cabeçalhos `@range` de `src/` e das marcas `matched`/`asm` em `symbols/`;
atualizar ao fechar um arquivo. Números do relatório ByteMatch de 2026-10-20 (`python3 tools/bytematch.py`).

| Módulo | Código batendo | Fuzzy | Funções batendo |
|---|---|---|---|
| total | 14,2% | 20,4% | 2744 / 11039 |
| BOOT | 44,0% | 64,3% | 1253 / 2208 |
| libduel_eng | 22,4% | 31,7% | 1489 / 3429 |
| libhtml_view | 9,8% | 9,8% | 2 / 16 |
| demais 19 gmodules | 0% | 0% | 0 / 5386 |

Funções "em asm" ficam no `.c` como `// @asm A-B` com o C equivalente sob `#ifdef NON_MATCHING`
(rascunho funcional, ainda não idêntico byte a byte).

## Maiores trechos ainda sem C

Saída de `MODULE=<m> python3 tools/list_funcs.py --gaps` (início-fim, funções, bytes). Bom lugar para começar:

**libduel_eng**

```
0x08894158-0x088C2F80   644 functions   192040 bytes
0x08832B54-0x0884ACD0   254 functions    98684 bytes
0x0884ED08-0x08864DF0   257 functions    90344 bytes
0x08820618-0x088324F0   123 functions    73432 bytes
0x08865F2C-0x0886CB44    68 functions    27672 bytes
0x08804000-0x08809B50    43 functions    23376 bytes
0x088D1DF4-0x088D6DE8    43 functions    20468 bytes
0x088CC8EC-0x088D13CC     7 functions    19168 bytes
```

**BOOT**

```
0x08804000-0x0881FFB0   700 functions   114608 bytes
0x08862D28-0x08865260    55 functions     9528 bytes
0x08820ED4-0x08822F84     1 functions     8368 bytes
```

Conhecido no BOOT: `0x08804000`-`0x0881FFB0` é libgu, MSL, libgcc e sceDeflt compilados com GCC (não casam com mwcc);
`0x08862D28`-`0x08865260` é libsas/libwave do SDK. Ver `docs/build.md`, "Pendências".

## BOOT (71 arquivos, 1310 funções batendo, 184 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`postfx.c`](../src/BOOT/postfx.c) | 0x0881FFB0-0x08820ED4 | 8 | 3 | efeitos de tela cheia por texturas de trabalho na VRAM: redução, cópia escurecida, blends aditivos de cópias deslocadas e limiar por CLUT |
| [`ehf_cmd.c`](../src/BOOT/ehf_cmd.c) | 0x08822F84-0x08823BD0 | 17 | 2 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd2.c`](../src/BOOT/ehf_cmd2.c) | 0x08823BD0-0x08824040 | 9 | 1 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd3.c`](../src/BOOT/ehf_cmd3.c) | 0x08824040-0x088243E4 | 3 | 0 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_cmd4.c`](../src/BOOT/ehf_cmd4.c) | 0x088243E4-0x088253FC | 18 | 4 | handlers dos comandos EHF: janelas e laços de quadros, palavras GE cruas, transformações, matrizes por mensagem, esferas de visibilidade, billboards, matriz de osso, escala, tremor aleatório, rastros em anel de uma ou duas trilhas suavizados por spline cíclica, interpolação de chaves, callbacks |
| [`ehf_prim.c`](../src/BOOT/ehf_prim.c) | 0x088253FC-0x0882644C | 15 | 1 | gizmo de eixos e primitivas cujos vértices vêm de comandos seguintes: início com espaço na lista ou vértices externos, coordenadas de textura, cores, posições, fita voltada para a câmera |
| [`particle.c`](../src/BOOT/particle.c) | 0x0882644C-0x08827030 | 7 | 2 | partículas EHF: pool com envelhecimento, emissor periódico com callbacks de posição/velocidade, geradores em caixa, caixa móvel e disco com o gerador rand3 |
| [`particle2.c`](../src/BOOT/particle2.c) | 0x08827030-0x08827B8C | 6 | 0 | geradores em anel, tetraédrico, casca esférica e cone, posições levadas ao espaço do modelo, velocidades constante e radial, movimento, aceleração e arrasto |
| [`particle3.c`](../src/BOOT/particle3.c) | 0x08827B8C-0x08828590 | 10 | 2 | geradores em anel, tetraédrico, casca esférica e cone, posições levadas ao espaço do modelo, velocidades constante e radial, movimento, aceleração e arrasto |
| [`ptcl_draw.c`](../src/BOOT/ptcl_draw.c) | 0x08828590-0x0882923C | 8 | 1 | partículas desenhadas como quads de faixa de triângulos no espaço da câmera, cor fixa ou por chaves de idade, textura, tamanho e uv do sprite |
| [`efs.c`](../src/BOOT/efs.c) | 0x0882923C-0x0882AD90 | 18 | 8 | efeitos .efs: carga com relocação por bytecode, raízes nomeadas com busca binária, 0x300 instâncias em lista circular no heap de efeitos, interpretador de árvores de comandos por fase, argumentos por formato, comandos de instância aninhada |
| [`camera.c`](../src/BOOT/camera.c) | 0x0882AD90-0x0882B65C | 20 | 1 | câmera com look-at e as quatro luzes do GE com bits de ativação reemitidos na lista |
| [`tms.c`](../src/BOOT/tms.c) | 0x0882B65C-0x0882C428 | 10 | 3 | geometria TGMS: relocação, materiais com quatro camadas de textura, hooks de textura e desenho, callbacks de mapa de ambiente, prims com cache de oito matrizes de osso, culling pela caixa |
| [`model.c`](../src/BOOT/model.c) | 0x0882C428-0x0882D208 | 21 | 1 | modelos: geometria TGMS com matrizes de osso e de skin, animações TGMT com cross-fade, buffers de animação descomprimida, animação de material TGMA, cópias de materiais e texturas, duas passadas de desenho com stencil e contorno |
| [`toon.c`](../src/BOOT/toon.c) | 0x0882D208-0x0882D9F0 | 5 | 1 | rampas de toon shading desenhadas na VRAM a partir das tabelas do contexto de desenho, hook de textura das camadas iluminadas pela luz 0, relocação TGMT |
| [`anim.c`](../src/BOOT/anim.c) | 0x0882D9F0-0x0882ECAC | 6 | 3 | animação esquelética TGMT: percurso em profundidade da hierarquia, translação/rotação/escala por chaves Bezier e quatérnios com slerp, cross-fade entre duas animações, quatérnios compactados em 16 bits |
| [`ehfile.c`](../src/BOOT/ehfile.c) | 0x0882ECAC-0x0882FC3C | 8 | 0 | VFS, 29 funções |
| [`ehf.c`](../src/BOOT/ehf.c) | 0x0882FC3C-0x08833420 | 35 | 9 | player de animações EHF, formato tipo Flash em twips: acessores de arquivo/instância, timeline com comandos de colocar/remover, pool de clips em listas circulares por profundidade, avanço por tick, desenho com máscara por stencil, transformação de cor e matrizes 2D |
| [`ehp.c`](../src/BOOT/ehp.c) | 0x08833420-0x0883451C | 25 | 1 | pacotes EHP: busca binária por nome ou ponteiro, relocação por extensão com ptrs.txt, TGMA/TGMT, TMS, EHF e GIM, handlers registrados, carga síncrona e assíncrona, tabela de pacotes montados, projeção 2D |
| [`gfx.c`](../src/BOOT/gfx.c) | 0x0883451C-0x08835650 | 22 | 2 | inicialização do GE/libgu, fim de quadro com limite de 30 fps e callback de flip, alvos de desenho, capturas BMP/TGA para host0:, rastreador do estado do GE com a lista de valores padrão |
| [`gim.c`](../src/BOOT/gim.c) | 0x08835650-0x08835F48 | 6 | 3 | texturas GIM: busca de blocos na árvore, modo e imagem de um quadro ou célula, CLUT da paleta, imagem residente na VRAM no lugar da assinatura PSP, troca de textura fora da lista de desenho |
| [`vram.c`](../src/BOOT/vram.c) | 0x08835F48-0x088362E8 | 8 | 1 | área de texturas na VRAM depois dos framebuffers: alocador linear, envio de um quadro de GIM para a VRAM, percurso dos blocos do GIM |
| [`mtx.c`](../src/BOOT/mtx.c) | 0x088362E8-0x088370A4 | 31 | 0 | pilhas de matrizes por tipo do GE com a matriz corrente em M300 da VFPU: init, emissão/flush das sujas para a lista, salvar/restaurar estado, identidade, rotações, translação, escala, perspectiva, ortho, look-at, push/pop; blocos `asm` VFPU no C com equivalente em C para o port |
| [`draw.c`](../src/BOOT/draw.c) | 0x088370A4-0x08837B28 | 10 | 1 | primitivas dentro da lista ou imediatas fechando e reabrindo a sublista, teste de esfera contra o frustum, flags de matriz suja |
| [`heap.c`](../src/BOOT/heap.c) | 0x08837B28-0x088390C4 | 6 | 6 | heap HOGE: tabela ordenada de fronteiras com bit de livre, guardada num bloco do próprio heap e dobrada quando enche, alocação best fit com divisão pelo topo, liberação com fusão, compactação |
| [`transform.c`](../src/BOOT/transform.c) | 0x088390C4-0x08839A8C | 6 | 2 | splines cúbicas pelas chaves: termos de segunda derivada por sistema tridiagonal, aberto ou em anel, e avaliação num índice fracionário |
| [`pad.c`](../src/BOOT/pad.c) | 0x08839A8C-0x08839F50 | 6 | 2 | controle: amostra por quadro com reaproveitamento, máscaras de borda e auto-repeat por botão, analógico dobrado em direcional com repetição própria, tempos de repetição pela taxa de quadros, ângulo do analógico ou do direcional |
| [`dl.c`](../src/BOOT/dl.c) | 0x08839F50-0x0883CEB0 | 72 | 7 | listas de comandos do GE: heap em duas metades com tabela de 16 camadas encadeadas por JUMP, troca de camada, e uma função por estado ou primitiva do GE montando as palavras no stack e empurrando com dl_push: enable/disable, matrizes, luzes, material, textura, CLUT, névoa, scissor, testes, blend, dither, sprite, clear, cópia de imagem; SysConf com os campos reais |
| [`sys.c`](../src/BOOT/sys.c) | 0x0883CEB0-0x0883D708 | 17 | 0 | System glue: display defaults, the "ehscript" scene thread (one gmodule at a time, cooperative with BOOT's main loop), init order and the per-scene state stack |
| [`ehf_cam.c`](../src/BOOT/ehf_cam.c) | 0x0883D708-0x0883DD58 | 5 | 0 | comandos EHF de câmera perspectiva/orto/look-at e animação de material TGMA |
| [`main.c`](../src/BOOT/main.c) | 0x0883DD58-0x0883E670 | 27 | 0 | BOOT entry after the C runtime: init order, the frame loop, soft reset, the loading overlay and small accessors of the game globals |
| [`save.c`](../src/BOOT/save.c) | 0x0883E670-0x0883F340 | 20 | 0 | Save state ("sysstate"): new game defaults, checksum, player name, day counter, money and the duel record counters |
| [`inv.c`](../src/BOOT/inv.c) | 0x0883F340-0x08840478 | 45 | 0 | Card inventory records (one 4-byte bitfield per card index), the current deck, tag partner slots and the player name inside the save state |
| [`cfg.c`](../src/BOOT/cfg.c) | 0x08840478-0x08840DC4 | 31 | 0 | Save-state flag records, options bitfields and the session game state (player slots filled from recipes / saved decks) |
| [`deck.c`](../src/BOOT/deck.c) | 0x08840DC4-0x08841BD0 | 34 | 0 | Stage progress, per-character deck slots, deck/recipe comparison, request flags, region and language, and the language-dependent name/message packs |
| [`wstr.c`](../src/BOOT/wstr.c) | 0x08841BD0-0x088420B4 | 13 | 0 | charmap, wcs* |
| [`wfmt.c`](../src/BOOT/wfmt.c) | 0x088420B4-0x088433F0 | 12 | 1 | formatação UCS-2 |
| [`menu.c`](../src/BOOT/menu.c) | 0x088433F0-0x08843D78 | 10 | 2 | registro de itens de janela |
| [`menu_draw.c`](../src/BOOT/menu_draw.c) | 0x08843D78-0x08845AC0 | 7 | 4 | moldura 9-slice |
| [`font.c`](../src/BOOT/font.c) | 0x08845AC0-0x08847008 | 12 | 7 | cache de glifos sceFont |
| [`font_text.c`](../src/BOOT/font_text.c) | 0x08847008-0x08848C68 | 0 | 0 | layout de texto UCS-2 com tags $R/$C/$f/$F, só C sob `NON_MATCHING` |
| [`font_flags.c`](../src/BOOT/font_flags.c) | 0x08848C68-0x0884900C | 19 | 0 | Font layer state accessors: layout flags, extra line spacing, and the display list used while a menu item's text is being built |
| [`font_tags.c`](../src/BOOT/font_tags.c) | 0x0884900C-0x0884A9B4 | 7 | 22 | Text tags and line emission: $R ruby (furigana over the base text), $C color, $f/$F small capitals, word-break measurement, and font_layout_end which turns FontCtx.cmds into glyph quads |
| [`menu_widget.c`](../src/BOOT/menu_widget.c) | 0x0884A9B4-0x0884C8D0 | 9 | 8 | janela com lista rolável e botões |
| [`card_db.c`](../src/BOOT/card_db.c) | 0x0884C8D0-0x0884EA38 | 61 | 2 | receitas RCP*.ydc, cardinfo_<lang>.ehp, acessores de carta, 64 + 2 callbacks de cena |
| [`card_groups.c`](../src/BOOT/card_groups.c) | 0x0884EA38-0x08850A70 | 41 | 1 | listas de ids hard-coded como `switch`, classes especiais e ids alternativos |
| [`pmf_umd.c`](../src/BOOT/pmf_umd.c) | 0x08850A70-0x0885133C | 22 | 0 | thread do ring buffer sceMpeg, wrappers DvdUmdRetry* da Sony, carga dos módulos de codec |
| [`movie.c`](../src/BOOT/movie.c) | 0x0885133C-0x08853A2C | 62 | 8 | player PSMF da Sony: movie_run, threads display/read/sound, filas de pts e sincronia A/V |
| [`cip.c`](../src/BOOT/cip.c) | 0x08853A2C-0x08854920 | 23 | 3 | pilha de cenas scene_push/pop_to/reset_to + leitor CIP/CPM das imagens de carta com anel de slots e prefetch |
| [`cardlist.c`](../src/BOOT/cardlist.c) | 0x08854920-0x08859044 | 50 | 8 | desenho de carta e painel: imagem com cross-fade, moldura cil_0000, estrelas, atributo, ATK/DEF, caixa de descrição com barra de rolagem, coluna de comparação do editor de deck, tabelas de vértices de 16 bits |
| [`modmgr.c`](../src/BOOT/modmgr.c) | 0x08859044-0x08859824 | 19 | 0 | Frame wait, a debug text menu, synchronous file/module loads and the module manager |
| [`fade.c`](../src/BOOT/fade.c) | 0x08859824-0x08859DF0 | 9 | 0 | fades de tela linear/quadrático/wipe |
| [`cardinfo.c`](../src/BOOT/cardinfo.c) | 0x08859DF0-0x0885C688 | 40 | 5 | visualizador de detalhes da carta: estado em globais 0x08BEA44C, abas descrição/gêneros/listas, scroll, dicas de botão |
| [`snd_mix.c`](../src/BOOT/snd_mix.c) | 0x0885C688-0x0885D948 | 7 | 3 | mixer: faders por quadro, parâmetros e key on/fim das vozes SAS, buffer de eco de 0x3000 quadros, ordenação dos streams, nível de bloco, decodificador Atrac que enche o anel PCM de cada stream e o mistura no bloco de saída |
| [`snd_out.c`](../src/BOOT/snd_out.c) | 0x0885D948-0x0885DE90 | 1 | 1 | thread de som que mistura vozes SAS e streams em dois buffers e entrega ao canal de áudio; 1 + efeito global em asm |
| [`snd_bank.c`](../src/BOOT/snd_bank.c) | 0x0885DE90-0x0885E634 | 29 | 4 | cabeçalho SHDS de psp_snddat.bin, grupos, cache de handles, listas intrusivas |
| [`snd_engine.c`](../src/BOOT/snd_engine.c) | 0x0885E634-0x0885F05C | 4 | 1 | inicialização: estado, listas, callbacks ou buffer de grupos, semáforos, SAS, pool de objetos e threads; início da thread, abertura direta do banco, passo de pausa/retomada com fader, atualização por quadro de todos os sons ativos |
| [`snd_alloc.c`](../src/BOOT/snd_alloc.c) | 0x0885F05C-0x0885FAA8 | 2 | 3 | histórico de variantes sorteadas, escolha do slot de objeto de um som novo, início das vozes SAS com pitch aleatório e envelope ou do stream Atrac; tabelas de pitch em C, as 3 funções em asm |
| [`snd_play.c`](../src/BOOT/snd_play.c) | 0x0885FAA8-0x08860DA4 | 16 | 2 | reset/anexação de objetos, início de som com variantes aleatórias e sequenciais, slots de BGM com suspensão, voz, parada com fade e callback, ducking, parada por grupo ou classe |
| [`snd_core.c`](../src/BOOT/snd_core.c) | 0x08860DA4-0x088615D0 | 15 | 4 | máscara de vozes SAS, volumes por canal, medidor de nível para lip sync, reverb |
| [`snd_obj.c`](../src/BOOT/snd_obj.c) | 0x088615D0-0x08862C5C | 25 | 3 | handles do pool de objetos de som, busca por id, ordem de início, slots do som atual, carga de grupos pelos callbacks ou VPL, curva de distância e modelo 3D com pan e Doppler, faders, volumes SAS por objeto |
| [`snd_rand.c`](../src/BOOT/snd_rand.c) | 0x08862C5C-0x08862D28 | 3 | 0 | gerador aleatório da biblioteca de som |
| [`sndapi.c`](../src/BOOT/sndapi.c) | 0x08865260-0x08866068 | 57 | 8 | API de som exportada aos gmodules: BGM/SE, callbacks de leitura/heap da biblioteca de som, tabelas de voz por personagem, filas de voz por jogador, movie_play, caminhos de save-data |
| [`savedata.c`](../src/BOOT/savedata.c) | 0x08866068-0x088668C0 | 9 | 1 | bloco de parâmetros do sceUtilitySavedata, textos SFO, ícones de msd.ehp, chave, inícios por modo |
| [`msd.c`](../src/BOOT/msd.c) | 0x088668C0-0x08868DC8 | 10 | 0 | máquinas de estado de save-data por operação: novo jogo, verificação, autoload, carregar, salvar, apagar; carga de msd.ehp e despacho por quadro |
| [`net_main.c`](../src/BOOT/net_main.c) | 0x08868DC8-0x0886A6EC | 44 | 5 | entrada de estados, listas do lobby, driver por quadro com watchdogs de pad/UMD/link, exports para os gmodules, diálogos netconf/mensagem, carga dos módulos de rede |
| [`net_match.c`](../src/BOOT/net_match.c) | 0x0886A6EC-0x0886B654 | 15 | 2 | sceNetAdhocMatching host/convidado, callback que enfileira eventos num anel de pacotes Mbx, tabela de pares |
| [`net_ctl.c`](../src/BOOT/net_ctl.c) | 0x0886B654-0x0886C904 | 22 | 1 | sceNetAdhocctl, máquina de estados net_update, handshake PTP com a lista de MACs, 22 + 1 em asm; `include/net.h` com NetSession/NetPeer |
| [`msgbox.c`](../src/BOOT/msgbox.c) | 0x0886C904-0x0886CF50 | 5 | 3 | caixa de mensagem do jogo, simples ou com botões OK/SIM/NÃO via menu widget |
| [`extdata.c`](../src/BOOT/extdata.c) | 0x0886CF50-0x0886E0A4 | 9 | 2 | arquivos YGTF .YGC/.YGR/.YGL da pasta do save: listagem, leitura, verificação e desembaralhamento |
| [`netsync.c`](../src/BOOT/netsync.c) | 0x0886E0A4-0x0886ED2C | 26 | 3 | troca de registros de 0x100 bytes por jogador via PDP ad hoc, fila de pacotes, transferência em blocos de 0x80, lado do jogador local |

## libduel_eng (91 arquivos, 1546 funções batendo, 309 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`ai_util.c`](../src/libduel_eng/ai_util.c) | 0x08809B50-0x0880B448 | 21 | 6 | ajudantes da IA: escolher cartas para um diálogo, comparar os monstros mais fortes dos dois jogadores, melhor alvo de ataque, cartas que a IA prefere, contagens por tipo na mão e no cemitério |
| [`ai_cond3.c`](../src/libduel_eng/ai_cond3.c) | 0x0880FDF4-0x08810370 | 6 | 5 | mais condições da IA por carta: ATK do mais forte contra os LP do oponente, fase e turno, monstros por posição |
| [`ai_cond4.c`](../src/libduel_eng/ai_cond4.c) | 0x08810370-0x08811BDC | 34 | 5 | condições da IA: LP contra o ATK mais forte, monstros e cartas especiais no campo, cartas da corrente, mão e cemitério, fase e turno |
| [`ai_cond5.c`](../src/libduel_eng/ai_cond5.c) | 0x088137BC-0x08814EC8 | 25 | 7 | condições da IA contra a carta sendo ativada: alvos no campo, tipo e dono da carta, monstros e cartas especiais, LP |
| [`ai_cond6.c`](../src/libduel_eng/ai_cond6.c) | 0x08814EC8-0x08815E90 | 8 | 4 | escolha da carta a ativar pelas tabelas de cartas e condições da IA, zonas embaralhadas por máscara com a de maior ATK/DEF, carta da mão por pontuação |
| [`ai_cond2.c`](../src/libduel_eng/ai_cond2.c) | 0x08819F94-0x0881C118 | 56 | 7 | condições da IA: alvos do descritor, cartas com o mesmo nome, contadores por zona, bits de posição dos monstros, PV contra o dano previsto, magia de campo, monstros da mão invocáveis por tributo |
| [`ai_cond1.c`](../src/libduel_eng/ai_cond1.c) | 0x0881C118-0x0881D0D8 | 9 | 9 | condições da IA: número de monstros, alvos com ATK alto, ATK/DEF do monstro escolhido contra o maior do oponente, tabela de cartas que a IA invoca do cemitério |
| [`ai_cond0.c`](../src/libduel_eng/ai_cond0.c) | 0x0881D0D8-0x0881E0A0 | 22 | 3 | mais condições da IA por carta: mão e cemitério sem cartas proibidas, PV, nível e ATK do monstro escolhido |
| [`ai_cond.c`](../src/libduel_eng/ai_cond.c) | 0x0881E0A0-0x08820618 | 74 | 4 | condições que o jogador do computador verifica antes de ativar cada carta: PV do oponente, contagens no campo, na mão e no cemitério, alvos aceitos pelo descritor |
| [`duel_event.c`](../src/libduel_eng/duel_event.c) | 0x088324F0-0x08832B54 | 32 | 0 | eventos do duelo para a apresentação: um registro pendente de jogador, tipo e duas meias-palavras ou uma lista curta, enviado ao outro console como mensagem 0x1E; 32 wrappers por tipo |
| [`duel_dialog.c`](../src/libduel_eng/duel_dialog.c) | 0x0884ACD0-0x0884B094 | 18 | 1 | pedidos de diálogo ao host, duel_host_call 0x31 por subcomando: mensagens, pergunta com cartas ou texto, sim/não, só para o jogador local |
| [`duel_pick.c`](../src/libduel_eng/duel_pick.c) | 0x0884C718-0x0884D050 | 21 | 1 | lista de cartas que um diálogo oferece, `DuelPick` em 0x0891524C com zona e número de cada carta em bitfield, flags de exibição, gerador aleatório LCG do duelo, tamanho das pilhas, Exodia e cartas de vitória especial |
| [`duel_end.c`](../src/libduel_eng/duel_end.c) | 0x0884D050-0x0884E228 | 14 | 5 | fim do duelo: motivo de vitória de cada jogador em +0x20..+0x2C do `DuelPlayer`, apresentação do fim, reprodução do log de entradas, passos finais por tabela, contadores e flags do registro do duelo em 0x08913C1C, pedido de desistência |
| [`card_desc.c`](../src/libduel_eng/card_desc.c) | 0x0884E228-0x0884ED08 | 24 | 0 | descritores de efeito das cartas: quatro tabelas de 0x18 bytes ordenadas por id conforme o tipo da ação, busca binária `static` cujos chamadores guardam argumentos em t-regs, chamada dos handlers de condição, alvo e resolução, máscara e contagens das zonas aceitas pelo alvo |
| [`effects_q.c`](../src/libduel_eng/effects_q.c) | 0x08864DF0-0x08865F2C | 23 | 6 | callbacks de descritores de efeito em 0x08864DF0: passos com contador, máscaras de zonas, valores da zona, escolhas na mão |
| [`act_cond.c`](../src/libduel_eng/act_cond.c) | 0x0886CB44-0x0886E784 | 44 | 6 | condições de ativação das cartas sobre o `DuelAct`: descartar da mão, zonas livres, fase e turno, tributos, cartas nas pilhas, carta especial como alvo, armadilhas como alvo da corrente, contadores e posição na zona, carta especial e mão do oponente, cartas no deck |
| [`act_cond2.c`](../src/libduel_eng/act_cond2.c) | 0x0886F23C-0x0887DB24 | 368 | 61 | mais condições: cemitério e mão, monstros na mão, alvos de uma corrente, cartas que voltam ao cemitério ou ao deck, monstros virados, tributos, cartas especiais, alvos virados para cima de um grupo, carta especial do oponente, descartes de custo, mão e campo cheios, LP baixo, buscas na mão, nível do monstro especial, cartas do oponente mandadas ao cemitério, zonas de magia livres, contadores exigidos na zona, entrada na fase de batalha, somas de contadores, cinco cartas no cemitério, diferença de LP, cartas exigidas no campo, zona F, alvos no cemitério, magia de campo, diferença de 7000 LP, DEF do monstro especial, alvos de ataque do monstro especial, alvos da corrente na zona, maior ATK na mão, bits de estado da zona, monstros de defesa do tipo 0x12, atributos 3 a 6 no campo, cópias no cemitério, atributos dos monstros oferecíveis, cartas na zona F, atributos da carta que causou a ação, nomes no cemitério e na mão, tipo da carta que causou a ação, cartas nomeadas no cemitério, bits 16 e 17 do jogador, cartas 0x19A3/0x19A4, zona de origem pelo valor da zona, nível das cartas da corrente, diferença de 8000 LP, imunidade da zona a 0x1A77, zonas de magia livres do oponente |
| [`effects_p.c`](../src/libduel_eng/effects_p.c) | 0x0887DB24-0x0888146C | 16 | 13 | Card effect handlers (DuelAct *a): what a's card does when it resolves |
| [`effects_r.c`](../src/libduel_eng/effects_r.c) | 0x0888146C-0x08883114 | 24 | 4 | handlers de efeito depois de effects_p: passos por duel_step, alvos e máscaras de zonas, trocas de controle, contadores de turno, compras e descartes |
| [`effects_s.c`](../src/libduel_eng/effects_s.c) | 0x08883114-0x08884118 | 8 | 4 | handlers: virar e destruir pela DEF, cartas viradas para baixo reveladas, trocas de zona de magia, metade do ATK como dano |
| [`effects_t.c`](../src/libduel_eng/effects_t.c) | 0x08884118-0x08884F18 | 8 | 5 | handlers: invocação especial da mão, comandos de virar, ATK/DEF originais guardados na zona, descarte de cartas da mão |
| [`effects_u.c`](../src/libduel_eng/effects_u.c) | 0x08884F18-0x08885B70 | 7 | 2 | handlers: cartas de mesmo nome mandadas de todas as pilhas, fichas invocadas por tabela, mãos devolvidas e recompradas, metade do ATK como dano, ataque forçado |
| [`effects_v.c`](../src/libduel_eng/effects_v.c) | 0x08885B70-0x08886984 | 6 | 5 | handlers: maior ou menor ATK/DEF entre os alvos, três alvos de uma vez, cartas do cemitério devolvidas, carta escolhida na mão do oponente, máscaras de zonas do oponente |
| [`effects_w.c`](../src/libduel_eng/effects_w.c) | 0x08886984-0x08887618 | 16 | 2 | handlers: zonas com um efeito destruídas, cartas viradas para cima todas de uma vez, invocação pela carta especial do turno, descarte e recompra |
| [`effects_x.c`](../src/libduel_eng/effects_x.c) | 0x08887618-0x08889BE4 | 13 | 7 | handlers: troca de controle e de alvo, cartas reveladas da mão do oponente, invocações especiais do deck, da mão e do cemitério com zonas livres por jogador, dano e PV pelo número de cartas no cemitério ou viradas; tabelas de salto no `@data` |
| [`effects_y.c`](../src/libduel_eng/effects_y.c) | 0x08889BE4-0x0888B904 | 17 | 5 | handlers: invocações do deck por jogador alternado, cartas do cemitério e da zona F de volta ao campo, troca de zona com a carta especial, dano igual ao ATK do alvo, máscaras de zonas por tipo de carta |
| [`effects_z.c`](../src/libduel_eng/effects_z.c) | 0x0888B904-0x0888C1A8 | 3 | 7 | handlers que chamam duel_088C87A8 sem protótipo: marcas em cartas do campo pelo jogador da vez, compras dos dois jogadores, carta do cemitério para a zona de magia, carta 0x13EA do deck |
| [`effects_za.c`](../src/libduel_eng/effects_za.c) | 0x0888C1A8-0x0888E5D4 | 21 | 2 | filtro de escolha na mão, troca de três cartas da mão com o oponente, ATK fixo e marcas no alvo, fichas por id da carta, invocação na fase 5, monstros do cemitério de volta pela escolha da lista, troca com a carta especial, marcas com ATK/5 e contadores, invocação pela carta especial do turno, cartas viradas para baixo na zona, troca de monstros entre jogadores, dano fixo, compras da mão |
| [`effects_zb.c`](../src/libduel_eng/effects_zb.c) | 0x0888E5D4-0x0888F420 | 7 | 3 | handlers: máscara de zonas pelos alvos, cartas no cemitério dos dois jogadores, soma de ATK dos monstros controlados, LP dos dois jogadores, marcas por cartas no cemitério, cartas da mão ou do deck para o campo |
| [`effects_zc.c`](../src/libduel_eng/effects_zc.c) | 0x0888F420-0x08890094 | 2 | 3 | handlers: troca de cartas da mão entre os jogadores, carta do cemitério para a zona de magia, contador de níveis 0x1497-0x149A, ataque forçado contra a carta especial |
| [`effects_zd.c`](../src/libduel_eng/effects_zd.c) | 0x08890094-0x08890820 | 4 | 2 | handlers: invocação do deck com pedido ao oponente, dano pelo ATK do monstro escolhido, carta do campo de volta, máscara de monstros com DEF até o ATK do alvo |
| [`effects_ze.c`](../src/libduel_eng/effects_ze.c) | 0x08890820-0x088917D4 | 13 | 1 | handlers: invocação por tributo do próprio monstro, invocação pelo oponente, alvo em S/T com máscara por jogador, carta virada com flip especial |
| [`effects_zf.c`](../src/libduel_eng/effects_zf.c) | 0x088917D4-0x08892554 | 8 | 3 | handlers: dano a todos os monstros do oponente, dano/ganho por contagem de alvos (switch de ids), swap de ATK entre dois alvos, fila de uids em duel_chain, callback de efeito 0x14C0 |
| [`effects_zg.c`](../src/libduel_eng/effects_zg.c) | 0x08892554-0x08894158 | 23 | 2 | handlers: tributo por nível vindo do cemitério, efeitos 0x14C4/0x17C2, alvos duplos, troca de mão entre jogadores, cartas de unk3832 de volta, dano por máscara, fim de turno com tabela de saltos, despacho por id (0x14FD-0x1694), invocação alternada pelos dois jogadores |
| [`duel_checks.c`](../src/libduel_eng/duel_checks.c) | 0x088C2F80-0x088C4448 | 11 | 6 | verificações de estado entre os passos: cartas contínuas cuja condição acabou são destruídas ou perdem o efeito |
| [`duel_lp.c`](../src/libduel_eng/duel_lp.c) | 0x088C4448-0x088C4864 | 9 | 0 | mudanças de PV: dano e recuperação com as cartas que os impedem, comandos 0x25-0x30, evento `DuelLpEv` |
| [`duel_draws.c`](../src/libduel_eng/duel_draws.c) | 0x088C4864-0x088C77D4 | 11 | 7 | pagamento de PV, compra e envio de cartas do deck e da zona D com seus comandos; os passos longos em asm |
| [`duel_cmds.c`](../src/libduel_eng/duel_cmds.c) | 0x088C77D4-0x088C7F18 | 14 | 1 | comandos à apresentação via duel_cmd com 0x8000 para o jogador 1, contador de aninhamento do efeito exibido |
| [`duel_reqs.c`](../src/libduel_eng/duel_reqs.c) | 0x088C87A8-0x088C936C | 13 | 4 | comandos à apresentação sobre uma zona: 0x36 carta chega, 0x37 efeito de uma carta, 0x38 contador, 0x42 movimento com seus efeitos |
| [`duel_reqs2.c`](../src/libduel_eng/duel_reqs2.c) | 0x088C936C-0x088CA4E8 | 4 | 8 | mais comandos à apresentação: virar/mudar posição 0x35, mover entre zonas 0x3D/0x3E, da mão ou do cemitério ao campo 0x3F |
| [`duel_hand.c`](../src/libduel_eng/duel_hand.c) | 0x088CA4E8-0x088CAF24 | 7 | 2 | cartas que saem da mão para o cemitério, a zona D ou o campo, comando 0x33 e evento com o bitfield `DuelMoveEv` |
| [`duel_moves.c`](../src/libduel_eng/duel_moves.c) | 0x088CB400-0x088CBB64 | 12 | 3 | comandos de movimento entre pilhas e campo 0x33/0x3F/0x41, ataques 0x29, embaralhar zonas 0x49, carta saindo de uma zona 0x43 |
| [`duel_counters.c`](../src/libduel_eng/duel_counters.c) | 0x088CC178-0x088CC8EC | 8 | 0 | comandos de valores e contadores numa carta 0x3A-0x3C e as cartas que reagem a eles |
| [`act_apply.c`](../src/libduel_eng/act_apply.c) | 0x088D13CC-0x088D1DF4 | 29 | 1 | wrappers de duel_act_apply 0x088D1094: um efeito do tipo 0xB, 0xD, 0xE ou 0xF numa máscara de zonas, teste de uma zona, carta de ação montada na pilha com `= {0}` |
| [`req_effect.c`](../src/libduel_eng/req_effect.c) | 0x088D6DE8-0x088D9080 | 3 | 9 | pedidos que mexem nas entradas de efeito e nos valores de uma zona: adicionar, mudar, liberar, ordem de ativação, valor, contadores, virar uma carta, mudar a posição, trocar duas zonas, mover para outra zona ou lugar, mover de qualquer lugar a uma pilha |
| [`req_step.c`](../src/libduel_eng/req_step.c) | 0x088DA5EC-0x088DEB2C | 46 | 11 | pedidos que o motor executa um passo por quadro, `duel_req` 0x08914A30: log de entradas, mensagens, cemitério e mão, invocar e baixar da mão, pedidos que só repassam ao host, mudança de fase, fim de turno, próximo turno, comprar, mandar do deck ao cemitério, efeito e contador numa zona, pôr uma carta num lugar, anunciar, ativar e apontar cartas, banir, mandar a uma pilha, deck (embaralhar, inverter, devolver uma carta), bits de modo, de jogador e de slot; localizações de 16 bits nas mensagens ao host |
| [`rules_check.c`](../src/libduel_eng/rules_check.c) | 0x088DEB2C-0x088DFFC0 | 21 | 2 | carta, id e uid, posição, face e contagem por zona vistas pelas regras, bits de modo, de jogador e de slot, pagar custo, cartas que proíbem ativar, invocar ou baixar, bloqueios dos dois jogadores, quem deve responder |
| [`effect_mod.c`](../src/libduel_eng/effect_mod.c) | 0x088DFFC0-0x088E0890 | 1 | 10 | alocar/liberar entradas de efeito, adicionar e remover por fonte, tipo ou carta |
| [`effect_scan.c`](../src/libduel_eng/effect_scan.c) | 0x088E0890-0x088E1FE0 | 20 | 6 | contagem regressiva das entradas temporárias, limpeza no fim do turno e ao sair de uma zona, troca da zona de origem, contagens e buscas das entradas que uma carta pôs numa zona, zona que uma carta afeta |
| [`effect_list.c`](../src/libduel_eng/effect_list.c) | 0x088E1FE0-0x088E2964 | 26 | 0 | listas de efeitos das zonas, das cartas especiais e da lista de cartas: buscar, contar, somar, percorrer |
| [`effect_live.c`](../src/libduel_eng/effect_live.c) | 0x088E2964-0x088E5800 | 9 | 2 | entrada de efeito ainda em vigor, contagem de bits, cartas usáveis por zona, `switch` por id da carta ativada: se tem alvo, o que espera antes de resolver, quantas escolhas, quem pode responder, de que lado vão as cartas ao cemitério |
| [`card_place.c`](../src/libduel_eng/card_place.c) | 0x088E5800-0x088E5F84 | 10 | 4 | alvos de uma ativação, cópia e troca de cartas, colocar uma carta numa zona de monstro ou de magia/armadilha |
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
| [`field_props.c`](../src/libduel_eng/field_props.c) | 0x088F6380-0x088F71D8 | 6 | 1 | nível, tipo e espécie efetivos do monstro, máscara de espécies |
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

## libhtml_view (2 arquivos, 2 funções batendo, 0 em asm)

| Arquivo | Faixa | Batendo | Asm | Conteúdo |
|---|---|---|---|---|
| [`lang_remap.c`](../src/libhtml_view/lang_remap.c) | 0x0880439C-0x08804414 | 1 | 0 | 0x0880439c: maps the config language id to the download-site language index |
| [`save_path.c`](../src/libhtml_view/save_path.c) | 0x088044B8-0x08804508 | 1 | 0 | 0x088044b8: builds "/PSP/SAVEDATA/<GAMEID>" |
