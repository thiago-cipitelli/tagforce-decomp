# Matching das funções em `@asm` — instruções de trabalho

Desde 2026-10-30 todo o `.text` de todos os módulos está em C. O que falta para a fidelidade são as funções
mantidas como `// @asm A-B`, com o C equivalente sob `#ifdef NON_MATCHING`. Este arquivo é o ponto de partida
de cada sessão dessa etapa: leia só ele (e o trecho de `docs/build.md` que ele indicar), não o repositório inteiro.

Estado em 2026-11-07, após a rodada 8 (funções grandes e arquivos com mais asm; ByteMatch): 76,0% do
código batendo, 95,0% fuzzy, 10099/11039 funções (libduel_eng 77,3%/97,2%, libduel_draw 64,6%/94,3%). Fila: 574
funções em `docs/matching_queue.tsv` (69 abaixo de 80%, nenhuma em 0%: toda função tem
C que compila), fora o SDK do BOOT (inclusive arquivos com um só `@asm` cobrindo
o `@range` inteiro), as de `docs/matching_skip.tsv` e as marcadas `matched` em `symbols/`.
A Tarefa 0 (drafts que não compilavam) está concluída. **libfield não tem mais função sem C** (76,2% código, 98,4% fuzzy).

**Atenção ao promover um `switch` denso:** a tabela de salto passa a sair da `.rodata` do objeto e a original
continua no asm de dados, o que desloca o módulo inteiro (`check_file` não vê; só o `make check` do módulo). Declarar
`// @rodata A-B` com o endereço da tabela (o `lui/addiu` antes do `jr`), como em `dd_event_cmd.c` (`dd_ev_04`).

**Todo `.text` com C (2026-11-04):** das 18 linhas com fuzzy 0% da fila, só 5 não tinham C (`duel_088C4864`,
`duel_088C5510`, `duel_act_0887E140`, `shop_glow_super`, `shop_glow_ultra`); agora todas têm, e `duel_088C5510`,
`duel_act_0887E140` e os três `*_save_panes`/`ds_set_panes` batem. As outras 13 têm C e o objdiff só não dá fuzzy
para elas (o report omite o campo quando a semelhança é baixa, ~25% de palavras diferentes): são matching comum.

## Protocolo de sessão (janela de tokens pequena)

1. **Escolha um lote** de 3 a 8 funções, de preferência do mesmo arquivo `.c` (um só build e um só commit):
   `python3 tools/asm_queue.py --next 15` (ou `--next 15 libstory` para um módulo). A fila vem ordenada da mais
   fácil (fuzzy alto, função pequena) para a mais difícil.
2. **Para cada função**, faça o laço abaixo. Limite: **4 reescritas reais** por função. Se não bater, siga para a
   próxima; se a causa for uma das "Não-corrigíveis", acrescente-a a `docs/matching_skip.tsv`.
3. **Feche o lote**: gate, commit, e pare. Não comece outro lote numa sessão já longa: é melhor uma sessão nova.

Economia de contexto (importa mais que tudo):
- Nunca `cat` de um `.c` ou `.s` inteiro. Ache a função com `grep -n "^nome\|@asm 0xADDR" src/<m>/<f>.c` e leia
  só aquele trecho (`Read` com offset/limit).
- Use `tools/asm_try.sh` (só as palavras diferentes) em vez de `mwcc_check.py` ou `func_diff.sh` completos.
  `func_diff.sh` só com limite de linhas, e só quando precisar ver o contexto de uma diferença.
- `make MODULE=<m> verify` é caro (roda o check-all de 23 módulos): **uma vez por lote**, no fim, filtrado por
  `grep "ALL MATCH\|BAD\|check-all"`. Durante o laço, use só `asm_try.sh`.
- Não rode `objdiff_gen.py`/`bytematch.py` a cada lote (minutos e saída grande): uma vez por dia ou ao fim de um
  módulo, para regenerar a fila (`python3 tools/asm_queue.py`).
- Prefira o modelo mais forte disponível. Medido em 2026-10-22: Opus fez 29 funções batendo onde Sonnet fez 0
  (Sonnet ajusta detalhes; o que resolve é reescrever a forma do C a partir do asm).

## Laço por função

```sh
M=libstory; F=st_chr_pick                       # módulo e função (da fila)
MODULE=$M tools/asm_try.sh $F                   # palavras diferentes + "N differing words"
MODULE=$M tools/raw_asm.sh 0xLO 0xHI            # o asm original (a verdade), quando precisar
```

1. Rode `asm_try.sh`. Ele compila o arquivo com `-DNON_MATCHING` e compara a função com o original
   (operandos relocados mascarados), sem editar nada.
2. Leia a diferença e o trecho do asm original correspondente; classifique (tabela abaixo) e reescreva a
   **forma** do C sob `NON_MATCHING`. Repita o passo 1.
3. Quando der `0 differing words`, promova a função:
   - apague a linha `// @asm 0xA-0xB` da função e o `#ifdef NON_MATCHING` / `#endif` em volta do C dela;
   - se o C emite strings, templates locais ou jump tables, o `// @data`/`// @rodata` do arquivo precisa cobrir
     esses dados (e um global definido só para o asm deixa de existir);
   - em `symbols/<m>.txt`, troque o status da linha para `matched src/<m>/<f>.c`
     (`MODULE=<m> python3 tools/sym_update.py arquivo` com `0xADDR nome type:func matched src/<m>/<f>.c`);
   - `python3 tools/check_file.py <m> src/<m>/<f>.c` deve dar OK para a função.
   - tire a função da fila: `sed -i "/\t$F\t/d" docs/matching_queue.tsv` (vai no mesmo commit).
   Antes de promover: compile sem `-DNON_MATCHING` também (o `asm_try.sh` não vê protótipos que só existem sob
   `NON_MATCHING`), e mantenha as funções em ordem de endereço no arquivo. Uma função cuja jump table vem depois das
   de uma função ainda em asm do mesmo `.c` não pode ser promovida mesmo com 0 palavras.
   `asm_try.sh` e `check_file.py` mascaram os operandos relocados: stores em campos do mesmo global em ordem
   trocada (`sw a0,%lo(g+0x0C)` contra `%lo(g+0x08)`) dão 0 palavras e só o `make check` acusa (`fbm_setup`,
   2026-11-03). Rode `make MODULE=<m> check` depois de promover.
   Verificação barata de lógica: compare a sequência de alvos `jal` do original com as relocações `R_MIPS_26` do
   objeto `-DNON_MATCHING` (achou chamadas trocadas e funções inexistentes em vários drafts).
   Draft com helpers `static` que o original expande inline: compile uma cópia com `static inline` e `-inline all`
   para comparar instrução por instrução (assim apareceram os bugs das cartas 0x18F1 e 0x1A0C em `battle_calc.c`).
   Contagem de constantes original × draft acusa campo ou global errado; compare também as constantes float
   (`lui/ori`: achou -400 por -800 e constantes erradas por 1 bit) e definições `void` cujo retorno os chamadores usam.
   `check_file` mascara relocações: depois de promover, só o `make check` vê dois globais carregados em ordem trocada. Pendência para o port: em código que já
   bate há protótipos locais com número de argumentos diferente da definição (`dd_mode_de48`, `dd_lp_load`,
   `func_08820E58`, `dd_field_camera0`, `dd_fx_start`/`dd_fx_stop`/`dd_fx3_*`, `dd_mon_on_*`, `duel_slot_ef098`,
   `duel_field_card_deb2c`, `duel_place_monster` com `pos` int no chamador e `unsigned short` na definição, `dl_tex_offset` com
   `(list, u, v)` nos gmodules e `(u, v, list)` no BOOT, `dk_list_find` redeclarado em bloco com u16 em `dk_deck_load`,
   `*_list_filter` declarados `void` em dt_fmenu/dt_sort/dk_sortmenu, `func_08820E58`/`dd_fx_start`/`dd_fx_stop` com
   argumentos a mais nos chamadores, `dd_popup_now` que deveria devolver `DdTask *`): o PSP tolera, o PC não; conferir cada um contra o asm antes do port.
4. Se não bater em 4 reescritas: atualize o comentário do `NON_MATCHING` (pt-BR, o que ainda difere, quantas
   palavras) e siga. Isso poupa a próxima sessão.

Permutadores (quando a forma já está certa e só registradores ou posições de pilha diferem):
- `MODULE=<m> python3 tools/perm_stmts.py src/<m>/<f>.c <func> LINHA N`: tenta todas as ordens de N linhas
  consecutivas (até 6) e deixa no arquivo a primeira que bate. É o primeiro a tentar: a **ordem das instruções**
  decide registradores e posições de pilha mais vezes que a das declarações (`fmoney_number`, 2026-10-30: 24
  ordens das 4 atribuições de um `case`, a 21ª bateu).
- `tools/perm_decls.py` (todas as ordens de declarações num só TU; `--alts` para variantes de corpo) e
  `MODULE=<m> python3 tools/perm_locals.py <f> <func> --apply` (lento: no máximo 5 declarações).

## Diagnóstico: diferença → o que mudar no C

O catálogo completo está em `docs/build.md`, seção "O que o mwccpsp faz com o fonte" (leia só a parte que casar).
Os casos mais comuns nesta etapa:

| No asm original | Forma do C que reproduz |
|---|---|
| `beq x,K → corpo; b senão` num teste único | `switch (x) { case K: ...; break; default: ...; }` (não `if`) |
| testes de um `switch` | ordem dos testes = inversa da ordem dos `case`; corpos na ordem do fonte; `case A: default:` dá o `li` morto |
| corpo do `default` que recarrega as constantes dos testes (`li v1,3` em vez de reusar o `a1` do `beq`) | `case 0: default:` em vez de só `default:` (`fld_bg_doors_update`) |
| cadeia de `if` longa | era um `switch`; o primeiro teste de cada grupo vira `beql` |
| `$s` trocados entre duas variáveis | ordem das **instruções** (não das declarações) decide; reutilizar uma variável antiga para um valor novo; separar uma variável usada em dois ramos |
| campos carregados para fora do laço em posições de pilha trocadas | ordem das atribuições no primeiro uso (`perm_stmts.py`) |
| valor calculado antes da chamada no original | atribua o temporário depois da chamada, em vez de inicializar na declaração |
| global recarregado por `lui` vs base em `$s` | `T *const p = &g;` declarado primeiro; ou primeiro acesso pelo global e o resto por `p` |
| base `lui`+`addiu` num `$s` só a partir de um store, os stores anteriores absolutos | `T *p;` sem inicializar e `(p = &g)->campo = v;` no primeiro store pela base (`fbm_cmd_open`, `fmsg_set_text`); `p = &g; p->campo = v;` em duas linhas não serve |
| `slti at` vs `slti v0` | `a > b`/`a <= b` dão `$at`; `b < a`/`a >= b` dão temporário; `x < 0` → `bgez`, `x <= -1` → `slti at` |
| `andi 0xFFFF`/`0xFF` a mais ou a menos | tipo do parâmetro (`unsigned short`/`char`); cast `(int)` num bitfield de 1 bit; protótipo ausente no original (`int f();`) |
| `seh` a mais | `(short)(int)expr` |
| ordem dos `mtc1`/`a0` numa chamada | ordem dos parâmetros no protótipo (int antes de float, ou ao contrário) |
| `mul.s fd, var, const` (constante no segundo operando) | o mwcc põe a constante literal sempre em `fs`; com ela num local (`float k = 38.0f; x * k`) sai na ordem do original (`fld_hud_move_draw`); um `mult` inteiro com o resultado trocado se resolve com o produto parcial num `int` local |
| cópia de struct pela pilha que o mwcc propaga | bloco com `T tmp = {...}; x = *(T *)&tmp;` |
| local `Mat4`/array acima dos outros na pilha | `__attribute__((aligned(16)))` **na variável**, não no typedef |
| laço com teste no topo e `b` de volta | `for (;;) { if (!c) break; ... }` (o `while` é rotacionado) |
| zerar bytes com `beqz p; li n; sb/addiu/bnez` | `if (p) do { *d++ = 0; } while (--n);` |
| campo carregado depois da aritmética do índice | `(T *)((char *)base + i * sizeof(T))` |
| `sltu v0,zero,v0` logo após `jal` | `int on = f() != 0; if (on)` |
| faixas de `case` em árvore de `slti` | `case A ... B:` (o mwcc aceita) |
| blocos mortos `li; b; nop` depois de uma cadeia de testes `slti`/`beql` | é um `switch` com faixas: os testes saem na ordem inversa dos `case`, os corpos ficam como blocos mortos na ordem do fonte (`fld_chr_kind`, `fld_in_range`: `switch (x) { case 100 ... 449: return 1; } return 0;`) |
| corpos de `case` mortos que o mwcc remove (`li a1,K; b; nop` depois dos `beql`) | todo `case` atribui um **local** (`int n = id; switch (id) { case A: n = X; break; … case B: n = id; break; }`), não o parâmetro; um `case` que só faz `break` precisa de `n = id;` explícito (`duel_088B52F0`); com valores iguais à constante do teste, `switch` em vez de cadeia de `if` (`op_deck_select`: ordem dos `case` por busca) |
| cópia de tabela para a pilha (laço `lb/lh` ou `lwc1/swc1`) | `T loc = *(T *)tabela_global;` ou local inicializado `short pos[4] = {...}` (o template fica no `@data`) |
| `sll/ext` relendo o valor recém-gravado | atribuição encadeada `m->a = m->b = 0;` |
| `ext x,16,16` / `srl` em vez de `sra` | `(pos >> 16) & 0xFFFF` / `(unsigned int)u16 >> 1` |
| parâmetro passado adiante sem `andi`, mas mascarado no índice | parâmetro `int` (ou K&R), índice `[(unsigned short)p]` |
| base em `&campo - 4` (`addiu s,base,16`, campos em `+4`) | struct de visão começando naquele campo |
| `mflo; bgezl; addiu +1` depois de `div` | está no fonte: `q = a / b; if (q < 0) q++;` |
| locais declarados dentro do laço em posições de pilha erradas | subir para o escopo da função, na ordem da pilha |
| `static` no helper do mesmo arquivo | o mwcc mantém registradores `$t`/`$a` vivos através do `jal` (`move t0,a0` no delay slot) |
| corpo do `default` logo depois dos testes | `default:` como primeiro rótulo do `switch` |
| `b L; nop` para um `return K` comum | os caminhos caem no `return` comum em vez de ter o seu próprio |
| ponteiro de global carregado só depois de uma chamada | atribua `s = &g;` logo antes do uso, não na declaração |
| campo relido depois de uma comparação | `((volatile T *)p)->campo` |
| `MAX(x, f())` num draft | o original chama `f()` uma vez só: bug de lógica, não forma |
| um teste que falha vai direto ao epílogo e os outros fazem `beqz; nop` para um bloco de retorno comum | tire os `return K` internos: todas as falhas caem no `return K` final |
| `bnezl` para o corpo e `b` epílogo no outro caminho / `beqzl` direto ao epílogo | `if (!c) return x;` antes do corpo / corpo dentro de `if (c) { ... }` |
| tabela de funções chamada por índice | `T *t = tabela;` testando pelo global e chamando por `t[x].f()` |
| laço que não é rotacionado (`beqzl saída; ...; b topo`) | `switch ((int)p) { case 0: goto out; }` como teste de saída |
| funções gêmeas entre módulos (dk_/ds_/dt_/sc_, tut_text_*/dt_text_*) | mesmo tamanho e asm igual a menos de endereços: porte a forma que já bate, com protótipos e tipos auxiliares |
| `$s` dos campos de requisição (p, arg, loc, w) trocados | ordem das declarações e `int`×`unsigned`: busca exaustiva de permutações (até 6 declarações) resolveu a maioria das `duel_req_*` |
| `DuelCard`/`DuelActX` de 4 bytes zerado na pilha (laço de bytes) | `{ int n = 4; unsigned char *d = (unsigned char *)&x; if (d) do { *d++ = 0; } while (--n); }`; `= {0}` não reproduz |
| campo de largura diferente da struct compartilhada | struct de visão local com o bitfield na largura que o original lê (`DuelListCard.placed` de 2 bits) |
| bitfield de local gravado com `lhu/and/or/sh` (o mwcc dá `lbu/sb`) | inicializador com valores `DdLoc l = { p, zone };` (seção "Bitfield de halfword" abaixo) |
| if/else-if em `a->id` com testes na ordem inversa | era `switch` linear |
| global relido depois de um store nele (sem encaminhamento) | declare como array `extern T g[]` e use `g[0]`, ou `*(volatile T *)&g` |
| `n = 2; if (c) n = 4;` puxa o próximo comando para o delay slot de um `bnel` | ternário `n = c ? 4 : 2;` |
| `li x,-1; addiu s,x,1` antes de um laço contado | `i = -1; while (++i < n)` |
| troca Fisher–Yates com `$s` rodados | temporário próprio da troca (`k = z[i]; z[i] = z[j]; z[j] = k;`), não o `t` do laço |
| switch inteiro preenchendo uma struct (um slot de pilha por case) | `{ T t = {...}; *p = t; }` em cada case |
| permutar comandos rápido | `perm_decls.py --alts` com `{DECLS}` no lugar dos comandos: 720 ordens num TU em segundos |
| argumento de chamada vindo de um registrador com uma constante de comparação | o argumento real **é** essa constante (bug de lógica comum nos drafts) |
| `switch` linear (`li; beq(l)` em cadeia): testes na ordem **inversa** dos `case`, corpos na ordem da fonte | escrever os rótulos na ordem inversa dos testes; alvos a 4 bytes um do outro são o mesmo corpo (o `beql` copia a 1ª instrução) |
| bitfields de um evento relidos da pilha depois de cada chamada | o endereço escapa: passar `*(unsigned int *)&ev` (protótipo com `unsigned int`) |
| mesma função chamada com struct e com `0` no 3º argumento | redeclarar no escopo do bloco: `{ extern int f(unsigned int, unsigned short, unsigned int); ... }` (mwcc aceita) |
| `blez; sw; move zero; sw` / `bgez; nop; move zero; negu` | `if (n > 0) n = 0;` com local; `-(n < 0 ? 0 : n)` |
| `x * -K` em shifts contra `mult` | shifts = multiplicação com sinal: `(int)` no campo `unsigned`; `mult` = campo realmente `unsigned` |
| `n = cond ? 1 : x` com o `1` vindo de um registrador de comparação anterior | `n = cond == 0 ? x : 1` |
| chamada com menos argumentos que a definição (protótipo K&R `f();`; lista: `tools/argaudit.py`) | ver no original o registrador do argumento logo antes do `jal`: se for carregado (muitas vezes no teste anterior: `li a0,1` de um `switch`, a global comparada em `a2`), o argumento existe (`dd_ev_61`, `cardinfo_update`, `duel_088A7C88`, `duel_ai_0881D0D8`); se não for, o original também chama com menos |
| função de 1 parâmetro cujo callee recebe `a1` sem que ninguém o carregue | a função tem 2 parâmetros e repassa o segundo (`duel_088A3CB4(a, b)`) |
| `lhu x,OFF(base)` com o deslocamento do campo fora da relocação | ponteiro para o elemento: `DsList *l = &d->list[tab]; return l->count;` |
| `v0` já vale a constante do teste, mas o original salta para um `li v0,K` próprio | agrupar os `case` com o mesmo corpo (`case 1: case 3:`) |
| `return` comum depois do `if/else` vira `li v0,1` em cada ramo | um `return 1;` em cada ramo |
| `lui` de uma global em `.bss` **antes** do último `sw $s/ra` do prólogo (só o prólogo difere) | a global era `static` **da função** (só ela a usa): `static int sorted;` dentro da função e `// @bss A-B` no cabeçalho (ver abaixo); `static`/global de arquivo não reproduz |
| `b fim; move v0,zero` e depois `beqz/bnez; nop` para o mesmo `move v0,zero` (o mwcc faz `b??l` + cópia no 1º) | `default: break;` (ou `goto` ao `return` comum) em vez de `default: return 0;`: a cópia vai para o `b` |
| cópia morta do 1º comando do `else` depois do `b` do `then` | `else { if (...) {...} return r; } return r;` em vez de `else if` |
| vértices com `lui K; sw` e `mtc1 K; neg.s` do mesmo valor (dd_fx_*) | raio num local: `float r = 75.0f; ... -r, r` (literal `-75.0f` e parâmetro de inline são dobrados) |
| `lui/lw rand3_slot` repetido a cada sorteio | macro que reatribui `r = &rand3_seeds[rand3_slot]` em cada `DD_RAND` |
| `andi; sltu; xori; beq x,1` no teste de um bit | `if (!(p->flags & 1) != 1) { ... }` |
| contador e ponteiro de laço com `$s` trocados | um contador próprio por laço (o do laço interno declarado no bloco do externo); dois laços seguidos podem dividir o mesmo |
| laço de bytes zerando um vetor local dentro de laço sem chamadas, sem ser içado | `float p[3] = { x, 0.0f, z };` (o idioma `do { *d++ = 0; }` é içado) |
| ordem dos vetores na pilha | locais da função de cima para baixo na ordem da declaração, locais de bloco abaixo deles; `aligned(16)` manda o vetor para o fundo (`dd_fx_twist`) |
| `mul.s` com a constante içada em `ft` (segundo operando) | constante num local (`float du = 0.25f; x * du`) |
| `beqzl i; sw A` / `bnel i,n-1; sw B` / `sw A` | `if (i == 0 \|\| i == n - 1) A; else B;` |
| `base + 8` num `$s` e campos a partir dele | ponteiro para uma sub-struct (`DdRingTex *q = &r->p`, `dd_fx_bolt`) |
| campos de um array local içados para a pilha antes de um laço externo (`lh` de `spr[0].w` no preheader), mas no draft calculados dentro do laço | outro laço do mesmo array com índice afim (`spr[17 + i]`) é reduzido a ponteiro pelo mwcc e mata o içamento na função inteira; o original indexa sem redução (`addiu; sll; addu sp` a cada volta): `spr[(unsigned int)(17 + i)]`, vizinhos como `spr[(unsigned int)(17 + i) - 1]` (`tcfg_draw`) |
| locais `short` inicializados no topo (`li s3,131` etc.) que só são lidos depois de `switch` sem `default` | inicializar na declaração (`short x = 131;`): o caminho sem `case` mantém o valor vivo e muda a pressão de registradores (`tcfg_draw`) |
| ordem dos operandos de um `mult` (`mult v1,t0` com o resto da divisão primeiro) | o fator num `int` local próprio: `{ int col = i % m->count; cx = x + col * m->item_w; }` (`*_draw_fmenu_values`) |
| bitfield decrementado com `addiu x,255` | `f->page--;` (não `f->page = old - 1`) e o `old` u16 lido depois do primeiro teste (`*_fmenu_values`) |

## `static` em `.bss`: `// @bss 0xA-0xB`

Sinal: o original carrega o endereço (`lui`) de uma global de `.bss` antes de terminar de salvar os
registradores no prólogo (`lui v0; sw ra; sw s0` contra `sw ra; sw s0; lui v0`). Só uma variável `static`
**de função** produz isso no mwcc; global, `extern` e `static` de arquivo dão a ordem normal. Conferir que
só aquela função usa a variável (`grep` em `src/` e `asm/`), declarar a variável dentro da função e pôr no
cabeçalho do `.c` a faixa do `.bss` do objeto: `// @bss 0x08C5B3C0-0x08C5B438`. `splat_config.py` divide o
`.bss` em volta dela (segmento `.bss` com o nome do arquivo) e o objeto do C ocupa aquele endereço.

Ordem dos objetos no `.bss` de um `.c` (mwcc): `static` de função e definições com inicializador zero
(`int x = 0;`, `T s = { 0 };`) saem no ponto em que aparecem no fonte; definições sem inicializador
(`int x;`, `static` ou não) saem no **fim** do objeto, em ordem **inversa** à do fonte (`dd_scene_mode.c`:
quatro `= 0` no topo, o `static` de `dd_text`, e quatro tentativas em ordem inversa). Um `static` de
arquivo usado só por uma função pode virar `static` dessa função para cair na posição certa
(`dd_movie_buf` em `dd_movie_preload`).

Cuidados: a faixa `@bss` tem de cobrir exatamente o que o objeto define (o resto continua `extern`, definido
pelo asm de `.bss`); uma função em `@asm` não pode referenciar um símbolo dentro da faixa (o asm deixa de
defini-lo); todas as funções com `static` do arquivo saem do asm juntas (o C sob `NON_MATCHING` não emite
o `static`, e o buraco desloca o resto do `.bss`); tirar do asm uma função com `switch` denso exige a
`@rodata`/`@data` da tabela (`dd_event_dispatch`: `@rodata 0x08870074-0x088700A8`). Feito em
`dd_chr_models*.c` (36 buscas de ossos), `dd_event.c`, `dd_chara06.c`, `dd_disc_hook.c`, `dd_scene_mode.c`,
`dd_mon_anim.c`, `f_sys.c` (`fld_sys_term`, 5 flags) e `movie.c` (`movie_poll_input`, `pad_prev/pad_cur`).

### Zona antes do jogador (`duel.players[p & 1].field[z]`)

Sintoma: o original extrai a zona (`ext …,8,8` ou `srl`) e/ou calcula `z * 20` (`sll; addu; sll`) antes do
`andi p,1; mult p,4012`; o C comum sai com o jogador primeiro. Observado (rodada 7): na aritmética de ponteiro o mwcc
põe a base `&duel + p * 4012` primeiro, e um jogador em local definido antes (ou bitfield que vira `ext` fundido com
o `& 1`) também sai antes; o original tinha a zona primeiro por uma de duas formas:

1. **Origem num `unsigned short` (`src`, `loc`, `t`): soma inteira com a zona à esquerda.** Macro `ZSLOT(p, z)` em
   `duel.h`: `((DuelPlayer *)(z * sizeof(DuelSlot) + (unsigned int)&duel.players[p & 1]))->field`, com `sz`/`sp` em
   locais `unsigned int` (`sz = src >> 8 & 0xFF; sp = src & 0xFF;`). Aritmética de ponteiro (`&…field[z]`,
   `(char *)… + z * 20`) é recanonizada com a base primeiro e não serve. Com dois usos da mesma zona, `ZSLOT` nos dois
   dá o endereço inteiro em CSE (bate em `duel_088C2BA0`); se o original recalcula base+jogador no 2º uso, não use.
   Bateu: `duel_slot_count_14c0`, `duel_effect_set`, `duel_08887618`, `duel_eff_fa288`, `duel_088C2BA0` (este com
   `tz = (unsigned int)(v & 0xFFFF) >> 8 & 0xFF;` numa só atribuição: duas atribuições a `tz` desfazem o efeito).
2. **Jogador de bitfield do `DuelAct` (`a->player`, `a->src.player`): sem local para o jogador no endereço.**
   `duel.players[(unsigned int)a->src.player & 1].field[z]` em cada uso (o cast evita um `andi 0xff` a mais) e
   `a->src.player` direto na chamada. Bateu: `duel_08870848`, `duel_eff_fbac8` (este com `return 0;` depois do
   `switch` interno para o `b fim; move v0,zero`).

3. **Deslocamento do campo dobrado na base** (`lui; addiu duel+0x38; addu p*4012; addu z*20; lhu 0(…)`): o
   original soma a constante do campo ao endereço do símbolo. `*(unsigned short *)(y * 20 + ((q & 1) *
   sizeof(DuelPlayer) + (unsigned int)&duel.players[0].field[0].faceup))` (`duel_0888FC28`, que também precisou
   de `p`/`z` locais do bloco do `if` no passo 0x7F para os `$s`). `ZSLOT` dá a ordem certa mas o `lhu 56(…)`.
   Bitfield do `DuelAct` com o jogador num local `unsigned char` (`duel_088862DC`, `duel_08884118`): locais
   `z`/`p` u8 do bloco, ou `a->player` direto no índice e `p = a->player` só depois do teste.

Inverso (`duel_slot_src_attr8`): ali o original tem o jogador primeiro; `(unsigned char)src & 1` e
`(unsigned char)(src >> 8)` direto no índice puxam a zona para cima; locais `sz`/`sp` resolvem.
Não resolvido: `duel_0888BA00` (o original lê `a->uid` antes do índice: um local `uid` é propagado de volta; 22
palavras), `duel_0888B9A8`/`duel_0888BBE4` (ordem de carga dos argumentos: `a->id` e `duel_turn_player` antes de
`1 - a->player`; só uma atribuição dentro do argumento, `f(q, …, (t == (q = 1 - a->player)) …)`, bate, e isso é
comportamento indefinido em C: não usar).

## Desvios likely: quem recebe a cópia (pesquisa da rodada 7)

O preenchimento de delay slot é do escalonador (`#pragma schedule off` some com todos). Regras observadas no
mwcc 219, que também explicam o original:

- **Uma cópia por destino.** A 1ª instrução de um bloco é copiada para o delay slot de **um só** desvio que
  chega nele: o primeiro na ordem do código (um `b` incondicional conta). Os outros desvios para o mesmo bloco
  ficam `bXX; nop`. Quando o original tem `b fim; move v0,zero` antes e os testes seguintes com `nop`, o `b` é
  o salto do fonte para o `return` comum: `switch` com `default: break;` (e não `default: return 0;`) caindo
  no `return 0` final, ou `goto` para o rótulo do `return` comum. Bateram assim `st_cond_daytime`,
  `st_cond_weekday`, `st_cond_day`, `st_cond_date` (libstory) e, em teste, `duel_08870DC4`/`duel_0887AD38`
  (`default: goto none;` + `none: return 0;`).
- **Cópia (fica a original) × movida (some do destino).** A cópia morta depois do `b` aparece quando o bloco
  destino do likely termina no seu próprio `return`: `if (a) {...} else { if (b) {...} return r; } return r;`
  em vez de `else if` (`sdate_is_daytime`). Com `else if` ou `goto` para o fim, o mwcc move a instrução.
  O `||` seguido de `return` (`if (a || b) return; ...`) também deixa a cópia.
- **`case` separados que voltam a se juntar:** `duel_08865ABC` tinha `case 0xB: case 0xC: case 0x10: return
  0x800;` (o `beql` do 0x10 leva a cópia, os de 0xB/0xC ficam `beq; nop`) e `default: goto none;` no 1º
  `switch`; `card_id_remap_1a6c`, dois `case …: break;` para o mesmo `return 0`. `snd_fx_setup`: `return r`
  dentro do `if` e `r = snd_fx_type` lido **antes** das gravações (o load sai depois de qualquer jeito, mas a
  coloração muda).
- **Cabeça de laço:** no original a cópia da 1ª instrução da cabeça é do desvio de volta do laço (`bnel` para
  cabeça+4, `dd_cobj_find`) ou de ninguém (o de volta já tem o delay preenchido: `st_scr_find_es`,
  `st_pl_card_bit_set`); o desvio de entrada fica `bnez; nop`. O 219 dá a cópia ao primeiro na ordem do código,
  a entrada. Nenhuma forma (while/for/do/goto, laço antes da entrada no texto, `opt_*`) reproduziu: pule.
- **`#pragma no_branch_likely on`** (não documentado; `#pragma no_branch_likely reset` depois da função)
  desliga todos os likely da função, inclusive os de laço. Só serve para função sem nenhum likely no original;
  o build 121 se comporta igual a ele (e tem frame de pilha diferente). Os outros pragmas do backend
  (`peephole`, `debug_1..5`, `late_conditional_move`, `opt_*`, níveis `-O`) não mudam essa escolha.

## Bitfield de halfword gravado com lhu/and/or/sh (inicializador com valores)

O mwcc 219 estreita **toda** gravação ou leitura de bitfield que cabe num byte para `lbu`/`sb` (inclusive com
`volatile`, `union` com u16, tipo base `int`/`short`/`char`/enum, `#pragma pack`/`options align`, e com qualquer
uma das ~1900 pragmas do binário ligada ou desligada; até em `-O0`: é a geração do front-end). O build 121 nunca
estreita, mas não serve no resto. Onde o original grava um local com `lhu; li -2/-63/…; and; or; sh` por campo,
o fonte era um **inicializador agregado com valores de tempo de execução**: o mwcc zera o local (laço de bytes,
se faltar campo), copia o template constante (se houver) e insere cada campo com RMW de meia palavra, sem
estreitar, com os registradores do original.

| No asm original | Fonte |
|---|---|
| laço `sb zero` de 2 bytes + RMW `lhu/sh` por campo (`player`, `zone`, …) | `DdLoc l = { p, zone };` (campos omitidos = 0) |
| idem com um campo do meio zerado | `DdPos at = { p, zone, 0, o->b5 };` |
| cópia `lwc1/swc1` de um template + RMW `lhu/and -256/or/sh` + `sb` de uma constante depois | `UiCounterB c[3] = { {0, {0}, 0, 0}, {5, {0}, 0xFF000000, it->partners}, … };` e `c[2].value = 99;` (a gravação avulsa estreita) |
| RMW sem zerar antes (lê o local não inicializado) | inicializador com todos os campos |

Bateram assim `popup_place`, `dd_hl_push`, `dd_hl_pop` (`DdLoc z = { 0 };`), `dd_cobj_flip` e
`func_08822E30` (este também com `__attribute__((aligned(16)))` nos `float[4]`). Máscaras explícitas
(`*(u16 *)&l = (*(u16 *)&l & -2) | (p & 1)`) também dão `lhu/sh`, mas o otimizador as trata como aritmética
comum (CSE entre campos, outra coloração): não use. Família ainda em asm com esse padrão (RMW de `lhu` em local
da pilha no original): `dd_cobj_goto`, `dd_cobj_to_top` (inicializador completo de `DdLoc` + campo de 2 bits
gravado à parte), `dd_mark_move`, `dd_ev_11`, `dd_board_piles`, `dd_board_cards`, `dd_board_card_draw`,
`debug_menu_update`, `shop_list_row` (o trecho dos contadores já bate) e várias `duel_req_*` de libduel_eng.

**Cópias de struct (`dt_draw_hint2/3`)**: o mwcc 219 decompõe toda cópia de struct campo a campo (com RMW de
`lhu` se o tipo tiver bitfields); o original decompõe as primeiras e copia a última em bloco (`lwc1` + `lh`).
`#pragma opt_decomposeaggregates off` dá a cópia em bloco, mas para a função inteira. Não reproduzido.

## Funções grandes (rodada 7): forma por seção

Para funções de 2 a 7 KB com centenas de palavras diferentes, a contagem do `asm_try.sh` não serve de guia (um
deslocamento de uma instrução muda tudo depois). O que funcionou em `ai_08839360.c` e `ai_08815E90.c`:
- **Diff estrutural**: desmontar original e compilado, trocar registradores por `R`, deslocamentos de memória por
  `N`, imediatos de `lui`/`addiu` relocados e alvos de desvio por constantes, e comparar com `difflib`
  (`SequenceMatcher(autojunk=False)`: o padrão descarta linhas repetidas e conta errado em funções grandes). Resolver
  primeiro a estrutura (0 linhas), depois os registradores.
- **Variantes num só TU**: o corpo da função com substituições `velho → novo` (várias por variante), um `f_N` por
  variante, uma compilação, ranqueadas pelo diff estrutural e pelo de palavras. Dezenas de variantes por minuto.
- **Permutação de declarações em lotes** de 40 funções por TU: com 720 cópias de uma função de 7 KB o `.o` passa de
  20 MB e o `object_function` relê o arquivo a cada cópia (o `perm_decls.py` leva mais de 20 minutos).

Idiomas novos (cada um resolveu pelo menos um trecho):
- **Tipo do parâmetro no protótipo decide a ordem de carga dos argumentos**: passar um campo `int` a um parâmetro
  `unsigned int` faz o mwcc carregar o campo antes do global do argumento anterior; com o protótipo igual à definição
  (`duel_088C4448(..., int uid)`, `duel_slot_effect_with(..., int src)`, `duel_088D1438(..., int a, int b)`,
  `duel_088C77D4(p, unsigned short id, int a, unsigned short b)`) a ordem bate. Um `andi 0xFFFF` a mais no argumento
  também aponta o tipo do parâmetro (o 4º de `duel_088C87A8` recebe `(unsigned short)(...)`).
- **Uma teia de outro `case` muda a cor das seguintes**: `ev`/`r` trocados (`s4`/`s5`) no `case 1` de
  `duel_0883B4A8` se resolveram usando outra variável (`xv`) para o evento do `case 0`; em `duel_08839360`,
  `int id; int r; int v;` declarados no bloco de cada seção.
- **`(unsigned char)x | (unsigned char)g << 8`** em vez de `(x & 0xFF) | ...` inverte os temporários (`v0`/`v1`) do
  `or`; `e * 4` em vez de `e << 2` e `(unsigned int)(i & 1) << 31` em vez de `(unsigned int)i << 31` mudam a ordem
  de avaliação dos operandos de um `|` (a subárvore "mais pesada" sai primeiro).
- **Global relido depois de um join**: o original recarrega `duel_atk_tp/tz` num `switch` alcançado por dois
  caminhos; `*(volatile int *)&duel_special.unk24 = ...` no store intermediário impede o CSE (`duel_08839360`).
- **`if (c == 10) {...}` com `beql`+`b` no original**: `switch (k = ef->kind & 0xF) { case 10: case 11: ... }`;
  campo carregado antes do teste de `SLOT_ACTIVE` = `id = ...card.id;` antes do `if`; base `&duel.players[q]` com o
  `+48` dentro das cargas = expressão direta `duel.players[sp & 1].field[sz]` (sem ponteiro `slot`).
- **`!f()` em `if (!f()) continue;` materializa `sltu/xori/bnezl`**; `f() == 0` dá `beqz`. Ao contrário, dentro de um
  `for` com `return`, `if ((mask & 1 << b) != 0)` evita o `sltu/xori` que `if (mask & 1 << b)` gerou.
- **Contadores de laço trocados entre laços do mesmo `switch`**: trocar qual variável é o contador e qual é o bit
  (`for (b = 0, z = p << 4; b < 5; b++, z++)`), usar outra variável livre (`q`) num dos laços, e a ordem dos
  incrementos (`b++, z++`) decidem os `$s` de cada laço.
- **Inline do original**: `act_x_init` virou macro com o laço de bytes (`ACT_X_INIT`); com o helper `static` o
  código saía 556 bytes menor.

- **Variável nova para um laço de um `case`** (rodada 8, `duel_0883E63C`): máscara e ponteiro induzido trocando `$s`
  só num `case`, com o mesmo laço batendo noutro `case`: a máscara reaproveitava um local usado em outros `case`s
  (`i`). Um local novo de bloco (`{ unsigned int bits; ... }`) bateu; permutar declarações e renomear para outros
  locais existentes não muda nada (a interferência vem do compartilhamento, não da ordem).
- **Ordem dos operandos de um `|` com campo de bitfield** (rodada 8, `duel_0883BB00`): `or a0,resto,id` no original
  contra `or a0,id,resto` — o mwcc reordena o `|` sozinho (a ordem no fonte não muda nada); o que bateu foi reler o
  campo pela expressão completa `duel.players[sp & 1].field[sz].card.id` (CSE com o ponteiro `slot`) em vez de
  `slot->card.id`.
- Sem forma até agora: slot de pilha de dois locais derramados trocado (`ocard`/`oon` em `duel_0883BB00`: ordem,
  tipo, nome, `register` e inicialização antecipada não mudam) e `sll` de `i` com `i & 1` reaproveitado pelo CSE
  (`duel_0883D798`: cópias de `i`, `% 2`, `unsigned` e reassociação do `|` não quebram o CSE).

## Funções grandes de efeito (`dd_fx_*`, rodada 7)

Seis das funções de 2-4 KB de efeitos do libduel_draw bateram com as mesmas regras (`dd_fx_swirl_update`,
`dd_fx_beam_update`, `dd_fx_splash_update`, `dd_fx_rock_update`, `dd_fx_aura_circle`). Ordem que funcionou:
1. **Fidelidade primeiro**: sequência de `jal` contra as relocações e multiconjunto de imediatos (`lui/ori`, offsets de
   `lw/sw`) do original contra o compilado; achou `x = -6` escrito como `-3` e 14,4 = `0x41666667` (não `0x41666666`).
2. **Constantes em locais**: `neg.s` de uma constante e o `sw` do mesmo valor como inteiro (`-r`/`r` nos vértices de um
   quadrado), ou `mul.s`/`add.s` com a constante em `ft` (`cu + cw`, `x + cx`), são uma variável local (`float r = 20.0f;`).
   A constante literal vai sempre para `fs`; a local segue a ordem do fonte. `x * 1.0f` só sobrevive com o 1 numa local.
3. **Um contador por laço**: quando contador e ponteiro de indução trocam de `$s` só em alguns laços, cada laço tem o
   seu `int` (o original não reaproveita `i`/`j`). Busca automática: renomear o contador de cada laço para um nome de um
   conjunto e manter o que reduz as palavras (descida por coordenadas; achou 4 de 4 em `dd_fx_beam_update`). O mesmo
   vale para `list` e para o ponteiro da partícula num laço de desenho.
4. **Sub-struct onde o original guarda `base + 8`**: `addiu s0,p,8` usado para campos seguintes indica uma struct
   interna (`{flags, t, {cell, dir}}`); o acesso passa por um ponteiro para ela.
5. **Modelos locais** (`FxVec3 a = {...}`, tabelas de `float`) vão para `.data` (declare `// @data`); a posição da
   cópia na pilha diz o bloco onde o local foi declarado (`{ FxVec4 up = {...}; ... }`). Entre modelos, um `FxVec4`
   alinha em 16 (os 12 bytes zero em `0x08876DC4` são preenchimento).
6. Outros: `sinf(x) * 0.0f * cosf(y)` (o mwcc dobra o seno mas mantém a chamada), laço de 1 iteração
   (`for (j = 0; j < 1; j++) b = &w->beam[j];`) que iça `buf * 1224` para antes do teste, teste `!(f & 1) != 1`
   (`sltu/xori/beq 1`), `x % 16` em vez de `& 0xF` (`bgez`+`beqzl`), `if (on == 1)` com o `li v0,1` no delay slot.

### Rodada 8 (`dd_fx_fusion_update` batendo, `dd_fx_glow_update` a 1 instrução)
- **Diff estrutural com o tamanho real**: o `asm_try.sh` compara só `len(código compilado)` do original; com a função
  menor que o original o diff não mostra o que falta. Compare até o símbolo seguinte de `symbols/` e normalize os
  `%lo` relocados (`REL`) e os imports de variável do BOOT (`rand3_*`, que valem 0 no ELF).
- **Ordem de avaliação entre chamadas**: um valor calculado no original entre duas chamadas (no delay do `jal`) ou
  guardado num `$f20+` através de uma chamada é um comando separado no fonte: `alpha = ... << 24; spin = ...;
  dl_ambient_light(list, alpha | 0xFFFFFF)`; `k = ang; mat3_identity(); mat3_rot_z(m3, m3, k)`; `in = 0.8f * a255;`
  antes da expressão com `sinf` (reaproveitar uma variável da função; um local novo de uso único é propagado de volta).
- **`fade * (x * 255.0f)` contra `fade * x * 255.0f`**: a ordem dos `mul.s` diz a associatividade; `a | (b | K | c)`
  idem para `or`.
- **Base do struct em vez de ponteiro do elemento**: `sw x,1232+k(s)` com `s` andando 48 = `fx->ring[i][k].campo`
  direto (não `DdColVert *rv = fx->ring[i]`); o mesmo para `note_quad`, `mesh`. O inverso (`addiu s0,s2,8`) é ponteiro
  para uma sub-struct com nome (`DdFusionPart *q = &s->p[j]`).
- **Pilha**: um vetor local de uso só num bloco (`FxVec3 q`) fica com o bloco (cada bloco tem o seu `q`, embaixo dos
  arrays do bloco); `__attribute__((aligned(16)))` na matriz `mvp` a manda para o fundo e libera o resto.
- **`switch` com `default:` primeiro** para `if (s == 1) … else if (s == 0) … else …` com o corpo do `else` logo
  depois dos testes; `case 0: if (n == 1) r = 0; else r = 75;` em vez do ternário quando o `b` final leva o `mtc1`.
- **Contadores de laço: descida automática com nomes novos** (`n0..n7`, declarados no fim das declarações) resolveu
  13 de 14 trocas de `$s` em `dd_fx_fusion_update` (101 → 0) e reduziu `dd_fx_glow_update`. Só nomes novos: renomear
  para uma variável existente pode colidir com um laço externo ou com um uso depois do laço.
### Funções de efeito (rodada 8)

`dd_ripple_update` bateu e `dd_ring/halo/tunnel_update` e `dd_orb_draw` ficaram com a forma idêntica (só registradores):
- **Pilha**: `ScePspFMatrix4`/`ScePspFVector4` com `__attribute__((aligned(16)))` na variável e declarados na ordem
  de cima para baixo da pilha original; um vetor zerado por laço de bytes no meio da função é local da função zerado
  com `{ unsigned char *c = (unsigned char *)v; int n = 12; if (c) do { *c++ = 0; } while (--n); }` no ponto do uso
  (`= {0}` zera no prólogo). Cópia de modelo de `.data` = `off = dd_ripple_off;` de um global nomeado.
- **Teste de bit `andi; sltu; (xori;) beq/bne x,1`**: `#define DD_BIT(f, m) ((f) & (m) ? 1 : 0)` e
  `DD_BIT(x, 2) == 1` / `!DD_BIT(x, 1) == 1` / `DD_BIT(x, 1) == 0`.
- **u/v em temporários** (`tu = i % 4 * du; ... tu + du`), constantes `0.25f`, `10.0f` em locais, e o elemento do
  array por ponteiro (`f = &d->flame[i]; f->...`, `mt = &d->mote[i]`) quando o original guarda `base + off` num `$s`.
- **Sorteio de sinal inline**: `x * (DD_RAND(r) % 2 * 2 - 1)` no mesmo comando (não num `sign` à parte), e laços de
  índices (`idx_a/idx_b`) num `for` próprio depois do laço dos vértices (o mwcc desenrola só esse).
- Um contador por laço (`build/tmp/fxd/ctr.py` da worktree fxd faz a descida por coordenadas) resolveu dezenas de
  palavras em cada função.
## Ordem de criação das variáveis decide os `$s` (pesquisa da rodada 8, `dd_fx32.c`)

O "par trocado" de laço (`s3`/`s4` entre o ponteiro da partícula e o contador do laço interno, `dd_fx32_s0/s2/s3/s5`)
não é ordem de instrução: é a ordem em que o mwcc cria os registradores virtuais. Regra observada:
- locais da função são criados primeiro, na ordem da declaração; num conflito, o criado antes fica com o `$s` mais
  alto (`s4` antes de `s3`); vale também para float (`float z; float tv;` trocou `f8`/`f9` em `dd_fx32_s6`);
- local declarado num bloco é criado na entrada do bloco, depois dos da função: fica com o `$s` mais baixo;
- ponteiros de indução da redução de força são criados por último (`s0`/`s1`/`s2`).

Receita (bateu `dd_fx32_s0`, `s2`, `s4`, `s5`, `s6`, `s7`, `s8`):
1. Contador do laço interno declarado no bloco do laço externo: `for (j3 ...) { int jd; sp = &d->spin[j3]; ... }`.
2. Ponteiro do laço de desenho que é o mesmo do laço de criação (`b`, `bl`, `sp`): variável própria (`b_0`) declarada
   depois das outras (ou logo depois de um `int` do fim da lista), e o contador interno no bloco.
3. Um contador que tem de ficar *acima* dos outros: variável nova declarada **antes** de todas (`int col;` em
   `dd_fx32_s0`, laço 64x32); o acumulador que tem de ficar abaixo, no bloco (`{ int x; ... }`).
4. Valor de linha içado para fora do laço interno num `$s` (`trunc` de `i / 15.0f * 272.0f`): `int gy` calculado no
   laço externo (`gy = i / 15.0f * 272.0f;` e `... = gy + roundf_vfpu(...)`), não `(int)(...)` dentro do interno.
5. Busca automática: renomear o contador de cada `for` para uma variável nova declarada no topo da função, no fim das
   declarações ou no início do bloco que contém o laço, e manter a que reduz as palavras (descida gulosa; script
   `build/tmp/fx32/loops.py` da worktree `fx32`, não versionado). Achou 74 → 18 em `dd_fx32_s5`, 71 → 3 em `dd_fx32_s4` e 107 → 21 em `dd_fx32_s1`.
6. O mesmo vale para float: um valor usado em dois laços diferentes (`rad` no tubo e nas faíscas de `dd_fx32_s1`) pega
   `f20`/`f21` trocados; uma variável própria por uso (`float rd;`) desfaz a troca.
Os 9 `dd_fx32_s*_update` são variações do mesmo molde (tubo, faíscas, grade): a forma que bate num serve de modelo
para os outros (tubo de `s2` em `s1/s4/s5/s8`, grade de `s5` em `s4`, lâminas de `s0` em `s7`, `out = 1.0f - lin`).

## Não-corrigíveis conhecidos (registre em `docs/matching_skip.tsv` e siga)

- **Cópia morta do destino depois de um `b`** (`bnezl/beql; b; <delay>; <cópia da 1ª instrução do destino>`) e
  **`bXX; nop` que o mwcc 219 faz likely**: em parte corrigíveis, ver "Desvios likely" acima. Sem forma: o
  destino é a cabeça de um laço (`st_scr_find_es`, `st_pl_card_bit_set`). `tools/likely_family.py` lista os da fila.
- **Original lê local da pilha não inicializado**, ou **guarda um store morto** que o mwcc elimina.
- **SDK do BOOT compilado com GCC** (arquivos com `asm` do mwcc sob `__MWERKS__`): trilha separada, fora da fila;
  ficam em `@asm` os que carregam dados por `lui`+`%lo` (ver `docs/build.md`, "Código GCC como funções asm").

## Gate e commit (uma vez por lote)

```sh
make MODULE=<m> verify 2>&1 | grep "ALL MATCH\|BAD\|check-all"   # tem de imprimir ALL MATCH
```
Se der `BAD`, `make MODULE=<m> check 2>&1 | grep -A3 "Error\|undefined"` mostra o motivo. Headers não têm
dependência no make: depois de editar `include/`, `rm -rf build/src/<m>`. Nunca dois `verify` ao mesmo tempo.

Commit em pt-BR, só os arquivos do lote (o `config/<m>.yaml` muda sempre que uma linha `@asm` sai — não esqueça): `src/<m>/`, `symbols/<m>.txt`, `config/<m>.yaml`, `docs/matching_queue.tsv`, `docs/matching_skip.tsv`.
Mensagem: `<m> <arquivo>: <funções> batendo (<idioma que resolveu>); <m> MATCH, check-all MATCH`.
Idioma novo que resolveu mais de uma função: acrescente uma linha na seção de `docs/build.md` e na tabela acima.

## Tarefa 0: drafts de NON_MATCHING que não compilam

Estes arquivos têm o C sob `NON_MATCHING` quebrado; por isso o objdiff mostra 0% para as funções em asm deles e
o port não tem C para elas. Consertar (fazer compilar com `-DNON_MATCHING`, fiel ao asm) vem antes do matching:
`src/BOOT/libm.c`, `src/libdeck/dk_filter.c`, `src/libdeck/dk_icons.c`, `src/libdeck_swap/ds_word.c`,
`src/libdeck_tutorial/dt_filter.c`, `src/libdeck_tutorial/dt_sort.c`, `src/libdeck_tutorial/dt_word.c`,
`src/libshop/shop_list.c`, `src/libtitle/t_partner.c`.
Teste: `tools/mwcc.sh -c -O4,p -sdatathreshold 0 -i include -DNON_MATCHING src/<m>/<f>.c -o /tmp/x.o`.

## Revisão de fidelidade (antes do port, não é matching)

C sob `NON_MATCHING` escrito sem comparação detalhada com o asm (pode ter divergência de lógica, não só de
registradores): libduel_eng `ai_08839360.c` (5 funções), `battle_calc.c`, `chain_resolve.c`; libduel_draw, as
funções grandes de atualização dos efeitos (`dd_fx_*`). Quando uma delas aparecer na fila, compare o fluxo com o
asm antes de mexer na forma.

## Sessões em paralelo

Se mais de uma sessão trabalhar ao mesmo tempo, cada uma num **módulo diferente** e numa worktree própria
(`git worktree add ../tf-wt/<x> -b agent-<x>`; ligar `build/elf`, `build/decomp`, `tools/ext`, `.venv`, `extracted`
por symlink e copiar o resto de `build/`; scripts auxiliares numa subpasta própria do scratchpad). Os agentes não
editam `docs/matching_queue.tsv`/`docs/matching_skip.tsv` (o coordenador regenera depois do merge). Um `check-all`
que falha só num módulo alheio costuma ser split velho copiado: `rm -rf build/asm/<m> build/src/<m>`. Merge: `symbols/<m>.txt` em conflito se resolve por endereço
(`build/tmp/resolve_status.py`, que descarta linhas `alias`: confira-as depois), `config/<m>.yaml` se regenera
(`python3 tools/splat_config.py <m> > config/<m>.yaml`), e `make MODULE=<m> verify` antes de commitar o merge.

## Reescrita estrutural de funções grandes (rodada 7)

Funções de vários KB com fuzzy baixo costumam ter a lógica certa e a forma errada. Roteiro que levou
`duel_088087C0` (5 KB, 2,7%) a bater e `duel_088CC8EC` (8,9 KB, 57,8%) a 24 palavras:
1. Alinhar por opcode antes de olhar registradores: `difflib.SequenceMatcher` sobre a coluna do mnemônico do
   original e do objeto `-DNON_MATCHING` mostra só as regiões de forma diferente (script no scratchpad do agente).
2. `switch` linear grande: extrair (valor, alvo) dos `li; beq/beql` (alvo do `beql` − 4 = mesmo corpo) e escrever
   os `case` na ordem dos corpos, rótulos de cada grupo na ordem inversa dos testes. Um `beql` cujo alvo é o fim do
   `switch` com a atribuição no delay é um `case` de uma linha (o corpo existe mais adiante).
3. Código de `duel_0884F30C` montado com máscaras: `((unsigned int)p << 31 & 0x80000000) | (kind << 25 & 0x7E000000)
   | flags | ((unsigned int)z << 16 & 0x1F0000) | (id & 0xFFFF)`. Sem o cast `unsigned`, o mwcc reduz a força do
   shift da variável do laço (`addu s0,s0,0x80000000` por volta); com o cast, recalcula como o original.
4. `slti at` no teste de um laço contado: `i <= 4` (não `i < 5`). Índice e ponteiro do laço trocados de `$s`:
   variável do laço declarada no bloco (`{ int j; for (j = 0; ...) }`), não a da função.
5. Dois `case` que usam `&&` com o mesmo corpo `trig = 1` e `beql ..., fim; li s3,1`: era `||` num só `if`.

## libduel_mgr (rodada 8): idiomas de `dmgr_janken.c`/`dmgr_result.c`

- **`andi x,zero,1` / `andi x,v0,1` antes de gravar um bitfield de 1 bit**: o valor vinha de uma **variável local**
  (`on = 1; dmgr_jk->bg_on = on; on = 0; dmgr_jk->partner_on = on;`), propagada depois do rebaixamento do bitfield; a
  constante literal dá `li`. O registrador reaproveitado (`v0` do `li v0,1` de um teste do `switch`) também vem daí.
  A ordem das gravações e das outras atribuições do `case` sai por `perm_stmts.py` (`dmgr_jk_bg_anim`).
- **Endereço de origem calculado antes do teste** (`addu a1,netbuf,off` no atraso do `beqz`): o fonte guardava o ponteiro
  num local antes do `if` (`unsigned char *src = netbuf + s->net_id * 0x180; if (s->type < 2) memcpy(..., src, ...)`).
- **`x * -K` em `negu/sll/subu` sem redução de força num laço**: `-i * 45 - 0x14` (não `-0x14 - i * 45`, que vira indução).
- **Posições `base + N` com N deslocado** (`addiu a1,s5,347` onde o draft tinha `x + 0x199`): o original somava a um
  local `l = x + 0x3E` (`l + 0x15B`); reescrever todas as posições relativas a `l` (`dmgr_res_page_summary`).
- **Chamado `static` mantém `$t` vivos através do `jal`**: valores do chamador em `t4`-`t7` depois de chamar uma função
  do mesmo arquivo (variáveis de laço, resultado da 1ª chamada) só saem com o chamado `static` e definido antes. Se
  outra função ainda em asm o chama, o `static` quebra o link: usar `#ifdef NON_MATCHING static #endif` até todas baterem
  (`dmgr_jk_cards_move`; `dmgr_jk_reveal` bate assim, presa a `dmgr_jk_pick`).
- **Cópias de struct que não matam o global, gravações de campo que matam**: o original relê `dmgr_jk` depois de
  `p->to.x = K` mas não depois de `p->from = p->cur`; o fonte recalculava o ponteiro a cada grupo
  (`a = &dmgr_jk->card[0][2]; a->from = a->cur; a->to = a->cur; if (...) a->to.x = 200;`).
- **Vetor local inicializado** (`lui; lh` de cada elemento do modelo em `.data`, `sh` na pilha): `unsigned short div[4] =
  { 0, 5, 6, 8 };` no bloco onde é copiado; o global do modelo fica só sem `NON_MATCHING` (`dmgr_result_init`).
- **Índice em vez de ponteiro de laço**: `((DmgrResItem *)tab)[i].on` no lugar de `it++` muda a ordem dos incrementos
  das induções; e `x` calculado no uso (`0xDC + i * 0x3B`) em vez de variável do `for` troca os `$s` (`dmgr_res_draw_series`).

## Rodada 8 (telas de opções e brilho da loja)

- **Funções gêmeas entre módulos com o mesmo tamanho**: `opt_*` de libduel_draw (`dd_duel_options.c`) e `tcfg_*` de
  libtitle (`t_cfg.c`/`t_cfgdraw.c`) são o mesmo código (asm igual a menos de endereços: confira com `raw_asm.sh` das
  duas faixas, endereços mascarados). Gerar uma a partir da outra com troca de nomes de campos/funções bateu `opt_update`, `opt_row`, `opt_switch_draw` de uma vez; o draft antigo
  de libduel_draw tinha protótipos e um campo com tipo errado (`frame` `unsigned int`, era `int`: `% 16` e
  `%= 0x1000000` mudam). Antes de reescrever uma função grande, procure a gêmea por tamanho em `symbols/`.
- **Cópia de struct escalonada fora da ordem** (`off = s[1]` com os `lh/sh` de u,v,w antes do `sh` de h no
  original): `off = ((TCfgSpr2 *)s)->s[1];` depois das leituras de `s[0]` (`tcfg_meter`, `tcfg_arrows`).
- **`static` de uma folha chamada por funções em asm**: `shop_glow_ratio` e `shop_glow_ratio_inv` viraram `static` (linha `type:func static
  matched` em `symbols/`, o `PROVIDE` do `fix_ld.py` atende o asm); `shop_glow_draw_bar` passou de 266 para 21
  palavras e bateu com um contador por laço (`i`, `j`, `k`). Sinal: o original mantém um `$f`/`$v` temporário (não
  `$s`/`$f20+`) vivo através do `jal` da folha.
- **Mesmo endereço por duas expressões = dois ponteiros de indução**: o original tinha `a0` e `a2` de
  `mat4_transform_vec4(out, &mvp, in)` em `$s` diferentes, ambos `&scr[i]`; `(Vec4 *)((char *)scr + i * 16)` num
  deles e `&scr[i]` no outro reproduz (`shop_glow_ultra`).
- **Temporário float calculado antes de uma chamada** (`div.s $f20` no delay slot do `jal`): o local `f` declarado
  no escopo da função, não no bloco do laço (no bloco o mwcc guarda o int em `$s` e converte depois da chamada).
- **Descida por coordenadas no nome do contador de cada laço**: renomeia o contador de
  cada `for (x = ...)` para um nome de um conjunto (sem repetir entre laços aninhados) e mantém o que reduz as
  palavras; levou `shop_glow_ultra` de 144 a 20 palavras (bateu depois com `aligned(16)` no `Mat4 mvp` do bloco) e
  `shop_glow_super` de 411 a 31 (este também com `ShopGlowSparks *q = &g->spark_a[i][j];` no laço do meio: o
  original guarda a base do grupo e lê `q->s[n]`).
Rodada 8 (`leave_field.c`: `duel_leave_mark` de 748 palavras a 0, `duel_088CFBA4` de 797 a 22):
6. **Cópias mortas espalhadas pela função vinham de um só `return`**: as três cópias mortas de `duel_leave_mark`
   (inclusive duas longe dali, no `if/else` do topo e no `case 0x12BE`) sumiram ao escrever
   `if (id == 0 || (duel.zone_off & 1 << (z + p * 16))) return;` em vez de dois `if (...) return;` seguidos
   (o `||` seguido de `return` da seção "Desvios likely"). Antes de dar uma cópia morta como não-corrigível,
   procure na função um par de `return` antecipados que possa ser um `||`.
7. **`andi 0xFFFF`/`0xFF` nos argumentos que o original não tem**: o original chamava sem protótipo
   (`int duel_card_can_affect();`, como em `effects_s.c`); o protótipo com `unsigned short`/`unsigned char`
   mascara cada argumento.
8. **Expressão invariante que o original recalcula dentro do laço** (`xor; sltu; addiu a3,v0,2` antes de cada
   chamada, e o mwcc a iça para um `$s`): `(q != p ? 1 : 0) + 2` em vez de `(q != p) + 2`. O `?:` só vira
   `sltu` depois do LICM, então não é içado; `(q != p ? 3 : 2)` dá desvios. `q != p` sozinho (2 instruções) já não
   é içado.
9. **`andi 0xFFFF` redundante depois de `andi 0x1FFF`** no `switch` de um campo de 13 bits:
   `switch ((unsigned short)(int)SLOT_AT(p, i)->card.id)`; o cast direto `(unsigned short)` é descartado.
10. **Parâmetro relido da pilha a cada uso** (`lw a0,60(sp)` antes de cada `andi`): é um parâmetro derramado; a ordem
   das inicializações dos flags derivados dele decide a coloração (`keep` antes de `destroy` em `duel_088CFBA4`).
## Família `duel_req_*` (rodada 8): coloração e CSE

Pesquisa em `req_effect.c`/`req_step.c`. **O bitfield de halfword não se aplica**: os `DuelLoc` locais dessas funções
são preenchidos campo a campo (`lbu/sb` estreitados, só `idx` de 8 bits sai `lhu/sh`), como o mwcc 219 já faz; o
que trava a família é coloração de `$s`, CSE de endereço e escalonamento. Idiomas que resolveram:
- **Local de bloco muda a coloração** (`duel_req_088D9504`, 553 → 0 palavras): o original usava `$s8` a mais porque o
  ponteiro da carta do ramo `if (keep)` era outra variável: `DuelCard *c = ...;` declarado **dentro** do bloco
  (o do `else` continua o da função). Sinal: o original usa um `$s` a mais e um dos registradores só tem um uso.
- **Seleção por ternário + `return` em cada `case`** (`duel_req_088D9DBC`, 475 → 0): `beq; nop; b` na escolha de um
  ponteiro = `s = c1 && c2 ? &a : &b;`; cópia morta da 1ª instrução de cada `case` depois do `b` do anterior =
  os `case` terminam em `return;` (não `break;`).
- **Deslocamento da zona num local** (`zo = z * sizeof(DuelSlot)` antes do jogador): quando o original calcula
  `z * 20` antes do `mult` do jogador **e** o põe como operando esquerdo do `addu`, nem `ZSLOT` nem a forma 3 bastam;
  `ZOSLOT(p, zo)` / `ZOFF(p, zo, T, campo)` em `req_effect.c` (change_pos 201 → 7, flip 202 → 2, move_zone 238 → 15).
- **Declarar separado de atribuir**: a ordem das declarações decide a coloração e a dos comandos decide a ordem das
  cargas; com inicializadores elas andam juntas. Declare sem inicializar (na ordem que dá os `$s` do original) e
  atribua na ordem das cargas do original (change_pos 22 → 9, flip 22 → 2).
- **Endereço de campo repetido sem CSE**: o original recalcula `lui/addiu duel+0x36` num segundo uso e o mwcc guarda
  num `$s8`; escrever o segundo como `(unsigned int)((char *)&duel.players[0].field[0] + 6)` evita o CSE.
- `(unsigned int)(x & 1) << 14` num termo de um `|` muda a ordem de avaliação (flip); `MIN(-n, max)` como ternário
  `max < -n ? max : -n` dá `beqz; nop; move` sem likely (counters).
Ainda em asm (registradores/escalonamento, formas esgotadas): `duel_req_effect_set` e `duel_req_order` (4),
`duel_req_flip` (2: segundo `lui` de `duel_order_ctr` antes do `addiu`, mesma família de `duel_place_monster/st`),
`duel_req_activate` (2), `duel_req_change_pos` (7), `duel_req_mill` (9, p/n trocados), `duel_req_move_zone` (15,
ap/az trocados), `duel_req_counters` (20), `duel_req_move_card` (51: `ext 16,16` de `t >> 16` que o 219 dá `srl`).
## `return 0` fora de linha e `if (x == y) { ... }` (rodada 8, `ai_08811BDC.c`)

`duel_08811BDC` (4,5 KB, 60 casos) e `duel_08812DBC` bateram corrigindo a **forma dos retornos**, não registradores:
- `bnel x,y,fim; move v0,zero` (likely direto ao epílogo com o `0` no delay) é um teste cujo `return 0` está **fora de
  linha**: `if (ap == q) { ... } return 0;` (ou `break` até o `return 0` final), não `if (ap != q) return 0;` (este dá
  `beq; nop; b fim; move v0,zero`). O corpo do `if` não termina com `return 0` próprio: as falhas internas caem no
  `return 0` depois do `if` (`bnez; nop` para ele; só o primeiro desvio leva a cópia).
- `beqz; nop` para o `move v0,zero` **final** da função = `break` do `switch` até o `return 0` comum.
- Teste com o `return 1` como bloco morto depois de `b próximo` = `switch (f()) { case K: return 1; }` de um só caso.
- `if (a || b) return 1;` quando o `beqzl` do primeiro teste copia o `li v0,1` do bloco do segundo.
- `sltu`/`xori` do `return f() != 0` / `return x >= K` fora do delay slot do `b fim` (o mwcc junta o `return 0` vizinho no
  mesmo `b`): escrever `if (f() != 0) return 1; return 0;` — o mwcc ainda gera o `sltu`, mas no delay slot.
- `&&` materializado (`xor; sltiu; beqz; sltu`) e `beqz v0,fim` reaproveitando o 0: `i = a && b; if (i) i = !f(); return i;`.
- `if (!c) {A} else {B}` quando o original põe o ramo `!c` primeiro e um trecho comum depois dos dois (`case 0x1316`).
- **Ordem das declarações** voltou a decidir os `$s` numa função grande: 400 permutações aleatórias das 8 primeiras
  (`direct, atk, pl, tz, tv, ap, az, tp`) levaram 142 palavras a 11; as últimas 11 eram `a >= b` × `b <= a` (`slt at`).
- Contagem de constantes: `t = dv + t` com o resultado num `$` novo = variável nova (`n = dv + t; if (av <= n) t = n - av;`).

`duel_ai_atk_update`: as flags (`f0/f6/f7`) e o `mode` são lidos pelo símbolo `duel_w_08914A2A` (`lbu 18986(v0)`), não
pela base `a`; os códigos de evento são montados com máscaras (`(q << 31 & 0x80000000) | (kind << 25 & 0x7E000000)`, o
id do slot `& 0xFFFF` em vez da constante comparada, `kind` em vez da constante 8/6 que o `if` garante).
## Idiomas da rodada 8 (BOOT: ehf.c, menu_draw.c, cardinfo.c, cardlist.c)

| No asm original | Fonte |
|---|---|
| `andi a0,sX,0xFFFF` antes de **cada** chamada, sem registrador a mais para a cópia u16 | parâmetro `int` na função e protótipo do chamado com `unsigned short` (`cardinfo_draw_list_entry` → `cardinfo_get_text(unsigned short)`); um local u16 ou `(unsigned short)p` gasta um `$s` a mais |
| `addu` com os operandos trocados numa soma de campos + globais | quebrar em `mw = a + 2 + b; mw += G1; mw += G2;` (o mwcc reassocia a expressão inteira, parênteses não mudam) |
| ponteiro de busca reaproveitado como ponteiro dos dados no mesmo `$t` (`addiu t1,t1,2`) | reusar a variável: `k += 2;` em vez de `pa = k + 2` (`ehf_track_eval`) |
| `b exact; move t1,t2` logo depois do teste | `if (!(*lo < f)) { k = lo; goto exact; }` (atribuição dentro do ramo, não antes) |
| `slt at` num teste que o C deu com temporário | inverter a comparação (`*hi <= f` em vez de `!(f < *hi)`), regra do `$at` da tabela acima |
| cópia de matriz `lwc1/swc1` em grupos de 3 dentro de um laço | 16 atribuições `g2[i] = m[i];` (struct com `float[16]` dá laço de `lw/sw`) |
| `bc1f; nop; lui; mtc1; swc1` (gravação incondicional) | `d->s = d->s == 0.0f ? 1.0f : d->s;` (com `if` ou local sai `bc1fl` com cópia) |
| campo lido uma vez, salvo e usado (`lw; sll; sw saved`) | local: `c = d->color; it->saved = c; d->color = lerp(c & 0xFFFFFF, c);` |
| `beqzl x; li K` / `b; subu` / `subu` | `cw = x != 0 ? x - mw : K - mw;` |
| `add.s` agrupado como `k + (a + b)` | escrever os parênteses: a soma de float não é associativa (bug de fidelidade em `menu_item_build_text`) |

Também: helper `static` definido antes no mesmo `.c` (`mat4_det`, `ehf_track_eval`) faz o chamador manter
`$a`/`$t` através do `jal` (ver `docs/build.md`); quando um asm ainda o chama, a linha em `symbols/` leva
`type:func static`. Família não resolvida: cópia morta do destino nos `u32 → float` (`bltzl; srl` com o `srl`
original mantido depois do `b`) em `cardlist_draw_arrow`: o mwcc 219 a remove nessa função, embora a mantenha em
`ehf_track_eval`.

## Rodada 9 (libduel_draw fora de `dd_fx*`)

| No asm original | Fonte |
|---|---|
| local preenchido depois de várias chamadas (laço `sb zero` no meio da função), mas no **topo** da pilha | abrir um bloco no ponto da inicialização com esse local declarado **primeiro** e todos os outros locais e o resto da função dentro dele (`dd_coin_tex`/`dd_die_tex`, `dd_sel_draw_item`); o mwcc não aceita declaração no meio do bloco |
| `mul.s fd, resultado, const` com a constante recarregada a cada uso | inline `static inline float dd_dice_mul(float x, float k) { return x * k; }` chamado com o literal (local `k` é propagado) |
| `lb` (não `lbu`) relendo um byte de bitfield de 1 bit | o tipo base do bitfield é `signed char` |
| `and; sltu v0,zero,v0; bnezl v0` (o bit materializado como 0/1 antes do teste) | `if (((*flags & bit) ? 1 : 0) == 0 && ...)` (`dd_cue_*`) |
| campo carregado antes do argumento anterior (global) na chamada | cast `(unsigned int)campo` no argumento (`dd_card_ready_m/h`, `dd_ev_attack_done`), a regra do tipo do parâmetro da rodada 7 |
| `beqzl` com a cópia do `move v0,zero` e o último teste `beqz; nop; b; li 1` com o `move v0,zero` separado | `return` direto em cada caso e um `return 0;` comum no fim, com os testes de igualdade dentro de `if (a == b) { ... } else { ... }` (`dd_sel_cmp_*`) |
| flag guardada num `$s` e gravada sempre (`move s0,zero` no delay, `li s0,1`, `sw s0`) | `hl = 0; if (A == 0 \|\| B == 0) { } else { hl = 1; } g = hl;` (`&&` ou `if` aninhado viram `sltu`); era bug de lógica: o draft só gravava o 1 (`dd_board_piles`) |
| `beql first,head` para o `switch` e logo outro `beq first,head` para pular o laço | `if (first != head) { for (o = first; o != head; ...) }` |
| bloco "card = 0" compartilhado entre dois testes distantes | `if (!(t < dur)) { if (b5) break; if (A && B) break; } card = 0; break;` |
| locais de pilha em outra ordem | declarar na ordem de cima para baixo do original (`float sc[3] = { s, s, s };` com a inicialização antes do teste `n <= 0` em `dd_board_pile`; `up`, `target`, `fwd` em `dd_camera_from_anim`); 8 bytes de folga acima de um vetor `char opts[0x60]` = `__attribute__((aligned(16)))` nele (`lpc_panel_2d`) |
| ponteiro de laço com o deslocamento do campo no `lw` (`lw s0,32(s1)`, `addiu s1,s1,4`) | o ponteiro anda pela struct: `DdTasks *g = &dd_tasks; ... g = (DdTasks *)((DdCard **)g + 1)` e `g->glow[0]` (`dd_glow_all_off`, `dd_mark_all_off`) |
| `move s1,v0` do 1º resultado e o 2º noutro `$s` | duas variáveis: `dl = f(dl, ...); list = g(dl, ...);` (`dd_sel_draw_item`) |
| `c->cur` carregado antes de `c->speed` num `1 - MIN(MAX(cur, 0), speed) / speed` | o valor limitado num local de bloco antes das conversões: `{ int v = MIN(MAX(c->cur, 0), c->speed); k = 1.0f - (float)v / (float)c->speed; }` (`lpc_panel_2d`) |
| `near < 1.0f ? 1.0f : near` com o resultado noutro registrador float | ternário em vez de `if (near < 1.0f) near = 1.0f;` |
| `c.lt.s; bc1tl → epílogo; move v0,zero` seguido dos testes de bits com `bnez` para um `return 0` comum (o C deu o float materializado em 0/1) | um só `if (o->t < o->dur \|\| o->a7 \|\| o->b0 \|\| o->a6) return 0;` (`dd_cobjs_idle`) |
| `move s5,v0; beqz s5` (atribuição separada do teste) e `beqzl first → epílogo; lw ra` / `beqz last; nop` | `o = f(); if (o != 0) {...}` em vez de `(o = f()) != 0` no `&&`; resto dentro de `if (first != 0 && last != 0) {...}`; `if (first == 0) first = o;` antes de `last = o;` (`dd_cobj_gather`) |

Famílias sem forma nesta rodada:
- **Laço interno com o teste no topo mantido** (`slti at,z,16; beqzl` antes do corpo, o externo rotacionado sem
  teste de entrada, e o índice interno sem redução de força): `dd_cobjs_update`, `dd_cobjs_busy`,
  `dd_cobjs_counters`, `dd_counter_tick`, `dd_zone_anim_update`, `dd_zone_anim_busy`, `func_08822520`, `func_08822B90`, `func_08822C2C`. O mwcc 219 sempre elimina
  o teste de entrada do laço interno (`z = 0` conhecido); `while`, `do`/`if`, `for (;;)`+`break`, `z = 0` no
  incremento do externo, inline, tipos de `z` e pragmas `opt_*` testados. Provável diferença de build do compilador.
- **`bnez v, corpo; nop; b epílogo; nop`** no 2º teste de `if (!a && !b) return;` (`dd_voice_enable`,
  `dd_info_hide`): o 219 inverte para `beqz v, epílogo`; `goto`, `do { break; } while (0)`, `switch` e `if`
  aninhados testados.
