all: clean
	g++ http_server.cpp -o http_server -std=c++14 -pedantic -pthread -lboost_system
	g++ console.cpp -o console.cgi -pthread -std=c++14
	g++ other.cpp -o other.cgi -pthread -std=c++14
	cp extra_files/cgi/panel.cgi .
	cp *.cgi ~/public_html/cgi
part1:
	g++ http_server.cpp -o http_server -std=c++14 -pedantic -pthread -lboost_system
	g++ console.cpp -o console.cgi -pthread -std=c++14
part2:
	g++ cgi_server.cpp -o cgi_server -lws2_32 -lwsock32 -std=c++14
clean:
	rm -f ./*.cgi
	rm -f ~/public_html/cgi/*.cgi
