server:	
	rm -rf ./server
	g++ ./src/client.cpp ./src/httpserver.cpp ./src/main.cpp -std=c++14 -lpthread -lmysqlclient -o ./server
debug: 
	rm -rf ./server-debug
	g++ ./src/client.cpp ./src/httpserver.cpp ./src/main.cpp -std=c++14 -g -lpthread -lmysqlclient -o ./server-debug

clean:
	rm -rf ./server
	rm -rf ./server-debug