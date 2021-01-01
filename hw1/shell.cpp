#include "macro.h"
#include "function.h"

int main(){
    //Global argument
    string input;
    vector<string> arg;
    setenv("PATH", "bin:.", 1);

    //main function loop
    char redir[MAXLEN];
    signal(SIGCHLD, SIG_IGN);
    while(1){
        cout << "% ";
        getline(cin, input);
        if(input.length() == 0)
            continue;
        arg = dealline(input);

        vector<int> is_bad = {0, 0, 0, 0, 0};   //[redirect, numpip, has_next_pipe, has_last_pipe, pipeth]
        int arg_num = arg.size();
        int pipfd[2*MAXPIPE];

        //check pipe
        arg_num = arg.size();
        int begin = 0, pipth = 0;
        for(int i=0; i<arg_num; i++){
            if(arg[i] == "|"){
                pipe(pipfd+2*pipth);
                IS_PIPE = 1;      //has_next_pipe
                is_bad[4] = pipth;  //pipth
                vector<string> tmp;
                for(int j=begin; j<i; j++)
                    tmp.push_back(arg[j]);
                choose_fun(tmp, is_bad, pipfd);
                IS_PIPE = 0;
                LAST_PIPE = 1;      //has_last_pipe
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
        int target = (line + abs(IS_NUMPIPE)) % MAXPIPE;
        if(IS_NUMPIPE && pidq[target].size() == 0)
            pipe(numpipfd + 2 * target);

        if(begin)
            arg.erase(arg.begin(), arg.begin()+begin);

        // jump to functions
        is_bad[4] = pipth;
        bool is_exit = choose_fun(arg, is_bad, pipfd);

        // change stdout back
        if(is_bad[0]) fclose(pfile);
        if(is_exit) break;
        line = (line + 1) % MAXPIPE;
    }
}

