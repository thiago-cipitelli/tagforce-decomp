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
	$(SPLAT) split $<
	@# ponytail: spimdisasm emits N64-style `.set gp=64`; psp-as (32-bit) rejects it
	find asm/$(MODULE) -name '*.s' -exec sed -i '/^\.set gp=64/d' {} +
	python3 tools/fix_asm.py $(AS) $(ASFLAGS) -- $$(find asm/$(MODULE) -name '*.s')
	python3 tools/fix_ld.py config/$(MODULE).yaml $(LD_SCRIPT)

build/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

build/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(MWCC) $(MWCC_FLAGS) -c $< -o $@
	@python3 tools/rename_sections.py $@

build/%.bin.o: %.bin
	@mkdir -p $(dir $@)
	$(LD) -r -b binary -o $@ $<

$(OUT): $(LD_SCRIPT) $(ASM_OBJS) $(C_OBJS) $(BIN_OBJS)
	$(LD) -EL -T $(LD_SCRIPT) -Map build/$(MODULE).map -o $@ $(ASM_OBJS) $(C_OBJS) $(BIN_OBJS)

clean:
	rm -rf build/asm/$(MODULE) build/src/$(MODULE) build/assets/$(MODULE) $(OUT) build/$(MODULE).map $(LD_SCRIPT) asm/$(MODULE) assets/$(MODULE)

.PHONY: all check check-all clean

check-all:
	@for m in $$(ls build/elf | sed 's/\.elf$$//'); do printf '%s: ' $$m; $(MAKE) --no-print-directory MODULE=$$m check 2>&1 | tail -1; done
