default: test

test: convert.c test.c
	clang -DTESTING -std=c11 -Wall -pedantic -g convert.c test.c -o $@ \
	    -fsanitize=undefined -fsanitize=address

convert: convert.c
	clang -std=c11 -Wall -pedantic -g convert.c displayfull.c -lSDL2 -o $@ \
	    -fsanitize=undefined -fsanitize=address

%: %.c
	clang -Dtest_$@ -std=c11 -Wall -pedantic -g $@.c -o $@ \
	    -fsanitize=undefined -fsanitize=address
