CC=g++
CC_OPTS=-g -I.
CC_LIBS=-pthread -lboost_serialization -lboost_system

ls: main.cpp link_state/*.cpp io/*.cpp
	@${CC} ${CC_OPTS} -o ls_router main.cpp link_state/*.cpp io/*.cpp ${CC_LIBS}

# clean is not a file
.PHONY: clean

clean:
	rm -f *.o ls_router
