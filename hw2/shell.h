void shell_printenv(vector<string> arg, int sockfd, int cli){
    if(arg.size() != 2){
        string errmsg = "usage: printenv [variable name]\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
        return;
    }
    if(! getenv(arg[1].c_str()))
        return;
    string msg = env[cli][arg[1]] + "\n";
    write(sockfd, msg.c_str(), msg.length());
}

void shell_setenv(vector<string> arg, int sockfd, int cli){
    if(arg.size() != 3){
        string errmsg = "usage: setenv [variable name] [value to assign]\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
        return;
    }
    env[cli][arg[1]] = arg[2];
}

void shell_other(vector<string> arg, vector<int> is_bad, int *pipfd, int sockfd, int cli){
    int arg_num = arg.size(), status, pipth = is_bad[4];
    char *argv[arg_num+1];

    for(int i=0; i<arg_num; i++){
        char tmp[MAXLEN];
        strcpy(tmp, arg[i].c_str());
        argv[i] = strdup(tmp);
    }
    argv[arg_num] = NULL;

    pid_t pid = fork();
    while(pid < 0){
        wait(&status);
        pid = fork();
    }

    if(pid == 0){
        dup2(sockfd, 1);
        dup2(sockfd, 2);
        check_pipe_in_child(pipth, is_bad, pipfd, cli);
        check_pipe_out_child(pipth, is_bad, pipfd, cli);

        int err = execvp(argv[0], argv);
        if(err != 0)
            cerr << "Unknown command: [" << argv[0] << "]." << endl;
        exit(0);
    }
    else
        check_pipe_parent(pipth, is_bad, pipfd, pid, cli);
}

void check_pipe_in_child(int pipth, vector<int> is_bad, int* pipfd, int cli){
    int line = process_line[cli];
    int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
    int last_pipe = pipth-1 >= 0 ? pipth-1 : pipth+MAXPIPE-1;
    if(LAST_PIPE){                           // normal pipe
        close(pipfd[2*last_pipe+1]);
        dup2(pipfd[2*last_pipe], 0);
        close(pipfd[2*last_pipe]);
    }
    else{
        if(pidq[cli][line].size() != 0){              // number pipe
            close(numpipfd[cli][2*line+1]);
            dup2(numpipfd[cli][2*line], 0);
            close(numpipfd[cli][2*line]);
        }
        if(USER_PIPE_IN > 0){
            stringstream ss;
            ss << setw(2) << setfill('0') << USER_PIPE_IN;
            ss << setw(2) << setfill('0') << cli;
            string user_pipe_key = ss.str();
            close(active_user_pipe[user_pipe_key].user_pipe[1]);
            dup2(active_user_pipe[user_pipe_key].user_pipe[0], 0);
            close(active_user_pipe[user_pipe_key].user_pipe[0]);
        }
        else if(USER_PIPE_IN < 0)
            dup2(DEVNULLI, 0);
    }
}

void check_pipe_out_child(int pipth, vector<int> is_bad, int* pipfd, int cli){
    int line = process_line[cli];
    if(IS_PIPE){      //pipe
        close(pipfd[2*pipth]);
        dup2(pipfd[2*pipth+1], 1);
        close(pipfd[2*pipth+1]);
    }
    if(IS_NUMPIPE){      //number pipe
        int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
        close(numpipfd[cli][2*target]);
        dup2(numpipfd[cli][2*target+1], 1);
        if(IS_NUMPIPE < 0)
            dup2(numpipfd[cli][2*target+1], 2);
        close(numpipfd[cli][2*target+1]);
    }
    if(IS_REDIRECT)      //redirect
        dup2(IS_REDIRECT, 1);
    if(USER_PIPE_OUT > 0){
        stringstream ss;
        ss << setw(2) << setfill('0') << cli;
        ss << setw(2) << setfill('0') << USER_PIPE_OUT;
        string user_pipe_key = ss.str();
        close(active_user_pipe[user_pipe_key].user_pipe[0]);
        dup2(active_user_pipe[user_pipe_key].user_pipe[1], 1);
        close(active_user_pipe[user_pipe_key].user_pipe[1]);
    }
    else if(USER_PIPE_OUT < 0)
        dup2(DEVNULLO, 1);
}

void check_pipe_parent(int pipth, vector<int> is_bad, int* pipfd, int pid, int cli){
    int line = process_line[cli];
    int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
    int last_pipe = pipth-1 >= 0 ? pipth-1 : pipth+MAXPIPE-1;
    int status;

    pidq[cli][line].push_back(pid);

    //last pipe in from pipe
    if(LAST_PIPE){
        close(pipfd[2*last_pipe+1]);
        close(pipfd[2*last_pipe]);
    }
    else{
        if(pidq[cli][line].size() != 1){
            close(numpipfd[cli][2*line+1]);
            close(numpipfd[cli][2*line]);
        }
    }

    //has next number pipe
    if(IS_NUMPIPE){
        for(auto i : pidq[cli][line])
            pidq[cli][target].push_back(i);
        pidq[cli][line].clear();
        pidq[cli][target].push_back(pid);
    }
    if(USER_PIPE_IN > 0){
        stringstream ss;
        ss << setw(2) << setfill('0') << USER_PIPE_IN;
        ss << setw(2) << setfill('0') << cli;
        string user_pipe_key = ss.str();
        close(active_user_pipe[user_pipe_key].user_pipe[0]);
        close(active_user_pipe[user_pipe_key].user_pipe[1]);
        active_user_pipe.erase(user_pipe_key);
    }

    //pipe_end
    if(! IS_PIPE){
        if(! IS_NUMPIPE && ! (USER_PIPE_OUT > 0)){
            for(auto i : pidq[cli][line])
                waitpid(i, &status, 0);
        }
        pidq[cli][line].clear();
    }
}
