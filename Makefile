SGDK=/sgdk
GCC_VER?=6.3.0
MAKE?=make
LIB?=lib
ASSEMBLY_OUT?=out
CC = m68k-elf-gcc
AS = m68k-elf-as
AR = m68k-elf-ar
LD = m68k-elf-ld
RANLIB = m68k-elf-ranlib
OBJC = m68k-elf-objcopy
OBJDUMP = m68k-elf-objdump
BINTOS = $(SGDK)/bin/bintos
JAVA= java
RESCOMP= $(JAVA) -jar $(SGDK)/bin/rescomp.jar
XGMTOOL= $(SGDK)/bin/xgmtool
PCMTORAW = $(SGDK)/bin/pcmtoraw
WAVTORAW = $(SGDK)/bin/wavtoraw
SIZEBND = $(SGDK)/bin/sizebnd
ASMZ80 = $(SGDK)/bin/zasm
RM = rm -f
NM = nm
INCS = -I. \
	-I$(SGDK)/inc \
    -I$(SGDK)/res \
	-I/usr/m86k-elf/include \
	-Isrc \
	-Isrc/mw \
	-Ires
CCFLAGS = -Wall \
	-Wextra \
	-std=c11 -Werror \
	-fno-builtin \
	-DBUILD='"$(BUILD)"' \
	-m68000 -O3 -c -fomit-frame-pointer -g
ifeq ($(ROM_TYPE), MEGAWIFI)
	CCFLAGS += -DMEGAWIFI
endif
ifeq ($(DEBUG_INFO), 1)
	CCFLAGS += \
		-Wno-unused-function \
		-DDEBUG_EVENTS \
		-DDEBUG_TICKS
endif
Z80FLAGS = -vb2
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L/usr/m68k-elf/lib \
	-L/usr/lib/gcc/m68k-elf/$(GCC_VER)/* \
	-L/sgdk/lib \
    -lmd -lnosys \
	--wrap=SYS_enableInts \
	--wrap=SYS_disableInts

LINKFLAGS = -T src/mw.ld \
	-Map=out/output.map \
	-nostdlib
ARCHIVES = /sgdk/lib/libmd.a
ARCHIVES += /usr/lib/gcc/m68k-elf/$(GCC_VER)/libgcc.a

RESOURCES=
BOOT_RESOURCES=

BOOTSS=$(wildcard boot/*.s)
BOOTSS+=$(wildcard src/boot/*.s)
NEWLIBSS=$(wildcard src/newlib/*.s)
BOOT_RESOURCES+=$(BOOTSS:.s=.o)
NEWLIB_RESOURCES+=$(NEWLIBSS:.s=.o)
RESS=$(wildcard res/*.res)
RESS+=$(wildcard *.res)
RESOURCES+=$(RESS:.res=.o)

CS=$(wildcard src/*.c)
CS+=$(wildcard src/*/*.c)
SS=$(wildcard src/*.s)
S80S=$(wildcard src/*.s80)
CS+=$(wildcard *.c)
SS+=$(wildcard *.s)
S80S+=$(wildcard *.s80)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)
RESOURCES+=$(S80S:.s80=.o)

OBJS = $(RESOURCES)

all: test bin/out.bin bin/out.elf

boot/sega.o: boot/rom_head.bin
	$(CC) -x assembler-with-cpp $(CCFLAGS) boot/sega.s -o $@

boot/sega.s: $(SGDK)/src/boot/sega.s
	cp $< $@

src/newlib/setjmp.o:
	$(AS) $(ASFLAGS) src/newlib/setjmp.s -o $@

bin/%.bin: %.elf
	mkdir -p bin
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync
	$(OBJDUMP) -D $< --source > $(ASSEMBLY_OUT)/out.s
	rm temp.bin
	echo $(BUILD) > bin/version.txt

%.elf: $(OBJS) $(BOOT_RESOURCES) $(NEWLIB_RESOURCES)
	$(LD) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(NEWLIB_RESOURCES) $(ARCHIVES) $(OBJS) $(LIBS)

%.o80: %.s80
	$(ASMZ80) $(Z80FLAGS) -o $@ $<

%.c: %.o80
	$(BINTOS) $<

%.o: %.c
	mkdir -p $(ASSEMBLY_OUT)
	$(CC) $(CCFLAGS) $(INCS) -c \
		-Wa,-aln=$(ASSEMBLY_OUT)/$(notdir $(@:.o=.s)) \
		$< -o $@

%.o: %.s res/sprite.s
	$(AS) $(ASFLAGS) $< -o $@

%.s: %.bmp
	bintos -bmp $<

%.rawpcm: %.pcm
	$(PCMTORAW) $< $@

%.raw: %.wav
	$(WAVTORAW) $< $@ 16000

%.pcm: %.wavpcm
	$(WAVTORAW) $< $@ 22050

%.s: %.tfd
	$(BINTOS) -align 32768 $<

%.s: %.mvs
	$(BINTOS) -align 256 $<

%.s: %.esf
	$(BINTOS) -align 32768 $<

%.s: %.eif
	$(BINTOS) -align 256 $<

%.s: %.vgm
	$(BINTOS) -align 256 $<

%.s: %.raw
	$(BINTOS) -align 256 -sizealign 256 $<

%.s: %.rawpcm
	$(BINTOS) -align 128 -sizealign 128 -nullfill 136 $<

%.s: %.rawpcm
	$(BINTOS) -align 128 -sizealign 128 -nullfill 136 $<

%.s: %.res
	$(RESCOMP) $< $@

boot/rom_head.bin: boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

unit-test:
	$(MAKE) -C tests clean-target unit
.PHONY: unit-test

test:
	$(MAKE) -C tests
.PHONY: test

clean:
	$(MAKE) -C tests clean-target
	$(RM) $(RESOURCES) res/*.s
	$(RM) *.o *.bin *.elf *.map *.iso
	$(RM) boot/*.o boot/*.bin
