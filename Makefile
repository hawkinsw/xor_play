all: xor loop_canary sxor ptr

xor: xor.c Makefile
	gcc -g -o xor xor.c
loop_canary: loop_canary.c Makefile
	gcc -g -o loop_canary loop_canary.c
sxor: sxor.s Makefile
	nasm -felf64 sxor.s && gcc -nostdlib sxor.o -o sxor
ptr: ptr.s Makefile
	nasm -felf64 ptr.s && gcc -nostdlib ptr.o -o ptr

clean:
	rm -f xor sxor.o sxor loop_canary ptr.o ptr
