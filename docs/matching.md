# Matching das funções em `@asm` — instruções de trabalho

Desde 2026-10-30 todo o `.text` de todos os módulos está em C. O que falta para a fidelidade são as funções
mantidas como `// @asm A-B`, com o C equivalente sob `#ifdef NON_MATCHING`. Este arquivo é o ponto de partida
de cada sessão dessa etapa: leia só ele (e o trecho de `docs/build.md` que ele indicar), não o repositório inteiro.

Estado em 2026-10-30 (ByteMatch): 60,4% do código batendo, 86,9% fuzzy, 9353/11039 funções.
Fila: 1436 funções em `docs/matching_queue.tsv` (fora o SDK do BOOT e as de `docs/matching_skip.tsv`).

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
| cadeia de `if` longa | era um `switch`; o primeiro teste de cada grupo vira `beql` |
| `$s` trocados entre duas variáveis | ordem das **instruções** (não das declarações) decide; reutilizar uma variável antiga para um valor novo; separar uma variável usada em dois ramos |
| campos carregados para fora do laço em posições de pilha trocadas | ordem das atribuições no primeiro uso (`perm_stmts.py`) |
| valor calculado antes da chamada no original | atribua o temporário depois da chamada, em vez de inicializar na declaração |
| global recarregado por `lui` vs base em `$s` | `T *const p = &g;` declarado primeiro; ou primeiro acesso pelo global e o resto por `p` |
| `slti at` vs `slti v0` | `a > b`/`a <= b` dão `$at`; `b < a`/`a >= b` dão temporário; `x < 0` → `bgez`, `x <= -1` → `slti at` |
| `andi 0xFFFF`/`0xFF` a mais ou a menos | tipo do parâmetro (`unsigned short`/`char`); cast `(int)` num bitfield de 1 bit; protótipo ausente no original (`int f();`) |
| `seh` a mais | `(short)(int)expr` |
| ordem dos `mtc1`/`a0` numa chamada | ordem dos parâmetros no protótipo (int antes de float, ou ao contrário) |
| cópia de struct pela pilha que o mwcc propaga | bloco com `T tmp = {...}; x = *(T *)&tmp;` |
| local `Mat4`/array acima dos outros na pilha | `__attribute__((aligned(16)))` **na variável**, não no typedef |
| laço com teste no topo e `b` de volta | `for (;;) { if (!c) break; ... }` (o `while` é rotacionado) |
| zerar bytes com `beqz p; li n; sb/addiu/bnez` | `if (p) do { *d++ = 0; } while (--n);` |
| campo carregado depois da aritmética do índice | `(T *)((char *)base + i * sizeof(T))` |
| `sltu v0,zero,v0` logo após `jal` | `int on = f() != 0; if (on)` |
| faixas de `case` em árvore de `slti` | `case A ... B:` (o mwcc aceita) |

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
por symlink e copiar o resto de `build/`). Merge: `symbols/<m>.txt` em conflito se resolve por endereço
(`build/tmp/resolve_status.py`, que descarta linhas `alias`: confira-as depois), `config/<m>.yaml` se regenera
(`python3 tools/splat_config.py <m> > config/<m>.yaml`), e `make MODULE=<m> verify` antes de commitar o merge.
