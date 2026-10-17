# Build reproduzível (fase 3)

## Resultado

Todos os 23 módulos (`BOOT` + 22 gmodules) são reconstruídos **byte a byte** a partir do split do splat (`make check-all`). Código já em C compilado pelo compilador original e linkado no lugar exato: `libhtml_view` (2 funções) e, no `BOOT`, `src/BOOT/ehfile.c` (VFS, 29 funções, 0x0882ECAC–0x0882FC3C) `src/BOOT/modmgr.c` (gerenciador de módulos + espera de frame + menu de depuração, 19 funções, 0x08859044–0x08859824, com a tabela estática de módulos em `.data`) e `src/BOOT/sys.c` (thread de cenas `ehscript`, `sys_init`, pilha de estados, 17 funções, 0x0883CEB0–0x0883D708).

## Compilador

- Binário: `mwccpsp.exe` **3.0.1 build 219** (`tools/ext/mwccpsp_219/`, da coleção do decomp.me), executado com `wibo` 1.1.0. O `.comment` dos módulos diz "MW MIPS C Compiler (2.4.1.01)", mas esse texto é o do front-end e é igual em todos os builds 3.0.1; o build 219 gera código idêntico nas funções testadas e o build 121 não (4 palavras diferentes em `save_path_build`).
- Flags confirmadas: **`-O4,p -sdatathreshold 0`**. `-O4,s` bate nas funções sem laço mas não em `ehfile_busy` (0x0882F468): com `,s` o laço indexa (`sll`+`addu`), com `,p` incrementa o ponteiro como no original. `-O2` não bate.
- Include path: `-i include` (o `-I` do mwcc exige `-Ipath` sem espaço).
- Strings literais vão para `.data` por padrão (o módulo original não tem `.rodata`); jump tables saem em `.rodata` do objeto e o linker original as fundia no `.data` logo após o `.data` do mesmo objeto. `tools/fix_ld.py` reproduz isso no script do splat.
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
- Fallthrough entre `case`s não adjacentes no fonte: `goto` para um rótulo dentro do `case` seguinte (`ehscript_thread`, 3→5 com `case 4: break;` no meio) bate; mover o `case 4` muda a ordem dos testes.
- Divisão com sinal por potência de 2 (`(int)x / 0x800`) gera o ajuste `bgez`/`addiu 0x7FF`/`sra`; ponteiros subtraídos como `(unsigned)a - (unsigned)b` dão `srl`, como `LbnEntry *` dão o ajuste com sinal.

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

## Convenção dos arquivos C

```c
// @range 0x088044B8-0x08804508   ← vram coberto pelas funções deste arquivo (em ordem de endereço)
// @data  0x08808830-0x08808840   ← vram do .data que este arquivo gera (literais, jump tables); opcional, repetível
```
A função também recebe seu nome em `symbols/<m>.txt` (`nome = 0xADDR; // type:func matched src/<m>/arq.c`) para o asm restante referenciá-la.

## Ferramentas externas (gitignored em `tools/ext/` e `~/opt`)

| Ferramenta | Versão | Origem |
|---|---|---|
| wibo | 1.1.0 | github.com/decompals/wibo |
| mwccpsp | 3.0.1 build 219 (e 121 para comparação) | github.com/decompme/compilers releases |
| pspdev | v20261001 (psp-as, psp-ld, psp-objdump) | github.com/pspdev/pspdev releases, extraído em `~/opt/pspdev` |
| splat64 / spimdisasm / rabbitizer | 0.50.1 / 1.42.4 / 1.16.2 | pip, venv `.venv` |

## Pendências

- `.bss`: os globais do C continuam `extern` (definidos pelo asm de `.bss`); quando um arquivo definir os próprios globais será preciso cortar a faixa correspondente do `.bss` (um `@bss` no cabeçalho, como `@data`).
- `include/` só tem o que o `ehfile` usa; crescer por subsistema.
