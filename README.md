# semestrial-work-1
Shell implementation in C language
  
##  What is Shell:
    In Unix, the shell is a program that interprets commands and acts as an intermediary between the user and the inner 
    workings of the operating system. Providing a command-line interface (that is, the shell prompt or command prompt), the 
    shell is analogous to DOS and serves a purpose similar to graphical interfaces like Windows, Mac, and the X Window 
    System.

---

## Shell Features:
1) execute command
2) pipes 
3) change directory 
4) background process 
5) change input/output
6) logical and(&&)
7) ctr + c signal to terminate foreground(all) process

---

## How to compile and run program:
1) download makefile and put it in the same directory with the code
2) run in terminal: make [file name]
3) run in terminal: ./[file name]

---

## Examples how to execute command:
**compile and run program**
1) cmd
2) ....
3) exit or quit(to exit from program)
  
---
  
## What is Pipes:
    A pipe is a form of redirection (transfer of standard output to some other destination) that is used in Linux and other 
    Unix-like operating systems to send the output of one command/program/process to another command/program/process for 
    further processing. You can make it do so by using the pipe character ‘|’. 

### How to run program with pipes:
**compile and run program**
* cmd1 | cmd2 | ... | cmdn

Also, acceptable: cmd1|cmd2 or cmd1| cmd2 or cmd1 |cmd2
     
---
     
## How to change working directory:
**compile and run program**
1)  cd [path]
2)  ....

Change the current directory to path.

**if path:**
  1) left empty or "~" change to home directory; 
  2) ".." change to parent directory; 
  3) "-" change to previous directory;
  
---
  
## What is Background process
    In Unix, a background process executes independently of the shell, leaving the terminal free for other work. To run a 
    process in the background, include an & (an ampersand) at the end of the command you use to run the job. 

### How to run program in background:
**compile and run program**
* cmd &



* Program will print number and PID of background process.
* If backgorund process is completed, program will print number and name of completed process.
  
---  
  
## What is change input and output:
    Redirection is a feature in Linux such that when executing a command, you can change the standard input/output devices 
    using '<' / '>' respectively.

---

### How to change input and output:
**compile and run program**
* cmd < input.txt > output.txt(take data from input.txt and store result in output.txt)
  
--- 
  
## What is logical AND(&&)
    Logical AND is used to chain commands together, such that the next command is run if and only if the preceding command 
    exited without errors (or, more accurately, exits with a return code of 0).
    
### How to run program with logical AND:
**compile and run program**
1) cmd1 && cmd2 &&...&& cmdn
2) ....

Also, acceptable: cmd1&&cmd2 or cmd1&& cmd2 or cmd1 &&cmd2

---

## ctrl + c signal:
      if something went wrong, press ctrl + c to kill all child processes
