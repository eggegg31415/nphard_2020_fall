# I. Introduction
In this project, you are asked to design 3 kinds of servers:
1. Design a Concurrent connection-oriented server. This server allows one client connect to it.
2. Design a server of the chat-like systems, called remote working systems (rwg). In this system, users can communicate with other users. You need to use the single-process concurrent paradigm to design this server.

# II. Scenario of Part One
You can use telnet to connect to your server.
Assume your server is running on nplinux1 and listening at port 7001.
```
bash$ telnet nplinux1.cs.nctu.edu.tw 7001
% ls | cat
bin test.html
% ls |1
% cat
bin test.html
% exit
bash$
```

# III. Scenario of Part Two
You are asked to design the following features in your server.
1. Pipe between different users. Broadcast message whenever a user pipe is used.
2. Broadcast message of login/logout information.
3. New commands:
    * **who**: show information of all users.
    * **tell**: send a message to another user.
    * **yell**: send a message to all users.
    * **name**: change your name.
4. All commands in hw1
