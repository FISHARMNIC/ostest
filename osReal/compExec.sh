i386-elf-gcc -Wall -c apps/printHello.c -o apps/printHello.o -nostdlib -nostartfiles -fno-asynchronous-unwind-tables -ffreestanding 
i386-elf-gcc -Wall -Wl,--oformat=binary -c apps/printHello.c -o apps/printHello.bin -nostdlib -nostartfiles -fno-asynchronous-unwind-tables -ffreestanding 
i386-elf-ld -T apps/linker/link.ld apps/printHello.o -o apps/printHello -n