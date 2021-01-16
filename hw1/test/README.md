# NP Project1 Demo Script (beta)

### Important information: 
1. This is a sample demo scipt. It is just a sample, the final one might be different.

2. The test cases in this directory are from 2016, the ones used for this year will be different.

### This directory contains the following
- demo.sh: 
    - Usage: `./demo.sh [npshell_path]`
    - demo.sh does the following:
    1. Contruct the working directory (work_template and work_dir)
    2. Compile the commands (noop, removetag...) and place them into bin/ inside the working directory
    3. Copy cat and ls into bin/ inside the working directory
    4. Run the npshell inside work_dir
    5. Redirect stdin of npshell to files in test_case/
    6. Redirect stdout and stderr of npshell to files in output/
    7. Use diff to compare the files inside output/ and answer/
    8. Show the result of demo

- compare.sh
    - Usage: `./compare.sh [n]`
    - Compare.sh will run vimdiff on the n'th answer and your output

- test_case/
    - Contains test cases

- answer/
    - Contains answers

- src/
    - Contains source code of commands (noop, removetag...) and test.html

### Scenario
- Assume there are two directories, np_project1_demo_sample and 0756000 inside a private directroy on NP server.
- My npshell is inside 0756000 directory
- I am now inside np_project1_demo_sample

```
  bash$ ./demo.sh ../0756000/npshell
  ...
  ... # Some messeges generated while compiling and setting up environment
  ...
  ===== Test case 1 =====
  Your answer is correct
  ===== Test case 2 =====
  Your answer is correct
  ===== Test case 3 =====
  Your answer is correct
  ===== Test case 4 =====
  Your answer is correct
  ===== Test case 5 =====
  Your answer is correct
  ===== Test case 6 =====
  Your answer is wrong
  ======= Summary =======
  [Correct]: 1 2 3 4 5
  [ Wrong ]: 6
  bash$ ./compare 6     # Check out the difference between the 6th output and answer
```

