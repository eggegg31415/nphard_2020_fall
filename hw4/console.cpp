#include "console_macro.h"

void print_head(){
    cout << "Content-type:text/html\r\n\r\n";
    cout << "<!DOCTYPE html>\n";
    cout << "<html lang=\"en\">\n";
    cout << "  <head>\n";
    cout << "    <meta charset=\"UTF-8\" />\n";
    cout << "    <title>NP Project 3 Sample Console</title>\n";
    cout << "    <link\n";
    cout << "      rel=\"stylesheet\"\n";
    cout << "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n";
    cout << "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n";
    cout << "      crossorigin=\"anonymous\"\n";
    cout << "    />\n";
    cout << "    <link\n";
    cout << "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n";
    cout << "      rel=\"stylesheet\"\n";
    cout << "    />\n";
    cout << "    <link\n";
    cout << "      rel=\"icon\"\n";
    cout << "      type=\"image/png\"\n";
    cout << "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n";
    cout << "    />\n";
    cout << "    <style>\n";
    cout << "      * {\n";
    cout << "        font-family: 'Source Code Pro', monospace;\n";
    cout << "        font-size: 1rem !important;\n";
    cout << "      }\n";
    cout << "      body {\n";
    cout << "        background-color: #212529;\n";
    cout << "      }\n";
    cout << "      pre {\n";
    cout << "        color: #cccccc;\n";
    cout << "      }\n";
    cout << "      b {\n";
    cout << "        color: #01b468;\n";
    cout << "      }\n";
    cout << "    </style>\n";
    cout << "  </head>\n";

}
void print_body(vector<sessions> sessionsq){
    cout << "  <body>\n";
    cout << "    <table class=\"table table-dark table-bordered\">\n";
    cout << "      <thead>\n";
    cout << "        <tr>\n";
    for(auto i : sessionsq)
        cout << "<th scope=\"col\">" << i.host << ":" << i.port  << "</th>\n";
    cout << "        </tr>\n";
    cout << "      </thead>\n";
    cout << "      <tbody>\n";
    cout << "        <tr>\n";
    for(auto i : sessionsq)
       cout << "<td><pre id=" << i.id << " class=\"mb-0\"></pre></td>\n";
    cout << "        </tr>\n";
    cout << "      </tbody>\n";
    cout << "    </table>\n";
    cout << "  </body>\n";
    cout << "</html>\n";
}

void console(vector<sessions> sessionsq){
    print_head();
    print_body(sessionsq);
    for(int i=0; i<sessionsq.size(); i++){
        make_shared<Console>(move(sessionsq[i]))->start();
    }
    ioservice.run();
}

void get_query_string(vector<sessions> &sessionsq){
    string QUERY_STRING = getenv("QUERY_STRING");
    string tmp;
    stringstream ss(QUERY_STRING);

    while(getline(ss, tmp, '&')){
        stringstream s2(tmp);
        string var, arg;

        getline(s2, var, '=');
        if(var[0] == 'h')
            getline(s2, sessionsq[atoi(&var[1])].host, '=');
        if(var[0] == 'p')
            getline(s2, sessionsq[atoi(&var[1])].port, '=');
        if(var[0] == 'f')
            getline(s2, sessionsq[atoi(&var[1])].file, '=');
        if(var[0] == 's' && var[1] == 'h')
            getline(s2, PHInfo.host, '=');
        if(var[0] == 's' && var[1] == 'p')
            getline(s2, PHInfo.port, '=');
    }
    int pos = 0, num = 0;
    for(num; num<MAXSERVER; num++){
        sessions s = sessionsq[pos];
        if( s.host == "" || s.port == "" || s.file == "")
            sessionsq.erase(sessionsq.begin()+pos);
        else{
            sessionsq[pos].id = to_string(num);
            pos ++;
        }
    }
}

void read_file(vector<sessions> &sessionsq){
    for(auto &i : sessionsq){
        ifstream fd("./test_case/" + i.file);
        string line;
        while(getline(fd,line)){
            i.cmd.push_back(line + "\r\n");
        }
        fd.close();
    }
}

int main (){
    vector<sessions> sessionsq(5);
    get_query_string(sessionsq);
    read_file(sessionsq);
    console(sessionsq);
}
