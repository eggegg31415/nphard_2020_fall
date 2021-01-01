vector<string> dealline(string input){
    stringstream ss;
    string s;
    vector<string> tmp;

    ss << input;
    while(ss >> s){
        tmp.push_back(s);
    }
    return tmp;
}

void fun_printenv(vector<string> arg){
    char env[MAXLEN];
    if(arg.size() != 2){
        cout << "usage: printenv [variable name]" << endl;
        return;
    }
    strcpy(env, arg[1].c_str());
    if(! getenv(env))
        return;
    cout << getenv(env) << endl;
}

void fun_setenv(vector<string> arg){
    char env[MAXLEN], envarg[MAXLEN];
    if(arg.size() != 3){
        cout << "usage: setenv [variable name] [value to assign]" << endl;
        return;
    }
    strcpy(env, arg[1].c_str());
    strcpy(envarg, arg[2].c_str());
    setenv(env, envarg, 1);
}

void fun_other(vector<string> arg, vector<int> is_bad, int *pipfd){
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
        check_pipe_in_child(pipth, is_bad, pipfd);
        check_pipe_out_child(pipth, is_bad, pipfd);

        int err = execvp(argv[0], argv);
        if(err != 0)
            cerr << "Unknown command: [" << argv[0] << "]." << endl;
        exit(0);
    }
    else
        check_pipe_parent(pipth, is_bad, pipfd, pid);
}

bool choose_fun(vector<string> arg, vector<int> is_bad, int *pipfd){
    if(arg[0] == "printenv")
        fun_printenv(arg);
    else if(arg[0] == "setenv")
        fun_setenv(arg);
    else if(arg[0] == "exit")
        return 1;
    else
        fun_other(arg, is_bad, pipfd);
    return 0;
}

void check_pipe_in_child(int pipth, vector<int> is_bad, int* pipfd){
    int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
    int last_pipe = pipth-1 >= 0 ? pipth-1 : pipth+MAXPIPE-1;
    if(LAST_PIPE){                           // normal pipe
        close(pipfd[2*last_pipe+1]);
        dup2(pipfd[2*last_pipe], 0);
        close(pipfd[2*last_pipe]);
    }
    else{
        if(pidq[line].size() != 1){              // number pipe
            close(numpipfd[2*line+1]);
            dup2(numpipfd[2*line], 0);
            close(numpipfd[2*line]);
        }
    }
}

void check_pipe_out_child(int pipth, vector<int> is_bad, int* pipfd){
    if(IS_PIPE){      //pipe
        close(pipfd[2*pipth]);
        dup2(pipfd[2*pipth+1], 1);
        close(pipfd[2*pipth+1]);
    }
    if(IS_NUMPIPE){      //number pipe
        int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
        close(numpipfd[2*target]);
        dup2(numpipfd[2*target+1], 1);
        if(IS_NUMPIPE < 0)
            dup2(numpipfd[2*target+1], 2);
        close(numpipfd[2*target+1]);
    }
    if(is_bad[0])      //redirect
        dup2(is_bad[0], 1);
}

void check_pipe_parent(int pipth, vector<int> is_bad, int* pipfd, int pid){
    int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
    int last_pipe = pipth-1 >= 0 ? pipth-1 : pipth+MAXPIPE-1;
    int status;

    pidq[line].push_back(pid);

    //last pipe in from pipe
    if(LAST_PIPE){
        close(pipfd[2*last_pipe+1]);
        close(pipfd[2*last_pipe]);
    }
    else{
        if(pidq[line].size() != 1){
            close(numpipfd[2*line+1]);
            close(numpipfd[2*line]);
        }
    }

    //has next number pipe
    if(IS_NUMPIPE){
        for(auto i : pidq[line])
            pidq[target].push_back(i);
        pidq[line].clear();
        pidq[target].push_back(pid);
    }

    //pipe_end
    if(! IS_PIPE){
        if(! IS_NUMPIPE){
            for(auto i : pidq[line])
                waitpid(i, &status, 0);
        }
        pidq[line].clear();
    }
}
