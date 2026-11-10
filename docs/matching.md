# Matching das funções em `@asm` — instruções de trabalho

Desde 2026-10-30 todo o `.text` de todos os módulos está em C. O que falta para a fidelidade são as funções
mantidas como `// @asm A-B`, com o C equivalente sob `#ifdef NON_MATCHING`. Este arquivo é o ponto de partida
de cada sessão dessa etapa: leia só ele (e o trecho de `docs/build.md` que ele indicar), não o repositório inteiro.

Estado em 2026-11-09, após a rodada 13 (funções de 95-98% nunca tentadas; ByteMatch): 82,0% do
código batendo, 95,5% fuzzy, 10280/11039 funções (libduel_eng 83,5%/97,5%, libduel_draw 76,9%/95,8%). Fila: 283
funções em `docs/matching_queue.tsv` (53 abaixo de 80%, nenhuma em 0%: toda função tem
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
### Rodada 9 (`dd_burst_update`, `dd_pyre_update` batendo; `dd_burst2_update` a 23 palavras, `dd_trail_update` a 16)
- **Vetor zerado no meio do laço**: `ScePspFVector3 v = { 0 };` declarado num **bloco interno** depois dos sorteios
  (`y = ...; a = ...; b = ...; { ScePspFVector3 v = { 0 }; ScePspFMatrix3 rot; v.y = ...; mat3_...(&rot, ...); }`)
  dá o laço de bytes no ponto certo e põe `v`/`rot` na pilha do bloco; o laço manual `do { *c++ = 0; }` no mesmo ponto
  não reproduz a alocação dos temporários do LCG.
- **Pilha por blocos**: locais da função (não alinhados) no topo, depois os de cada bloco **na ordem do fonte, sem
  sobreposição entre blocos irmãos**, e as variáveis da função com `aligned(16)` no fundo. Para um quadro como o de
  `dd_pyre_update` (656 bytes) bastou pôr cada vetor/matriz no bloco onde é usado (`node`/`tmp` em cada ramo do
  `if (flip == 1)`, `w` no laço das faíscas, `axis`/`tilt`/`up` no bloco das labaredas, `off`..`rot` no laço interno).
  Em `dd_trail_update` (só função) a ordem das declarações com `aligned(16)` em todos os vetores de 16 deu o quadro.
- **Laço de 4 voltas sem chamadas desenrolado pelo mwcc** (o original não): `i = 0; do { ... } while (++i < 4);` ou
  `(unsigned int)i < 4` mantêm o laço (`dd_pyre_update`, sorteio das labaredas).
- **Cópia de modelo campo a campo**: `scale.x = g.x; scale.y = g.y; scale.z = g.z; ...; scale.y = in;` (a primeira
  gravação de `y` sobrevive) reproduz a ordem lw/lw/sw/sw/lw/sw; a cópia de struct agenda diferente.
- **Expressão que o original calcula antes de um desvio** (conversão float→u32 da cor): temporário de uso único é
  propagado para o uso; reatribuir uma variável já usada (`ca = -ca + x; sa = -sa + y;` depois de gravar `q[0]`) a
  mantém no bloco básico de cima (`dd_burst2_update`, 1058 → 23 palavras).
- **Testes**: `if (id == 80) 0.2 else 1.0` com `beql`+`b` = `switch (s->id) { case 80: ...; default: ...; }`;
  `if (a == 18 && b == 1)` com `beql`+`b; nop` = `switch (a) { case 18: if (b == 1) ...; break; }`;
  `beq` para `move s0,zero` + `b; li s0,1` = `if (id != 90 && id != 84) flip = 1; else flip = 0;`.
- **Laço com teste de bit e ponteiro no atraso do `beq`**: `{ T *p2 = &d->arr[i]; if (!DD_BIT(d->arr[i].flags, 1) != 1)
  { int jb; ... } }` (ponteiro e contador interno no bloco: `$s` como no original).
- **Três ponteiros de vértice** (`base+0x44/0x54/0x64` em três `$s`): `v0 = &d->strip[n][k][0]; v1 = ...[1]; v2 = ...[2];`
  em vez de um ponteiro com deslocamentos; ponteiro do ponto da curva (`cv = &d->curve[i][j]`) no topo do laço.
- **Constantes em local com operando trocado**: `du * (i % 4)` / `tv + du` (constante primeiro na multiplicação e
  segundo na soma) = `du` local na ordem do fonte; `0.25f` literal vai sempre para `fs`.
- **Constantes dobradas parcialmente** (`dd_fx_shatter_update`): o original guarda meia-célula e deslocamento em locais
  (`hw = 7.75f; ox = 2.5833333f;`); `hw + ox` sai dobrado (`sw` do inteiro) e `-hw + ox` em tempo de execução (`neg.s`).
- **Busca de contadores** (`build/tmp/burst/ctr.py` da worktree `burst`, não versionado): para cada `for (x = ...` renomeia
  `x` só no laço para um nome novo declarado no topo, no fim das declarações ou num bloco em volta, e guarda o melhor;
  levou `dd_trail_update` de 107 a 16 palavras. Sobra em `dd_trail_update` um par de posições de derrame (`list` e o
  contador externo das fitas, 0x60/0x64) que não muda com ordem de declaração nem variável nova.
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

## Rodada 9 (`dd_fx_*`): registradores que a ordem de criação sozinha não resolve

`dd_fx_glow_update`, `dd_fx32_s1_update`, `dd_ring_update`, `dd_halo_update`, `dd_tunnel_update` e `dd_fx_summon_update` bateram; a forma já
era idêntica e só faltavam registradores. O que resolveu, além da regra da rodada 8:
- **Ponteiro de laço declarado no topo da função** (antes de todas as outras, inclusive da primeira com
  inicializador): o ponteiro do elemento num laço de desenho que tem o mesmo nome do laço de criação (`f`, `sp`, `st`)
  vira uma variável própria declarada na primeira linha (`DdRingFlame *fl;`, `DdHaloSpark *ns;`, `DdStreak *sk;`). Vale
  para float também (`float place;` em `dd_ring_update`). Busca automática: renomear cada uso por faixa de linhas para
  uma variável nova no topo / no fim das declarações / no bloco, ou para uma variável existente, em descida gulosa
  (`build/tmp/fxe/ren.py` da worktree `fxe`, não versionado).
- **Expressão repetida em vez de variável**: `void *tb = d->tube_buf[buf]; memcpy(tb, ...); f(..., tb)` dá `$s`
  diferentes de `memcpy(d->tube_buf[buf], ...); f(..., d->tube_buf[buf])` (o temporário do CSE é criado depois),
  com o contador do laço sendo o `i` da função (`dd_fx32_s1_update`).
- **`&local` mantido num `$s` através de uma chamada** (`addiu s1,sp,404` antes do `sinf`, passado ao sprite depois):
  o mwcc rematerializa `&c` em qualquer forma direta; o que reproduz é um ponteiro de indução de um laço de uma volta,
  declarado antes de `list`: `pos = &c; for (n = 0; n < 1; n++) { in = ...; list = f(list, pos++, ...sinf...); }`
  (`dd_fx_glow_update`).
- **Cópia de modelo por campo**: `scale.x = g.x; scale.y = g.y; scale.z = g.z;` intercala `lwc1/swc1` como o
  original (`lw x; lw y; sw x; sw y; lw z; sw z`); `scale = *(V3 *)&g` agenda a última carga antes do `sw y`.
- **Ordem dos comandos sem dependência decide o `$s` de uma flag**: `off = tmpl; tilt = 0.0f; flip = 1;` (não
  `tilt; off; flip`) fez `flip` herdar o registrador de `s` (`dd_halo_update`).
- **Constante em `ft`** num `fr * K + C`: local no bloco do laço (`float dr = -600.0f; radius = fr * dr + 800.0f;`).
- **`list` dividida em duas variáveis** (`dd_orb_draw`, 12 → 2 palavras): a partir do primeiro `draw_sprite`, o
  resto da função usa `l2 = draw_sprite(list, ...)`; o valor de 31 do halo reaproveita `v` em vez de `t`.
- **`dd_fx_summon_update` (290 → 0)**: descida gulosa de nomes (contadores próprios no topo, cores do anel em
  variáveis próprias) levou a 30; o resto: `bb[16] __attribute__((aligned(16)))` (a matriz vai para o fundo da pilha),
  `alpha` sem o `<< 24` e o deslocamento na chamada (`alpha << 24 | 0xFFFFFF`), constantes das somas dos vértices em
  locais (`tv + q`, `q = 0.25f`; a literal nos produtos fica), e o modelo `{0, 0, 1}` com o `e` gravado entre a cópia
  do 1º e do 2º campo = cópia por campo de um global nomeado (`s[0] = g[0]; s[1] = g[1]; s[2] = g[2]; s[0] = e;`).
  O outro modelo do mesmo `@data` vira global também (`DdSumVec3 ofs = *(DdSumVec3 *)g;`), para manter a ordem no `.data`.
- **Conferir os operandos que o `-d` mascara**: o diff estrutural com relocações normalizadas escondeu 2 palavras que
  eram outra forma da mesma lógica (`&ring_idx[0]` sobre `ring[i] + 1` na segunda faixa do anel, não `&ring_idx[4]` sobre `ring[i]`: mesmos vértices, offsets diferentes).

### Rodada 10 (gêmeas do libdeck e quase-acertos de libduel_draw)
- **Gêmeas**: `hguide_draw`, `list_draw`, `ui_nine_patch`, `hscene_init`, `hscene_update` bateram copiando a forma de
  `dk_help_bar_draw`, `dk_pager_draw`, `dk_help_frame`, `dk_help_init`, `dk_help_update` (só nomes trocados). Ao promover
  um `switch` denso copiado, estender o `// @rodata` até cobrir a tabela nova (`hguide_draw`: +0x18).
- **Parâmetro float fora de ordem**: no EABI da PSP o primeiro float vai para `$f12` em qualquer posição do protótipo;
  a posição só muda a ordem de avaliação dos argumentos. `dd_die_tex(float scale, list, ...)` → `dd_die_tex(list, ...,
  face, float scale)` levou o `mtc1` do 1.5f para depois do `y + 40` (`dd_dice_draw`). Vale testar quando um `mtc1` de
  constante sai antes/depois de um argumento inteiro.
- **Operando de `mul.s` trocado** que nenhuma associação muda: o produto num inline (`static inline float m(float a,
  float b) { return a * b; }` chamado com `m(expr, in)`) fixa a ordem dos operandos (`dd_fx_spin_update`).
- **Ponteiro de indução com o deslocamento do campo no argumento** (`move s1,d` e `addiu a0,s1,0x5610` na chamada,
  contador abaixo do ponteiro): `char *pc` no topo, `for (n = 0, pc = (char *)d; ...; n++, pc += sizeof(T))` e
  `((Struct *)pc)->campo[0]` (`dd_trail_update`, laço de `spline_eval`: 16 → 9 palavras, não aplicado; sobra o derrame).

### Rodada 11 (família dk_/dt_, librecipe_viewer)
- **Temporários de um laço idêntico a outro que bate** (`dk/dt_set_mode`, 39 → 0): o 2º laço de releitura dos painéis
  tinha a mesma forma do 1º e outros `$v`/`$a`/`$t`. O original toma o ponteiro antes das gravações:
  `keep = 0; p = &v.pane[i]; p->top = p->list->u18; ...` (no 1º laço, `v.pane[i].top = ...; p = &v.pane[i];`).
  A ordem em que o ponteiro e os temporários nascem decide as cores do laço inteiro.
- **Zeros iniciais como comandos, não inicializadores** (`dt_draw_highlight`, 121 → 0): a ordem dos `move sN,zero`
  do original (`pane, v, fixed, h, w, y, x`) saiu com declarações sem inicializador e `pane = 0; v = 0; ...` na
  ordem do original antes do `switch`; os `$s` vêm da ordem das declarações (achada movendo duas por vez:
  `build/tmp/fam/mv2.py` da worktree `fam`, não versionado). O alfa era `unsigned short t` (0..8 espelhado) e
  `unsigned char a = t * 0xA0 / 8` (dois `andi 0xffff` / `andi 0xff` no original).
- **Parâmetro reaproveitado** (`rv_draw_grid_cursor`): `x1` declarado no lugar de `x0` (antes de `d`) e `x0` no fim.

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

### Rodada 9 (`dmgr_result.c` todo batendo menos `dmgr_res_summary_row`)

- **Endereço de um byte de bitfields num registrador** (`addiu v0,s1,376` no atraso do teste e depois `sb x,0(v0)` /
  `lbu x,0(v0)` em casos distantes): todos os acessos a esses campos passam por um ponteiro de visão
  (`DmgrResMf *m = (DmgrResMf *)&r->pad179 - 1;`, recalculado junto com `r = dmgr_res`), inclusive os testes; o mwcc dobra
  `m->campo` em `376(s1)` nos testes e materializa `v0` para os outros usos. Misturar `r->campo` e `m->campo` impede o
  reaproveitamento do `lbu` do teste no RMW (`dmgr_result_update`).
- **`dmgr_res` em `s1` através de chamadas**: um local `r = dmgr_res;` no topo, recarregado depois de `menu_update`, e
  `switch (r->state)` / `r->sub` / `r->state = 7` onde o original usa `s1`; o resto com `dmgr_res->` (relido).
- **Bitfield gravado com o literal 0** sai `andi x,zero,K` (como o local da rodada 8); só constantes não nulas dão `li`.
- **`k = (prev + 4 - i) % 4` num `for (i = 1; i < 4; i++)`** com `int k` (resto com sinal, `bgez/andi/addiu -4`) e o
  `tab + 4` içado: a forma das buscas de página anterior/seguinte de `dmgr_res_input`/`dmgr_res_draw_footer`.
- **Indução `i * 2` acima de um contador** (`s2` para o deslocamento, `s1` para `partners`, `s0` para `i`): o mwcc cria a
  indução reduzida depois de todas as variáveis, então ela fica abaixo; só um deslocamento explícito declarado antes
  (`int off; int partners; int c;` num bloco, `off += 2`) e o laço guardado (`c = 0; if (c < n) { off = 0; do {...}
  while (c < n); }`, para o `off = 0` sair depois do `blez`) reproduziu (`dmgr_result_init`).
- **Gêmeas entre módulos**: `dmgr_res_draw_scrollbar` é `tlist_draw_scrollbar` (libtutoriallist); portar a forma (track
  reaproveitado como deslocamento, `h - 2 - len` recalculado, `period_up`/`period_down`, `len + (y + track - 3)`) bateu
  de primeira, depois de ordens de declaração e reescritas locais não mudarem nada.
- **`andi a0,v0,0xff` antes de `font_set_size`** = `font_measure_wtext` devolve `int` (o protótipo local dizia `unsigned char`).
- **`dmgr_jk_pick` (janken, 25 palavras, sem solução)**: no `case 2` o original não usa `v0` depois de `lb v1,2(v0)` até o
  fim do laço (todos os temporários deslocados um registrador). O mesmo efeito aparece no `case 3` (que bate): depois de um
  `peer_get` o mwcc evita `v0` no resto do laço, mas quando `peer` é lido no bloco de junção (`peer[2]`) o `lb` reaproveita
  `v0`. Uma leitura de `peer` depois do `if` reproduz todos os registradores (prova de que `peer` vive até lá no original),
  mas acrescenta instruções. Não mudaram: escopo/posição da declaração de `peer`, `peer[2]` num local, `switch (i)`,
  `continue`, `do/while`, `goto`, `volatile`, função `inline`, atribuições encadeadas, leituras mortas de `peer`.

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
### Rodada 9 (`req_effect.c`/`req_step.c`): tipo antes de ordem, busca de ordem dos comandos

Bateram `duel_req_change_pos`, `duel_req_move_zone`, `duel_req_counters`, `duel_req_send` e `duel_req_banish` (333 → 0);
`duel_req_move` 470 → 19 (reescrito do asm), `duel_req_move_card` 51 → 25.
- **Tipo do local decide se a ordem das declarações importa.** Com todos os locais `unsigned int`, 1000+ permutações das
  declarações davam o mesmo resultado; trocar um só (`az` de `int` para `unsigned int` em move_zone: 15 → 2; `z` de
  `unsigned` para `int` em counters: 20 → 16) fez a ordem das declarações voltar a decidir (counters 16 → 1 com busca).
  Experimente `int`/`unsigned` de cada local antes de permutar; comparações que mudam de `slti` para `sltiu` voltam com
  `(int)x`.
- **Ordem das atribuições do topo** (com declarações separadas): em `duel_req_move` só a ordem das 7 atribuições dos
  campos de `arg`/`loc` mudava os `$s` (37 → 19, busca nas 5040 ordens num TU só); declarações e tipos não mudavam nada.
- **Argumento montado com temporários de bloco** (change_pos 7 → 0): `{ unsigned int fu = flip ^ s->faceup; unsigned int ps
  = 1 - s->pos; f(..., (ps & 1) << 15 | ((fu & 1) << 14 | lo), ...); }` deu as cargas e os temporários do original.
- **`lvl = c->b15:16` e regravação** (banish): o original relê a palavra (`lw` de novo) depois do teste: campo de 2 bits
  numa struct de visão (`DuelCardB15`), não `*(unsigned int *)c` com máscara (o mwcc reaproveita a carga). O máximo é
  `down = lvl < down ? down : lvl;` (`sltu at; beqzl; move`), não `if`.
- `tz == 0xC || tz == 0xD || tz == 0xB` com testes separados no original (`beql`/`beq` para o mesmo corpo) = `switch`
  com `case 0xB: case 0xD: case 0xC:`; o `||` vira `addiu -11; sltiu 3`.
- `(int)t >> 16 & 0xFFFF` dá o `ext x,16,16` do original (`t >> 16 & 0xFFFF` com `t` unsigned dá `srl`).
- Ferramentas (não versionadas, `build/tmp/req2/` da worktree `req2`): `hill.py` (descida gulosa sobre ordem e tipo
  das declarações, um TU por rodada) e `batch.py` (troca um trecho por N variantes, todas num TU).
Ainda em asm: `duel_req_move` (19: ti/sp trocados), `duel_req_move_card` (25: escalonamento depois da chamada),
`duel_req_swap_zones` (o original recalcula o endereço da zona A e guarda bo na pilha; o mwcc 219 faz CSE: 7
instruções a menos), `duel_req_host_3b_table` (7), `duel_req_end_turn` (15): nenhuma ordem/tipo/forma mudou.

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

## Rodada 9: família dk_/ds_/dt_/sc_ e `tools/find_twins.py`

`python3 tools/find_twins.py` lista as gêmeas entre módulos (palavras mascaradas idênticas, `=`; ou, para funções da
fila, ≥ 90% iguais a uma que já bate, `~`; `--ratio 0.85` alarga, `--all` mostra também grupos só de fila). Com o
grupo em mãos: case a cópia mais próxima, compare as fontes das outras com `sed` trocando o prefixo e aplique as
mesmas substituições (as quatro `*_draw_detail` e `*_sort_value` bateram assim).

| No asm original | Fonte |
|---|---|
| `addiu v1,cols,-1; addu v1,count,v1` | `(count + (cols - 1)) / cols` (o mwcc não reassocia `count + cols - 1`) |
| `y + q` primeiro, depois `- h / 2` | `y = y + hh * r / rows - h / 2;` (com `y += ...` a subtração vai antes) |
| `bXX; nop` de um teste e do `default` interno para o mesmo `li v0,K` no fim, e o primeiro (`b fim; li K`) com a cópia | `switch (x) { case 8: ...; break; default: goto none; }` e `default: none: v = K;` no `switch` interno (`*_sort_value`) |
| `case 0` de um `switch` que termina em `return 0` com `beqzl; move v0,zero` e o `default` interno `b; nop` | `break` no lugar do `return 0` do caso (cai no `return 0` comum: `dt_press_update`) |
| campos de uma tabela pequena carregados cada um com seu `lui` (sem base comum) | a tabela é `const` (`.rodata`, `// @rodata`): `dt_text_icon_spr` como `fmsg_cursor_spr` |
| `or v1,a2,v1` com o ternário das colunas como 1º operando | sem o local `view`: as duas expressões direto no argumento (`*_set_mode`, laço 1) |

Ao promover uma função com `switch` denso (`sltiu; lui; addiu; jr`), as tabelas de salto vão para a `.rodata` do
objeto: declarar `// @rodata` cobrindo-as (as duas tabelas de `*_draw_detail` ficam juntas, a do 2º `switch` primeiro).
Abertos (todas as cópias com a mesma causa): `*_draw_info` (3 palavras: `list` em `$s1` no original, aqui divide
`$s0` com `r`), `*_draw_rows[_small]` (12: o original carrega `row_w` depois da junção do ternário de `DK_ENT`),
`ds/sc_input_move` e `dk/dt_flight_undo` (3–5: lado 0/1 com `bnezl; li 1; move zero`, o mwcc 219 faz `sltu` ou
inverte o desvio); `*_set_mode` bateu na rodada 11 (ponteiro do painel antes das gravações no 2º laço).
## Rodada 9 (libtitle, libcharalist, librecipe_viewer): retornos, contadores e parâmetros copiados

Bateram `title_newgame_update`, `tfd_adhoctop_init`, `tfd_duelinfo_draw`, `tmenu_update`, `cl_costume_anim`,
`cl_draw_number`, `rv_build_list`, `rv_fill_grid`, `rv_apply_select`. Idiomas:
- **`case K: return;` ou `case K: r = -1; goto done;` num `switch` interno → `case K: break;`** quando o código depois do
  `switch` faz o mesmo. Com `break` o mwcc *move* o corpo para o atraso do `beql` (some do destino); com `return` ele o
  *copia* e os `b` vizinhos levam cópias (`li a0,1`, `li v0,-1`) que o original não tem. Um teste que salta para o
  bloco `return K` do `default` (e não para um `return` próprio) é `goto` ao rótulo dentro do `default:` (`tmenu_update`:
  `if (mode != 1) goto dflt;`). Uma mudança de uma instrução no topo deslocava 315 palavras.
- **Contador reaproveitado**: o 2º laço de `tfd_duelinfo_draw` e o último de `rv_build_list` usam o `i` da função (criado
  primeiro), não um contador próprio do bloco; um `int i` no lugar de `unsigned int` resolve `bgez`/`sltu`.
  O inverso também vale (`tmenu_update`: `int i, k, found;` no bloco de cada busca; `tpanel_draw_split`: contador novo
  declarado antes de todos no laço das abas, 114 → 39). `build/tmp/tcl/loops.py` (worktree `tcl`) faz a descida.
- **Coordenada como indução**: `x2 = 0xA6 + i2 * 0x3C` dentro do laço (o mwcc cria a indução no pré-cabeçalho, depois
  do teste de guarda) em vez de `x2 += 0x3C` no `for`; e `ps->card[i]` indexado em vez de ponteiro próprio.
- **Parâmetro copiado para um local** (`move sX,aY` depois de um laço de zeragem, não na entrada): `cx = x; v = n; if
  (v > 999) v = 999;` (`cl_draw_number`).
- **Valor de argumento que também vai para um campo** (`move t0,zero; sw t0,268(a2)` com `t0` = 5º argumento):
  `deck = 0;` antes das gravações e a chamada com `k->deck` (encaminhado da gravação), `tfd_adhoctop_init`.
- **`ptr = &arr[i]` declarado antes dos outros locais do laço** (`RvEntry *e = &c->main[i];` primeiro) quando o original
  calcula `e + 2` num `$s` antes do 1º uso (`rv_fill_grid`); `RvPos *p = &rv.pos[t]; p->top + p->cursor` quando o
  original lê os dois campos pela base do elemento (`rv_apply_select`).
- **Entrada da lista sem o inline**: `cl_entry()` (inline com dois `return`) deixa o resultado no `$s` mais baixo; o
  cálculo num bloco (`if (i >= cl.count[t]) e = 0; else e = &cl.list[t][i];`) dá a ordem do original.
- `x = (b = &tab[set])->campo` dentro do teste quando o original carrega o global comparado antes de calcular `b`.
- Promover uma função com `switch` denso exige `@rodata` da tabela: `title_newgame_update` (0x0882AD1C) e
  `tmenu_update` (0x0882AC40) deslocaram o módulo inteiro até entrarem no cabeçalho.
Sem forma encontrada (registradores/escalonamento): `tcfg_draw` (229: os dois `li 1` içados, `found = 0` no atraso, a
cópia de `rows[]` em pares), `tpanel_init` (2), `ll_move_grid` (6), `tfd_main_text` (8: `y + 16` antes do `move a1`),
`rv_draw_grid_cursor` (9), `tut_script_preload_faces` (8 depois de `int i` antes de `done`: o `!done` depois do
incremento de `tut_pc`), `tdl_rules_draw` e `ttab_draw` (o original usa `beqzl` para a instrução seguinte com o corpo
no atraso; o 219 dá `bnez; nop`), `tfd_chr_copy_draw` (o 219 junta os `case 2`/`case 3` iguais), `tfd_regulation_select`
(constante 1 num registrador comparada com `slt`).
## Rodada 9 (libfield: telas do PDA, `fld_scene_frame`, `fld_cell_marked`)

| No asm original | Fonte |
|---|---|
| máscara montada em tempo de execução (`move v1,zero; ori 1; ori 2; ori 4; sw`) que o mwcc dobra para `li 7` | `|=` direto no campo da struct: `s->padac[2] = 0; s->padac[2] |= 1; …` (os stores mortos somem depois da dobra; um local não serve) (`fld_scene_frame`) |
| argumentos guardados em `$s` (`(x + 9) << 6`, `480 - …`, `rows + i`) calculados **depois** do `jal` da última chamada-argumento; o mwcc os calcula antes | resultado da chamada num temporário de bloco e os argumentos depois: `{ unsigned short *t = table_get_entry(…); lw = …; lx = …; font_draw_text(lx, …, lw, t); }` (`pda_draw_status`, `pda_draw_options`: de ~450 para 0 palavras com isso e o item abaixo) |
| laços com várias variáveis de indução (`x + i*K`, ponteiro de tabela, `i*2`) só depois do teste de entrada | escrever as expressões em função do contador (`x + 3 + row * 0x36`, `pda_tab_names[i]`) e deixar o mwcc criar as variáveis de indução; com vírgulas no `for` elas saem antes do teste (`pda_draw_calendar`, `pda_draw_tabs`) |
| `$s` de contadores trocados em laços aninhados | trocar qual variável é o contador de fora e qual o de dentro (busca exaustiva das 4 escolhas por par de laços); um `tx` próprio no bloco de um laço o põe abaixo dos ponteiros de indução |
| `i*2` mantido como variável de indução e `+ 2` somado no uso (o C dava a variável de indução `i*2+2`) | `n <= (int)(i * 2 + 2u)` / `d = i * 2 + 2u - n` (`pda_draw_tabs`) |
| deslocamento em bytes como variável de indução e `base + desl` a cada volta | `(FieldCell *)((char *)b->cells + (unsigned int)(i * 4))` (com sinal reduz a ponteiro, com `sizeof` refaz o `sll`) (`fld_cell_marked`) |
| retorno de função sem `andi 0xFF` | conferir o tipo de retorno da definição: `fpda_cal_day` devolve `int`, o protótipo local dizia `unsigned char` (bug de fidelidade) |
| cópia `lh/lh/sh/sh` de um global de 2 halfwords para a pilha, o global no início do `@data` | struct de visão: `PdaText2 t = *(PdaText2 *)pda_times_text;` (um local inicializado põe o template no fim do `.data` do objeto) |
| `ty = y + 0x17` no atraso do `jal` seguinte | a atribuição logo antes do comando que a usa (depois das chamadas de fonte), não antes delas |
| `(x + 9) << 6` repetido e `tx` no mesmo `$s` | `tx = x + 9; if (i >= 2) tx += 7;` (não `tx = x + 0x10`) |
| ponteiro de dados (`gim`) e `list` com `$s` trocados | declarar o local inicializado por último |

Não resolvidos: ordem do `sw` do argumento de pilha contra um `move` de argumento (`fbm_icon`, `fld_digits`: 2
palavras, os dois em sentidos opostos); `move a0,v0; jal; nop` antes de `fld_voice_play` em `fld_scene_update` (1
instrução, desloca o resto); constante 0.5 em `v1` em vez de `v0` (`fld_scene_input`); cópias mortas `li s4,18`/`b; nop`
de `fbm_draw_marks` (família "cópia × movida"); `fld_dir_vec` (`lui` de `field_dirs` antes do `sw ra`, mas a tabela é
usada por duas funções, então não é `static` de função).
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
## Rodada 9 (BOOT: postfx.c, efs.c, cardlist.c, font_tags.c)

| No asm original | Fonte |
|---|---|
| parâmetro com o `$s` **mais baixo** (criado por último), os outros em ordem | parâmetro `void *in` copiado para um local tipado: `EfsInst *inst = in;` (`efs_cmd_inst`, 28 → 0); com o mesmo tipo no parâmetro não muda nada |
| valores novos de um bloco no `$s` de parâmetros já mortos (`ty` no registrador de `y`, `x - scroll` no de `mask`) | variáveis novas declaradas no bloco (`int sc, sx, ty;` no `if` do nome rolante), em vez de reatribuir o parâmetro (`card_draw_stats`, 72 → 0) |
| segundo laço sobre o mesmo vetor com `i`/`pos` e `id`/`s` trocados | ponteiro do laço num bloco próprio em volta do `for` (`{ int *pos2; for (...) }`) e `int id2;` como **primeira** declaração da função (`cardlist_draw_pair_at`, 47 → 0); o modelo do vetor local passa a sair do objeto: apague o global que o asm usava (`cardlist_pair_init`) |
| `edram + fb` calculado logo depois da chamada num `$s` (`addu s6,s0,v0`), não no uso | `frame = edram; frame += (*gfx_get_state() & 1) ? 0 : 0x88000;` (`frame = edram + fb` e o ternário inteiro são dobrados para dentro do uso); endereços das tabelas de vértices em locais atribuídos antes da chamada (`postfx_glow_a/b`, `postfx_blur`) |
| `move a0,sX` com o `u16` cru para um chamado e o `andi` só no teste | declaração K&R local `int wc_no_line_start();` dentro da função (o protótipo do header mascara e o mwcc junta a máscara do teste com a do argumento) (`font_ruby_layout`) |
| carga do 4º argumento antes do 2º (`lw a3,12(s0)` antes de `lw a1,4(s0)`) | tipo do parâmetro no protótipo local: `unsigned int slot` em vez de `int` reordenou as cargas (`font_layout_end`, 7 → 2) |
| `s6 = f + i*16` e `s0 = s6 + off` a cada volta | laço por índice com `c = &f->cmds[i];` no corpo, não ponteiro incrementado (`font_layout_end`, 273 → 7) |

Busca automática: gerar variantes inteiras da função (declarações em blocos, renomeações, ordens) e compilar
todas num só TU em lotes de 200 (`perm_decls.py --alts` estoura o limite de argumentos com mais de ~1000
variantes; o script `build/tmp/boot2/alts.py` da worktree boot2 lê as variantes de um JSON). 1100 variantes de
`cardlist_draw_at` (subconjuntos e ordens de locais movidos para um bloco) em ~3 min levaram 14 → 5 palavras.
## Rodada 9 (`duel_draws.c`, `duel_result.c`, `effects_0886AAB4.c`, `effects_088B7D94.c`, `field_active.c`)

| No asm original | Fonte |
|---|---|
| contador de laço num `$s` **acima** de um parâmetro copiado no prólogo (`move s3,a2` e o contador em `s4`) | o laço usa o **parâmetro não usado** como contador (`for (b = 0; ...)` em `duel_088B94F4(p, b, c)`): parâmetros são criados antes dos locais |
| uid de um evento recalculado em cada chamada depois de gravado no evento (o mwcc reaproveita) | `ev.uid = CARD_UID(*(DuelCard *)c);` (o cast quebra o CSE); campos `id` lidos pela base (`duel.players[p & 1].zone_d[i].id`) quando o original usa `928(s1)` (`duel_088C5E78`) |
| `addu v1,rec,i; sb a0,0xA0(v1)` sem ponteiro induzido | `*(unsigned char *)((unsigned int)rec + i + 0xA0) = 1;` (a forma com `[]` vira ponteiro de indução); `for (i = 0, best = rec; ...)` põe o `i = 0` antes da cópia (`duel_records_update`) |
| ponteiro de slot induzido e um local trocados de `$s` num laço por jogador | sem `pl = &duel.players[q & 1]`: `duel.players[q & 1].field[z]` direto; `1 << (z + q * 16)` em vez de um `bits += 16` próprio (`duel_0886B6B8`) |
| `beql id,K → corpo; b próximo` dentro de um laço | `switch (f(...)) { case K: ... }` de um caso só (vale também dentro de `for`) |
| argumento carregado antes da constante do delay (`srl a3; jal; li a2,K`) | protótipo K&R no bloco: `extern int duel_slot_effect_with();` (`duel_088C4864`) |
| redeclaração `unsigned int p;` num bloco igual à da função | apagar a do bloco: a variável nova muda a coloração (`duel_count_active`) |
| `$s` de laço trocados em muitas funções da fila | busca gulosa por laço (`build/tmp/misc/greedy_for.py` da worktree `misc`, não versionado): renomear o contador/variável de cada `for` para uma nova (topo, fim das declarações ou bloco em volta do laço), só quando a variável é atribuída antes de ser lida no laço e não é lida depois dele |
## Rodada 9 (`ai_*` de libduel_eng): constantes que o original não dobra, contadores de laço, posições de pilha

Bateram `duel_ai_atk_update`, `duel_0881657C`, `duel_08816A74`, `duel_0880DC1C`, `duel_0880E690`, `duel_0880C60C`, `duel_08827DA4`,
`duel_08828740`. Idiomas novos:
- **`r |= a; r |= b;` em comandos separados** (`duel_08827DA4`, `duel_08828740`): o mwcc dobra `r = r | a | b` (e
  `r = 4`) com `r == 0` conhecido, mas não dobra `r |= x`. Quando o original tem `or v0,s1,s0; or s1,v0,s5` /
  `ori s1,s1,4` num caminho onde `r` só pode valer 0, o fonte era `r |= a; r |= b;` / `r |= 4;`.
  `#pragma opt_propagation off` também reproduz, mas não é preciso. Nos `case` de retorno, `if (c) r |= x; return r;`
  dá o `or s1; move v0,s1; b; nop` do original (com `{ r |= x; return r; } return r;` o mwcc funde no `v0`).
- **Valor u16 mascarado depois do teste** (`andi s7,a1,0xffff` logo depois de `beqz a1; nop`): o id passa por um
  `int` (`faceup = slot.card.id; if (faceup != 0) { id = (unsigned short)faceup; ...`) com `id` declarado `int`; com
  `id` u16 o mwcc remascara cada uso.
- **Zona do índice sem CSE com a do evento**: `field[(unsigned char)a->zone]` no índice e `ev.zone = a->zone`, com
  `p` atribuído dentro do índice (`duel.players[(p = a->player) & 1]`), separou `z & 0xFF` de `a->zone` como no original.
- **Posição de pilha de um local cujo endereço é tomado**: `*(unsigned short *)&atk` põe `atk` no topo da área de
  locais; o original o tinha embaixo de tudo: era `(atk & 0xFFFF)` (o mwcc relê o derramamento com `lhu`).
- **Ordem de criação na inline**: os `$s` da chave inline de `duel_0880DC1C` bateram com `int i;` antes dos outros
  locais e o parâmetro copiado para um local (`int zz` → `int z = zz;`): parâmetros são criados antes dos locais.
- **Mesma inline duas vezes com `$s` diferentes**: em `duel_0880E690` o segundo `duel_ai_view_refresh()` só bateu
  escrevendo os dois laços na função com as **mesmas** variáveis `q`/`t` (vidas unidas mudam a coloração).
- **`if (x < 0 || duel_rand(2)) x = i;`** em vez de `if (x >= 0) { if (rand) x = i; } else x = i;` (`bltzl` para
  depois da atribuição + `move; b; nop`), e `if (keep) return -1; return other;` (`duel_08816A74`).
- **Cópia de struct com campos escalares**: `duel_0880C60C` copia `duel_special` com `lw/sw` nos 11 primeiros
  campos (escalares no original; um `int[2]` sai `lwc1/swc1`), laço para `act[2]` e `lwc1/swc1` para os arrays
  pequenos. Uma struct de visão com os campos na forma do original só se decompõe se a cópia for de um **global**
  (`extern DuelSpecialCopy duel_special_save;`, alias em `symbols/`); por ponteiro (`*(T *)&g`) sai cópia em bloco.
- Laços: `for (i = 0; i <= 4; i++)` (`slti at`), `pb` atribuído no `for` (`for (i = 0, b = pb = p << 4; ...)`) para o
  `sw` do derramamento sair depois de `i = 0`, e a descida por contadores de laço (um nome novo por laço/jogador).
- `duel_0880C60C` também: `forced = 0;` depois da cópia (não na declaração), `n = g->dmg_opp > 0; if (n == 0) n =
  g->won > g->lost; g->ok = n;` (o `sw` comum com `bnezl` + cópia), campos de `st[p][z]` sem ponteiro, `atk > def`,
  `level <= 4`, a ordem dos `case` pelos corpos e `(int)` no `+=` de um campo para a ordem das cargas.
- Tabelas de salto de um arquivo que sai inteiro do asm: as duas tabelas de `ai_08827DA4.c` são contíguas
  (`@rodata 0x08907908-0x08907940`); com só a primeira, o `make check` acusa a `.data` deslocada.
- Ainda em asm: `duel_0880C15C` (16 linhas: o mwcc agenda `tz * 44` e o `mult` do jogador antes de `(1 - p) * 220`;
  operandos trocados, ponteiro de linha, índice achatado e `ZSLOT` não resolvem), `duel_ai_sum_088239EC` (875 contra
  846 instruções: o original relê campos de `a` (`lbu 0(s0)`) depois de chamadas/gravações que o C atual reaproveita,
  além das cópias mortas dos `?:`), `duel_08817598` (não tentado).
## Rodada 10 (BOOT, libdeck, libstory, libtitle): ordem de criação também por atribuição e por uso

| No asm original | Fonte |
|---|---|
| contador e ponteiro de laço com `$s` trocados só no 2º e 3º laço de uma série de blocos (`dk_deck_load`) | `dst` e `i` **da função**, compartilhados pelos blocos, e só `id` em cada bloco (`DkList *dst; unsigned int i;` no topo, `dst = &dk_lists[k];` em cada bloco) |
| `li` de uma constante no atraso de um `bnezl` (o mwcc dá `bnez`) | inicializar o local **depois** do primeiro teste (`int i; if (day > 99) return 0; i = 1;`), não na declaração (`day_threshold`) |
| locais inicializados na declaração com `$s` acima do esperado | declarar sem inicializar, na ordem dos `$s` (do mais alto ao mais baixo), e atribuir depois de todas as declarações (`st_mail_open`: `int k;` primeiro, `p = st_mail_find(id) + 10;` depois) |
| variáveis de indução (`x+8`, `x+10`, `x+0x22`, ponteiro da tabela) em ordem diferente | o mwcc as cria na ordem do primeiro uso no corpo: `x = i * 0x74 + 0x22;` entre a 2ª chamada e o acesso à tabela (`tfd_adhoc_pass_init`) |

Busca: `build/tmp/bootdeck/alts.py` (worktree bootdeck) compila variantes inteiras de uma função em lotes de 150
num só TU (cabeçalho = o `.c` sem `NON_MATCHING`); `greedy.py` faz a descida gulosa movendo cada declaração
(topo, outra posição, bloco marcado `{BLK}`): `movie_run` 35 → 20.
Sem forma: `font_layout_end` (2: `n * 32` em v1), `cardlist_draw_at` (3, escalonamento dos argumentos),
`snd_group_cmd` (1: `bne v0,s3` com operandos invertidos, tipos/ordens/formas testados), `*_draw_info` (3, `list` e `r`
dividem `$s0`; ordem de declarações, `r` em bloco, ramos invertidos testados), `vm_skip` (10: `depth`/`ca` trocados,
3600 variantes), `snd_obj_begin` (43, a descida gulosa não mexe).

## Rodada 10 (gêmeas pequenas: libshop, libtutorial, libfield, libcharalist)

Bateram `shop_msgwin_draw`, `tut_text_draw` (forma de `dt_text_draw`), `fmsg_create` (`dt_text_build`), `cl_draw_portrait`
(`dmgr_pt_draw_chr`), `pda_draw_frame`, `fdate_draw`, `fld_bg_draw`, `shop_glow_super`. Idiomas:
- **Tabela de ícone `const` mesmo num módulo sem `.rodata`** (`tut_text_icon_spr`): cada campo com seu `lui` só sai de
  objeto `const`; nos gmodules o `.rodata` do objeto vai logo depois do `.data` dele, então a tabela no fim do `@data` serve.
- **Posições de pilha de structs locais**: o mwcc agrupa por alinhamento; um `{ Vec3; int w; }` sem `aligned(16)` vai para
  depois dos `Vec4` (`fld_bg_draw`). Com o mesmo alinhamento, a ordem das declarações decide (`perm_decls.py`).
- **`$f` trocados em `240 - w * 0.5f`**: a largura num `int` local antes da conversão (`fdate_draw`).
- **Argumentos `u + 8`/`v + 8` depois de `x + 8`/`y + 8`**: reaproveitar locais mortos (`rx = u + 8; by = v + 8;`) (`pda_draw_frame`).
- **Contador por laço interno** declarado no bloco do laço de fora, depois do ponteiro do elemento (`ShopGlowOrb *o = …; int jb;`):
  resolveu a rotação de `$s` dos três laços de `shop_glow_super` (busca em `build/tmp/twsmall/gl.py`).
- Cuidado com `sed` sem âncora ao trocar um argumento: `menu_item_rect(&m->menu, d, …)` aparecia em outras três funções e
  quebrou duas que batiam (o `check_file` acusa).
Sem forma: `fld_scene_update` (`move a0,v0; jal; nop` de `fld_voice_play`), `fld_chr_draw_model` e `fclk_time_draw` (ordem
de dois `lw` de argumentos vizinhos, 4 palavras), `pda_draw_tabs` (4: `bne`+`b; li` contra `beql`).

## Rodada 10 (`engA`: libduel_eng, funções de 99% fuzzy)

Bateram `duel_08865658`, `duel_0884E02C`, `duel_0888293C`, `duel_088C914C`, `duel_0880FFAC`, `duel_088C7954`,
`duel_0888B434`, `duel_088B6A04`. Quase todas eram `$s` trocados que nenhuma ordem de declaração resolvia:
- **O tipo `int` × `unsigned int` de um local decide a coloração** mesmo sem mudar nenhuma instrução: `int p` em
  `duel_08865658`, `int id` em `duel_0880FFAC`, `int q` em `duel_088C7954`. Quando a permutação das declarações
  não mexe em nada, varie os tipos (um local por vez) e depois **tipos × ordens juntos**: `duel_0888B434`
  (`int z0` + ordem) e `duel_0888293C` (`int z`, `int qz` + ordem) só bateram na combinação.
- **Parâmetro não usado como contador** (idioma da rodada 9) também nos três laços de `duel_088B6A04(p, b, mode)`,
  com `b` `unsigned int` na definição (o `sltu` do teste).
- **`ZSLOT` com `sz`/`sp` de bloco** num `case` que cai no seguinte: `case 6: { unsigned int sz = …, sp = …;
  if (ZSLOT(sp, sz)->card.id != K) break; }` (`duel_088C914C`).
- **Registrador de argumento "estranho" para uma global** (`lw a2,duel_w_08919650` usado num `switch`): a
  chamada seguinte recebe esse valor como argumento a mais. `duel_0884E02C` chamava `duel_088348BC(0, p)`
  sem o 3º argumento (zona) — bug de lógica; a mesma chamada sem argumento está em `chain_prompt.c`.
- **`switch` interno cujos `case` saltam para um `b` comum** (sem threading até o fim): o `case` externo termina
  em `return;` em vez de `break;` (`duel_0884E02C`). Ao promover um `switch` denso, `@rodata` da tabela.

## Rodada 10 (`libduel_eng`: quase-acertos)

Bateram `duel_req_move`, `duel_088CFBA4` (contador `q` e jogador `v` no case 0x150E, como no 0x12BE), `duel_088B1854`, `duel_0880A54C`, `duel_088B0C88` e `duel_req_088D31F0`. O que resolveu:
- **Varredura de posição de 1 ou 2 declarações** (`duel_req_move`, 19 → 0): mover só as duas variáveis trocadas
  (`ti`, `sp`) para todas as posições da lista (N² variantes num TU por vez, segundos) achou a ordem que a busca de
  permutações completas não achou (`sp` primeiro, `ti` logo depois de `tp`). Barato; tente antes de permutar tudo.
- **Contador reaproveitado de outro laço** (`duel_088B1854`: o `k` do laço de uids vira `q`; `duel_req_088D31F0`: os
  laços em `z` passam a usar `i`, e `z` some): a descida de nomes por laço (`ren.py`) inclui "renomear para um local
  existente que não aparece no laço".
- **Argumento que faltava** (bugs de lógica): `duel_088348BC(0, p, duel_w_08919650)` em `chain_prompt.c` (o `a2` da
  diferença era a zona comparada no `switch`; `duel_end.c` tem a mesma chamada com 2 argumentos) e
  `duel_slot_ef098(p, z, a->id)` em `ai_util.c` (protótipo K&R escondia o 3º argumento). Diferença em `a2`/`a3` de
  registrador "trocado" logo antes de um `jal`: confira o número de argumentos antes de mexer em registradores.
- `case A: case B: v += 500; break; case C: v += 500; break;`: o original tinha os três rótulos no mesmo corpo.

## Rodada 11 (`engC`: libduel_eng, quase-acertos)

Bateram `duel_0880ADF4`, `duel_0882AAA0`, `duel_08830E94`, `duel_0886268C`. Idiomas:
- **Global que é campo de `duel`** (`duel_08830E94`): o original relia `duel_w_0891968C` depois do `sh` em
  `duel.unk3832[k]`; o mwcc só deixa de adiantar a carga quando os dois acessos são do mesmo objeto. Esse endereço é
  `duel + 0x3830` (`duel.unk3830`): usar o campo. Vale conferir `endereço - 0x08915E5C` de toda global de libduel_eng
  que cai dentro de `Duel` quando uma carga sobe acima de um store.
- **Ponteiro local para um campo × expressão repetida** (`duel_0880ADF4`): `unsigned int *n = &…n_hand` dava `$s`
  trocados em qualquer ordem/tipo (3840 variantes); `duel.players[q & 1].n_hand` nos dois laços (CSE do mwcc) bateu.
- **`(c ? 1 : 0) + 1`** em vez de `(c) + 1` mudou a coloração de um temporário (`duel_0882AAA0`).
- **`ZSLOT` com `sz`/`sp` de bloco** num `case` que só usa a zona para o endereço (`duel_0886268C`).
- Busca em lote: `build/tmp/engC/varb.py ARQ FUNC VARIANTES` (worktree engC, não versionado) compila centenas de
  variantes inteiras da função num só TU (cabeçalho = o `.c` até a função) e dá as palavras diferentes de cada uma.
Sem forma (rodada 11): a família "contador de bits × ponteiro do slot" (`duel_min_order`, `duel_find_effect_card`,
`duel_count_id_flag18`, `duel_count_monster_active`: ordem/tipos/bloco/ponteiro explícito/`ZSLOT`/inline testados),
`duel_wfmt_s`/`duel_wfmt_d` (`dst` e o 3º parâmetro trocam `$s0`/`$s2`; K&R, `register`, cópias locais, formas de
laço testadas), `duel_08858440` (prólogo `lhu` de `a->kind`, como `duel_08856860`), `duel_08863780` (o mwcc sabe
que `v0` vale 1 no `default`), `duel_piles_swap` (o original soma `sp+24` e `+6` em dois passos; o mwcc dobra em
qualquer forma testada), `duel_bonus_calc` (36: 1680 posições de declarações sem efeito).
## Rodada 11 (`bfs`: BOOT, libstory, libfield, libshop)

Bateram `gamestate_fill_partner_list`, `heap_create`, `vm_skip`, `vm_update`, `movie_run`. O que resolveu:
- **Contador de outro laço reaproveitado** também em `$a`/`$t` (não só `$s`): os três laços do deck usam o `k` do laço
  dos parceiros e `j` some (`gamestate_fill_partner_list`; ordem/tipos de declaração não mexiam em nada).
- **`x - x` que o original não dobra** (`subu v0,a0,a0`): a cópia passa por `+ 0` (`h = (Heap *)((char *)mem + 0);
  h->pad = (char *)h - (char *)mem;`); `h = mem` ou atribuição encadeada dobram para 0. Bitfield de 6 bits gravado com
  `li 252` sem os registradores do `0x3F`: `h->pad = -1` (`heap_create`).
- **`default:` primeiro no `switch` interno** com `return K` (`vm_update`); o `@rodata` da tabela do `switch` externo
  pode cair num `data_N` sem dono (antes do `@data`): declarar `// @rodata` e regenerar o config.
- **Argumento que faltava**: `lw a0,reader; beqzl a0` antes de `jal pmf_reader_term` = `pmf_reader_term(reader)`
  (definição com parâmetro não usado). Busca de posição de 2 declarações + `(int)mem + es_size` + mover um comando de
  zeragem (`avc_es = 0;` logo depois de `mem = ...`) levou `movie_run` de 20 a 0.
Sem forma (skip): `fld_chr_collide`, `fld_player_update` (e `fpt_update`, `fld_draw_add`, mesma família: o original
divide `q.x / 0.5f` antes de carregar `q.z` reusando `$f0`; locais, inline, ponteiro, cast, ordem e `float h` não
mudam), `tms_draw_prims` (7, `d` em v0), `pda_draw_mail_item` (13, `face` em t0, arg 4 = 0 materializado tarde),
`fld_map_walkable` (5, último teste `beqz; nop` + bloco `r = 0` próprio), `gim_walk` (cópia morta do `case 0xFF`),
`font_glyph_cache` (60, `f`/`font` em s1/s5 trocados; posição e inicialização das declarações não mudam),
`shop_shelf_input` (testes 1 depois 2 com corpos na mesma ordem; nenhuma forma de switch/if chegou perto).

## Rodada 12 (`engE`: libduel_eng, funções 97-98% nunca tentadas)

Bateram `duel_0888D74C`, `duel_088C8F04`, `duel_eff_f9c00`, `duel_088766B0`, `duel_eff_fdaf8`. Idiomas:
- **`a->player`/`a->zone` direto em vez de locais `p`/`z`** (`duel_eff_fdaf8`, 26 → 0): os temporários (`$v`/`$t`)
  só mudam assim; o mwcc reaproveita a leitura de `a->player` até o `case` que a usa (sem chamadas no caminho).
  Tipos e ordem dos locais não mexiam em nada (36 variantes iguais).
- **Variável própria para um valor da 2ª metade** (`duel_0888D74C`: o uid de `a->idx >> 1` não é o `uid` do ramo
  de cima; 25 → 5) e soma com o resultado de uma chamada num local (`int c = f(); g(c + uid * 2)` dá `addu v0,v0,v1`).
- **Retorno comum por queda**: `if (f()) { ... }` sem `return 0;` interno, caindo no `return 0` de fora, dá o
  `beqz; nop` para o bloco `move v0,zero; b fim` do original (com o `return 0` interno o mwcc faz `beqzl` direto).
- **`if/else if` sem `default`** (`duel_eff_f9c00`): `if (k == 6) { switch ... case 0x7D: ...; return 0; } } else if
  (k == 15) ... return 0;` dá o `b fim; move v0,zero` do `switch` interno e o `move v0,zero` próprio no fim.
- **Declarações sem inicializador + ordem das atribuições** (`duel_088C8F04`: `next, sp, sz, v, k` declarados e
  atribuídos `sp, next, k, sz`; busca em 600 de 2880 combinações de ordem × tipo × ordem das atribuições).
- **Contador reaproveitado** de novo (`duel_088766B0`: o 2º laço usa o `i` do 1º) e `case 0xB: case 0xC: case 0x10:
  return 1;` num corpo só (o `beql` do 1º teste leva a cópia, os outros `beq; nop`).
- `duel_088C6BC8` (75 → 54): o parâmetro não usado `i` é o `grave` do original.
Sem forma: `duel_count_tributes` (3 instruções a menos: cópias mortas depois de `b` no `blezl` do início e no
`beql` do `case 0x194C`, e `b; nop` sem preenchimento depois do `xori`; `else`/`goto`/`switch` no teste do início
e 5 formas do `case` testadas).

## Rodada 12 (`engD`): `bnezl` + `li K` de um `return K` antecipado

Quando o original faz `bnezl …; li v0,K` para um `return K` no começo e o mwcc gera `beqz; nop; b; li`, inverta
o teste e deixe o retorno por último: `if (!A || !B || !C) { …resto… } return K;` (`duel_0888BF08`). Outros
achados: o 5º argumento de `duel_088C87A8` é o byte do jogador (`duel_088859A8`, era bug de lógica); tabelas locais
inicializadas (`t0[side][k]`) pedem `// @data` com os modelos (`duel_tag_slot`).
`bootx`: `- 12U` em `x + 20 + (d + 2) - 12U` impede o mwcc de dobrar `20 - 12` e reproduz a aritmética do original
(`shop_shelf_cursor`); em `ehp_unload`, atribuir o ponteiro auxiliar só depois do teste de entrada do laço põe o
ponteiro da entrada no delay slot do primeiro desvio.

## Rodada 13 (`engF`: libduel_eng, funções 95-98% nunca tentadas)

Bateram `duel_088ADF28`, `duel_088ABE60`, `duel_hand_add`, `duel_088CA184`, `duel_088C28A4`, `duel_0888EB8C`,
`duel_0886D828`, `duel_08887B34`. Idiomas:
- **`void` que era `int`** (`duel_088ADF28`, 19 → 0): `li v0,1; sh v0,…` no fim (e `v0`/`v1` trocados em todo o
  corpo) é `a->w[0] = 1; return 1;`; o valor de retorno ocupa `v0` e muda a coloração dos `$s` também.
- **`if (c != 0 || bit) return;`** dá `bnez c, fim; nop` + `beqz bit, corpo; nop; b fim; nop` (`duel_088C28A4`);
  `if (c == 0 && !bit) {…}`, `switch` de um `case` e `if` aninhados não dão.
- **Global que é campo de struct** (`duel_w_0891525C` = `duel_w_08915258.cards`, `DuelCandList`): com o struct e um
  ponteiro local (`c = duel_w_08915258.cards; o = c->owner; f(o, c, …)`) sai o `lui; addiu a1` antes do `lbu 1(a1)`;
  pelo símbolo do array o mwcc gera dois `lui` (`duel_08887B34`). A jump table nova do `switch` estendeu o `@data`
  do arquivo para trás (`0x089117B8`): o `.rodata` do objeto vem logo depois do `.data` vazio.
  Cuidado: tirar uma função do `@asm` exige as funções do `.c` em ordem de endereço; `duel_08887ED8` estava antes de
  `duel_08887B34` e `duel_088C9878` antes de `duel_088C96EC` (`duel_reqs2.c`), o que só aparece no `make check`
  (jals deslocados), não no `check_file`.
- **Zona antes do jogador num ponteiro da mão** (`duel_hand_add`): `((DuelPlayer *)(*n * sizeof(DuelCard) +
  (unsigned int)&duel.players[p & 1]))->hand` (mesma ideia do `ZSLOT`).
- **Valor do teste num local do topo** (`duel_088CA184`): `unsigned int pb = b & 0xFF;` declarado junto com `pa`,
  `za`, `zb` (busca de ordem) e `if` aninhados dão o `beqz` com o `andi` no delay slot do original.
- **`?:` no corte** (`v = v > 0xFFFF ? 0xFFFF : v;`) dá `bnez; nop; li` sem likely; `atk = f(); v = atk * n / 10;` dá
  a ordem dos operandos do `mult`; `p`/`z` do bloco do `if` corrigiram os `$s` (`duel_0888EB8C`), e `default:
  break;` + `else if` levaram o `default` do `switch` direto ao `return 0` comum.
- **`(unsigned short)id` nas duas chamadas** (CSE do `andi` num `$s`) e o contador `i` de outro laço reaproveitado
  (`duel_0886D828`, 233 → 0: o frame caiu de 32 para 16 bytes).
Sem forma: `duel_088D1F1C` (8: o original copia o `lbu a0,a->player` nos dois caminhos do `||` e o mwcc copia o
`xori` de `grave`; ordem/tipo dos locais, forma do `!`, posição de `opp` e do laço de zeragem testados),
`duel_088A6F00` (108-115: coloração `p`/`a`/`k` rodada; ordem e tipos das declarações sem efeito).
## Rodada 13 (`engG`: libduel_eng, funções 95-97% nunca tentadas)

- **`andi 0xFF` num bit de bitfield passado como argumento** (`duel_0884C8E4`, 8 bytes a mais): declarar o callee
  **sem protótipo** (`unsigned int duel_grave_find_uid();`) tira o `andi` e põe a leitura do `uid_hi` antes da do dono.
- **Ponteiro de slot que anda a partir da base do jogador** (`addu s1,duel,p*4012` e acessos `54(s1)` = `field[0].pos`
  com `s1 += 20`) com o bit da zona `b` num `$s` abaixo dele: `DuelPlayer *s` avançado de `sizeof(DuelSlot)` por volta
  (`s->field[0]` é `field[i]`), iniciado no `for` **depois** de `b` (`for (i = 0, b = p << 4, s = &duel.players[p & 1];
  ...)`) e a carta num local de bloco (`int c = field_get_card(p, i); if (c == id ...)` dá `bnel v0,sN`; `id == c` não)
  (`duel_count_monster_active`, `_atk`; `duel_find_effect_card` 10 → 2).
- **Globais que caem dentro de `duel_chain`** (`duel_w_089149F0/F8` = `+0x6A0/+0x6A8`, `duel_w_089147E4/E8` =
  `uids_head`/`uids`): acessados como campos de `duel_chain` (macros `CHAIN_N`/`CHAIN_I` sobre `pad638` em
  `ai_0882D398.c`), o mwcc relê depois de cada store no anel como o original, sem `volatile` (`duel_0882E1B8`, 23 → 0).
  Só nos trechos que releem: no passo 0 os stores a `ids[]` precisam continuar pelos globais.
- **`beql` para o epílogo com o `li` no atraso e os outros testes `beq; nop` para um corpo `li v0,1; b fim`**: é um só
  `case` com todos os ids (o 1º teste é encadeado até o epílogo), não dois grupos com `return 1` (`duel_08810D08`);
  `n_hand` relido na condição do `for` em vez de num local.
- Halfword `jogador | zona << 8` testado com `andi 0xFF` e `ext 8,8` depois de `andi 0xFFFF`: ler o halfword num local
  `unsigned int w` e usar `w & 0xFF` / `(unsigned short)w >> 8 & 0xFF`; `(unsigned int)ef->kind << 8` fixa a ordem do
  `or` (`duel_setup_load_text`).
- Busca de declarações: tipos × ordem × ordem das atribuições (`duel_ai_0881A7C0`: `int id; int uid; int n;` na
  função, `id` atribuído antes de `uid`).
## Rodada 13 (`menus`): gêmeas de `*_set_mode`, soma sem sinal, chamada indireta com float

- **Protótipo da chamada decide os temporários** (`ds_set_mode`, `sc_set_mode`, 56 → 0): além da forma de
  `dk_set_mode` (ponteiro do 2º laço antes das gravações), o 2º parâmetro de `*_layout_scroll` tem de ser `int` como
  em libdeck; com `unsigned short` o mwcc carrega `u1a` antes do ternário e sobram 36 palavras.
- **Soma que o original não dobra**: `x = x + 5U` (`cl_draw_detail_frame`) e `x = (d + 2) + (x + 20) - 12U` com `x`/`y`
  reaproveitados (`shop_shelf_cursor`): o sufixo `U` corta a dobra de constantes; a ordem dos operandos decide qual
  `addiu` sai primeiro.
- **Contador incrementado no corpo e antes do `continue`** (`shop_shelf_icons`): `pack++` dentro do laço, não no
  incremento do `for`, dá a ordem `pack`, `col` dos `addiu`.
- **Constantes de troca como locais** (`tpartner_update`): `f2 = b2` (não o literal) e as declarações `f1..e1` antes de
  `a1..b1` puseram `b2` na pilha como no original; `mv.py` (mover 1 ou 2 declarações) achou de 55 para 3.
- **Chamada indireta com float** (`dd_tasks_update`): o original monta `a0` antes de `$f12`; o tipo do ponteiro tinha
  `dt` por último (mesmos registradores na EABI). Cast sob `#ifdef __MWERKS__`, chamada portável no `#else`.
- **Sem solução**: `dk/dt/sc_draw_rows(_small)` (o mwcc 219 põe o `lhu` de `row_w` da junção no atraso do teste
  do ternário em vez de fazer `bnezl` com o `sll` do ramo; 12 palavras em todas); `snd_curve_volume` (6, posição do
  `lhu vol_far`); `menu_layout` (67, cópia desenrolada).
## Rodada 14 (`engI`: libduel_eng/mgr/draw, funções 94-98% nunca tentadas)

Bateram `dmgr_partner_update` (gêmea de `tpartner_update`; `dmgr_partner_draw` saiu junto, só esperava as jump tables:
`@rodata` estendido), `duel_088C8DC4`, `duel_ai_0881D17C`, `duel_req_088D6168`, `duel_ai_0881F58C`, `duel_0884D72C`,
`duel_08843AE0`, `duel_0882F320`, `func_088250E8`, `dd_m1_update`. Idiomas:
- **Dois `return 1` que compartilham um bloco** (`li v0,1; b fim; nop` sem likely, com outro teste `bnezl … li v0,1`
  antes): é um `if` só com `||` (`if ((a && b) || c) return 1;`, `duel_0884D72C`). O `li` não vai para o atraso porque
  o bloco é destino de outro desvio.
- **`bnel x,y,fim; li v0,1` + `b fim; move v0,zero`**: `if (x != y) goto ret1; return 0;` com `ret1: return 1;` no fim
  da função (`duel_ai_0881F58C`; if/else, `==`, `?:` e `switch` não dão).
- **Global dentro de `Duel` com duas máscaras seguidas** (`and -3; and -2` num só `lw`/`sw`): `duel.mode &= ~2;
  duel.mode &= ~1;` (`duel_w_08917DB4` = `duel.mode`); pelo símbolo o mwcc funde as máscaras.
- **`&pl->field[z]` num `$s` a mais**: o original lê um dos campos por `duel.players[p & 1].field[z]` (`duel_08843AE0`).
- **Zona num `$s` e o halfword mascarado no outro trocados**: recalcular `loc >> 8 & 0xFF` no argumento em vez de
  guardar num local (`duel_088C8DC4`).
- **`slt v0` (não `at`) com o campo lido depois do `?:`**: `((volatile T *)&g)->n >= max` (`duel_0882F320`);
  `max <= n` lê na ordem certa mas dá `at`, `n >= max` dá `v0` mas o mwcc antecipa o load.
- **`beqzl v0; move s,zero` + `b; nop` + `move s,zero`**: `card = f() ? card : 0;` (`dd_m1_update`).
- **Struct de bitfields copiada por `lh/sh`**: `*(short *)&l2 = *(short *)&l;` (`l2 = l` copia campo a campo).
- **Função `void` que devolve o `dl`** (`func_088250E8`, tipo `MenuDrawRectFn`): `return dl;` e
  `th / 2 + (y + 20) + 1U` (o `U` impede `y + 21`; a ordem dá `addu s0,v1`).
