clean:
	rm ./coordinator ./peer ./lib/randomUtil.o

coordinator:
	gcc -o ./lib/randomUtil.o -c ./lib/randomUtil.c
	gcc -o ./peer ./peer.c ./lib/randomUtil.o -lm -pthread
	gcc -o ./coordinator ./coordinator.c ./lib/randomUtil.o -lm -pthread