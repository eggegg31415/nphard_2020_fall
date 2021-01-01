#include "socks_server.h"

class SocksConnect : public enable_shared_from_this<SocksConnect>{
private:
    shared_ptr<boost::asio::ip::tcp::socket> cli_socket;
    boost::asio::ip::tcp::socket ser_socket;
    SocksReq req;
    array<char, MAXLEN> bytes;
public:
    SocksConnect(boost::asio::ip::tcp::socket socket, SocksReq request):
            cli_socket(make_shared<boost::asio::ip::tcp::socket>(move(socket))),
            ser_socket(ioservice),
            req(request){}
    void start(int op){
        bytes.fill('\0');
        if(op == 1)         //Connect
            do_connect();
        else if(op == 2)    //Bind
            do_accept();
        else
            SocksReply(req, false);
    }
private:
    void SocksReply(SocksReq req, bool granted){
        auto self(shared_from_this());
        string msg;
        msg += '\0';
        msg += (granted ? 0x5A : 0x5B);
        for(int i=0; i<2; i++){
            msg += req.DSTPORT[i];
        }
        for(int i=0; i<4; i++){
            msg += req.DSTIP[i];
        }
        cli_socket->async_send(boost::asio::buffer(msg), [this, self, msg](boost::system::error_code ec, size_t){});
    }

    void SocksRedirect(bool enableser_socket, bool enablecli_socket){
        auto self(shared_from_this());
        if(enableser_socket){
            ser_socket.async_read_some(boost::asio::buffer(bytes, bytes.size()), [this, self](boost::system::error_code ec, size_t length){
                        if(!ec){
                            cli_socket->async_send(boost::asio::buffer(bytes, length), [this, self](boost::system::error_code ec, size_t){});
                            bytes.fill('\0');
                            SocksRedirect(1, 0);
                        }
                        if(ec == boost::asio::error::eof){
                            cli_socket->close();
                        }
                    });
        }
        if(enablecli_socket){
            cli_socket->async_read_some(boost::asio::buffer(bytes, bytes.size()), [this, self](boost::system::error_code ec, size_t length) {
                        if(!ec){
                            ser_socket.async_send(boost::asio::buffer(bytes, length), [this, self](boost::system::error_code ec, size_t){});
                            bytes.fill('\0');
                            SocksRedirect(0, 1);
                        }
                    });
        }
    }

    void do_connect(){
        auto self(shared_from_this());
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(req.D_IP), req.D_PORT);
        ser_socket.async_connect(ep, [this, self](const boost::system::error_code &ec){
                    if(!ec){
                        cout << "connection build to " << req.D_IP << ":" << req.D_PORT << endl;
                        SocksReply(req, true);
                        SocksRedirect(1, 1);
                    }
                    else
                        cout << "connect failed" << endl;
                });
    }

    void do_accept(){
        boost::asio::ip::tcp::acceptor tcp_acceptor(ioservice, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
        tcp_acceptor.listen();
        int b_port = tcp_acceptor.local_endpoint().port();
        req.D_PORT = b_port;
        req.DSTPORT[0] = b_port/256;
        req.DSTPORT[1] = b_port%256;
        for(int i=0; i<4; i++)
            req.DSTIP[i] = 0;
        SocksReply(req, true);
        try{
            tcp_acceptor.accept(ser_socket);
            SocksReply(req, true);
            SocksRedirect(1, 1);
        }
        catch (boost::system::error_code ec){
            cout << "failed: " << ec << endl;
        }
    }
};

class SocksSession : public enable_shared_from_this<SocksSession>{
private:
    boost::asio::ip::tcp::socket tcp_socket;
    boost::asio::ip::tcp::socket rsock;
    array<char, MAXLEN> bytes;

public:
    SocksSession(boost::asio::ip::tcp::socket socket) : tcp_socket(move(socket)), rsock(ioservice) {}
    void start() { do_read(); }

private:
    bool CheckFW(SocksReq req){
        char D_Mode = req.CD == 1 ? 'c' : 'b';
        ifstream infile("socks.conf");
        string line;

        while(getline(infile, line)){
            string str, mode, ip;
            stringstream ss(line);
            vector<string> sub_ip;

            ss >> str;
            ss >> mode;
            ss >> ip;
            if(str == "permit"){
                if(mode[0] == D_Mode){
                    stringstream ssip(ip);
                    string s;
                    while(getline(ssip, s, '.'))
                        sub_ip.push_back(s);

                    for(int i=0; i<4; i++){
                        s = to_string(Ch2Int(req.DSTIP, i, 1));
                        if(sub_ip[i] != s && sub_ip[i] != "*")  break;
                        if(i == 3)  return 1;
                    }
                }
            }
        }
        return 0;
    }

    int Ch2Int(string data, int begin, int bytes_num){
        int power = 1, ans = 0;
        for(int i=begin+bytes_num-1; i>=begin; i--){
            char ch = data[i];
            for(int j=0; j<8; j++){
                ans += ((ch & (1 << j))? power : 0);
                power *= 2;
            }
        }
        return ans;
    }

    void printinfo(SocksReq req){
        cout << "<S_IP>: " << req.S_IP << endl;
        cout << "<S_PORT>: " << req.S_PORT << endl;
        cout << "<D_IP>: " << req.D_IP << endl;
        cout << "<D_PORT>: " << req.D_PORT << endl;
        cout << "<Command>: ";
        if(req.CD == 1)
            cout << "CONNECT" << endl;
        else if(req.CD == 2)
            cout << "BIND" << endl;
    }

    void do_read(){
        auto self(shared_from_this());
        tcp_socket.async_read_some(boost::asio::buffer(bytes), [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec){
                        string data(length, '\0');
                        for(int i=0; i<length; i++)
                            data[i] = bytes[i];
                        bytes.fill('\0');

                        //parsing
                        SocksReq new_req;
                        new_req.VN = data[0];
                        new_req.CD = data[1];
                        new_req.D_PORT = Ch2Int(data, 2, 2);
                        for(int i=0; i<2; i++){
                            new_req.DSTPORT[i] = data[i+2];
                        }
                        new_req.D_IP = "";
                        for(int i=0; i<4; i++){
                            new_req.D_IP += to_string(Ch2Int(data, i+4, 1));
                            new_req.D_IP += (i == 3 ? "" : ".");
                            new_req.DSTIP[i] = data[i+4];
                        }

                        bool is_socks4a = 1;
                        for(int i=0; i<4; i++){
                            if(i != 3){
                                if(new_req.DSTIP[i] != 0){
                                    is_socks4a = 0;
                                    break;
                                }
                            }
                            else{
                                if(new_req.DSTIP[i] == 0){
                                    is_socks4a = 0;
                                    break;
                                }
                            }
                        }
                        new_req.DOMAIN_NAME = "";
                        if(is_socks4a){
                            int pos = 8;
                            while(data[pos++] != 0);
                            while(pos != length-1) new_req.DOMAIN_NAME += data[pos++];
                            cout << new_req.DOMAIN_NAME << endl;
                            boost::asio::ip::tcp::resolver resolv{ioservice};
                            auto results = resolv.resolve(new_req.DOMAIN_NAME, to_string(new_req.D_PORT));
                            for(auto entry : results) {
                                if(entry.endpoint().address().is_v4()) {
                                    new_req.D_IP = entry.endpoint().address().to_string();
                                    stringstream ss(new_req.D_IP);
                                    string s;

                                    for(int i=0; i<4; i++){
                                        getline(ss, s, '.');
                                        new_req.DSTIP[i] = Ch2Int(s, 0, s.length());
                                    }
                                    break;
                                }
                            }
                        }

                        boost::asio::ip::tcp::endpoint remote_ep = tcp_socket.remote_endpoint();
                        boost::asio::ip::address remote_ad = remote_ep.address();
                        new_req.S_IP =  remote_ad.to_string();
                        new_req.S_PORT = (int)remote_ep.port();

                        printinfo(new_req);
                        if(new_req.CD == 1){        //socks connect
                            if(CheckFW(new_req)){
                                cout << "<Reply> Accept" << endl << endl;
                                make_shared<SocksConnect>(move(tcp_socket), new_req)->start(1);
                            }
                            else{
                                cout << "<Reply> Reject" << endl << endl;
                                make_shared<SocksConnect>(move(tcp_socket), new_req)->start(0);
                            }
                        }
                        else if(new_req.CD == 2){   //socks bind
                            if(CheckFW(new_req)){
                                cout << "<Reply> Accept" << endl << endl;
                                make_shared<SocksConnect>(move(tcp_socket), new_req)->start(2);
                            }
                            else{
                                cout << "<Reply> Reject" << endl << endl;
                                make_shared<SocksConnect>(move(tcp_socket), new_req)->start(0);
                            }
                        }
                        do_read();
                    }
                });
    }
};

class SocksServer {
private:
    boost::asio::ip::tcp::acceptor tcp_acceptor;
    boost::asio::ip::tcp::socket tcp_socket;

public:
    SocksServer(short port) : tcp_acceptor(ioservice, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), tcp_socket(ioservice) {
                do_accept();
            }

private:
    void do_accept(){
            boost::asio::socket_base::reuse_address option(true);
            tcp_acceptor.set_option(option);
            tcp_acceptor.async_accept(tcp_socket, [this](boost::system::error_code ec) {
                    if (!ec) {
                        ioservice.notify_fork(boost::asio::io_service::fork_prepare);
                        if(fork() == 0){
                            ioservice.notify_fork(boost::asio::io_service::fork_child);
                            make_shared<SocksSession>(move(tcp_socket))->start();
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

    SocksServer server(port);
    ioservice.run();
}
