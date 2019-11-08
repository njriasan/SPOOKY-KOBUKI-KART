CC=gcc
FLAGS= -g -Wall
LIBRARIES=-lhidapi-hidraw

%.o: %.c
	$(CC) $(FLAGS) -c $<
	

controller: connection.o controller.o read_joycon_input.o 
	$(CC) $(FLAGS) -o $@ $^ $(LIBRARIES)

clean:
	rm controller *.o
