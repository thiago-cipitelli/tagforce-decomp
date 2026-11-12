# usage: make MODULE=libhtml_view   (default)   | make MODULE=... check
MODULE ?= libhtml_view
PSPDEV ?= $(HOME)/opt/pspdev
AS      := $(PSPDEV)/bin/psp-as
LD      := $(PSPDEV)/bin/psp-ld
OBJCOPY := $(PSPDEV)/bin/psp-objcopy
MWCC    := tools/mwcc.sh
MWCC_FLAGS ?= -O4,p -sdatathreshold 0 -i include
ASFLAGS := -march=allegrex -mabi=eabi -EL -I include -I asm/$(MODULE)
SPLAT   := .venv/bin/splat

TARGET   := build/elf/$(MODULE).elf
LD_SCRIPT:= build/$(MODULE).ld
OUT      := build/$(MODULE).rebuilt.elf

ASM_SRCS := $(shell find asm/$(MODULE) -name '*.s' 2>/dev/null)
C_SRCS   := $(shell find src/$(MODULE) -name '*.c' 2>/dev/null)
BIN_SRCS := $(shell find assets/$(MODULE) -maxdepth 1 -name '*.bin' 2>/dev/null)
ASM_OBJS := $(ASM_SRCS:%.s=build/%.s.o)
C_OBJS   := $(C_SRCS:%.c=build/%.c.o)
BIN_OBJS := $(BIN_SRCS:%.bin=build/%.bin.o)

# split first in a sub-make: the object lists above are computed when the Makefile is parsed,
# so the asm files must already exist when the link rule is evaluated.
all:
	$(MAKE) --no-print-directory $(LD_SCRIPT)
	$(MAKE) --no-print-directory $(OUT)

check: all
	python3 tools/compare_sections.py $(TARGET) $(OUT)

$(LD_SCRIPT): config/$(MODULE).yaml
	python3 tools/splat_symbols.py $(MODULE) > build/$(MODULE).syms.txt
	rm -rf asm/$(MODULE)
	$(SPLAT) split $<
	@# ponytail: spimdisasm emits N64-style `.set gp=64`; psp-as (32-bit) rejects it
	find asm/$(MODULE) -name '*.s' -exec sed -i '/^\.set gp=64/d' {} +
	python3 tools/fix_asm.py $(AS) $(ASFLAGS) -- $$(find asm/$(MODULE) -name '*.s')
	python3 tools/fix_ld.py config/$(MODULE).yaml $(LD_SCRIPT)

build/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<
	@python3 tools/min_align.py $@

build/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(MWCC) $(MWCC_FLAGS) -c $< -o $@
	@python3 tools/rename_sections.py $@
	@python3 tools/min_align.py $@

build/%.bin.o: %.bin
	@mkdir -p $(dir $@)
	$(LD) -r -b binary -o $@ $<
	@python3 tools/min_align.py $@

$(OUT): $(LD_SCRIPT) $(ASM_OBJS) $(C_OBJS) $(BIN_OBJS)
	$(LD) -EL -T $(LD_SCRIPT) -Map build/$(MODULE).map -o $@ $(ASM_OBJS) $(C_OBJS) $(BIN_OBJS)

clean:
	rm -rf build/asm/$(MODULE) build/src/$(MODULE) build/assets/$(MODULE) $(OUT) build/$(MODULE).map $(LD_SCRIPT) asm/$(MODULE) assets/$(MODULE)

.PHONY: all check check-all clean verify host-check host-lto host-warn

check-all:
	@for m in $$(ls build/elf | sed 's/\.elf$$//'); do printf '%s: ' $$m; $(MAKE) --no-print-directory MODULE=$$m check 2>&1 | tail -1; done

# full gate before a commit: regenerate the splat config from the @range/@data headers, re-split,
# check $(MODULE), refresh the generated imports of every symbols file, check all modules.
verify:
	python3 tools/splat_config.py $(MODULE) > config/$(MODULE).yaml
	rm -rf build/asm/$(MODULE)
	@$(MAKE) --no-print-directory check 2>&1 | tail -1 | grep -qx MATCH || { echo "$(MODULE) BAD"; exit 1; }
	python3 tools/link_map.py build/elf symbols tools/psp_nids.txt >/dev/null
	@n=$$($(MAKE) --no-print-directory check-all 2>&1 | grep -c ': MATCH$$'); t=$$(ls build/elf/*.elf | wc -l); \
	  [ "$$n" = "$$t" ] || { echo "check-all $$n/$$t"; exit 1; }; echo "ALL MATCH"

# port gate (phase 5): every src/*.c through the host gcc as the PSP sees it (32-bit, NON_MATCHING drafts);
# prints the files that do not compile and their count (docs/port_survey.md)
host-check:
	@n=0; for f in src/*/*.c; do gcc -m32 -std=gnu89 -fsyntax-only -w -DNON_MATCHING -Iinclude -I$$(dirname $$f) $$f \
	  >/dev/null 2>&1 || { echo "FAIL $$f"; n=$$((n+1)); }; done; echo "host-check: $$n file(s) fail"

# port gate (phase 5): every src/*.c with gcc -flto, linked together (-r) so that -Wlto-type-mismatch compares each
# declaration with the definition across files and modules; tools/lto_mismatch.py prints the count per class
# (severe = argument count, float position, struct by value, float return; docs/port_survey.md)
host-lto:
	@rm -rf build/host-lto; mkdir -p build/host-lto; for f in src/*/*.c; do \
	  gcc -m32 -std=gnu89 -w -O0 -flto -fno-builtin -DNON_MATCHING -Iinclude -I$$(dirname $$f) -c $$f \
	  -o build/host-lto/$$(basename $$(dirname $$f)).$$(basename $$f .c).o || echo "FAIL $$f"; done
	@gcc -m32 -flto -r -nostdlib -Wlto-type-mismatch build/host-lto/*.o -o build/host-lto/all.o 2> build/host-lto/lto.log; \
	  python3 tools/lto_mismatch.py build/host-lto/lto.log

# port gate (phase 5): gcc -Wall -Wextra over every src/*.c (32-bit, NON_MATCHING drafts), minus the classes that are
# matching idioms or type noise; prints the count per warning class (full log in build/host-warn.log;
# docs/port_survey.md, step 3)
HOST_WARN := -Wall -Wextra -Wno-parentheses -Wno-missing-field-initializers -Wno-missing-braces -Wno-cast-function-type \
  -Wno-unterminated-string-initialization -Wno-builtin-declaration-mismatch -Wno-unknown-pragmas -Wno-comment \
  -Wno-unused -Wno-sign-compare -Wno-unused-parameter
host-warn:
	@for f in src/*/*.c; do gcc -m32 -std=gnu89 -fsyntax-only $(HOST_WARN) -DNON_MATCHING -Iinclude -I$$(dirname $$f) $$f; \
	  done > build/host-warn.log 2>&1; grep -o '\[-W[a-z-]*\]' build/host-warn.log | sort | uniq -c | sort -rn
