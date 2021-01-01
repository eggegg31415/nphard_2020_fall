#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXLEN 15001
#define MAXPIPE 1001
#define MAXPROCESS 400
#define IS_NUMPIPE is_bad[1]
#define IS_PIPE is_bad[2]
#define LAST_PIPE is_bad[3]
using namespace std;

int numpipfd[2*MAXPIPE];
vector<vector<int>> pidq(MAXPIPE);
int line = 0;

void check_pipe_in_child(int pipth, vector<int> is_bad, int* pipfd);
void check_pipe_out_child(int pipth, vector<int> is_bad, int* pipfd);
void check_pipe_parent(int pipth, vector<int> is_bad, int* pipfd, int pid);
vector<string> dealline(string input);
void fun_printenv(vector<string> arg);
void fun_setenv(vector<string> arg);
void fun_removetag(vector<string> arg, vector<int> is_bad, int *pipfd);
void fun_removetag0(vector<string> arg, vector<int> is_bad, int *pipfd);
void fun_number(vector<string> arg, vector<int> is_bad, int *pipfd);
void fun_other(vector<string> arg, vector<int> is_bad, int *pipfd);
bool choose_fun(vector<string> arg, vector<int> is_bad, int *pipfd);
