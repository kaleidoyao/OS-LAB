main: my_print.asm main.cpp
	nasm -f elf32 -o my_print.o my_print.asm
	g++ -std=c++11 -m32 -c main.cpp -o main.o
	g++ -m32 my_print.o  main.o -o fat
clean:
	rm my_print.o
	rm main.o