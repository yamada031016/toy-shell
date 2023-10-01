CFLAGS=-std=c11 -Os -s

toysh: toysh.c

test: toysh
				./test.sh

clean: 
				rm -f toysh *.o *~ tmp*
	
.PHONY: test clean
