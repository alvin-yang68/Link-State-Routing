CC=g++
CC_OPTS="-g -I."
CC_LIBS="-pthread"
CC_DEFINES=
CC_INCLUDES=
CC_ARGS=${CC_OPTS} ${CC_LIBS} ${CC_DEFINES} ${CC_INCLUDES}

ls: main.cpp monitor_neighbors.hpp
	@${CC} ${CC_ARGS} -o ls_router main.cpp monitor_neighbors/*.cpp link_state/*.cpp

# clean is not a file
.PHONY: clean

clean:
	rm *.o ls_router
