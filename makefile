init:
	i686-elf-gcc -c boot.s -o boot.o
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-objcopy -O elf32-i386 -B i386 -I binary font.psf font.o
	i686-elf-gcc -T linker.ld -o boot/myos -ffreestanding -O2 -nostdlib boot.o kernel.o font.o

clean:
	rm -f *.o boot/myos

sanity:
	i686-elf-grub-file --is-x86-multiboot boot/myos && echo "multiboot ok" || echo "no multiboot"

iso:
	mkdir -p isodir/boot/grub
	cp boot/myos isodir/boot/myos
	cp grub.cfg isodir/boot/grub/grub.cfg
	i686-elf-grub-mkrescue -o myos.iso isodir

go:
    qemu-system-i386 -cdrom myos.iso -vga std -display cocoa,zoom-to-fit=on