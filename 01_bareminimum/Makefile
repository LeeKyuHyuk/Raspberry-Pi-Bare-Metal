CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean kernel8.img

start.o: start.S
	aarch64-elf-gcc $(CFLAGS) -c start.S -o start.o

kernel8.img: start.o
	aarch64-elf-ld -nostdlib -nostartfiles start.o -T link.ld -o kernel8.elf
	aarch64-elf-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
