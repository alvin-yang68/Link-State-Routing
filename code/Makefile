CC=g++
CC_OPTS=-g -I.
CC_LIBS=-pthread

ls: *.cpp */*.cpp
	@${CC} ${CC_OPTS} -o ls_router *.cpp */*.cpp ${CC_LIBS}

# clean is not a file
.PHONY: clean

clean:
	rm -f *.o ls_router
