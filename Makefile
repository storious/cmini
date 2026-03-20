%.o: %.c
	@clang $< -o $@ -std=c23

test_ptr: pointer_test.o utest.h
	@./pointer_test.o

test_utest: utest_test.o utest.h
	@./utest_test.o

run: main.o
	@./main.o

clean:
	@rm -f *.o
