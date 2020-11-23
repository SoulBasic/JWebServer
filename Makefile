server: 
	mkdir -p ./bin
	g++ ./src/client.cpp ./src/httpserver.cpp ./src/main.cpp -std=c++14 -lpthread -lmysqlclient -o ./bin/server
debug: 
	mkdir -p ./debug
	g++ ./src/client.cpp ./src/httpserver.cpp ./src/main.cpp -std=c++14 -g -lpthread -lmysqlclient -o ./debug/server

clean:
	rm -rf ./bin/server
	rm -rf ./debug/server