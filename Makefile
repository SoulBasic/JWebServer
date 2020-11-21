server: 
	mkdir -p bin
	g++ main.cpp -std=c++14 -lpthread -o ./bin/server

clean:
	rm -rf ./bin/server