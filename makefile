all:
	i686-elf-gcc -c boot.s -o boot.o
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-objcopy -O elf32-i386 -B i386 -I binary font.psf font.o
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o font.o

clean:
	rm -f *.o myos.bin

run:
	qemu-system-i386 -kernel myos.bin