// NP shell include
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// rwg system include
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// NP shell define
#define MAXLEN 15001
#define MAXPIPE 1001
#define IS_REDIRECT is_bad[0]
#define IS_NUMPIPE is_bad[1]
#define IS_PIPE is_bad[2]
#define LAST_PIPE is_bad[3]
#define USER_PIPE_IN is_bad[5]
#define USER_PIPE_OUT is_bad[6]

// rwg system define
#define SA struct sockaddr
#define MOTD "****************************************\n** Welcome to the information server. **\n****************************************\n"
#define LISTENQ 35

using namespace std;

// NP shell global
int numpipfd[LISTENQ][2*MAXPIPE];
vector<vector<vector<int>>> pidq(LISTENQ, vector<vector<int>>(MAXPIPE));
int DEVNULLO = fileno(fopen("/dev/null", "w"));
int DEVNULLI = fileno(fopen("/dev/null", "r"));

// rwg system global
vector<string> name(LISTENQ);
vector<string> ipaddr(LISTENQ);
vector<int> port(LISTENQ);
vector<int> client(LISTENQ);
vector<int> process_line(LISTENQ);
vector<map<string, string>> env(LISTENQ);
struct user_pipe { int user_pipe[2]; };
map<string, struct user_pipe> active_user_pipe;
fd_set allset, rset;

// NP shell function
vector<string> dealline(string);
void shell_function_preprocess(vector<string> &, string, int, int);
bool choose_rwg_fun(vector<string>, int, int);
bool choose_shell_fun(vector<string>, vector<int>, int*, int, int);
void check_pipe_in_child(int, vector<int>, int*, int);
void check_pipe_out_child(int, vector<int>, int*, int);
void check_pipe_parent(int, vector<int>, int*, int, int);

// shell.h function
void shell_printenv(vector<string>, int, int);
void shell_setenv(vector<string>, int, int);
void shell_other(vector<string>, vector<int>, int*, int, int);


// rwg.h function
void rwg_who(vector<string>, int, int);
void rwg_tell(vector<string>, int, int);
void rwg_yell(vector<string>, int, int);
void rwg_name(vector<string>, int, int);
void rwg_exit(vector<string>, int, int);
