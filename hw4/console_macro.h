#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <boost/asio.hpp>
#include <memory>
#include <utility>
#include <array>
#define MAXSERVER 5
#define MAXLEN 1024
using namespace std;

//basic cgi argument
vector<string> console_host(MAXSERVER);
vector<string> console_port(MAXSERVER);
vector<string> console_file(MAXSERVER);
vector<string> console_id;
struct sessions{
    string host;
    string port;
    string file;
    string id;
    vector<string> cmd;
};

// boost asio define
boost::asio::io_service ioservice;
sessions PHInfo;

class Console : public enable_shared_from_this<Console> {
private:
    boost::asio::ip::tcp::resolver resolv{ioservice};
    boost::asio::ip::tcp::socket tcp_socket{ioservice};
    sessions info;
    array<char, MAXLEN> bytes;

public:
    Console(sessions s) : resolv(ioservice), tcp_socket(ioservice), info(move(s)){}
    void start(){
        connect_server();
    }
private:
    void read_handler(){
        auto self(shared_from_this());
        tcp_socket.async_read_some(boost::asio::buffer(bytes), [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec) {
                        string str = bytes.data();
                        bytes.fill('\0');
                        deal_html(str);
                        cout << "<script>document.getElementById('" << info.id << "').innerHTML += '<font color=\"white\">" << str << "</font>';</script>\n";
                        cout.flush();
                        if(info.cmd.size() != 0){
                            if(str.find("%") != -1){
                                string msg = info.cmd[0];
                                tcp_socket.async_send(boost::asio::buffer(msg), [this, self, msg](boost::system::error_code ec, size_t){});
                                deal_html(msg);
                                cout << "<script>document.getElementById('" << info.id << "').innerHTML += '<font color=\"green\">" << msg << "';</script>\n";
                                cout.flush();
                                info.cmd.erase(info.cmd.begin());
                                read_handler();
                            }
                            else
                                read_handler();
                        }
                    }
                });
    }

    void socks_connect(){
        auto self(shared_from_this());
        string msg;
        msg += 0x04;
        msg += 0x01;
        int port = stoi(info.port);
        boost::asio::ip::tcp::resolver::query q{info.host, info.port};
        msg += port/256;
        msg += port%256;
        resolv.async_resolve(q, [this, self, msg](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it)mutable{
                    boost::asio::ip::tcp::endpoint ep = *it;
                    string ips = ep.address().to_string();
                    for(int i=0; i<4; i++){
                        int pos = ips.find(".");
                        string part = ips.substr(0, pos);
                        msg += stoi(part);
                        ips.erase(0, pos+1);
                    }
                    msg += '\0';
                    tcp_socket.async_send(boost::asio::buffer(msg), [this, self, msg](boost::system::error_code ec, size_t){});
                });
    }

    void connect_server(){
        auto self(shared_from_this());
        int session_id = stoi(info.id);
        string file_name = "test_case/" + info.file;

        boost::asio::ip::tcp::resolver::query q{PHInfo.host, PHInfo.port};
        resolv.async_resolve(q, [this, self](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it){
                    tcp_socket.async_connect(*it, [this, self, it](const boost::system::error_code &ec){
                                if(!ec){
                                    socks_connect();
                                    read_handler();
                                }
                                else
                                    cout << "<script>document.getElementById('" << info.id << "').innerHTML += '<font color=\"white\">Error: " << ec << "</font>';</script>\n";
                            });
                });
    }

    void deal_html(string &str){
        for(int i=0; i<str.length(); i++){
            if(str[i] == '\r')
                str.erase(str.begin()+(i--));
            else if(str[i] == '\n')
                str.replace(i, 1, "&NewLine;");
            else if(str[i] == '\'')
                str.replace(i, 1, "&apos;");
            else if(str[i] == '\"')
                str.replace(i, 1, "&quot;");
            else if(str[i] == '>')
                str.replace(i, 1, "&gt;");
            else if(str[i] == '<')
                str.replace(i, 1, "&lt;");
        }
    }
};
