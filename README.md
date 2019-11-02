# semestrial-work-1
Shell implementation in C language
  
##  What is Shell:
    shell is a command-line interpreter or shell that provides a command line user interface for Unix-like operating systems.           
    The shell is both an interactive command language and a scripting language, and is used by the operating system to 
    control the execution of the system using shell scripts.

## shell supports:
    1) execute command
    2) pipes 
    3) change directory 
    4) background process 
    5) change input/output
    6) ctr + c signal to terminate current(all) process

## How to run program:
    1) download makefile and put it in the same directory with the code
    2) run in terminal: make (file name)
    3) run in terminal: ./(file name)

## Examples how to execute programm:
    compile and run program
    1) cmd
    2) ....
    3) exit or quit(to exit from program)
  
 ## what is pipes:
    A pipe is a form of redirection (transfer of standard output to some other destination) that is used in Linux and other 
    Unix-like operating systems to send the output of one command/program/process to another command/program/process for 
    further processing. You can make it do so by using the pipe character ‘|’. 
    
    how to run program with pipes:
        compile and run program
        1) cmd1 | cmd2 | ... | cmdn 
        2) ....
        3) exit or quit(to exit from program)
        
  ## How to change directory:
      compile and run program
      1)  cd path
      2)  ....
      if path:
          1) left empty or "~" change to home directory; 
          2) ".." change to parent directory; 
          3) "-" change to previous directory;
            
  ## How run to run program in background:
    compile and run program
    * cmd &
    program will print number and PID of background process
    -----------
    if backgorund process is completed, program will print number and name of completed process 

## ctrl + c signal:
      if something went wrong, press ctrl + c to kill all child processes
