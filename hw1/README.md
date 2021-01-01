CM]

[TOC]
# I. Introduction
In this project, you are asked to design a shell with special piping
mechanisms.
# II. Specification
## A. Input
1. The length of a single-line input will not exceed 15000 characters.
2. Each command will not exceed 256 characters.
3. There must be one or more spaces between commands and symbols (or arguments), but no spaces between pipe and numbers.
```sh
% cat hello.txt | number
% cat hello.txt |4
% cat hello.txt !4
```
4. There won't xist any `/` character in test cases.

## B. NPShell Behavior
1. Use `% ` as the command line prompt. Notice that there is one space
character after `%`.
2. The npshell terminates after receiving the exit command or `EOF`.
3. Notice that you must handle the forked processes properly, or there might be zombie processes.
4. Built-in commands (setenv, printenv, exit) will appear solely in a line. No
command will be piped together with built-in commands.

## C. setenv and printenv
1. The initial environment variable PATH should be set to `bin/` and `./` by default.
```sh
% printenv PATH
bin:.
```
2. setenv usage: `setenv [variable name] [value to assign]`
3. printenv usage: `printenv [variable name]`
```sh
% printenv QQ # Show nothing if the variable does not exist.
% printenv LANG
en_US.UTF-8
```
4. The number of arguments for setenv and printenv will be correct in all test cases.

## D. Numbered-Pipes and Ordinary Pipe
1. `|N` means the STDOUT of the left hand side command should be piped to the first command of the next N-th line, where 1 ≤ N ≤ 1000.
2. `!N` means both STDOUT and STDERR of the left hand side command
should be piped to the first command of the next N-th line, where 1 ≤ N ≤ 1000.
3. `|` is an ordinary pipe, it means the STDOUT of the left hand side command
will be piped to the right hand side command. It will only appear between
two commands, not at the beginning or at the end of the line.
4. The command number still counts for unknown commands.
```
% ls |2
% ctt
Unknown command: [ctt].
% number
1 bin/
2 test.html
```
5. setenv and printenv count as one command.
```
% ls |2
% printenv PATH
bin:.
% cat
bin
test.html
```
6. Empty line does not count.
```
% ls |1
% # press Enter
% number
1 bin/
2 test.html
```

## E. Unknown Command
1. If there is an unknown command, print error message as the following format: `Unknown command: [command].` e.g.
```
% ctt
Unknown command: [ctt].
```
2. You don't have to print out the arguments.
```
% ctt -n
Unknown command: [ctt].
```
3. The commands after unknown commands will still be executed.
```
% ctt | ls
Unknown command: [ctt].
bin/ test.html
```
4. Messages piped to unknown commands will disappear.
```
% ls | ctt
Unknown command: [ctt].
```

