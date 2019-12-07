CC=gcc
FLAGS= -g -Wall -std=gnu11 -pthread
LIBRARIES= -lhidapi-hidraw -lm

%.o: %.c
	$(CC) $(FLAGS) -c $<
	

process_manager: connection.o location.o process_manager.o read_joycon_input.o 
	$(CC) $(FLAGS) -o $@ $^ $(LIBRARIES)

clean:
	rm process_manager *.o
