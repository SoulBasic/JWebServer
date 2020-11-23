server: 
	mkdir -p ./bin
	g++ ./src/httpserver.cpp ./src/main.cpp -std=c++14 -lpthread -lmysqlclient -o ./bin/server

clean:
	rm -rf ./bin/server