all:
	ca65 -o main.o main.s; ld65 -C mpt26.cfg -o mpt26.bin main.o
