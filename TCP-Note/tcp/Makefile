.PHONY:all
all:client server
client:client.c
	gcc -g -o $@ $^ -lpthread
server:server.c
	gcc -g -o $@ $^ -lpthread
.PHONY:clean
clean:
	rm -f client server
