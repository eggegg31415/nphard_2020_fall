#include <iostream>
#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;
using namespace boost::asio;

io_service global_io_service;

struct HttpRequest{
    string request_method;
    string request_uri;
    string query_string;
    string server_protocol;
    string http_host;
    string server_addr;
    string server_port;
    string remote_addr;
    string remote_port;
    string filename;
};

class HttpSession : public enable_shared_from_this<HttpSession>{
    private:
        enum { max_length = 1024 };
        ip::tcp::socket _socket;
        array<char, max_length> _data;
        vector<string> httpRequest;

    public:
        HttpSession(ip::tcp::socket socket) : _socket(move(socket)) {}

    void start() { do_read(); }

    private:
        void do_read() {
            auto self(shared_from_this());
            _socket.async_read_some(
                buffer(_data, max_length),
                [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec){
                        string request = _data.data();
                        _data.fill('\0');
                        //parsing
                        stringstream ss(request);
                        HttpRequest new_req;
                        string str;
                        ss >> str;          new_req.request_method = str;
                        ss >> str;          new_req.request_uri = str;
                        new_req.filename = "./" + str.substr(1, str.find('?') - 1);
                        str = str.substr(str.find('?') + 1, str.length() - str.find('?'));
                                            new_req.query_string = str;
                        ss >> str;          new_req.server_protocol = str;
                        ss >> str >> str;   new_req.http_host = str;
                        new_req.server_addr = _socket.local_endpoint().address().to_string();
                        new_req.server_port = to_string(_socket.local_endpoint().port());
                        new_req.remote_addr = _socket.remote_endpoint().address().to_string();
                        new_req.remote_port = to_string(_socket.remote_endpoint().port());

                        setenv("REQUEST_METHOD", new_req.request_method.c_str(), true);
                        setenv("REQUEST_URI", new_req.request_uri.c_str(), true);
                        setenv("QUERY_STRING", new_req.query_string.c_str(), true);
                        setenv("SERVER_PROTOCOL", new_req.server_protocol.c_str(), true);
                        setenv("HTTP_HOST", new_req.http_host.c_str(), true);
                        setenv("SERVER_ADDR", new_req.server_addr.c_str(), true);
                        setenv("SERVER_PORT", new_req.server_port.c_str(), true);
                        setenv("REMOTE_ADDR", new_req.remote_addr.c_str(), true);
                        setenv("REMOTE_PORT", new_req.remote_port.c_str(), true);

                        _socket.send(buffer(string("HTTP/1.1 200 OK\r\n")));
                        if (new_req.filename.find(".cpp") != -1){
                            string cmd = "clang++ " + new_req.filename;
                            const char *cmd_c = cmd.c_str();
                            system(cmd_c);
                            new_req.filename = "./a.out";
                        }
                        int fd = _socket.native_handle();
                        dup2(fd, STDOUT_FILENO);
                        char *argv[2];
                        strcpy(argv[0], new_req.filename.c_str());
                        argv[1] = (char*)NULL;
                        // cerr << argv[0] << endl;
                        if(execvp(argv[0], argv) == -1){
                            cerr << "Unknown command: [" << argv[0] << "]. " << endl;
                            exit(-1);
                        }
                        exit(0);
                    }
                });
        }
};

class HttpServer {
    private:
        ip::tcp::acceptor _acceptor;
        ip::tcp::socket _socket;

    public:
        HttpServer(short port)
            : _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
              _socket(global_io_service) {
            do_accept();
        }

    private:
        void do_accept() {
            _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
            if (!ec) {
                global_io_service.notify_fork(boost::asio::io_service::fork_prepare);
                if(fork() == 0){        //child
                    global_io_service.notify_fork(boost::asio::io_service::fork_child);
                    make_shared<HttpSession>(move(_socket))->start();
                }else{                  //parent
                    global_io_service.notify_fork(boost::asio::io_service::fork_parent);
                    _socket.close();
                    do_accept();
                }
            }
            do_accept();
        });
  }
};

void Child_Handler(int signo){
    int status;
    while ( waitpid(-1, &status, WNOHANG) > 0 );
}

int main(int argc, char* const argv[]) {
    if (argc != 2) {
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }
    try {

        unsigned short port = atoi(argv[1]);

        signal(SIGCHLD, Child_Handler);

        HttpServer server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
