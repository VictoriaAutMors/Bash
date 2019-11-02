# semestrial-work-1
Shell implementation in C language
  
##  What is Shell:
    shell is a command-line interpreter or shell that provides a command line user interface for Unix-like operating systems.           
    The shell is both an interactive command language and a scripting language, and is used by the operating system to 
    control the execution of the system using shell scripts.

## shell features:
    1) execute command
    2) pipes 
    3) change directory 
    4) background process 
    5) change input/output
    6) ctr + c signal to terminate current(all) process

## How to compile and run program:
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
        
  ## How to change working directory:
      compile and run program
      1)  cd path
      2)  ....
      if path:
          1) left empty or "~" change to home directory; 
          2) ".." change to parent directory; 
          3) "-" change to previous directory;
            
  ## What is background process
    In Unix, a background process executes independently of the shell, leaving the terminal free for other work. To run a process in the background, include an & (an ampersand) at the end of the command you use to run the job. 
    
    How to run program in background:
    <b> compile and run program </b>
    1) cmd &
    program will print number and PID of background process
    -----------
    if backgorund process is completed, program will print number and name of completed process 
  
  ## What is change input and output:
      Redirection is a feature in Linux such that when executing a command, you can change the standard input/output devices. The basic workflow of any Linux command is that it takes an input and give an output.
      
      How to change input and output:
      1)
## ctrl + c signal:
      if something went wrong, press ctrl + c to kill all child processes
