# Matching das funções em `@asm` — instruções de trabalho

Desde 2026-10-30 todo o `.text` de todos os módulos está em C. O que falta para a fidelidade são as funções
mantidas como `// @asm A-B`, com o C equivalente sob `#ifdef NON_MATCHING`. Este arquivo é o ponto de partida
de cada sessão dessa etapa: leia só ele (e o trecho de `docs/build.md` que ele indicar), não o repositório inteiro.

Estado em 2026-11-04 (ByteMatch): 63,9% do código batendo, 91,1% fuzzy, 9570/11039 funções. Fila: 1131
funções em `docs/matching_queue.tsv` (274 abaixo de 80%), fora o SDK do BOOT (inclusive arquivos com um só `@asm` cobrindo
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

## Não-corrigíveis conhecidos (registre em `docs/matching_skip.tsv` e siga)

- **Cópia morta do destino depois de um `b`** (`bnezl/beql; b; <delay>; <cópia da 1ª instrução do destino>`):
  o mwcc 219 remove em if/else, goto, inline, pragmas e no build 121. Só sobrevive dentro do corpo de um `case`
  e no último termo de `||` seguido de `return`. Tente essas duas formas **uma vez**; se não der, pule.
  É o padrão de ~2000 pontos do libduel_eng, o maior bloqueio aberto.
- **`static` local** (flag em `.bss` do objeto): as ferramentas não têm `@bss`; um global de arquivo não
  reproduz o agendamento. (As 29 buscas de ossos de `dd_chr_models.c` já batem com `-DNON_MATCHING`; ficam em
  asm só por isso e não estão na fila.)
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
