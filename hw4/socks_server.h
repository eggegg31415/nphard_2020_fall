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
#include <sys/types.h>
#include <bitset>
#define MAXLEN 20000
using namespace std;

boost::asio::io_service ioservice;

struct SocksReq{
    int VN;
    int CD;
    int S_PORT;
    string S_IP;
    int D_PORT;
    char DSTPORT[2];
    string D_IP;
    char DSTIP[4];
    string DOMAIN_NAME;
};
