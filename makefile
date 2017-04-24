all:
	gcc -o -Wall -std=gnu11 server.c utils.c -o server -lrt
	gcc -o -Wall -std=gnu11 client.c utils.c -o client -lrt

runserver:
	./server

runclient:
	./client
