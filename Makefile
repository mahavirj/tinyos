# $@ = target file
# $< = first dependency
# $^ = all dependencies

GCC := gcc
LD := ld

C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)
# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

CFLAGS = -ffreestanding

all: run

%.o: %.c ${HEADERS}
	${GCC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

kernel.bin: boot/entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^ --oformat binary

os-image.bin: boot/boot.bin kernel.bin
	cat $^ > $@

run: os-image.bin
	qemu-system-i386 -fda os-image.bin -boot a

boot:
	nasm boot/boot.asm -f bin -o boot/boot.bin
	nasm boot/entry.asm -f elf -o boot/entry.o

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o libc/*.o
