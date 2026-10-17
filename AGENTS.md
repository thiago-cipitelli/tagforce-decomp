# Yu-Gi-Oh! GX Tag Force (PSP) — decompilação e port nativo para PC

> Engenharia reversa do jogo *Yu-Gi-Oh! GX Tag Force* (PSP, ULUS-10136, Konami 2006) para reconstruir o código-fonte em C e, a partir dele, produzir um port nativo para PC (Linux/Windows). Projeto pessoal, inspirado em [ygo-tf-5-decomp](https://github.com/angelof-exe/ygo-tf-5-decomp).
> **Este resumo é mantido manualmente pelo desenvolvedor. Não edite sem solicitação explícita.**

---

## Produto

### Objetivos

- **Objetivo final:** o jogo rodando nativamente em PC, lendo os assets da ISO original do usuário, sem emulador.
- **Objetivo intermediário:** código C legível e organizado por módulo, com a lógica do jogo (motor de duelo, história, menus) separada da camada de plataforma PSP.
- **Matching byte-a-byte** (recompilar e obter binário idêntico) é uma *ferramenta de verificação*, usada onde a fidelidade importa (motor de duelo, parsers de dados). Não é meta global: o código do jogo soma ~5,7 MB de MIPS e matching completo seria trabalho de anos.

**Fora do escopo:** distribuir qualquer asset ou binário do jogo. O repositório contém só ferramentas, código reconstruído e documentação. A ISO é fornecida pelo usuário.

### Como funciona (fluxo do jogo original)

1. O firmware da PSP carrega `PSP_GAME/SYSDIR/BOOT.BIN` (módulo `modehsys`), que é o *runtime* do jogo: inicialização, loop principal, carregador de módulos, VFS, áudio, vídeo.
2. `BOOT.BIN` carrega sob demanda os módulos de `USRDIR/gmodule/*.prx` (title, field, story, shop, deck, duel_eng, duel_draw, duel_mgr etc.). Cada um exporta suas funções por nome através de uma biblioteca PSP (`.lib.ent`).
3. Os dados vêm de arquivos EHP (arquivos-pacote), CIP (imagens de cartas), PMF (vídeo), `psp_snddat.bin` (áudio), scripts de história em `USRDIR/script/` e banco de cartas em `USRDIR/duelsys/cardinfo_*.ehp`.
4. Um duelo roda em `libduel_eng` (regras e estado) com renderização em `libduel_draw` e orquestração em `libduel_mgr`.

### Como vai funcionar (o port)

```
código do jogo (C portável, reconstruído)
      ├── platform/psp  → chamadas sce* originais  (build PSP, roda no PPSSPP: prova de fidelidade)
      └── platform/pc   → SDL2 + OpenGL + stdio    (build PC: o objetivo)
assets: lidos direto da ISO extraída pelo usuário
```

---

## Plano do projeto — fases

Cada fase tem um *entregável verificável*. Não avance de fase sem o entregável.

| # | Fase | Entregável | Estado |
|---|---|---|---|
| 0 | Infraestrutura de análise | ISO extraída, PRX convertidos em ELF estático, REA+Ghidra abrindo `BOOT.elf` | **concluída** (2026-10-16) |
| 1 | Mapa do binário | `docs/architecture.md`, `docs/gmodules.md`, `symbols/*.txt` | **concluída** (2026-10-16): init, VFS, cenas, modmgr, gráficos; 680/680 funções da libehsys usadas por gmodules nomeadas (a maioria `inferred`, 30 com `[?]`); 66 globais do BOOT nomeadas; 22 gmodules classificados (cena / biblioteca de sub-tela importada por NID / biblioteca importada) com entrada, máquina de estados e dados. Pendente para fases seguintes: structs de estado por gmodule, 460 exports da libehsys que nenhum gmodule chama |
| 2 | Formatos de dados | `tools/` lendo EHP, CIP, cardinfo, scripts, snd; `docs/formats.md` | **em andamento**: EHP, GIM, CIP/CPM e banco de cartas prontos e validados; faltam snd, scripts de história, ehf, efs, ydc |
| 3 | Build reproduzível (matching) | `splat` + mwccpsp via wibo + Makefile; `libhtml_view` (21 KB) 100% matching como prova de pipeline | **concluída** (2026-10-16): `make MODULE=libhtml_view check` → MATCH; compilador = mwccpsp 3.0.1 build 219, `-O4,p -sdatathreshold 0`; pipeline estendido a todos os 23 módulos (`make check-all` → MATCH); ver `docs/build.md` |
| 4 | Decompilação do código do jogo | `src/<módulo>/*.c` funcionalmente equivalentes, com matching nas partes críticas | **em andamento** (início 2026-10-16): BOOT `ehfile` (VFS, 29 funções), `modmgr` (19), `sys`/`ehscript` (17), `main` (26), `save` (19), `inv` (45), `cfg` (30), `deck` (34), `wstr` (charmap, wcs*, 13), `wfmt` (formatação UCS-2, 12), `menu` (registro de itens de janela, 10) `menu_draw` (moldura 9-slice, 7) `font` (cache de glifos sceFont, 16), `font_flags` (19), `font_tags` (7), `font_text` (layout de texto UCS-2 com tags $R/$C/$f/$F, só C sob `NON_MATCHING`), `menu_widget` (janela com lista rolável e botões, 9) e `card_db` (receitas RCP*.ydc, cardinfo_<lang>.ehp, acessores de carta, 64 + 2 callbacks de cena) `card_groups` (listas de ids hard-coded como `switch`, classes especiais e ids alternativos, 41) `pmf_umd` (thread do ring buffer sceMpeg, wrappers DvdUmdRetry* da Sony, carga dos módulos de codec, 22) `movie` (player PSMF da Sony: movie_run, threads display/read/sound, filas de pts e sincronia A/V, 62 matching + 8 em asm) `cip` (pilha de cenas scene_push/pop_to/reset_to + leitor CIP/CPM das imagens de carta com anel de slots e prefetch, 21 matching + 3 em asm) `fade` (fades de tela linear/quadrático/wipe, 9) `cardinfo` (visualizador de detalhes da carta: estado em globais 0x08BEA44C, abas descrição/gêneros/listas, scroll, dicas de botão, 21 matching + 5 em asm) e `sndapi` (API de som exportada aos gmodules: BGM/SE, callbacks de leitura/heap da biblioteca de som, tabelas de voz por personagem, filas de voz por jogador, movie_play, caminhos de save-data; 51 matching + 8 em asm) e `snd_bank` (cabeçalho SHDS de psp_snddat.bin, grupos, cache de handles, listas intrusivas, 29 + 4 em asm) e `snd_core` (máscara de vozes SAS, volumes por canal, medidor de nível para lip sync, reverb, 15 + 4 em asm) em `src/BOOT/` (668 funções em C, 77 mantidas em asm via `@asm` com C sob `NON_MATCHING`: `wsprintf`, `menu_layout`, `menu_item_build*`, `font_glyph_cache`, `font_layout_*`, `deck_validate`, …); `make MODULE=BOOT check` → MATCH |
| 5 | Camada de plataforma | `platform/psp` e `platform/pc` com a mesma API; jogo compila nos dois alvos | a fazer |
| 6 | Port PC jogável | Menu → história → duelo completo rodando em PC com assets da ISO | a fazer |
| 7 | Paridade e polimento | Vídeo PMF, áudio Atrac3+, save compatível, resolução/aspect, input | a fazer |

### Fase 1 — Mapa do binário

Perguntas a responder, nessa ordem, cada uma com Evidence ID do REA citado em `docs/architecture.md`:

1. `module_start` do BOOT (`0x08804000`): threads criadas, ordem de init (sceGu, sceAudio, sceCtrl, VFS).
2. Loop principal e máquina de estados de telas (título → campo → duelo).
3. Carregador de gmodules: como resolve `disc0:/PSP_GAME/USRDIR/gmodule/`, `sceKernelLoadModule`, e a tabela de funções exportadas que o BOOT chama.
4. VFS e parser EHP: função que abre `*.ehp`, estrutura do header (`EHP\x03`, count, tabela nome→offset).
5. Importações sce* usadas por módulo (`.lib.stub`): essa lista **é** a API que `platform/` precisa cobrir.
6. Por gmodule: ponto de entrada exportado, estruturas principais, dependências entre gmodules.

Saída: `symbols/<módulo>.txt` no formato splat (`nome = 0xADDR; // tipo:func`), porque a sessão Ghidra do REA é efêmera e os nomes dados por `annotate_native_function` **se perdem ao fechar**. O arquivo de símbolos é a fonte de verdade.

### Fase 2 — Formatos de dados

Ordem: EHP (tudo depende dele) → cardinfo (banco de cartas, necessário para o motor) → CIP (imagens) → scripts de história → `psp_snddat.bin` → PMF (só no port; usa ffmpeg).
Ponto de partida: o repositório do TF5 já tem extrator EHP; o header aqui é o mesmo (`EHP\x03`). Confirmar antes de reescrever.

### Fase 3 — Build reproduzível

- Compilador: `MW MIPS C Compiler (2.4.1.01)` (seção `.comment`). Procurar `mwccpsp` 2.4.1 na coleção de compiladores do decomp.me; rodar via `wibo`. Flags prováveis: `-O4,p` ou `-O4,s`; descobrir comparando uma função pequena no decomp.me.
- `splat` com target PSP gera `asm/` e linker script a partir de `build/elf/<módulo>.elf`; `make` reconstrói e compara SHA1 com o ELF convertido (não com o PRX: a relocação já foi aplicada, então comparar contra `build/elf/*.elf`).
- Prova de pipeline: `libhtml_view` (menor gmodule). Só depois disso vale decompilar em volume.

### Fase 4 — Ordem de decompilação

1. `BOOT` (runtime, ~437 KB de `.text`): sem ele nada carrega.
2. `libduel_eng` (1,5 MB): coração do jogo, matching desejável nas regras.
3. `libduel_mgr`, `libduel_draw`.
4. `libfield`, `libstory` (mundo e história).
5. `libtitle`, `libdeck*`, `libshop`, `libselect_card`, demais menus.

### Fase 5 e 6 — Plataforma e port

- API mínima de `platform/`: janela/frame, submissão de display list (substitui sceGu), textura, áudio PCM streaming, input, arquivos, thread/timer, save.
- O build PSP (`platform/psp`) continua existindo e rodando no PPSSPP: é o teste de regressão de que o C reconstruído se comporta como o original.
- Build PC com CMake, SDL2 e OpenGL 3.3. Vídeo e Atrac3+ via ffmpeg (libavcodec suporta `atrac3p`).

---

## Tecnologias

- **Python 3 (pyenv):** todas as ferramentas em `tools/`. Só stdlib + `pycdlib` (extração de ISO).
- **Ghidra 12.1.3 + ghidra-allegrex v21.4:** análise estática. Instalado em `~/opt/ghidra_12.1.3_PUBLIC`, extensão em `Ghidra/Extensions/ghidra-allegrex`.
- **REA (rea-agents 6.5.0, MCP):** interface do agente para o Ghidra. Registrado em `~/.claude.json` com `GHIDRA_INSTALL_DIR` e `JAVA_HOME` (JDK 21 via sdkman). Hopper está instalado mas **não suporta MIPS**; não usar.
- **splat64 0.50.1 + spimdisasm + rabbitizer** no venv `.venv`; **wibo 1.1.0 + mwccpsp 3.0.1 build 219** em `tools/ext/` (gitignored); **pspdev v20261001** (`psp-as`, `psp-ld`) em `~/opt/pspdev`. Pipeline em `docs/build.md`: `make MODULE=<m> check`, `make check-all`. Flags do compilador: `-O4,p -sdatathreshold 0 -i include`.
- **PPSSPP:** executar e depurar o original e o build PSP (ainda não instalado).
- **SDL2, OpenGL, ffmpeg, CMake:** port PC (fase 5+).

**Serviços externos:** nenhum. decomp.me é usado manualmente para testar flags de compilador.

### Setup de Desenvolvimento

- **Pré-requisitos:** Python 3, `pip install pycdlib`, JDK 21, Ghidra 12.1.3 com ghidra-allegrex, a ISO do jogo na raiz (gitignored).
- **Extrair e converter:**
  ```sh
  python3 tools/extract_iso.py "Yu-Gi-Oh! GX - Tag Force (USA).iso" extracted
  mkdir -p build/elf
  python3 tools/prx2elf.py extracted/PSP_GAME/SYSDIR/BOOT.BIN build/elf/BOOT.elf
  for p in extracted/PSP_GAME/USRDIR/gmodule/*.prx; do python3 tools/prx2elf.py "$p" "build/elf/$(basename "$p" .prx).elf"; done
  ```
- **Analisar via REA:** `open_binary` com `path` absoluto de `build/elf/<módulo>.elf` e `provider_id: "ghidra"`. A primeira abertura de cada binário roda a auto-análise do Ghidra (1 a 3 min). Fechar com `close_binary` ao trocar de módulo.
- **Testes:** cada ferramenta em `tools/` tem um sanity check executável; `prx2elf.py` é validado por: todos os `j/jal` do BOOT caírem dentro de `.text`, entry `0x08804000` começando com prólogo, e Ghidra reconhecendo ~1900 funções.
- **Build (matching):** `make MODULE=BOOT check` (split → assemble/compile → link → compara seções com `build/elf/<m>.elf`). Arquivo inteiro: `tools/check_file.py <m> src/<m>/x.c`; função isolada: `tools/mwcc_check.py src/<m>/x.c func <m> 0xADDR`; variantes de fonte em massa: `tools/perm_decls.py`. Convenção dos `.c`: cabeçalho `// @range` e `// @data`; funções e globais nomeados em `symbols/<m>.txt` com `matched src/...` (ver `docs/build.md`, seção "O que o mwccpsp faz com o fonte").

---

## Arquitetura

### Estrutura de Módulos / Pacotes

| Caminho | Responsabilidade |
|---|---|
| `tools/extract_iso.py` | Extrai a ISO (pycdlib) para `extracted/`. |
| `tools/prx2elf.py` | Aplica relocações PRX na base `0x08804000`, converte em `ET_EXEC`, anexa `.MIPS.abiflags`, resolve entry (module_start ou início de `.text`), corrige tamanho do `.bss`. Exigido pelo perfil PSP do REA. |
| `tools/ghidra/ExportDecomp.java` | Headless Ghidra: aplica `symbols/<m>.txt` (cria funções que faltam), exporta C decompilado de todas as funções em `build/decomp/<m>/*.c` + `index.tsv` (endereço, nome, tamanho, callees, callers, strings). Leitura em massa: `grep` no índice. |
| `tools/nids.py` | Gera `build/nids.txt` (lib, NID, nome) a partir dos stubs do pspsdk. |
| `tools/link_map.py` | Resolve os stubs de import de cada gmodule para funções do BOOT (`libehsys`) por NID e regenera a parte gerada de `symbols/<gmodule>.txt`. Rodar após nomear funções do BOOT. |
| `tools/rea_seeds.py` | Gera `build/rea_seeds.tsv` (linhas `seed` dos símbolos) para `REA_GHIDRA_SEED_FILE`. |
| `tools/ehp.py`, `tools/gim.py` | Extrai pacotes EHP; decodifica texturas GIM para PNG. |
| `build/` (gitignored) | ELFs convertidos, decompilação exportada, dados extraídos de EHP, saída do build. |
| `extracted/` (gitignored) | Conteúdo da ISO. |
| `docs/` | `architecture.md` (mapa do binário), `formats.md` (formatos de dados). |
| `symbols/` | Nomes de funções/dados por módulo, formato splat. Fonte de verdade de nomenclatura. Em `symbols/<gmodule>.txt`, só edite acima do marcador `// --- generated imports below`. |
| `src/<módulo>/` | C reconstruído, um arquivo por subsistema do módulo original (`src/BOOT/ehfile.c`, ...). Cada arquivo cobre uma faixa contígua de código (`@range`). |
| `include/` | `psp.h` (sce*), `libc.h` (MSL), um header por subsistema (`ehfile.h`). |
| `tools/splat_config.py`, `tools/splat_symbols.py`, `tools/fix_ld.py`, `tools/fix_asm.py` | Geração da config do splat a partir do ELF, símbolos em formato estrito, correções do linker script e do asm (ver `docs/build.md`). |
| `tools/mwcc.sh`, `tools/mwcc_check.py`, `tools/check_file.py`, `tools/perm_decls.py`, `tools/compare_sections.py`, `tools/diff_words.py` | Compilador original via wibo e verificação de matching por função, por arquivo, por seção. |
| `platform/` (a criar) | `psp/` e `pc/` implementando a mesma API. |
| `config/` | Configs do splat por módulo, geradas por `tools/splat_config.py` (regenerar ao criar/alterar `@range`/`@data`). |

### Fatos do binário (não reinvestigar)

- `BOOT.BIN` e `gmodule/*.prx`: ELF PSP relocável (`e_type 0xffa0`), **sem criptografia**. `USRDIR/module/*.prx` são libs Sony com header `~SCE`, criptografadas: ignorar, o port as substitui.
- Um único segmento LOAD por módulo; relocações só dos tipos R_MIPS_32, R_MIPS_26, HI16, LO16; dados das seções começam no offset `0x60` do arquivo.
- `e_flags = 0x10a23001` (MIPS II, Allegrex, EABI32, noreorder). Nenhum módulo traz `.MIPS.abiflags`; o conversor adiciona.
- gmodules não têm `module_start` (entry 0 é correto): exportam `module_info` (NID `0xF01D73A7`) e uma biblioteca nomeada com as funções do módulo.
- Tamanhos: BOOT `.text` 437 KB, `.data` 516 KB; `libduel_eng.prx` 1,5 MB; `libduel_draw.prx` 644 KB; total gmodule ≈ 4,1 MB.
- Compilador: Metrowerks `MW MIPS C Compiler (2.4.1.01)` em todos os módulos.

### Padrão de trabalho com o REA

```
open_binary(build/elf/X.elf, ghidra)
  → binary_overview                 inventário (segmentos, nº de funções, strings)
  → search_strings / xrefs          achar pontos de entrada por string ("fatal error", caminhos disc0:/)
  → analyze_function / batch_decompile   pseudocódigo + chamadas + referências
  → annotate_native_function        nomear na sessão (efêmero!)
  → anotar em symbols/X.txt e docs/  persistência real
  → verify_reconstruction           comparar C reconstruído com o original (fase 4)
close_binary
```

Regras:
- Toda afirmação em `docs/` sobre comportamento do jogo cita o Evidence ID do REA ou o endereço + instrução. Pseudocódigo não é prova de semântica de VFPU: o ghidra-allegrex não modela prefixos VFPU por completo; em código vetorial, ler o assembly.
- Distinguir sempre *observado* (no binário), *inferido* e *desconhecido*. Dúvidas abertas vão para `docs/unknowns.md`.

### Decisões Técnicas Chave

- **Decomp funcional primeiro, matching onde importa:** o objetivo é o port; matching é verificação. Alternativa descartada: matching 100% antes de portar (TF5 tem 15 funções após muito trabalho).
- **Converter PRX em ELF estático em vez de ensinar relocação às ferramentas:** o kernel da PSP faz exatamente isso ao carregar; um script de 100 linhas destrava REA, Ghidra e splat de uma vez. Base `0x08804000` é o endereço padrão de carga de user modules.
- **Comparar builds contra `build/elf/*.elf`, não contra o PRX:** depois da relocação os bytes de `.text` mudam; o alvo do matching é o ELF relocado.
- **Símbolos no repo, não no Ghidra:** sessões REA/Ghidra são efêmeras. `symbols/*.txt` é o que sobrevive e o que o splat consome.
- **Camada de plataforma dupla (psp + pc):** manter o build PSP rodando no PPSSPP é o único teste de regressão fiel da lógica reconstruída.
- **Nada do jogo no repositório:** ISO, extração e binários gerados são gitignored. O usuário fornece a própria cópia.

### Integrações Externas

Nenhuma em runtime. Ferramentas externas: REA (MCP local), Ghidra (headless, disparado pelo REA), decomp.me (manual, navegador).

---

## Convenções para agentes

- Idioma: documentação e commits em pt-BR; identificadores de código em inglês.
- Antes de decompilar uma função, verificar se já existe em `symbols/` ou `src/`. Antes de escrever um parser, verificar se o repositório do TF5 já tem um equivalente.
- Ferramentas em Python só com stdlib (exceção: `pycdlib`). Sem frameworks.
- Cada ferramenta nova em `tools/` traz um sanity check mínimo executável.
- Commits pequenos por fase/entregável. Nunca commitar `build/`, `extracted/` ou `*.iso`.
- Ao concluir uma fase ou mudar uma decisão desta lista, atualizar este arquivo (tabela de fases e seção de decisões).
