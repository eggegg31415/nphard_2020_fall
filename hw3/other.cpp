#include <iostream>
#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <vector>
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

io_service global_io_service;

struct query_request{
    string hostname;
    string port;
    string filename;
    string session;
    vector<string> command;
};

void output_shell(string session, string content);
void output_command(string session, string content);
string xml_escape(string cmd);
void parse_query(string query_string, vector<query_request> &vect);

class Console : public enable_shared_from_this<Console>{
    private:
        tcp::resolver _resolver;
        tcp::socket _socket;
        query_request _request;
        enum {max_length = 15000};
        array<char, max_length> _data;

    public:
        Console(query_request request)
            : _resolver(global_io_service), _socket(global_io_service), _request(move(request)){}
        void start(){
            do_resolve();
        }
    private:
        void do_resolve() {
            auto self(shared_from_this());
            tcp::resolver::query q(_request.hostname, _request.port);
            _resolver.async_resolve(q, [this, self](const boost::system::error_code &ec, tcp::resolver::iterator it){
                _socket.async_connect(*it, [this, self](const boost::system::error_code &ec){
                    do_read();
                });
            });
        }
        void do_read(){
            auto self(shared_from_this());
            _socket.async_read_some(
                buffer(_data, max_length),
                [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec) {
                        string res = _data.data();
                        _data.fill('\0');
                        output_shell(_request.session, res);
                        if(_request.command.size() > 0){
                            if(res.find("% ") != -1){

                                string cmd = _request.command[0];
                                _request.command.erase(_request.command.begin());
                                if(cmd == "exit\n")
                                    _request.command.clear();
                                do_write(cmd);
                            }else
                                do_read();
                        }
                    }
                });
        }
        void do_write(string cmd){
            auto self(shared_from_this());
            _socket.async_send(
                buffer(cmd, cmd.length()),
                [this, self, cmd](boost::system::error_code ec, size_t /* length */) {
                    if (!ec) {
                        output_command(_request.session, cmd);
                        do_read();
                    }
                });
        }
};

int main(){
    string query_string = getenv("QUERY_STRING");
    vector<query_request> vect;
    parse_query(query_string, vect);

    cout << "Content-type: Text/html" << endl << endl;
    cout << R"(
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8" />
                <title>NP Project 3 Console</title>
                <link
                rel="stylesheet"
                href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css"
                integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
                crossorigin="anonymous"
                />
                <link
                href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
                rel="stylesheet"
                />
                <link
                rel="icon"
                type="image/png"
                href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
                />
                <style>
                * {
                    font-family: 'Source Code Pro', monospace;
                    font-size: 1rem !important;
                }
                body {
                    background-color: #212529;
                }
                pre {
                    color: #cccccc;
                }
                b {
                    color: #ffffff;
                }
                </style>
            </head>
            <body>
                <table class="table table-dark table-bordered">
                <thead>
                    <tr>
    )";
    for(int i = 0; i < vect.size(); i++)
        cout << "<th scope=\"col\">" << vect[i].hostname << ":" << vect[i].port << "</th>" << endl;

    cout << R"(
                    </tr>
                </thead>
                <tbody>
                    <tr>
    )";
    for(int i = 0; i < vect.size(); i++)
        cout << "<td><pre id=\"s" << i << "\" class=\"mb-0\"></pre></td>" << endl;

    cout << R"(
                    </tr>
                </tbody>
                </table>
            </body>
        </html>
    )";

    for(int i = 0; i < vect.size(); i++){
        ifstream f("./test_case/" + vect[i].filename);
        string cmd;
        while(getline(f,cmd))
            vect[i].command.push_back(cmd + '\n');
        f.close();
        make_shared<Console>(move(vect[i]))->start();
    }
    global_io_service.run();

    return 0;
}

void output_shell(string session, string content){
    content = xml_escape(content);
    cout << "<script>document.getElementById('"
        << session << "').innerHTML += '"
        << content << "';</script>";
    cout.flush();
}

void output_command(string session, string content){
    content = xml_escape(content);
    cout << "<script>document.getElementById('"
        << session << "').innerHTML += '<b>"
        << content << "</b>';</script>";
    cout.flush();
}

string xml_escape(string cmd){
    stringstream ss(cmd);
    string res;
    for(int i = 0; i < cmd.length(); i++){
        char c = cmd[i];
        switch(c){
            case '&':   res += "&amp;";     break;
            case '\"':  res += "&quot;";    break;
            case '\'':  res += "&apos;";    break;
            case '<':   res += "&lt;";      break;
            case '>':   res += "&gt;";      break;
            case '\n':  res += "&NewLine;"; break;
            case '\r':                      break;
            default:   res += c;            break;
        }
    }
    return res;
}

void parse_query(string query_string, vector<query_request> &vect){
    stringstream ss(query_string);
    string str;
    int i = 0;
    while(getline(ss, str, '&')){
        query_request new_req;
        bool input = true;
        int pos = str.find('=');
        new_req.hostname = str.substr(pos + 1, str.length() - pos);
        if(str.length() <= 3) input = false;
        getline(ss, str, '&');
        pos = str.find('=');
        new_req.port = str.substr(pos + 1, str.length() - pos);
        if(str.length() <= 3) input = false;
        getline(ss, str, '&');
        pos = str.find('=');
        new_req.filename = str.substr(pos + 1, str.length() - pos);
        if(str.length() <= 3) input = false;
        new_req.session = "s" + to_string(i);
        i++;
        if(input)
            vect.push_back(new_req);
    }

}
