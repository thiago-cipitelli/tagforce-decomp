# Formatos de dados (fase 2)

Legenda: **[obs]** lido do código do BOOT, **[val]** validado por ferramenta contra os arquivos do disco, **[inf]** inferido, **[?]** aberto.

## EHP — pacote de arquivos [obs][val]

Ver `docs/architecture.md` §10 e `tools/ehp.py` (lista/extrai; 303 arquivos validados).

## GIM — textura PSP (`MIG.00.1PSP`) [val]

Formato padrão da Sony; `tools/gim.py` decodifica para PNG com stdlib. Validado visualmente (`card_frame_all.gim`) e estruturalmente em 250 texturas dos EHPs extraídos.

```
+0   "MIG.00.1PSP\0" + 4 bytes zero
blocos encadeados: { u16 id, u16 _, u32 size, u32 next, u32 data_off=0x10 }
  id 2 root, 3 picture (contêineres: só avançam 16 bytes), 4 image, 5 palette, 0xFF file info
cabeçalho de image/palette em bloco+0x10:
  u16 hdr_size(0x30) u16 _ u16 format u16 order u16 w u16 h u16 bpp u16 pitch_align u16 height_align u16 _
  u32 _ u32 level_table_off u32 data_off ...  pixels em bloco+0x10+data_off
format: 0 RGBA5650 1 RGBA5551 2 RGBA4444 3 RGBA8888 4 INDEX4 5 INDEX8 6 INDEX16 7 INDEX32
order 1 = swizzle PSP (blocos 16 bytes × 8 linhas)
```

Uso no jogo: 161 texturas INDEX8 swizzled, 83 INDEX4 swizzled, 5 lineares, 1 de formato 8 [?]. A engine lê o cabeçalho em `gim_set_texture` (`0x08835704`) e emite TMODE/TPSM/TSIZE/TBP.

## Banco de cartas — `duelsys/cardinfo_<idioma>.ehp` [obs][val]

Carregado por `card_db_load` (`0x0884d4f8`) em uma struct global (`PTR_DAT_088eee88`): `+4 Name, +0xc Indx, +0x14 Desc, +0x18 Sort, +0x1c DLG_Indx, +0x20 DLG_Text, +0x24 Genre, +0x28 Top, +0x2c SamePict, +0x30 Prop, +0x34 Pass, +0x38 IntID`. Sufixo de idioma (`_E`, `_J`...) vem de `cfg_get_language`.

IDs de carta começam em 4000. `card_index_from_id(id)` (`0x0884d794`) = `CARD_IntID.bin[id − 4000]` (u16, 2958 entradas: ids 4000..6957). Índice 0 é nulo; 2480 cartas.

| Arquivo | Registro | Conteúdo |
|---|---|---|
| `CARD_Prop.bin` | 8 bytes × 2480 | ver abaixo |
| `CARD_Pass.bin` | u32 × 2480 | senha de 8 dígitos (`card_get_password`) |
| `CARD_Sort_E.bin` | u16 × 2480 | ordem alfabética (`card_get_sort`) |
| `CARD_Genre.bin` | 6 bytes × 2480 | 48 flags de gênero (`card_has_genre_bit(id, bit)`) |
| `CARD_Indx_E.bin` | {u32 name_off, u32 desc_off} × 2481 | offsets em Name/Desc |
| `CARD_Name_E.bin`, `CARD_Desc_E.bin` | UTF-16LE, NUL-terminado | nome e texto |
| `CARD_Top_E.bin` | {u16 índice, u16 letra} | primeiro índice por inicial (navegação) |
| `CARD_SamePict_E.bin` | {u16 índice, u16 n} | cartas que compartilham arte (`card_get_samepict`) |
| `CARD_IntID.bin` | u16 × 2958 | id → índice |

`CARD_Prop.bin`, por carta (acessores em `0x0884d8xx..0x0884dcxx`, todos via `card_prop_cur`):

```
u32 @0: bits 0-12 id (4007 para Blue-Eyes), bits 13-21 ATK/10 (0x1ff = "?"), bits 22-30 DEF/10, bit 31 flag (card_get_flag31)
byte @4: nibble baixo [?], nibble alto [?]
u32 @4: bits 15-19 tipo de monstro (card_get_type; Dragon=1, Spellcaster=18 [inf])
byte @5: nibble baixo = nível (card_get_level), bits 4-6 [?]
byte @6: bits 4-6 [?]  (Blue-Eyes 4, Mystical Elf 0: não é atributo)
```

Verificado: Blue-Eyes (índice 1) → `0xbea58fa7 0x00408810` = id 4007, ATK 3000, DEF 2500, nível 8. Mystical Elf → 800/2000, nível 4.

## CIP / CPM — imagens das cartas (`card/cardh_e.cip`, `card/cardm_e.cip`) [obs parcial]

Abertos por `cip_open_h` / `cip_open_m` (`0x08853d1c` / `0x08853dd4`), nome por idioma (`card/cardh_<l>.cip`). Lidos assincronamente pelo ehfile em blocos de 0x800. Cabeçalho (0x30 bytes): magic `CIP\x1a` (h) ou `CPM\x1a` (m), u32 tamanho, 0, 0, u32 `0x5f18` [?], u32 primeiro id (4007), u32 último id (6957), u32 0x30, u32 chunks por imagem (h=2, m=1). Depois do cabeçalho vem uma tabela e, alinhadas a 0x800, as imagens em slots fixos de `chunks × 0x800` bytes (`FUN_08854708(ctx, índice)` lê `índice × chunks × 0x800`). Decodificação dos pixels [?]: `FUN_088548a4` chama `gim_set_texture`, então cada slot provavelmente é um GIM.

## Scripts de tutorial/puzzle [obs][val]

Ver `docs/architecture.md` §11 (`NN_field_set.txt` em `tutorial/tutorial_<l>.ehp`).

## Pendentes

- `.ehf` (`loading.ehf`), `psp_snddat.bin` (áudio SAS/Atrac3+), `script/*.ehp` (história), `.efs`, `.ydc` (receitas de deck `RCP%03d.ydc`), `.cip` pixels.
