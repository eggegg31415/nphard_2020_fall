#include "macro.h"
#include "function.h"
#include "shell.h"
#include "rwg.h"

int main(int argc, char* argv[]){
    // Check argument number
    if(argc != 2){
        cerr << "./server [port]" << endl;
        return 69;
    }

    // Server prepare
    int listenfd, connfd;
    int nready;
    int flag;
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);


    // Prepare client array
    int maxfd, maxi;
    maxfd = listenfd;
    maxi = -1;
    for(int i=0; i<LISTENQ; i++){
        client[i] = -1;
        name[i] = "(no name)";
        ipaddr[i] = "";
        port[i] = 0;
        process_line[i] = 0;
        env[i]["PATH"] = "bin:.";
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    //Loop
    while(1){
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        socklen_t clilen = sizeof(cliaddr);
        if(FD_ISSET(listenfd, &rset)){
            //Cew client connection
            connfd = accept(listenfd, (SA*) &cliaddr, &clilen);
            if(connfd < 0)
                printf("Connected fail\n");

            //Add new user fd to array
            int i;
            for(i=1; i<LISTENQ; i++){
                if(client[i] < 0)
                    break;
            }
            if(i == LISTENQ)
                printf("Too many client\n");
            else{
                int j;
                char addr[20];

                inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, addr, sizeof(cliaddr));
                client[i] = connfd;
                port[i] = ntohs(cliaddr.sin_port);
                ipaddr[i] = string(addr);

                string msg = "*** User '" + name[i]  + "' entered from " + ipaddr[i] + ":" + to_string(port[i]) + ". ***\n";
                for(j=1; j<LISTENQ; j++){
                    if(j == i){
                        string newer_msg = MOTD + msg + "% ";
                        write(client[j], newer_msg.c_str(), newer_msg.length());
                    }
                    else
                        write(client[j], msg.c_str(), msg.length());
                }
                cout << "*** User '" << name[i]  << "' entered from " << ipaddr[i] << ":" << port[i] << ". ***" << endl;
            }
            //Add new user to set
            FD_SET(connfd, &allset);
            if(connfd > maxfd)  maxfd = connfd;
            if(i > maxi)        maxi = i;
            if(--nready <= 0)   continue;
        }
        for(int i=0; i<=maxi; i++){
            //Check all clients for data
            int sockfd = client[i];
            if(sockfd < 0)  //empty
                continue;
            if(FD_ISSET(sockfd, &rset)){
                func(sockfd, i);
                write(sockfd, "% ", strlen("% "));
                if(--nready <= 0)
                    break;
            }
        }
    }
    return 0;
}
