CC=gcc
FLAGS= -g -Wall
LIBRARIES=-lhidapi-hidraw

%.o: %.c
	$(CC) $(FLAGS) -c $<
	

process_manager: connection.o process_manager.o read_joycon_input.o 
	$(CC) $(FLAGS) -o $@ $^ $(LIBRARIES)

clean:
	rm controller *.o
