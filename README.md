# semestrial-work-1
Shell implementation in C language

## shell supports:
    1) pipes
    2) background process
    3) ctr + c signal to terminate current(all) process
    4) change directory
    5) change input/output
    6) execute command
  
##  What is Shell:
    shell is a command-line interpreter or shell that provides a command line user interface for Unix-like operating systems.           
    The shell is both an interactive command language and a scripting language, and is used by the operating system to 
    control the execution of the system using shell scripts.
  
## Examples how to execute programm:
    1) make sem1 // compile program
    2) ./sem1  // run program
    3) cmd
    4) ....
    5) exit or quit(to exit from program)
  
 ## what is pipes:
    A pipe is a form of redirection (transfer of standard output to some other destination) that is used in Linux and other 
    Unix-like operating systems to send the output of one command/program/process to another command/program/process for 
    further processing.
    
    how to run program with pipes:
        1) make sem1 // compile program
        2) ./sem1  // run program
        3)  cmd1 | cmd2 or cmd1|cmd2 or cmd1 |cmd2 or cmd1| cmd2
        4) ....
        5) exit or quit(to exit from program)
  ## How to change directory:
      1) make sem1 // compile program
      2) ./sem1  // run program
      3)  cd path(left empty or write "~" change to home directory ; ".." change to parent directory; "-" change to previous directory
      4)  ..
      5) exit or quit(to exit from program) 
 ## ctrl + c signal:
      if something went wrong, push ctrl + c to kill all child processes
