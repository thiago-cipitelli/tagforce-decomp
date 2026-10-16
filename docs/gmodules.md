# gmodules (fase 1)

Fonte: `build/decomp/<módulo>/index.tsv` gerado por `tools/ghidra/ExportDecomp.java`; tabela por `tools/gmodule_summary.py`. Endereços na base `0x08804000`.

## Três tipos de módulo

1. **Cena** — tem `module_start` (PRX `e_entry` ≠ 0, ou `.text+0` quando `e_entry` = 0) que chama `ehscript_register(main, exit, dados)`. É carregada pela thread `ehscript` via `ehscript_set_module(nome)` / `scene_push` / `scene_pop_to`, usando a tabela de ids do BOOT (`FUN_08853a2c`): 1 libdebug_menu, 2 libstory, 3 libduel_draw, 4 libdeck, 5 libfield, 6 libduel_mgr, 7 libshop, 8 libsdchr_viewer, 9 liblabo, 10 libmovie_viewer, 11 libcutin_viewer, 12 libsand_box, 13 libtitle, 14 libhtml_view. Os módulos `libdebug_menu`, `libsdchr_viewer`, `libmovie_viewer`, `libcutin_viewer` e `libsand_box` **não existem no disco**: cenas de desenvolvimento removidas.
2. **Biblioteca de sub-tela** — `module_start` retorna 0 e exporta 3 a 7 funções (tipicamente `init(modo, heap)`, `update()` com máquina de estados, `draw()`/`close()`). A cena dona **importa essas funções por NID** (stubs `lib<x>_<addr>` em `symbols/<cena>.txt`) e, quando precisa da sub-tela, pede o carregamento com `ehfile_request_module(2, "lib<x>.prx", callback, 0)`; o callback recebe o id do módulo e chama `sceKernelStartModule(id, 4, &1, 0, 0)`; o kernel da PSP resolve os stubs pendentes da cena nesse momento (ligação tardia). Observado em `libtitle` (`FUN_08805ac0` → `FUN_08805b14` → `FUN_0880a8ac`) e nas chamadas `libcharalist_08804F48()` / `librecipe_viewer_08805874()` da máquina de estados do título.

Grafo de dependências (quem importa quem, nº de funções): libtitle ← cardalbum 3, charalist 7, deck_tutorial 3, duelrecord 3, limitlist 3, recipe_viewer 3, select_card 4, tutoriallist 4 · libstory ← os mesmos + conv_machine 3, password 3, field 6, shop 5 · liblabo ← cardalbum, charalist, conv_machine, duelrecord, limitlist, password, select_card, tutoriallist · libdeck ← deck_tutorial 3, recipe_viewer 3 · libconv_machine ← select_card 4 · libduel_mgr ← duel_eng 143, duel_draw 38, deck_swap 3, tutorial 7 · libtutorial ← duel_eng 143, duel_draw 38 · libdeck_swap ← duel_draw 38 · libduel_draw ← duel_eng 143. Para o port: a ordem de carga dos módulos é dada por esse grafo, e os stubs viram chamadas diretas.

3. **Biblioteca importada** — `libduel_eng` (143 exports) é importada por NID pela `libduel_draw`, como a `libehsys`.

## Tabela

| Módulo | Papel | main / exit da cena | Funções | Maior função | libehsys distintas | Chamadas de engine mais frequentes |
|---|---|---|---|---|---|---|
| libcardalbum | biblioteca | — | 1308 | `FUN_088077b8` (1304 B) | 121 | pad_get (7), region_pick_value (7), dl_set_tex_filter (6), mtx_store (6), se_play (6) |
| libcharalist | biblioteca | — | 1291 | `FUN_08808844` (2392 B) | 86 | dl_set_tex_filter (16), draw_set_offsets (16), draw_sprite (16), dl_ambient_color (16), gim_set_texture (15), font_set_size (8) |
| libconv_machine | biblioteca | — | 1291 | `FUN_088076d8` (2568 B) | 132 | dl_enable (7), mtx_store (7), dl_disable (6), dl_set_list (6), dl_select_layer (6) |
| libdeck | cena | `FUN_0880b98c` / `FUN_0880bab4` | 1558 | `FUN_088060b0` (2920 B) | 176 | dl_ambient_color (33), gim_set_texture (30), table_get_entry (27), dl_set_tex_filter (27), draw_sprite (27) |
| libdeck_swap | biblioteca | — | 1477 | `FUN_08814c24` (1780 B) | 123 | dl_ambient_color (25), dl_set_tex_filter (24), draw_sprite (24), draw_set_offsets (24), gim_set_texture (22) |
| libdeck_tutorial | biblioteca | — | 1520 | `FUN_0880cdc8` (2980 B) | 140 | dl_ambient_color (33), draw_set_offsets (32), draw_sprite (32), dl_set_tex_filter (31) |
| libduel_draw | cena | `FUN_0880647c` / `FUN_08806688` | 2689 | `FUN_08835954` (5956 B) | 348 | dl_select_layer (100), dl_set_list (100), mtx_store (89), mtx_push (71), mtx_identity (70) |
| libduel_eng | biblioteca | — | 4334 | `FUN_088f2d70` (12260 B) | 104 | card_is_monster (243), card_get_type (174), card_get_level (61), card_get_prop5_hi (55), card_is_attribute9 (53), card_same_name (44) |
| libduel_mgr | cena | `FUN_08804034` / `FUN_08804a34` | 1591 | `FUN_0880a2b8` (2800 B) | 159 | game_state_get (43), sysvar_get_a98 (21), dl_set_tex_filter (20), dl_ambient_color (19), gim_set_texture (19) |
| libduelrecord | biblioteca | — | 1270 | `FUN_088060a4` (1548 B) | 56 | dl_set_tex_filter (8), draw_set_offsets (7), gim_set_texture (7), dl_ambient_color (7), draw_sprite (7), font_set_size (5) |
| libfield | cena | `FUN_08804038` / `FUN_0880412c` | 1847 | `FUN_0880d4f4` (3140 B) | 247 | table_get_entry (53), gim_set_texture (47), dl_ambient_color (46), cfg_get_language (42), font_set_size (37) |
| libhtml_view | cena | `FUN_08804034` / `LAB_08804084` | 1283 | `FUN_08804508` (300 B) | 16 | strcat (2), ehscript_register (1), sys_frame_wait (1), sceKernelChangeCurrentThreadAttr (1), scene_pop_to (1) |
| liblabo | cena | `FUN_08804034` / `LAB_088048c4` | 1285 | `FUN_08804034` (2192 B) | 36 | font_printf (4), sprintf (3), pad_get (2), region_pick_value (2), dl_select_layer (2) |
| liblimitlist | biblioteca | — | 1305 | `FUN_088076ec` (1700 B) | 111 | dl_set_tex_filter (10), draw_set_offsets (9), dl_ambient_color (9), draw_sprite (9), gim_set_texture (8) |
| libpassword | biblioteca | — | 1277 | `FUN_08804b1c` (1996 B) | 99 | table_get_entry (6), pad_get (6), se_play (5), wtext_count_tags (5), wcscpy (5) |
| librecipe_viewer | biblioteca | — | 1313 | `FUN_08807c0c` (1640 B) | 105 | draw_sprite (13), draw_set_offsets (13), dl_ambient_color (13), dl_set_tex_filter (12), gim_set_texture (10) |
| libselect_card | biblioteca | — | 1441 | `FUN_08808d88` (1948 B) | 150 | dl_ambient_color (24), draw_set_offsets (23), draw_sprite (23), dl_set_tex_filter (23), gim_set_texture (21) |
| libshop | cena | `FUN_08804304` / `FUN_08804394` | 1456 | `FUN_08815b9c` (4620 B) | 240 | dl_set_tex_filter (34), draw_set_offsets (31), draw_sprite (31), gim_set_texture (30), dl_ambient_color (29) |
| libstory | cena | `FUN_08804038` / `LAB_08804258` | 1938 | `FUN_08806d40` (2216 B) | 95 | sys_frame_wait (51), gfx_overlay_enable (13), wcscpy (10), rand_next (5), sysflag_set (5) |
| libtitle | cena | `FUN_08804034` / `LAB_08804218` | 1698 | `FUN_0880d454` (3168 B) | 197 | se_play (28), ehp_find (27), font_set_size (26), font_set_color (26), font_set_flag4 (24) |
| libtutorial | biblioteca | — | 1613 | `FUN_0880812c` (1004 B) | 93 | dl_disable (7), dl_enable (7), dl_color (6), pad_get (5), ehp_find (5) |
| libtutoriallist | biblioteca | — | 1268 | `FUN_088048f0` (1332 B) | 63 | dl_set_tex_filter (8), draw_set_offsets (7), dl_ambient_color (7), gim_set_texture (7), draw_sprite (7), table_get_entry (4) |

Nota: para `libcharalist`, `libdeck`, `libdeck_swap`, `libduel_draw`, `liblimitlist`, `librecipe_viewer`, `libshop` e `libtutorial` o PRX tem `e_entry` ≠ 0; a função em `.text+0` é só a primeira do módulo (`text_start`), e o `module_start` real está em `symbols/<m>.txt`.

## Máquinas de estado das cenas

Função com mais `case` por módulo (candidata a máquina de estados principal): libtitle `FUN_08806ef0` (40 estados, estado em `DAT_094306f8`, 0x28 = sair), libfield `FUN_0880ed50` (54), libstory `FUN_08806d40` (165), libduel_mgr `FUN_08808540` (75), libdeck `FUN_0881e5c0` (80), libshop `FUN_08805ab4` (15), libtutorial `FUN_08804ca4` (32), libduel_draw `FUN_08838d74` (364 — despacho de efeitos visuais), libduel_eng `FUN_088d4dc4` (103 — despacho de comandos do duelo, par de `duel_cmd`).

## Dados e strings por módulo

- **libfield**: tabela de 29 salas `bg_NN_NN [R_HOSTEL_OUTSIDE]`, `[R_HOSTEL_101]`… (dormitório, mapa do campus); carrega `field/bustup/bu_%s.ehp.gz` (retratos). Usa `cfg_get_language` 42 vezes: textos por idioma.
- **libstory**: scripts em `script/st01_t01_*`, `script/st02_t01_*` e `e3/script/st01_t01_` (resíduo da demo E3), arquivos `.bin.gz` dentro dos EHPs (`SysMsg`, `ImpChrMsg`, `ImpMapMsg`, `FieldCon.bin`, `FieldPos.bin`, `ItemCon.bin`, `CardCon.bin`, `MailESM.bin`, `Lesson*`); heap `HeapMovie`. 51 chamadas a `sys_frame_wait`: a história é um script interpretado quadro a quadro.
- **liblabo**: "Labo Debug Menu", "Password Machine", "Rental", `ChrID : %03d`, `VOICE : %03d : %s` — menu de depuração que sobrou no jogo final (cena 9).
- **libhtml_view**: URLs `http://www.konami.jp/gs/game/yugioh_tf/dl/`, `dleu/`, `dlus/` e `/PSP/SAVEDATA/`: visualizador de conteúdo baixável (16 funções da engine apenas).
- **libduel_draw**: `cutin_da_faces/`, `DA_boy_B01.iff`…, `0000.iff`: modelos/texturas `.iff` por personagem; `host0:../src/duel_draw/DuelViewParam.txt`.
- **libduel_eng**: vocabulário do parser de setup (`TurnPlayer`, `Phase`, `EquipAct`, durações).
- **libtutorial**: `tutorial/`, `tutorial_`, `CardIn`, `[END]`, formato `7%s$C0`.
- **libshop**: `box_side%02d.iff`, `box_tex%02d.iff` (modelos dos boosters); heap próprio de 6 MB.
- **libcardalbum/charalist/duelrecord/limitlist**: cada um lê `database/<nome>.ehp`; **libpassword/conv_machine**: `labo/password.ehp`, `labo/conv_machine.ehp`.
- Strings `gfisb` / `jegfis` em vários módulos: ordem de idiomas (g, f, i, s, b / j, e, g, f, i, s) usada para montar nomes de arquivo por idioma.
