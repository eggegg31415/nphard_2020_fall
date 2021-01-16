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
//#include <sys/wait.h>
#define MAXLEN 1024
using namespace std;
using namespace boost::asio;

boost::asio::io_service ioservice;

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
    ip::tcp::socket tcp_socket;
    array<char, MAXLEN> bytes;
    vector<string> httpRequest;

public:
    HttpSession(ip::tcp::socket socket) : tcp_socket(move(socket)) {}
    void start() { do_read(); }

private:
    void do_read() {
        auto self(shared_from_this());
        tcp_socket.async_read_some(boost::asio::buffer(bytes), [this, self](boost::system::error_code ec, size_t length) {
                if (!ec){
                    string str = bytes.data();
                    bytes.fill('\0');

                    stringstream ss(str);
                    HttpRequest info;
                    /******************************** example http request ************
                     * GET /~yian31415/cgi/console.cgi?h0=nplinux1.cs.nctu.edu.tw&p0=1234&f0=t1.txt&h1=&p1=&f1=&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4= HTTP/1.1
                     * Host: nplinux1.cs.nctu.edu.tw:8888
                     * Connection: keep-alive
                     * Cache-Control: max-age=0
                     * Upgrade-Insecure-Requests: 1
                     * User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36
                     * Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,* / *;q=0.8,application/signed-exchange;v=b3;q=0.9
                     * Accept-Encoding: gzip, deflate
                     * Accept-Language: zh-TW,zh;q=0.9
                     ******************************************************************/
                    ss >> str;
                    info.request_method = str;
                    cout << info.request_method;
                    ss >> str;
                    info.request_uri = str;
                    info.filename = "./" + str.substr(1, str.find('?') - 1);
                    str = str.substr(str.find('?') + 1, str.length() - str.find('?'));
                    info.query_string = str;
                    ss >> str;
                    info.server_protocol = str;
                    ss >> str >> str;
                    info.http_host = str;
                    info.server_addr = tcp_socket.local_endpoint().address().to_string();
                    info.server_port = to_string(tcp_socket.local_endpoint().port());
                    info.remote_addr = tcp_socket.remote_endpoint().address().to_string();
                    info.remote_port = to_string(tcp_socket.remote_endpoint().port());

                    setenv("REQUEST_METHOD", info.request_method.c_str(), 1);
                    setenv("REQUEST_URI", info.request_uri.c_str(), 1);
                    setenv("QUERY_STRING", info.query_string.c_str(), 1);
                    setenv("SERVER_PROTOCOL", info.server_protocol.c_str(), 1);
                    setenv("HTTP_HOST", info.http_host.c_str(), 1);
                    setenv("SERVER_ADDR", info.server_addr.c_str(), 1);
                    setenv("SERVER_PORT", info.server_port.c_str(), 1);
                    setenv("REMOTE_ADDR", info.remote_addr.c_str(), 1);
                    setenv("REMOTE_PORT", info.remote_port.c_str(), 1);

                    tcp_socket.send(buffer(string("HTTP/1.1 200 OK\r\n")));

                    int fd = tcp_socket.native_handle();
                    dup2(fd, STDOUT_FILENO);
                    char *argv[2];
                    strcpy(argv[0], info.filename.c_str());
                    argv[1] = (char*)NULL;
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
    boost::asio::ip::tcp::acceptor tcp_acceptor;
    boost::asio::ip::tcp::socket tcp_socket;

public:
    HttpServer(short port) : tcp_acceptor(ioservice, ip::tcp::endpoint(ip::tcp::v4(), port)), tcp_socket(ioservice) {
            do_accept();
    }

private:
    void do_accept(){
            tcp_acceptor.async_accept(tcp_socket, [this](boost::system::error_code ec) {
                    if (!ec) {
                        ioservice.notify_fork(boost::asio::io_service::fork_prepare);
                        if(fork() == 0){
                            ioservice.notify_fork(boost::asio::io_service::fork_child);
                            make_shared<HttpSession>(move(tcp_socket))->start();
                        }
                        else{
                            ioservice.notify_fork(boost::asio::io_service::fork_parent);
                            tcp_socket.close();
                            do_accept();
                        }
                    }
                    do_accept();
            });
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "./http_server [port]" << endl;
        return 69;
    }
    unsigned short port = atoi(argv[1]);
    signal(SIGCHLD, SIG_IGN);

    HttpServer server(port);
    ioservice.run();
}
