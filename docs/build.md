# Build reproduzível (fase 3)

## Resultado

Todos os 23 módulos (`BOOT` + 22 gmodules) são reconstruídos **byte a byte** a partir do split do splat (`make check-all`). Código já em C compilado pelo compilador original e linkado no lugar exato: `libhtml_view` (2 funções) e, no `BOOT`, `src/BOOT/ehfile.c` (VFS, 29 funções, 0x0882ECAC–0x0882FC3C) `src/BOOT/modmgr.c` (gerenciador de módulos + espera de frame + menu de depuração, 19 funções, 0x08859044–0x08859824, com a tabela estática de módulos em `.data`) `src/BOOT/sys.c` (thread de cenas `ehscript`, `sys_init`, pilha de estados, 17 funções, 0x0883CEB0–0x0883D708) e `src/BOOT/main.c` (`main`, laço de frame, reset, overlay de loading, acessores dos globais do jogo, 27 funções, 0x0883DD58–0x0883E670, com `dir_table`/`game_module_table`/`sys_config` em `.data`).

## Compilador

- Binário: `mwccpsp.exe` **3.0.1 build 219** (`tools/ext/mwccpsp_219/`, da coleção do decomp.me), executado com `wibo` 1.1.0. O `.comment` dos módulos diz "MW MIPS C Compiler (2.4.1.01)", mas esse texto é o do front-end e é igual em todos os builds 3.0.1; o build 219 gera código idêntico nas funções testadas e o build 121 não (4 palavras diferentes em `save_path_build`).
- Flags confirmadas: **`-O4,p -sdatathreshold 0`**. `-O4,s` bate nas funções sem laço mas não em `ehfile_busy` (0x0882F468): com `,s` o laço indexa (`sll`+`addu`), com `,p` incrementa o ponteiro como no original. `-O2` não bate.
- Include path: `-i include` (o `-I` do mwcc exige `-Ipath` sem espaço).
- Strings literais vão para `.data` por padrão (o módulo original não tem `.rodata`); jump tables saem em `.rodata` do objeto. Nos gmodules o linker original as fundia no `.data` logo após o `.data` do mesmo objeto (`fix_ld.py` reproduz isso por padrão). No BOOT todas as `.rodata` dos objetos vêm **antes** de todos os `.data` (região 0x08872F50–0x088EB0xx, 129 jump tables): o `.c` declara `// @rodata 0xA-0xB`, `splat_config.py` emite um subsegmento `.data` chamado `<arq>__rodata` e `fix_ld.py` o troca por `<arq>.c.o(.rodata)` naquela posição (`deck.c`).
- Headers em `include/`: `psp.h` (protótipos sce* usados, tipos Sce*), `libc.h` (MSL), `ehfile.h` (tipos do subsistema). Sem headers da MSL no pacote: `sceIoLseek` é declarado devolvendo `int` porque o código original só testa `$v0`.
- O mwcc emite **uma seção `.text` por função** e **uma `.data` por literal**; `mwcc_check.py` localiza a função pela seção do símbolo. Literais idênticos de um mesmo `.c` são unificados (o `""` do nome da thread e do FPL).

## O que o mwccpsp faz com o fonte (aprendido no `ehfile`)

Cada item custou tentativas; vale consultar antes de brigar com uma função.

- **Base de struct em registrador salvo:** `EhfileState *s = &ehfile_state;` faz o compilador materializar `lui/addiu` em `$sN` e acessar `0x40C($s1)`; acesso direto `ehfile_state.campo` vira `lui/lw %lo(sym+off)`. O original mistura os dois estilos por função (ver `ehfile_read_async` vs `ehfile_init`).
- **`T *const p = &g;`** impede a propagação da constante no primeiro acesso: só assim `ehfile_term` carrega `0x410($s0)` antes do primeiro `blezl`. Ponteiro simples dobra o primeiro acesso em `lui/lw` direto.
- **Ordem de declaração dos locais** decide a atribuição de `$s0..$s7` e a posição na pilha (`ehfile_get_size`: `st, path, size`; `ehfile_build_lbn_table`: `pvd, ptab, ptab_size, ptab_lbn, fd`). `tools/perm_decls.py` testa todas as permutações contra os bytes originais.
- **Stores a campos distintos são reordenados** pelo escalonador: a ordem no fonte que bate pode ser diferente da ordem dos `sw` (`names_end` antes de `table_end`/`table`).
- **`if (x)` materializa booleano** (`sltu`/`xori`) quando `x` é um valor carregado; `== 0` em resultado de chamada vira `beqz` direto. `break`/`return` dentro de laço mudam o lado do desvio; `do { } while (cond)` com `break` reproduz os laços de retry.
- **`switch` emite os testes em ordem inversa à dos `case`** e os corpos na ordem do fonte (`ehfile_thread`: `case 0` antes de `case 1`; `modmgr_load_ex`: `case -1` escrito primeiro para ser testado por último). Um `if`/`else if` com dois valores também pode ser um `switch` no original (`modmgr_init`).
- **Tail call:** `return f(...)` com os mesmos argumentos vira `j f` (os aliases de exportação `ehfile_*_alias`, `ehfile_close`).
- **Acesso desalinhado:** struct com `#pragma pack(1)` gera `lwl/lwr` (registro de diretório ISO9660).
- **Intrínsecos Allegrex:** `max`/`min`/`ins`/`ext` (e `clz`, `seb`, `rotr`, `bitrev`, `sqrt_s`…) só saem de `__builtin_allegrex_*`; nenhuma forma em C puro gera essas instruções (união de bitfields vai para a pilha, `a > b ? a : b` vira `slt`/`beqzl`). `include/allegrex.h` define `ALLEGREX_MAX/MIN/INS/EXT` com fallback em C para o port. Lista completa: `strings mwccpsp.exe | grep __builtin_allegrex`. O mwcc também aceita funções `asm tipo f(args) { ... }` como último recurso.
- Expressões booleanas: `!p` gera `sltu v0,zero,p; xori 1`; `p == 0` gera `xor; sltiu 1`. A ordem das instruções independentes no prólogo segue a ordem dos statements no fonte.
- `*p++ = *q++` incrementa `q` antes de `p`; o original (`menu_count_lines`) tem `p++` antes de `q++`, isto é, foi escrito como `*p = *q; p++; q++;`.
- `return x` logo após `x = f()` reaproveita `$v0` (sem `move`): `if (id <= 0) return id;` bate onde `if (id > 0) {...} return id;` não.
- Função que no original não escreve `$v0` antes de voltar é `void` (um `int f(void)` com `return 1;` muda a alocação de registradores mesmo onde o `li v0,1` some).
- `asm { addi $v0, $ra, 0 }` dentro de uma função C reproduz instruções soltas do original (`ehscript_set_static` tem um `addi` sem efeito).
- Comparação com constante: `x >= 0x20F580` bate onde `x > 0x20F57F` troca os registradores (`$at`).
- Ordem de avaliação: `SysSave *s = sysstate_get(); s->flags |= 1 << bit;` carrega a máscara depois da chamada; `sysstate_get()->flags |= 1 << bit` calcula a máscara antes e a guarda em `$s0`.
- `f(x = g())` faz o argumento sair de `$v0` em vez do registrador da variável.
- **Caudas idênticas são fundidas:** um `switch` em que cada `case` faz o mesmo store com índice diferente vira uma cadeia `beql` com o `addiu` no delay slot e um bloco comum, e os corpos originais ficam como código morto (`stat_event_record`, 616 B dos quais 264 mortos). Escrever `kind += N; break;` + store comum gera o código sem os blocos mortos (não bate).
- Fallthrough entre `case`s não adjacentes no fonte: `goto` para um rótulo dentro do `case` seguinte (`ehscript_thread`, 3→5 com `case 4: break;` no meio) bate; mover o `case 4` muda a ordem dos testes.
- **Registrador do resultado de comparação com constante:** `x < c`, `x > c` e `x <= c` saem em `$at` (`slti $at`); `x >= c` sai em um temporário (`slti $v0`/`$v1`). Semanticamente iguais, bytes diferentes: `if (n >= 5)` vs `if (n > 4)` (`deck_slot_set_level`), `do {} while (i <= 0x4A)` vs `< 0x4B` (`req_flags_any`), `if (v > 8000) … else …` vs `v >= 0x1F41` (`deck_slot_set_value`), `if (v >= 2) { … } return 0;` vs `if (v < 2) return 0;` (`value_to_tier`).
- Tipo de retorno `int` em getters de `unsigned short`: o chamador original compara `$v0` direto (sem `andi 0xFFFF`) em `deck_slot_get_level`/`req_flag_get`; declarar o retorno `int` bate nos dois lados.
- `p[i + 0x97D0]` com `unsigned char *p = (unsigned char *)s` gera `addu; lui 1; lbu -0x6830(...)`; `s->byte_97d0[i]` gera `addiu 0x7FFF; addiu 0x17D1` (`sysstate_get_byte_97d0`), mas no setter vizinho é o acesso por campo que bate (`sysstate_max_byte_97d0`).
- Ordem das funções no objeto = ordem das **definições** no fonte (`static` usadas antes de definidas precisam de protótipo: `state_noop_*` ficam depois de `sys_state_set_default_callbacks`). `check_file.py` não detecta isso; só o `make check`.
- Variável `unsigned short` ou `int` para laço com `lhu`: `int bestv` + indexação `d[i].value` bate em `deck_find_best_slot`; ponteiro incrementado troca `$a0`/`$a2`.
- **`while (*s)` vs `while (*s != 0)`:** com `!= 0` o valor carregado fica vivo no laço (um `lhu` por iteração, em `$a2`); com `while (*p)` sobre uma **cópia local do ponteiro** (`const u16 *p = s;`) o compilador recarrega `*p` a cada iteração em `$v1`, como o original de `wcschr_index`. `while (*s)` sem cópia gera `andi v1,v1,0xFFFF` no lugar da recarga.
- **Máscara de argumento `unsigned short`:** o chamador faz `andi a0,x,0xFFFF` só se o protótipo visível declara o parâmetro `unsigned short`; função sem protótipo (declaração implícita) ou com parâmetro `int` passa o registrador direto (`inv_get_state` em `inv_collection_permille`). Protótipos em `include/game.h` devem copiar a assinatura da definição.
- `if (flag) { A } else { B }` gera `beql flag==0 → B` com A em sequência; a ordem dos ramos no fonte é a ordem no binário (`partner_table_lookup`).
- Tabelas de dados inicializadas (`static T tab[N] = {...}`) saem uma seção `.data` por objeto, alinhadas a 4, na ordem das definições; `const` não é necessário (o original as tem em `.data`). As de `wstr.c` foram geradas a partir dos bytes do ELF.
- `*p++ = *s++` em ramo `if/else` com `p++` duplicado nos dois ramos (`wcsncpy`): o original tem o `addiu p` em cada ramo, isto é, `if (*s == 0) { *p = 0; p++; } else { *p = *s; s++; p++; }`.
- **Laços desenrolados ×8** (`wstr_reverse`, cópias e preenchimentos de `wsprintf`, `wfmt_float_digits`): são obra do `-O4,p`; um `for` simples reproduz o desenrolamento e o laço de resto. `s[n - i - 1]` (não `s[n - 1 - i]`) dobra o `-1` no offset do `lhu`.
- **Varargs:** `include/stdarg.h` próprio. `va_start` = `__builtin_next_arg(parm) - ((__builtin_args_info(2) >= 8) ? 0 : (8 - __builtin_args_info(2)) * 4)` (não dobra a constante, como o original: `li v1,2; slti v0,v1,8 …`); `va_arg` avança o ponteiro (alinhando a 8 para `double`) e lê em `ap - sizeof`. `(float)double` chama `__truncdfsf2` (nomes libgcc no runtime da MSL). `(k >> 1) & 1` gera `ext` sem builtin.
- `build/shape_diff.py SRC FUNC ADDR [--regs]`: diff estrutural ignorando nomes de registradores; útil para separar forma de alocação (`wsprintf`: forma igual, alocação diferente — fica em `@asm`).
- **Soma de cadeia com variável:** `*w = -2 - a - b - c + d->w` gera `addu w, soma`; o original tem `addu soma, w`, que só sai com a cadeia numa variável local: `t = -2 - a - b - c; *w = t + d->w;` (`menu_item_rect`, `menu_cursor_y`). Operandos "variável + variável" saem na ordem do fonte; "expressão + variável" é reordenado.
- Conversão u16→float: `(float)(int)(unsigned short)w` gera `andi; mtc1; cvt.s.w`; `(float)(unsigned short)w` gera a sequência de unsigned→float com `bltz` (`menu_cursor_y`).
- Getters com índice: `if (i != -1) { ...; return 1; } return 0;` bate (`beql i == -1 → fim` com o `0` no delay slot); `if (i == -1) return 0;` não (`menu_item_open`, `menu_remove_item`, `menu_item_state`).
- Constantes `const` globais e strings de `const char []` vão para `.rodata` (uma seção por objeto): no BOOT ficam na região de rodata (0x08875600…), longe dos literais do TU; `menu.c` as usa como `extern const` definidas pelo asm de dados (símbolos `type:data` em `symbols/`).
- **Alocação de registradores em funções grandes:** `wsprintf`, `menu_layout`, `menu_item_is_open`, `deck_validate`, `gamestate_fill_partner_list` e `card_id_remap_1a6c` (um `beq` que todas as formas de fonte transformam em `beql`) têm a forma exata (diff estrutural igual) mas registradores/`$fp`/layout de pilha diferentes; ficam em `@asm` com o C sob `NON_MATCHING`. Não vale insistir: o objetivo é o port.
- **Chamadas com menos argumentos que o protótipo** no original (`gim_set_texture(*dl)` em `menu_item_build_text`): declarar a função sem protótipo (`void *gim_set_texture();`) e chamar como o original. Já `mem_swap(heap, a, b)` com 3 argumentos era ilusão do Ghidra: o `li a3, 2` estava içado e reaproveitado na comparação `flip == 2` (`menu_sprite`); passar `2` bate.
- Parâmetros `short` geram `seh` no prólogo (normalização); se o original não tem `seh`, o parâmetro é `int` (`menu_sprite`). `uvoff[0] + (p->x + p->w)` reproduz a ordem dos `addu`.
- Jump tables de funções mantidas em `@asm` ficam no asm de dados; o `@rodata` do `.c` só cobre as tabelas das funções em C (`menu_draw.c`: 0x08875718-0x08875734, sem o padding até 0x08875740).
- **Arrays `const` com inicializador vão para `.rodata`**; se o original tem a tabela em `.data`, declarar sem `const` (`font_color_table`). Ordem das definições = ordem no `.data`.
- Um `.c` com funções em `@asm` que têm literais/templates próprios (`font_init`, `font_glyph_quad`): o `@data` do `.c` só cobre os objetos definidos pelo C (`font.c`: 0x088EE8C4-0x088EE904); os templates ficam no asm de dados com nome e `size:` em `symbols/`. mwcc emite uma seção `.data` por objeto, mas `fix_ld` ainda coloca todas juntas: um `.c` não pode ter dois `@data` separados (pendência: `rename_sections` para `.data.<sym>`).
- `while (lo <= hi) { mid = (lo + hi) / 2; … }` bate a busca binária de `u16_bsearch`; a forma `do/while` com `mid` pré-calculado não.
- `switch` com `case`s compartilhando corpo: um dos casos recebe o corpo no delay slot do `beql`, os demais pulam para um bloco comum (`wc_break_class`: tests na ordem inversa dos `case`, corpos na ordem do fonte).
- O `Makefile` apaga `asm/<m>/` antes do `splat split`: a renumeração dos segmentos (`text_N.s`) deixava arquivos velhos que o `find` continuava montando e linkando (símbolos duplicados).
- Templates de inicializadores locais (`unsigned int cmds[20] = {…}` em `font_dl_glyph_header`) viram um objeto anônimo em `.data` no ponto da função; o `@data` do `.c` tem de cobrir exatamente esses bytes (`font_tags.c`: 0x088EE9E0-0x088EEA30) e os valores têm de ser copiados do ELF palavra a palavra (um erro de byte desloca tudo).
- Divisão com sinal por potência de 2 (`(int)x / 0x800`) gera o ajuste `bgez`/`addiu 0x7FF`/`sra`; ponteiros subtraídos como `(unsigned)a - (unsigned)b` dão `srl`, como `LbnEntry *` dão o ajuste com sinal.
- **`switch` esparso vira cadeia de comparações** (`addiu v0,K; beq a0,v0`), com os testes na ordem inversa dos `case` e **sem ordenação**: em `card_is_in_group_170b` dois ids aparecem fora da ordem crescente porque foram acrescentados no fim da lista original. Testes de pertinência `id == A || id == B || …` nunca batem (a cauda muda); escrever como `switch` com `return 1; default: return 0;` bate sempre (`card_is_in_group_*`, `card_attribute_to_group`, `card_same_name`, `cardinfo_*_cb`). Corpos que terminam em `break` compartilham o `return 0` depois do `switch`; `return 0` dentro de cada `case` gera saltos a mais.
- Comparação com constante dentro de `||`: o primeiro termo pode sair em temporário (`sltiu v0`) e os demais em `$at`; `deck->n_main < 0x28 || deck->n_main > 0x50` bate, e `n_side >= 0x1F` tem de ser escrito `n_side > 0x1E` (regra `$at` acima).
- Variável `unsigned short` lida de memória e usada em `&` com máscara calculada (`card_has_genre_bit`): só com a cópia local `unsigned short w = g[i];` a ordem dos operandos do `and` bate; `(g[i] & mask)` direto inverte os operandos.
- Expressão com chamada embutida (`card_db_ptr->genre + card_index_from_id(id) * 6`) carrega o ponteiro global **depois** da chamada; com o resultado da chamada numa variável o `lui/lw` do global é içado para antes da multiplicação (`card_get_genre`).
- `if (pass == 0) return -1; id = K; do { … } while (id < N); return -1;` reproduz o laço com teste inicial separado (`card_id_from_password`); o `for` equivalente gera o teste no topo.
- Ordem das atribuições iniciais num `case` (`anim1 = 0; anim2 = 0; pack = …; obj[0] = …`) muda a alocação de `$s1..$s3` mesmo com as declarações fixas (`cardinfo_model_cb`: `perm_decls.py` com as instruções como "declarações"). Locais na pilha: o primeiro declarado fica no endereço mais alto (`anim[4]` em 0x60, `params[0x34]` em 0x2C).
- **Dados zerados em `.data`:** `unsigned short x[2] = {0, 0}` vai para `.bss`; o original tem `card_empty_text` em `.data`, o que só sai com `#pragma explicit_zero_data on` em volta da definição.
- `static` em função referenciada por asm (`u16_cmp` passado ao `qsort` dentro de `deck_validate` em `@asm`) tem de cair: o `.s` referencia o símbolo global.
- Ponteiro global definido no `.c` tem de estar na posição certa do `.data` entre os literais das funções vizinhas (`CardDb *card_db_ptr = &card_db;` entre `"recipe_name_x.bin"` e `"cardinfo_"`): literais de string são emitidos na ordem das funções que os usam, e um literal repetido (`"RCP%03d.ydc"`) é emitido uma vez só.
- **Cadeias de `switch` em detalhe** (`card_groups.c`, 41 funções geradas a partir do desassembly): os testes saem na ordem inversa dos `case` e os corpos na ordem do fonte; o **primeiro teste de cada grupo** (último `case` do grupo no fonte) vira `beql` com a primeira instrução do corpo no delay slot, os demais `beq` para o bloco do grupo; se o delay slot do primeiro teste já foi usado pelo prólogo (`sw ra`), o próximo teste do grupo recebe o `beql` (`card_is_group_c`). `case 2: case 4: return 0;` vs dois `case` separados muda o código (merge de ambos). Um `switch` denso (`id - 0x1A90 < 6`) vira tabela de saltos em `.rodata` (`@rodata`). O fim do `@range` deve incluir o `nop` do delay slot do último `jr ra`: cortá-lo duplica a palavra no segmento asm seguinte e desloca todo o resto do módulo em 4 bytes.
- `return !f(x)` gera `sltu v0,zero,v0; xori v0,v0,1`; `f(x) == 0` sai como `xor/sltiu`. `if (hi == 1) return 3; return 2;` dá `bnel`; a forma negada dá `beql`.
- **`__FUNCTION__` vai para `.rodata`** (os nomes `DvdUmdRetryOpenCB` etc. do código de exemplo da Sony em `pmf_umd.c`); por isso essas funções mantêm o nome original, e os `__LINE__` viram literais (401, 459, 525). O fim do `@rodata` exclui o preenchimento de alinhamento do objeto seguinte (o EHP `pack_loading` é alinhado a 16): o segmento `data` gerado cobre os zeros.
- **Ordem das funções no `.c` = ordem das seções no objeto = ordem no binário**, mesmo com `fix_ld` (`.text .text.*` mantém a ordem do objeto): uma função fora de ordem desloca todas as seguintes (`modmgr_none_loading_alias` entre `duel_modules_load` e `duel_modules_unload`).
- Laços de retry (`DvdUmdRetry*`): todos os `return` do original são `break` + um único `return ret` no fim. `ret = f(); if (ret < 0) break;` dá `move s0,v0; bltzl s0 …; move v0,s0`; `if ((ret = f()) < 0) break;` dá `bltz v0 …; move s0,v0` (teste em `v0`, cópia no delay slot); `result = f(); ret = result; if (result < 0) { printf(…, result); break; }` reproduz o `printf` lendo `v0` em vez de `s0`; e `result = g(); if (result < 0) ret = result; else {…}` adia a cópia para o ramo `else` (`DvdUmdRetryReadCB`). Nenhuma dessas formas é intercambiável.
- `if (g >= 0) return 0; x = f(); if (x < 0) return x; g = x; return 0;` (`umd_retry_init`): o `bltzl` para o bloco de criação e o `b` de retorno antecipado só saem com o teste invertido no início.
- **`if ((x = f()) op c)`** (atribuição dentro da condição) testa `v0` e copia para `$sN` no delay slot (`bnez v0,…; move s0,v0`); `x = f(); if (x op c)` copia antes e testa `$sN`. Decide o matching de `movie_decode_*` e dos laços de `DvdUmdRetry*` (que precisam ainda de duas variáveis quando o `printf` lê o valor).
- Mistura de tipos em `a + b` muda a ordem dos `lw`: com `unsigned *mem + int bufsize` o operando convertido é carregado primeiro; `bufsize` `unsigned` em `MovieSnd` bateu `msnd_init`. Alinhamentos em cadeia (`ALIGN64(p + sizeof)`) dobram a constante só se a macro não tiver cast no meio.
- Variáveis mantidas em `$s0..$s8` parecem ser ordenadas por número de usos (`movie_main_loop`: `r` em `$s0` no original, `$s5` na reconstrução); `movie_run`, `movie_main_loop`, `movie_poll_input`, `snd_output`, `*_push_pts`, `vid_init` e `movie_file_init` ficam em `@asm` por isso. Nomes de função duplicados no `symbols/` (`snd_init`, `wave_init`, `movie_play`) quebram o `splat`: prefixar (`msnd_*`).

## Pipeline

```
build/elf/<m>.elf  (PRX relocado, tools/prx2elf.py)
   │  tools/splat_config.py <m>  → config/<m>.yaml   (seções → segmentos; .text e .data divididos nas faixas @range/@data dos .c)
   │  tools/splat_symbols.py <m> → build/<m>.syms.txt (só name/addr/type, formato estrito do splat)
   ▼  splat split                → asm/<m>/*.s, assets/*.bin, build/<m>.ld
   │  sed: remove `.set gp=64` (N64) ; tools/fix_ld.py reordena o bloco DATA e funde .rodata dos .c
   ▼  psp-as (pspdev v20261001) para .s ; tools/mwcc.sh para src/<m>/*.c ; psp-ld -r -b binary para .bin
   ▼  psp-ld -T build/<m>.ld      → build/<m>.rebuilt.elf
   ▼  tools/compare_sections.py build/elf/<m>.elf build/<m>.rebuilt.elf   (compara cada seção pelo vram)
```

Comandos: `make MODULE=BOOT check`, `make check-all` (todos os módulos), `make clean`. Verificação antes de integrar: `tools/check_file.py MODULE src/MODULE/x.c` (todas as funções do `@range`), `tools/mwcc_check.py SRC.c FUNC MODULE 0xADDR` (uma função, mostra as palavras diferentes), `tools/diff_words.py build/elf/M.elf build/M.rebuilt.elf .text` (depois do link). Desmontar o objeto compilado: `psp-objdump -dr build/x.check.o`.
Correções automáticas no split (`tools/fix_asm.py`): formas VFPU que o psp-as rejeita (`vrot` com lista de rotação, `vpfxd` com máscara) e `.double`/`.dword` (o psp-as alinharia a 8 e deslocaria o `.data`) viram `.word` com os bytes originais.

## Funções que ficam em asm dentro de um arquivo C (`@asm`)

O mwcc emite **uma seção `.text` por função**. `tools/rename_sections.py` (chamado pelo Makefile após cada compilação) renomeia cada uma para `.text.<função>`; assim o linker script pode colocar funções individualmente. Um `.c` declara `// @asm 0xA-0xB` para cada função da sua faixa que continua em asm: `splat_config.py` divide o segmento em `[c, arq] [asm, arq_asm_A] [c, arq] …` e `fix_ld.py` troca cada `arq.c.o(.text)` repetido pela lista `arq.c.o(.text.f1); arq.c.o(.text.f2); …` das funções daquela sub-faixa (pelos endereços em `symbols/`). Os demais objetos C recebem `(.text .text.*)`. A versão C da função fica no arquivo sob `#ifdef NON_MATCHING` para o port. `check_file.py` ignora as faixas `@asm`.

Hoje em asm: `stat_event_record` (0x0883E13C, blocos mortos), `day_threshold` (0x0883EAB4, laço desenrolado pelo compilador; corpo bate, prólogo não), `day_calc` (0x0883ED7C, layout do `case 2`), `gamestate_load` (0x08840A78, só um temporário `$a1`/`$a0` difere).

## Convenção dos arquivos C

```c
// @range 0x088044B8-0x08804508   ← vram coberto pelas funções deste arquivo (em ordem de endereço)
// @asm   0x0883E13C-0x0883E3A4   ← função dentro da faixa que fica em asm (opcional, repetível)
// @data  0x08808830-0x08808840   ← vram do .data que este arquivo gera (literais); opcional, repetível
// @rodata 0x08875598-0x08875600  ← vram das jump tables quando ficam longe dos literais (BOOT); opcional
```
A função também recebe seu nome em `symbols/<m>.txt` (`nome = 0xADDR; // type:func matched src/<m>/arq.c`) para o asm restante referenciá-la.

## Ferramentas externas (gitignored em `tools/ext/` e `~/opt`)

| Ferramenta | Versão | Origem |
|---|---|---|
| wibo | 1.1.0 | github.com/decompals/wibo |
| mwccpsp | 3.0.1 build 219 (e 121 para comparação) | github.com/decompme/compilers releases |
| pspdev | v20261001 (psp-as, psp-ld, psp-objdump) | github.com/pspdev/pspdev releases, extraído em `~/opt/pspdev` |
| splat64 / spimdisasm / rabbitizer | 0.50.1 / 1.42.4 / 1.16.2 | pip, venv `.venv` |

- **Parâmetro `void *` + cast local muda a alocação de `$s`** (`cip.c`): com `Cip *c` como parâmetro, `c` recebe o `$s` mais alto entre os parâmetros (ordem inversa dos argumentos); com `void *handle` e `Cip *c = (Cip *)handle;` dentro da função, `c` passa a ser alocado junto com os locais (um `$s` abaixo) e `cip_read_card`, `cip_get_image`, `cip_query` e `cip_open_*` batem. Em `cip_query` o teste de nulo é feito no parâmetro (`handle == 0`), não na cópia (`beqz a0` com `move s0,a0` no delay slot).
- **Funções `static` já compiladas têm o conjunto de registradores alterados conhecido pelo chamador**: `cip_draw` mantém `list` em `$a2` através de `jal cip_is_cpm` e `cip_prefetch_cb` mantém `$t0-$t2`/`$a2`/`$a3` através de `jal cip_table`, porque as duas folhas são `static` e definidas antes no mesmo `.c`; sem `static` o chamador salva em `$s`. Só vale para funções definidas antes: `cip_alloc_slot`/`cip_load_next` (antes de `cip_table`) usam a convenção normal. Uma função em `@asm` que chame uma dessas folhas obriga a tirar o `static` (ou perde-se o matching do chamador).
- **Cadeias `a || b || c` em `if (...) return X;`** compartilham um único bloco `move v0,X; b END; nop` colocado após o último teste; os testes intermediários saltam direto para o epílogo com o valor no delay slot (`bnezl v0,END; move v0,zero`). Testes `if` separados geram o padrão "salta por cima do return" (`beqzl cond,CONT; <1ª instrução>; b END; move`). Decide `cip_read_card`/`cip_get_image`/`cip_query` (`c == 0 || !c->ready || c->closed`, `id < first || last < id`, `e == 0 || e->chunk == 0`).
- **`if (a && b) return 0;` gera `bltz`/`bgez` sem likely e com `nop`** (`cip_is_idle`: `if (c->prefetch && c->prefetch_idx >= 0) return 0;`); `if (a) { if (b) return 0; return 1; }` vira `slt`.
- **Bloco compartilhado entre dois caminhos** (`cip_load_cb`): `if (n <= 0) { if (!closed) return 0; } else { ...; if (load_next()) goto done; } busy = 0; done: return 1;` só sai assim com o `goto` (ou equivalente): sem ele o compilador duplica o `busy = 0` ou troca o sentido do `bnezl`.
- **`if (!c->busy) { ... }` antes de um `return` final** gera `bnezl busy,END; move v0,X` (salto direto ao epílogo); `if (c->busy) return X;` gera o padrão "salta por cima". Mesma lógica, código diferente.
- **Ordem de instruções segue a ordem de avaliação do fonte**: `buf = slots + (next * chunks) << 11; next++;` vs `i = next++; buf = ...` (`cip_alloc_slot`); `base = ...; nread = ...;` troca `$t0`/`$t1` (`cip_prefetch_cb`); `(unsigned char *)c + size + 0xAC` vs `(unsigned char *)&c->hdr + size` troca os operandos do `addu` (`cip_header_ready`); `idx = id - c->hdr.first_id;` antes de `cip_table(c)` deixa o `subu` no delay slot do `jal` e carrega `first_id` antes da chamada.

- **`min`/`max`/`ext`/`ins` do Allegrex só saem por intrínseco**: `if (a < b) b = a`, `?:` e variantes com `unsigned short` nunca viram `min`; `__builtin_allegrex_min(a, b)` (macro `ALLEGREX_MIN` em `include/allegrex.h`, com fallback em C para o port) gera a instrução (`fade_apply`). `(x >> 24) & 0xFF` vira `ext` sem intrínseco.
- **Loads de globais não são reaproveitados depois de um join**: em `fade_apply` o `duration` lido para o `min` é reusado no `t = duration - t` (mesmo bloco estendido) mas recarregado para a divisão após o `if`; um local `int dur = fade_state.duration` mantém o valor em registrador e quebra o matching. Usar o global diretamente onde o original recarrega.
- **Ordem dos operandos de `|`, `<`, `addu`** segue o fonte: `rgb | (a << 24)` vs `(a << 24) | rgb` troca os operandos do `or`; `counter > duration` carrega `counter` primeiro, `duration < counter` carrega `duration` primeiro (mesmo `slt`). `if (x1 <= x0) return` vs `if (x0 >= x1)` muda `slt v0`/`slt at` (`fade_wipe_line`).
- **Alocação de `$s` em laços consecutivos** (`fade_apply`): o segundo laço `for` recebe `(j, x)` em vez de reutilizar `(i, y)`: com as mesmas variáveis o par `s0/s1` não troca como no original. **Operando esquerdo de `|`**: `(alpha << 24) | rgb` só bate com `alpha` como variável local atribuída antes (`alpha = (unsigned char)(...)`); inline, o `or` sai com os operandos invertidos.
- **`if (x1 > 480) x1 = 480; x1 -= x0;`** duplica o `subu` no delay slot do `bnezl` e no caminho else; escrever o if/else com duas subtrações não bate.

## Pendências

- `.bss`: os globais do C continuam `extern` (definidos pelo asm de `.bss`); quando um arquivo definir os próprios globais será preciso cortar a faixa correspondente do `.bss` (um `@bss` no cabeçalho, como `@data`).
- `include/` só tem o que o `ehfile` usa; crescer por subsistema.
