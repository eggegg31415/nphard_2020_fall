void func(int sockfd, int cli){
    vector<string> arg;
    char input[MAXLEN] = "";
    int i=0;
    read(sockfd, input, sizeof(input));
    string msg = string(input);
    arg = dealline(msg);
    for(int i=0; i<msg.length(); i++){
        if(msg[i] == '\r' || msg[i] == '\n'){
            msg.erase(msg.begin()+i);
            i --;
        }
    }
    if(arg.size() == 0)
        return;
    if(choose_rwg_fun(arg, sockfd, cli))
        shell_function_preprocess(arg, msg, cli, sockfd);
}

vector<string> dealline(string input){
    stringstream ss;
    string s;
    vector<string> tmp;

    ss << input;
    while(ss >> s)
        tmp.push_back(s);
    return tmp;
}

bool choose_rwg_fun(vector<string> arg, int sockfd, int cli){
    if(arg[0] == "who")
        rwg_who(arg, sockfd, cli);
    else if(arg[0] == "tell")
        rwg_tell(arg, sockfd, cli);
    else if(arg[0] == "yell")
        rwg_yell(arg, sockfd, cli);
    else if(arg[0] == "name")
        rwg_name(arg, sockfd, cli);
    else if(arg[0] == "exit")
        rwg_exit(arg, sockfd, cli);
    else
        return 1;
    return 0;
}

bool choose_shell_fun(vector<string> arg, vector<int> is_bad, int *pipfd, int sockfd, int cli){
    if(arg[0] == "printenv")
        shell_printenv(arg, sockfd, cli);
    else if(arg[0] == "setenv")
        shell_setenv(arg, sockfd, cli);
    else
        shell_other(arg, is_bad, pipfd, sockfd, cli);
    return 0;
}

void shell_function_preprocess(vector<string> &arg, string input, int cli, int sockfd){
    char redir[MAXLEN];
    vector<int> is_bad = {0, 0, 0, 0, 0, 0, 0};   //[redirect, numpip, has_next_pipe, has_last_pipe, pipeth, user_pipe_in, user_pipe_out]
    int arg_num = arg.size();
    int pipfd[2*MAXPIPE];
    signal(SIGCHLD, SIG_IGN);

    //environment argument setup
    map<string, string>::iterator iter;
    for(iter=env[cli].begin(); iter != env[cli].end(); iter++)
        setenv(iter->first.c_str(), iter->second.c_str(), 1);

    //check user pipe
    int tmp_user_pipe_out = 0;
    for(int i=0; i<arg_num; i++){
        if((arg[i][0] == '<' || arg[i][0] == '>') && arg[i].length() > 1){
            int user_pipe_num = stoi(&arg[i][1]);
            for(int j=1; j<arg[i].length(); j++){
                if(arg[i][j] > '9' || arg[i][j] < '0'){
                    user_pipe_num = 0;
                    break;
                }
            }
            if(user_pipe_num){
                // user does not exist
                if(client[user_pipe_num] < 0){
                    cout << "User does not exist " << user_pipe_num << endl;
                    if(arg[i][0] == '<')
                        USER_PIPE_IN = -1 * user_pipe_num;
                    else if(arg[i][0] == '>')
                        tmp_user_pipe_out = -1 * user_pipe_num;
                    arg.erase(arg.begin()+i);
                    arg_num --;
                    i --;
                    continue;
                }

                stringstream ss;
                ss << setw(2) << setfill('0') << user_pipe_num;
                string other = ss.str();
                ss.str("");
                ss << setw(2) << setfill('0') << cli;
                string me = ss.str();
                string user_pipe_key = arg[i][0] == '<' ? other + me : me + other;

                // User pipe in
                if(arg[i][0] == '<'){
                    if(active_user_pipe.find(user_pipe_key) != active_user_pipe.end())
                        USER_PIPE_IN = user_pipe_num;
                    else
                        USER_PIPE_IN = -1 * user_pipe_num;
                }
                //User pipe out
                else if(arg[i][0] == '>'){
                    if(active_user_pipe.find(user_pipe_key) == active_user_pipe.end()){
                        tmp_user_pipe_out = user_pipe_num;
                        struct user_pipe new_pipe;
                        pipe(new_pipe.user_pipe);
                        active_user_pipe[user_pipe_key] = new_pipe;
                    }
                    else
                        tmp_user_pipe_out = -1 * user_pipe_num;
                }
                arg.erase(arg.begin()+i);
                arg_num --;
                i --;
            }
        }
    }
    //return user pipe in msg
    if(USER_PIPE_IN > 0){
        string msg = "*** " + name[cli] + " (#" + to_string(cli) + ") just received from " + name[USER_PIPE_IN] + " (#" + to_string(USER_PIPE_IN) + ") by \'" + input + "' ***\n";
        for(int i=1; i<LISTENQ; i++){
            if(client[i] > 0)
                write(client[i], msg.c_str(), msg.length());
        }
    }
    else if(USER_PIPE_IN < 0){
        int src = -1 * USER_PIPE_IN;
        string errmsg;
        if(client[src] < 0)
            errmsg = "*** Error: user #" + to_string(src) + " does not exist yet. ***\n";
        else
            errmsg = "*** Error: the pipe #" + to_string(src) + "->#" + to_string(cli) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    }

    //check pipe
    int begin = 0, pipth = 0;
    arg_num = arg.size();
    for(int i=0; i<arg_num; i++){
        if(arg[i] == "|"){
            pipe(pipfd+2*pipth);
            IS_PIPE = 1;      //has_next_pipe
            is_bad[4] = pipth;  //pipth
            vector<string> tmp;
            for(int j=begin; j<i; j++)
                tmp.push_back(arg[j]);
            choose_shell_fun(tmp, is_bad, pipfd, sockfd, cli);
            IS_PIPE = 0;
            LAST_PIPE = 1;      //has_last_pipe
            USER_PIPE_IN = 0;
            begin = i+1;
            pipth = (pipth + 1) % MAXPIPE;
        }
    }

    //check redirect
    FILE* pfile;
    for(int i=0; i<arg_num; i++){
        //check redirect
        if(arg[i] == ">" && !is_bad[0]){
            if(i == arg_num-1){
                cout << " [command] > [redirect file]]" << endl;
                break;
            }
            string str = arg[i+1];
            strcpy(redir, str.c_str());
            pfile = fopen(redir, "w");
            is_bad[0] = fileno(pfile);
        }
        if(is_bad[0])
            arg.pop_back();
    }

    //check number pipe
    for(int i=0; i<arg_num; i++){
        if((arg[i][0] == '|' || arg[i][0] == '!') && arg[i].length() > 1){
            arg[i][0] == '|' ? IS_NUMPIPE = 1 : IS_NUMPIPE = -1;
            for(int j=1; j<arg[i].length(); j++){
                char ch = arg[i][j];
                if(ch < '0' || ch > '9'){
                    IS_NUMPIPE = 0;
                    break;
                }
            }
            if(IS_NUMPIPE){
                IS_NUMPIPE = IS_NUMPIPE * stoi(&arg[i][1]);
                arg.pop_back();
                break;
            }
        }
    }
    int target = (process_line[cli] + abs(IS_NUMPIPE)) % MAXPIPE;
    if(IS_NUMPIPE && pidq[cli][target].size() == 0)
        pipe(numpipfd[cli] + 2 * target);

    // return message if user pipe
    if(tmp_user_pipe_out){
        USER_PIPE_OUT = tmp_user_pipe_out;
        if(tmp_user_pipe_out > 0){
            // return msg
            string msg = "*** " + name[cli] + " (#" + to_string(cli) + ") just piped '" + input + "' to " + name[USER_PIPE_OUT] + " (#" + to_string(USER_PIPE_OUT) + ") ***\n";
            for(int i=1; i<LISTENQ; i++){
                if(client[i] > 0)
                    write(client[i], msg.c_str(), msg.length());
            }
        }
        else if(tmp_user_pipe_out < 0){
            int dst = -1 * tmp_user_pipe_out;
            string errmsg;
            if(client[dst] < 0)
                errmsg = "*** Error: user #" + to_string(dst) + " does not exist yet. ***\n";
            else
                errmsg = "*** Error: the pipe #" + to_string(cli) + "->#" + to_string(dst) + " already exists. ***\n";
            write(sockfd, errmsg.c_str(), errmsg.length());
        }
    }

    // clean arguments if there is pipe in command
    if(begin)
        arg.erase(arg.begin(), arg.begin()+begin);

    // jump to functions
    is_bad[4] = pipth;
    choose_shell_fun(arg, is_bad, pipfd, sockfd, cli);

    // change stdout back
    if(is_bad[0]) fclose(pfile);
    process_line[cli] = (process_line[cli] + 1) % MAXPIPE;

    for(iter=env[cli].begin(); iter != env[cli].end(); iter++)
        setenv(iter->first.c_str(), "", 1);
}
