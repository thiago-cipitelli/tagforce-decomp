# Matching das funções em `@asm` — instruções de trabalho

Desde 2026-10-30 todo o `.text` de todos os módulos está em C. O que falta para a fidelidade são as funções
mantidas como `// @asm A-B`, com o C equivalente sob `#ifdef NON_MATCHING`. Este arquivo é o ponto de partida
de cada sessão dessa etapa: leia só ele (e o trecho de `docs/build.md` que ele indicar), não o repositório inteiro.

Estado em 2026-11-06, após a rodada 7 (pesquisa das famílias bloqueadoras + funções grandes; ByteMatch): 73,5% do
código batendo, 94,6% fuzzy, 10057/11039 funções (libduel_eng 75,4%/96,9%, libduel_draw 56,8%/92,7%). Fila: 632
funções em `docs/matching_queue.tsv` (85 abaixo de 80%, nenhuma em 0%: toda função tem
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
