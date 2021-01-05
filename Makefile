yoga-unlock:
	${CC} -o ./yoga-bios-unlock ./src/yoga-bios-unlock.c -O2 -Wall

clean:
	rm ./yoga-bios-unlock
