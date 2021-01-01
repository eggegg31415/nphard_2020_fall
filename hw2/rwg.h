void rwg_who(vector<string> arg, int sockfd, int cli){
    string msg = "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";
    for(int i=1; i<LISTENQ; i++){
        if(client[i] > 0){
            msg = msg + to_string(i) + "\t" + name[i] + "\t" + ipaddr[i] + ":" + to_string(port[i]);
            if(i == cli)
                msg = msg + "\t<-me";
            msg = msg + "\n";
        }
    }
    write(sockfd, msg.c_str(), msg.length());

    cout << "who" << endl;
}

void rwg_tell(vector<string> arg, int sockfd, int cli){
    string msg = "*** " + name[cli] + " told you ***: ";

    for(auto i : arg[1]){
        if(i < '0' || i > '9'){
            string errmsg = "*** tell <user id> <message> ***\n";
            write(sockfd, errmsg.c_str(), errmsg.length());
            return;
        }
    }
    int towho = stoi(arg[1]);
    for(int i=2; i<arg.size()-1; i++)
        msg = msg + arg[i] + " ";
    msg = msg + arg[arg.size()-1] + "\n";

    if(client[towho] == -1){
        string errmsg = "*** Error: user #" + to_string(towho) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    }
    else
        write(client[towho], msg.c_str(), msg.length());

    cout << "tell" << endl;
}

void rwg_yell(vector<string> arg, int sockfd, int cli){
    string msg = "*** " + name[cli] + " yelled ***: ";
    for(int i=1; i<arg.size()-1; i++)
        msg = msg + arg[i] + " ";
    msg = msg + arg[arg.size()-1] + "\n";

    for(int i=1; i<LISTENQ; i++){
        if(client[i] >= 0){
            write(client[i], msg.c_str(), msg.length());
        }
    }
    cout << "yell" << endl;
}

void rwg_name(vector<string> arg, int sockfd, int cli){
    string newname = arg[1];
    string msg = "*** User from " + ipaddr[cli] + ":" + to_string(port[cli]) + " is named '" + newname + "'. ***\n";

    for(int i=1; i<LISTENQ; i++){
        if(name[i] == newname){
            string msg = "*** User '" + newname + "' already exists. ***\n";
            write(sockfd, msg.c_str(), msg.length());
            return;
        }
    }

    name[cli] = newname;
    for(int i=1; i<LISTENQ; i++){
        if(client[i] >= 0){
            write(client[i], msg.c_str(), msg.length());
        }
    }
    cout << "name" << endl;
}

void rwg_exit(vector<string> arg, int sockfd, int cli){
    string msg = "*** User '" + name[cli] + "' left. ***\n";
    for(int i=1; i<LISTENQ; i++){
        if(client[i] >= 0 && i != cli)
            write(client[i], msg.c_str(), msg.length());
    }

    // Clean user pipe
    stringstream ss;
    ss << setw(2) << setfill('0') << cli;
    string user_id = ss.str();
    map<string, struct user_pipe>::iterator iter;
    for(iter=active_user_pipe.begin(); iter != active_user_pipe.end(); iter++){
        string key = iter->first;
        if(key.compare(0, 2, user_id) || key.compare(key.size()-2, 2, user_id))
            active_user_pipe.erase(key);
    }
    for(int i=0; i<MAXPIPE; i++)
        pidq[cli][i].clear();
    for(int i=0; i<MAXPIPE*2; i++)
        numpipfd[cli][i] = 0;

    // initialize left user
    close(sockfd);
    FD_CLR(sockfd, &allset);
    client[cli] = -1;
    name[cli] = "(no name)";
    ipaddr[cli] = "";
    port[cli] = 0;
    process_line[cli] = 0;
    env[cli].clear();
    env[cli]["PATH"] = "bin:.";
    cout << "exit" << endl;
}
