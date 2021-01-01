#include <iostream>
#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <windows.h>
#define MAXLEN 1024

using namespace std;
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
struct sessions{
    string host;
    string port;
    string file;
    string id;
    vector<string> cmd;
};

/*----------------------NPShell----------------------*/
class NPShell : public enable_shared_from_this<NPShell>{
private:
    boost::asio::ip::tcp::resolver tcp_resolver;
    shared_ptr<boost::asio::ip::tcp::socket> tcp_socket;
    boost::asio::ip::tcp::socket npsocket;
    sessions tcp_request;
    array<char, MAXLEN> bytes;

public:
    NPShell(shared_ptr<boost::asio::ip::tcp::socket> socket, sessions request): tcp_resolver(ioservice), npsocket(ioservice), tcp_socket(socket), tcp_request(move(request)){}
    void start(){ do_resolve(); bytes.fill('\0'); }

private:
    void do_resolve(){
        auto self(shared_from_this());
        boost::asio::ip::tcp::resolver::query q(tcp_request.host, tcp_request.port);
        tcp_resolver.async_resolve(q, [this, self](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it){
                npsocket.async_connect(*it, [this, self](const boost::system::error_code &ec){
                    if(!ec) read_handler();
                });
        });
    }
    void read_handler(){
        auto self(shared_from_this());
        npsocket.async_read_some(
            boost::asio::buffer(bytes), [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec) {
                        string res = bytes.data();
                        bytes.fill('\0');
                        deal_html(res);
                        string output = "<script>document.getElementById('" + tcp_request.id + "').innerHTML += '<font color=\"white\">" + res + "';</script>\n";
                        tcp_socket->async_send(boost::asio::buffer(output, output.length()), [this, self](boost::system::error_code ec, size_t){});
                        if(tcp_request.cmd.size() > 0){
                            if(res.find("% ") != -1){
                                string cmd = tcp_request.cmd[0];
                                tcp_request.cmd.erase(tcp_request.cmd.begin());
                                if(cmd == "exit\n")
                                    tcp_request.cmd.clear();
                                npsocket.async_send(boost::asio::buffer(cmd, cmd.length()), [this, self, cmd](boost::system::error_code ec, size_t)mutable{
                                        deal_html(cmd);
                                        string output = "<script>document.getElementById('" + tcp_request.id + "').innerHTML += '<b>" + cmd + "</b>';</script>\n";
                                        tcp_socket->async_send(boost::asio::buffer(output, output.length()), [this, self](boost::system::error_code ec, size_t){});
                                });
                                read_handler();
                            }
                            else
                                read_handler();
                        }
                    }
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
        }
    }
};

/*--------------------Console.cgi--------------------*/
class Console : public enable_shared_from_this<Console>{
private:
    shared_ptr<boost::asio::ip::tcp::socket> tcp_socket;
    HttpRequest tcp_request;
    string output;
    vector<sessions> vect;

public:
    Console(boost::asio::ip::tcp::socket socket, HttpRequest request): tcp_socket(make_shared<boost::asio::ip::tcp::socket>(move(socket))), tcp_request(move(request)){}
    void start(){
        for(int i=0; i<5; i++){
            sessions s;
            vect.push_back(s);
        }
        get_query_string(tcp_request.query_string);
        print();
    }

private:
    void get_query_string(string QUERY_STRING){
        auto self(shared_from_this());
        string tmp;
        stringstream ss(QUERY_STRING);

        while(getline(ss, tmp, '&')){
            stringstream s2(tmp);
            string var, arg;

            getline(s2, var, '=');
            if(var[0] == 'h')
                getline(s2, vect[atoi(&var[1])].host, '=');
            if(var[0] == 'p')
                getline(s2, vect[atoi(&var[1])].port, '=');
            if(var[0] == 'f')
                getline(s2, vect[atoi(&var[1])].file, '=');
        }
        int pos = 0, num = 0;
        for(num; num<5; num++){
            sessions s = vect[pos];
            if( s.host == "" || s.port == "" || s.file == "")
                vect.erase(vect.begin()+pos);
            else{
                vect[pos].id = to_string(num);
                pos ++;
            }
        }
    }
    void parse_query(string query_string){
        auto self(shared_from_this());
        stringstream ss(query_string);
        string str;
        int i = 0;
        while(getline(ss, str, '&')){
            sessions new_req;
            bool input = true;
            int pos = str.find('=');
            new_req.host = str.substr(pos + 1, str.length() - pos);
            if(str.length() <= 3) input = false;
            getline(ss, str, '&');
            pos = str.find('=');
            new_req.port = str.substr(pos + 1, str.length() - pos);
            if(str.length() <= 3) input = false;
            getline(ss, str, '&');
            pos = str.find('=');
            new_req.file = str.substr(pos + 1, str.length() - pos);
            if(str.length() <= 3) input = false;
            new_req.id = "s" + to_string(i);
            i++;
            if(input)
                vect.push_back(new_req);
        }
    }

    void print(){
        auto self(shared_from_this());
        output = R"(Content-type: text/html

        <!DOCTYPE html>
        <html lang="en">
          <head>
            <meta charset="UTF-8" />
            <title>NP Project 3 Sample Console</title>
            <link
              rel="stylesheet"
              href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
              integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
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
                color: #01b468;
              }
            </style>
          </head>
          <body>
            <table class="table table-dark table-bordered">
              <thead>
                <tr>)";
        for(auto i : vect)
            output += "<th scope=\"col\">" + i.host + ".cs.nctu.edu.tw:" + i.port + "</th>";
        output += R"(</tr>
              </thead>
              <tbody>
                <tr>)";
        for(auto i : vect)
            output += "<td><pre id=\"" + i.id +"\" class=\"mb-0\"></pre></td>";
        output += R"(</tr>
              </tbody>
            </table>
          </body>
        </html>
        )";
        tcp_socket->async_send(boost::asio::buffer(output, output.length()),[this, self](boost::system::error_code ec, size_t){
                if(!ec){
                    for(auto &i : vect){
                        ifstream fd("./test_case/" + i.file);
                        string line;
                        while(getline(fd,line)){
                            i.cmd.push_back(line + "\r\n");
                        }
                        fd.close();
                        make_shared<NPShell>(tcp_socket, move(i))->start();
                    }
                }
        });
    }
};

/*--------------------HTTP Server--------------------*/
class HttpSession : public enable_shared_from_this<HttpSession>{
private:
    boost::asio::ip::tcp::socket tcp_socket;
    array<char, MAXLEN> bytes;
    vector<string> httpRequest;

public:
    HttpSession(boost::asio::ip::tcp::socket socket) : tcp_socket(move(socket)) {}
    void start() { do_read(); }

private:
    void do_read() {
        auto self(shared_from_this());
        tcp_socket.async_read_some(boost::asio::buffer(bytes), [this, self](boost::system::error_code ec, size_t length) {
                if (!ec){
                    string request = bytes.data();
                    bytes.fill('\0');

                    //parsing
                    stringstream ss(request);
                    HttpRequest info;
                    string str;
                    ss >> str;
                    info.request_method = str;
                    ss >> str;
                    info.request_uri = str;
                    info.filename = str.substr(1, str.find('?') - 1);
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

                    tcp_socket.send(boost::asio::buffer(string("HTTP/1.1 200 OK\r\n")));
                    if(info.filename == "panel.cgi"){
                        Panel();
                    }else if(info.filename == "console.cgi"){
                        make_shared<Console>(move(tcp_socket), move(info))->start();
                    }
                }
            });
    }

    void Panel(){
        auto self(shared_from_this());
        int N_SERVER = 5;
        string output = "";
        {output = R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <title>NP Project 3 Panel</title>
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
                href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
                />
                <style>
                * {
                    font-family: 'Source Code Pro', monospace;
                }
                </style>
            </head>
            <body class="bg-secondary pt-5">
                <form action="console.cgi" method="GET">
                <table class="table mx-auto bg-light" style="width: inherit">
                    <thead class="thead-dark">
                    <tr>
                        <th scope="col">#</th>
                        <th scope="col">Host</th>
                        <th scope="col">Port</th>
                        <th scope="col">Input File</th>
                    </tr>
                    </thead>
                    <tbody>
                    <tr>
                        <th scope="row" class="align-middle">Session 1</th>
                        <td>
                        <div class="input-group">
                            <select name="h0" class="custom-select">
                            <option></option><option value="nplinux1.cs.nctu.edu.tw">nplinux1</option><option value="nplinux2.cs.nctu.edu.tw">nplinux2</option><option value="nplinux3.cs.nctu.edu.tw">nplinux3</option><option value="nplinux4.cs.nctu.edu.tw">nplinux4</option><option value="nplinux5.cs.nctu.edu.tw">nplinux5</option><option value="nplinux6.cs.nctu.edu.tw">nplinux6</option><option value="nplinux7.cs.nctu.edu.tw">nplinux7</option><option value="nplinux8.cs.nctu.edu.tw">nplinux8</option><option value="nplinux9.cs.nctu.edu.tw">nplinux9</option><option value="nplinux10.cs.nctu.edu.tw">nplinux10</option><option value="nplinux11.cs.nctu.edu.tw">nplinux11</option><option value="nplinux12.cs.nctu.edu.tw">nplinux12</option>
                            </select>
                            <div class="input-group-append">
                            <span class="input-group-text">.cs.nctu.edu.tw</span>
                            </div>
                        </div>
                        </td>
                        <td>
                        <input name="p0" type="text" class="form-control" size="5" />
                        </td>
                        <td>
                        <select name="f0" class="custom-select">
                            <option></option>
                            <option value="t1.txt">t1.txt</option><option value="t2.txt">t2.txt</option><option value="t3.txt">t3.txt</option><option value="t4.txt">t4.txt</option><option value="t5.txt">t5.txt</option><option value="t6.txt">t6.txt</option><option value="t7.txt">t7.txt</option><option value="t8.txt">t8.txt</option><option value="t9.txt">t9.txt</option><option value="t10.txt">t10.txt</option>
                        </select>
                        </td>
                    </tr>
                    <tr>
                        <th scope="row" class="align-middle">Session 2</th>
                        <td>
                        <div class="input-group">
                            <select name="h1" class="custom-select">
                            <option></option><option value="nplinux1.cs.nctu.edu.tw">nplinux1</option><option value="nplinux2.cs.nctu.edu.tw">nplinux2</option><option value="nplinux3.cs.nctu.edu.tw">nplinux3</option><option value="nplinux4.cs.nctu.edu.tw">nplinux4</option><option value="nplinux5.cs.nctu.edu.tw">nplinux5</option><option value="nplinux6.cs.nctu.edu.tw">nplinux6</option><option value="nplinux7.cs.nctu.edu.tw">nplinux7</option><option value="nplinux8.cs.nctu.edu.tw">nplinux8</option><option value="nplinux9.cs.nctu.edu.tw">nplinux9</option><option value="nplinux10.cs.nctu.edu.tw">nplinux10</option><option value="nplinux11.cs.nctu.edu.tw">nplinux11</option><option value="nplinux12.cs.nctu.edu.tw">nplinux12</option>
                            </select>
                            <div class="input-group-append">
                            <span class="input-group-text">.cs.nctu.edu.tw</span>
                            </div>
                        </div>
                        </td>
                        <td>
                        <input name="p1" type="text" class="form-control" size="5" />
                        </td>
                        <td>
                        <select name="f1" class="custom-select">
                            <option></option>
                            <option value="t1.txt">t1.txt</option><option value="t2.txt">t2.txt</option><option value="t3.txt">t3.txt</option><option value="t4.txt">t4.txt</option><option value="t5.txt">t5.txt</option><option value="t6.txt">t6.txt</option><option value="t7.txt">t7.txt</option><option value="t8.txt">t8.txt</option><option value="t9.txt">t9.txt</option><option value="t10.txt">t10.txt</option>
                        </select>
                        </td>
                    </tr>
                    <tr>
                        <th scope="row" class="align-middle">Session 3</th>
                        <td>
                        <div class="input-group">
                            <select name="h2" class="custom-select">
                            <option></option><option value="nplinux1.cs.nctu.edu.tw">nplinux1</option><option value="nplinux2.cs.nctu.edu.tw">nplinux2</option><option value="nplinux3.cs.nctu.edu.tw">nplinux3</option><option value="nplinux4.cs.nctu.edu.tw">nplinux4</option><option value="nplinux5.cs.nctu.edu.tw">nplinux5</option><option value="nplinux6.cs.nctu.edu.tw">nplinux6</option><option value="nplinux7.cs.nctu.edu.tw">nplinux7</option><option value="nplinux8.cs.nctu.edu.tw">nplinux8</option><option value="nplinux9.cs.nctu.edu.tw">nplinux9</option><option value="nplinux10.cs.nctu.edu.tw">nplinux10</option><option value="nplinux11.cs.nctu.edu.tw">nplinux11</option><option value="nplinux12.cs.nctu.edu.tw">nplinux12</option>
                            </select>
                            <div class="input-group-append">
                            <span class="input-group-text">.cs.nctu.edu.tw</span>
                            </div>
                        </div>
                        </td>
                        <td>
                        <input name="p2" type="text" class="form-control" size="5" />
                        </td>
                        <td>
                        <select name="f2" class="custom-select">
                            <option></option>
                            <option value="t1.txt">t1.txt</option><option value="t2.txt">t2.txt</option><option value="t3.txt">t3.txt</option><option value="t4.txt">t4.txt</option><option value="t5.txt">t5.txt</option><option value="t6.txt">t6.txt</option><option value="t7.txt">t7.txt</option><option value="t8.txt">t8.txt</option><option value="t9.txt">t9.txt</option><option value="t10.txt">t10.txt</option>
                        </select>
                        </td>
                    </tr>
                    <tr>
                        <th scope="row" class="align-middle">Session 4</th>
                        <td>
                        <div class="input-group">
                            <select name="h3" class="custom-select">
                            <option></option><option value="nplinux1.cs.nctu.edu.tw">nplinux1</option><option value="nplinux2.cs.nctu.edu.tw">nplinux2</option><option value="nplinux3.cs.nctu.edu.tw">nplinux3</option><option value="nplinux4.cs.nctu.edu.tw">nplinux4</option><option value="nplinux5.cs.nctu.edu.tw">nplinux5</option><option value="nplinux6.cs.nctu.edu.tw">nplinux6</option><option value="nplinux7.cs.nctu.edu.tw">nplinux7</option><option value="nplinux8.cs.nctu.edu.tw">nplinux8</option><option value="nplinux9.cs.nctu.edu.tw">nplinux9</option><option value="nplinux10.cs.nctu.edu.tw">nplinux10</option><option value="nplinux11.cs.nctu.edu.tw">nplinux11</option><option value="nplinux12.cs.nctu.edu.tw">nplinux12</option>
                            </select>
                            <div class="input-group-append">
                            <span class="input-group-text">.cs.nctu.edu.tw</span>
                            </div>
                        </div>
                        </td>
                        <td>
                        <input name="p3" type="text" class="form-control" size="5" />
                        </td>
                        <td>
                        <select name="f3" class="custom-select">
                            <option></option>
                            <option value="t1.txt">t1.txt</option><option value="t2.txt">t2.txt</option><option value="t3.txt">t3.txt</option><option value="t4.txt">t4.txt</option><option value="t5.txt">t5.txt</option><option value="t6.txt">t6.txt</option><option value="t7.txt">t7.txt</option><option value="t8.txt">t8.txt</option><option value="t9.txt">t9.txt</option><option value="t10.txt">t10.txt</option>
                        </select>
                        </td>
                    </tr>
                    <tr>
                        <th scope="row" class="align-middle">Session 5</th>
                        <td>
                        <div class="input-group">
                            <select name="h4" class="custom-select">
                            <option></option><option value="nplinux1.cs.nctu.edu.tw">nplinux1</option><option value="nplinux2.cs.nctu.edu.tw">nplinux2</option><option value="nplinux3.cs.nctu.edu.tw">nplinux3</option><option value="nplinux4.cs.nctu.edu.tw">nplinux4</option><option value="nplinux5.cs.nctu.edu.tw">nplinux5</option><option value="nplinux6.cs.nctu.edu.tw">nplinux6</option><option value="nplinux7.cs.nctu.edu.tw">nplinux7</option><option value="nplinux8.cs.nctu.edu.tw">nplinux8</option><option value="nplinux9.cs.nctu.edu.tw">nplinux9</option><option value="nplinux10.cs.nctu.edu.tw">nplinux10</option><option value="nplinux11.cs.nctu.edu.tw">nplinux11</option><option value="nplinux12.cs.nctu.edu.tw">nplinux12</option>
                            </select>
                            <div class="input-group-append">
                            <span class="input-group-text">.cs.nctu.edu.tw</span>
                            </div>
                        </div>
                        </td>
                        <td>
                        <input name="p4" type="text" class="form-control" size="5" />
                        </td>
                        <td>
                        <select name="f4" class="custom-select">
                            <option></option>
                            <option value="t1.txt">t1.txt</option><option value="t2.txt">t2.txt</option><option value="t3.txt">t3.txt</option><option value="t4.txt">t4.txt</option><option value="t5.txt">t5.txt</option><option value="t6.txt">t6.txt</option><option value="t7.txt">t7.txt</option><option value="t8.txt">t8.txt</option><option value="t9.txt">t9.txt</option><option value="t10.txt">t10.txt</option>
                        </select>
                        </td>
                    </tr>
                    <tr>
                        <td colspan="3"></td>
                        <td>
                        <button type="submit" class="btn btn-info btn-block">Run</button>
                        </td>
                    </tr>
                    </tbody>
                </table>
                </form>
            </body>
            </html>)";
        }
        tcp_socket.async_send(boost::asio::buffer(output, output.length()),[this, self](boost::system::error_code ec, size_t){
                tcp_socket.close();
        });
    }
};

class HttpServer {
private:
    boost::asio::ip::tcp::acceptor tcp_acceptor;
    boost::asio::ip::tcp::socket tcp_socket;


public:
    HttpServer(short port) : tcp_acceptor(ioservice, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), tcp_socket(ioservice) {
            do_accept();
    }

private:
    void do_accept() {
            tcp_acceptor.async_accept(tcp_socket, [this](boost::system::error_code ec) {
                    if (!ec) {
                    make_shared<HttpSession>(move(tcp_socket))->start();
                    }
            do_accept();
            });
    }
};
/*---------------------------------------------------*/
BOOL WINAPI consoleHandler(DWORD signal){
    if (signal == CTRL_C_EVENT){
        cout << "User Interrupted";
        exit(-1);
    }
    return true;
}

int main(int argc, char* const argv[]){
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        cerr << "\nERROR: Could not set control handler";
        return 1;
    }
    if (argc != 2){
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }
    unsigned short port = atoi(argv[1]);
    HttpServer server(port);
    ioservice.run();
    return 0;
}
