all:
	g++ server.cpp -o server.out -lpthread
	g++ client.cpp -o client.out -lpthread
clean:
	rm -f client.out
	rm -f server.out
