%.o: %.c
	@clang $< -o $@ -std=c23


run: main.o
	@./main.o

clean:
	@rm -f *.o
