CC:=g++
CFLAGS:=-std=c++14 -pedantic -pthread -lboost_system

all: clean
	$(CC) $(CFLAGS) http_server.cpp -o http_server
	$(CC) console.cpp -o console.cgi -pthread -std=c++14
	$(CC) other.cpp -o other.cgi -pthread -std=c++14

part1:
	$(CC) $(CFLAGS) http_server.cpp -o http_server
	$(CC) console.cpp -o console.cgi -pthread -std=c++14

part2:
	$(CC) cgi_server.cpp -o cgi_server -lws2_32 -lwsock32 -std=c++14

clean:
	rm -f ./*.cgi ~/public_html/cgi/*.cgi
