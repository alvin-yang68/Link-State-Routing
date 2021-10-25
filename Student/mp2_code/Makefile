CC=g++
CC_OPTS=-g -I.
CC_LIBS=-pthread

all: ls manager

ls: main.cpp link_state/*.cpp io/*.cpp
	@${CC} ${CC_OPTS} -o ls_router main.cpp link_state/*.cpp io/*.cpp common/*.cpp ${CC_LIBS}

manager: manager_send.cpp
	@${CC} -o manager_send manager_send.cpp

# clean is not a file
.PHONY: clean

clean:
	rm -f *.o ls_router manager_send
