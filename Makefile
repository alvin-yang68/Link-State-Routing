CC=g++
CC_OPTS=-g -I ./link_state
CC_LIBS=-pthread

all: ls manager

ls: link_state/*.cpp link_state/common/*.cpp link_state/io/*.cpp link_state/topology/*.cpp
	@${CC} ${CC_OPTS} -o ls_router link_state/*.cpp link_state/common/*.cpp link_state/io/*.cpp link_state/topology/*.cpp ${CC_LIBS}

manager: example/manager_send.cpp
	@${CC} -o example/manager_send example/manager_send.cpp

# clean is not a file
.PHONY: clean

clean:
	rm -f *.o ls_router
