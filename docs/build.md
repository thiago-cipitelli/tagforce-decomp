# Build reproduzível (fase 3)

## Resultado

`libhtml_view` é reconstruído **byte a byte** a partir do split do splat, com duas funções já em C compiladas pelo compilador original e linkadas no lugar exato (`make MODULE=libhtml_view check` → `MATCH` em todas as 13 seções carregáveis).

## Compilador

- Binário: `mwccpsp.exe` **3.0.1 build 219** (`tools/ext/mwccpsp_219/`, da coleção do decomp.me), executado com `wibo` 1.1.0. O `.comment` dos módulos diz "MW MIPS C Compiler (2.4.1.01)", mas esse texto é o do front-end e é igual em todos os builds 3.0.1; o build 219 gera código idêntico nas funções testadas e o build 121 não (4 palavras diferentes em `save_path_build`).
- Flags confirmadas: `-O4,s -sdatathreshold 0`. `-O4,p` produz o mesmo código nas funções testadas até agora; a distinção ficará para uma função com laços. `-O2` não bate.
- Strings literais vão para `.data` por padrão (o módulo original não tem `.rodata`); jump tables saem em `.rodata` do objeto e o linker original as fundia no `.data` logo após o `.data` do mesmo objeto. `tools/fix_ld.py` reproduz isso no script do splat.
- Sem headers da MSL no pacote: os `.c` declaram protótipos próprios (`extern`), por enquanto.

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

Comandos: `make MODULE=libhtml_view check`, `make clean`. Verificação por função antes de integrar: `tools/mwcc_check.py SRC.c FUNC MODULE 0xADDR` (mascara campos relocados usando o `.rel.text` do objeto).

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

- `-O4,s` vs `-O4,p`: achar função discriminante.
- Headers: reconstruir um `include/` com protótipos da `libehsys` e tipos comuns em vez de `extern` por arquivo.
- Estender `make` para BOOT (segmentos extras: `.rodata.sce*`, `.lib.*`) e para módulos com `e_entry` ≠ 0; o gerador de config já lê as seções do ELF, então deve ser direto.
