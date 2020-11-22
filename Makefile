server: 
	mkdir -p bin
	g++ httpserver.cpp main.cpp -std=c++14 -lpthread -lmysqlclient -o ./bin/server

clean:
	rm -rf ./bin/server