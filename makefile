serverName=server
clientName=client

all: compServer compClient

server: compServer runServer

client: compClient runClient

compClient:
	gcc -o $(clientName) $(clientName).c lib/$(clientName).c

compServer:
	gcc -o $(serverName) $(serverName).c lib/$(serverName).c

runClient:
	./$(clientName)

runServer:
	./$(serverName)
