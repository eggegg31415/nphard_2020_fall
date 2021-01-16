# NP Project3 Sample Demo Script
## This directory contains the following
- src/
    - Contains your source codes

- np_single/
    - Contains the working directory for np_single_golden, commands, and other needed files

- working_dir/
    - Contains test cases and CGI programs

- demo.sh:
    - The Entry point of the sample demo script


## Usage
- You should put your source codes under `src/` before running `demo.sh`
    1. Create a folder naming by your student id
    2. Put all source codes and Makefile in the folder created in step1

- `./demo.sh`
    - demo.sh create two panes in tmux
    - The upper pane
        - Execute three np_single_golden automatically
        - The hostname and the port for listening will be displayed
    - The lower pane
        - Compile your source codes
        - Put the executables into the correct working_dir
          (You can use `cat ~/.npdemo3` to get the path of working_dir)
        - Copy CGI programs into `~/public_html/npdemo3/`
        - Execute your `http_server`
        - Display some links for demo
    - If the script successfully finishes, you will see the two panes similar to the below screenshot. You can open the links to check the result
        
        ![](https://i.imgur.com/YHuCMUW.png)

## Result
### Part 1-1
- The results should be similar to the following screenshots
    - printenv.cgi

        ![](https://i.imgur.com/Rdio6wf.png)

    - hello.cgi
        
        ![](https://i.imgur.com/VAGCOax.png)

    - welcome.cgi

        ![](https://i.imgur.com/tOELTwM.png)

### Part 1-2 & 1-3
- The execution flow and the results in the web page should be similar to the [video](https://drive.google.com/file/d/16Zj3aFqdmhu-3qz3vwEQc2A4p9H7Ergv/view)





