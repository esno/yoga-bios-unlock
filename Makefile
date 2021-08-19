all: yoga-bios-unlock

yoga-bios-unlock: src/yoga-bios-unlock.c
	${CC} -o ./yoga-bios-unlock ./src/yoga-bios-unlock.c -O2 -Wall -Wextra -Wfloat-equal -Wshadow -Wstrict-prototypes -Wstrict-overflow=5 -Wcast-qual -Wconversion -Wunreachable-code

clean:
	rm ./yoga-bios-unlock
