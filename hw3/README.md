# I. Introduction
The project is divided into two parts. This is the first part of the project.
Here, you are asked to write a Remote Batch System, which consists of a simple HTTP server called
http server and a CGI program console.cgi. We will use **Boost.Asio** library to accomplish this project.

# II. Specification
## A. http server
1. In this project, the URI of HTTP requests will always be in the form of /${cgi name}.cgi (e.g., /panel.cgi, /console.cgi, /printenv.cgi), and we will only test for the HTTP GET method.
2. Your http server should parse the HTTP headers and **follow the CGI procedure** (fork, set environment variables, dup, exec) to execute the specified CGI program.
3. The following environment variables are required to set:
(a) REQUEST METHOD
(b) REQUEST URI
(c) QUERY STRING
(d) SERVER PROTOCOL
(e) HTTP HOST
(f) SERVER ADDR
(g) SERVER PORT
(h) REMOTE ADDR
(i) REMOTE PORT
For instance, if the HTTP request looks like:
```
GET /console.cgi?h0=nplinux1.cs.nctu.edu.tw&p0= ... (too long, ignored)
Host: nplinux8.cs.nctu.edu.tw:7779
User-Agent: Mozilla/5.0
Accept: text/html,application/xhtml+xml,applica ... (too long, ignored)
Accept-Language: en-US,en;q=0.8,zh-TW;q=0.5,zh; ... (too long, ignored)
Accept-Encoding: gzip, deflate
DNT: 1
1
Connection: keep-alive
Upgrade-Insecure-Requests: 1
```
Then before executing console.cgi, you need to set the corresponding environment variables.
In this case, REQUEST METHOD should be "GET" HTTP HOST should be
"plinux8.cs.nctu.edu.tw:7779" and so on and so forth.
## B. console.cgi
1. You are highly recommended to inspect and run the CGI samples before you start this section.
2. The console.cgi should parse the connection information (e.g. host, port, file) from the environment variable QUERY STRING, which is set by your HTTP server.
For example, if QUERY STRING is: `0=nplinux1.cs.nctu.edu.tw&p0=1234&f0=t1.txt&h1=nplinux2.cs.nctu.edu.tw&p1=5678&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=`
It should be understood as:
```
h0=nplinux1.cs.nctu.edu.tw # the hostname of the 1st server
p0=1234 # the port of the 1st server
f0=t1.txt # the file to open
h1=nplinux2.cs.nctu.edu.tw # the hostname of the 2nd server
p1=5678 # the port of the 2nd server
f1=t2.txt # the file to open
h2= # no 3rd server, so this field is empty
p2= # no 3rd server, so this field is empty
f2= # no 3rd server, so this field is empty
h3= # no 4th server, so this field is empty
p3= # no 4th server, so this field is empty
f3= # no 4th server, so this field is empty
h4= # no 5th server, so this field is empty
p4= # no 5th server, so this field is empty
f4= # no 5th server, so this field is empty
```
3. After parsing, console.cgi should connect to these servers. Note that the maximum number of the servers never exceeds 5.
4. The remote servers that console.cgi connects to are Remote Working Ground Servers with shell prompt `%` , and the files we sent (e.g., t1.txt) are the commands for the remote shells. However, you should not send the entire file to the remote server and execute them all at once. Instead, send them line-by-line whenever you receive a shell prompt `%` from remote.
5. Your console.cgi should display the hostname and the port of the connected remote server at the top of each session.
6. Your console.cgi should display the remote server’s replies in real-time. Everything you send to remote or receive from remote should be displayed on the web page as soon as possible.
For example:
```
% ls
bin
test.html
```
Here, the blue part is the content (output) you received from the remote shell, and the brown part is the content (command) you sent to the remote. The output order matters and needs to be preserved.
You should make sure that commands are displayed right after the shell prompt `%`, but before the execution result received from remote.
7. Regarding how to display the server' reply (console.cgi), please refer to sample console.cgi.
Since we will not judge your answers with diff for this project, feel free to modify the layout of the web page. Just make sure you follow the below rules:
(a) Each session should be separate.
(b) The commands and the outputs of the shell are displayed in the right order and at the right time
(c) The commands can be easily distinguished from the outputs of the shell.
## C. panel.cgi (Provided by TA)
1. This CGI program generates the form in the web page. It detects all files in the directory test case/ and display them in the selection menu.

