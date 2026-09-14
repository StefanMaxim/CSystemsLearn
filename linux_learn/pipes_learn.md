### Pipes and Redirects (WIP)

Pipes "|" connects the standard output of the command on the left to the standard input of the command on the right.

**KEY**
stdin, stdout, and stderr are just file descriptors, defaulting to 
Keyboard ───► stdin (fd 0)
Terminal ◄── stdout (fd 1)
Terminal ◄── stderr (fd 2)
(NOTE; in kernel, only file descriptors exist, stdin/stdout/strerr are their nicknames by convention, and 
Keyboard/Terminal is the file description initially paired with it when in the terminal)

#### **FILE DESCRIPTORS** 
are just numbers used in a linux process to refer to different periferals, dictated by a 
kernel-side file descriptor table, unique to every process.


when you read(fd=0, ptr = buf, num_bytes=100);, its saying "read from whatever file descriptor 1 refers to, read 100 bytes into buffer buf"
The operating system keeps another data structure that represents an open file (more generally, an open I/O object),
called a **FILE DESCRIPTION**.
a regular file
a terminal
a pipe
a TCP socket
a device like /dev/null


**NOTE** file descriptOR is the number, file descriptION is the datastructure representing the open file
containing things like current file offset, file status flags (O_APPEND, O_NONBLOCK, etc.), pointer to the underlying inode or file object, and reference count.

IN PRACTICE:

echo hello > out.txt

0: In the beginning, we are running the terminal as our process, with stdin=Keyboard and stdout=Terminal App
THE VERY FIRST THING THAT HAPPENS IS:
Forks process, using fork() //USES COPY-ON-WRITE, meaning shares underlying memory pages until a write occurs
**KEY** FORK FIRST, BC DONT WANT TO MESS WITH THE STDIN AND STDOUT OF THE CURRENT PROCESS

(WE ARE NOW IN THE FORK)

1: Before running echo, shell opens out.txt
This returns a new file descriptor, say
fd 3 -> out.txt
**KEY** opens here bc cannot access this data in the child process


3: In new child process, Copies fd3 onto fd1 using dupe2(fd1=1, fd2= 3)
fd1 -> out.txt
fd3 -> out.txt

4: closes fd3
fd1 -> out.txt

5: Then, it starts echo via execve()
where the child process of echo will already have the modified file descriptor table. 
**NOTE** THIS IS BECAUSE EACH PROCESS'S FILE DESCRIPTOR TABLE IS KERNEL-SIDE, SO REMAINS AFTER EXECVE
that being said, each process has its own table, however they can point to the same description after a fork.

These file descriptions are stored in the kernel in a larger table. However
**CRUTIAL** you cannot see these tables in kernelspace!

you can only interact with them via systemcalls, like open, which return the file descriptor you can use
but you cannot directly edit the tables

**Crutial Processes**
You load in kernelspace into the process, but that stuff is shared among all processes, and has vastly different perms
than usual. Most programs cannot access kernelspace, but it is loaded into the process for speed of systemcalls. 

Virtual address space of Process A

+----------------------------+
|                            |
| User space                 |
|                            |
|  program code              |
|  heap                      |
|  stack                     |
|  shared libraries          |
|                            |
+----------------------------+
|                            |
| Kernel space               |
|                            |
| kernel code                |
| kernel data structures     | #like the VM_AREA_STRUCT defining that processes valid memory locations
| process tables             | ##PROCESS TABLE IS TABLE CONTAINING ALL PROCESSES RUNNING
| file descriptor tables     |
| page tables                | ##PAGE TABLES DESCRIBE MEMORY
| device drivers             | ## THIS TURNS THE SYSTEMCALLS AND THE KERNELS INTENTIONS INTO 
|                            | ## CODE THE PERIFERALS UNDERSTAND
+----------------------------+
You need kernel priviledges to access kernelspace ofc.


Thus, when you 

echo hello > out.txt

first open via syscall: fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC); #fd = 3 for now for example

then, duplicate it into 1: dup2(fd,1) #copies file description in 3 into entry 1

lastly, close the last one close(fd) (THIS IS FOR CHANING THE OUTPUT, THE INPUT IS DETERMINED FROM COMMAND LINE ARGS)

then exec 

**NOTE: THIS IS ALL DONE INSIDE THE FORK!!!**
fork() / clone()
        │
        ├── child:
        │     openat(AT_FDCWD, "out.txt",
        │            O_WRONLY|O_CREAT|O_TRUNC, 0666)
        │
        │     dup2(fd, 1)
        │
        │     close(fd)
        │
        │     execve("/bin/echo", ...)
        │
        └── parent:
              wait4(...) / waitpid(...)

WITH SHELL DOING ROUGHLY:
pid_t pid = fork();

if (pid == 0) {
    int fd = open("output.txt",
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);

    dup2(fd, STDOUT_FILENO);
    close(fd);

    execve("/usr/bin/cat",
           {"cat", "input.txt", NULL},
           environ);
}

waitpid(pid, ...); 

**HUH????***

#### ASIDE ON FORK, AND HOW IT WORKS:
(how do programs know when to work in the child or in the parent)
Earlier, I said "we are now in the child", but thats not technically correct:
(READ FORK_EXEC_LEARN!!!)

but loose idea is that the code after the fork() but before the wait will check the pid, 
with the pid being 0 if it is the child, and PID if its the parent. 
Then, it will check the remaining code for its PID, and that way it isolated from the parent







Before:

Process (PID 1234)

Kernel state:
    Processes' Kernel File descriptor table
        0 -> terminal
        1 -> out.txt
        2 -> terminal
    PID = 1234
    Current directory = /home/alice
    UID = 1000
    ...

User-space:
    bash code
    bash stack
    bash heap


After:
Process (PID 1234)

Kernel state:
    File descriptor table
        0 -> terminal
        1 -> out.txt
        2 -> terminal
    PID = 1234
    Current directory = /home/alice
    UID = 1000
    ...

User-space:
    echo code
    echo stack
    echo heap










For pipes, its usually different in that pipes created before the fork

exe:

ls | grep txt

the shell does:

pipe(pipefd);
fork();   // child 1
fork();   // child 2

pipe() is used when you want to have inter-communication between processes.
when you want to send stuff between processes, one method is using a pipe

A pipe is a communication channel made my the kernel, living in the kernel
One process puts bytes into it, and another reads bytes out of it.
When created, kernel allocates a pipe buffer where bytes can be stored in transit, and 2 new kernel file descriptions(aka file descriptor entries),
a read and a write object that lets you read and write to the buffer respectively

Current Process's File Descriptor Table:
Initial:

fd 0 -> stdout
fd 1 -> stdin
fd 2 -> stderr

Final:

fd 0 -> stdout
fd 1 -> stdin
fd 2 -> stderr
fd 3 -> pipe read #NOTE, doesnt have to be 3 and 4, its just whatever 2 numbers is returned in the array fd
fd 4 -? pipe write

usage:

int fd[2] # this creats a 2 element array [fd[0], fd[1]], used to store the file descriptors the kernel gives us
fd[0] is the read end, and fd[1] is the write end

pipe(fd) # here, you are passing the address of the first element of the array, and the kernel fills in the 
file descriptors for the read end and write end. NOTE: the kernel does not give you the read/write objects themselves,
just 2 numbers you can use to refer to them.

thus, after calling:

int fd[2]
pipe(fd)

how have 
fd 0 -> stdout
fd 1 -> stdin
fd 2 -> stderr
fd 3 -> pipe read #NOTE, doesnt have to be 3 and 4, its just whatever 2 numbers is returned in the array fd
fd 4 -? pipe write #NOTE, need 2 descriptors so kernel can enforce directions, only read from read and only write to write


Next, the forks, to create the two twin processes.
fork()
fork()

Then:

Child 1:

dup2(pipefd[1], STDOUT_FILENO); # now, its stdout is the write end of the pipe
execve("ls", ...);

Child 2:

dup2(pipefd[0], STDIN_FILENO); # and this stdin is the read end of the pipe
execve("grep", ...);

Thus, when you run them, child 2 wont run untill recieved input in stdin, and child 1 will run, and its output
will go to child 2's stdin, letting it run then and completing the pipe.






What about stuff like:

cat ~/.ssh/id_ed25119_linux_desktop | ssh bob@100.223.13.14 "cat >> ~/.ssh/authorized_keys"

The cats do interact here, but not directly.

flow: currently in some process, exe PID 1234


First, creates pipe. (doesnt open the file, that happens way later)

int pipefd[2];
pipe(pipefd) # creates kernel side pipe object with its buffer, and read and write objects with descirptors here

f1 0 stdin 
f2 1 stdout
fd 2 stderr
fd 3 pipe read
fd 4 pipe write

Next it forks the first child
pid_t pid1 = fork()
and
dup2(pipefd[1],1)

Now
f1 0 stdin 
f2 1 pipe write
fd 2 stderr
fd 3 pipe read
fd 4 pipe write

and closes redundant fd:
close(pipefd[0]) #not needed so might as well close
close(pipefd[1])

fd 0 stdin
fd 1 pipe write
fd 2 stderr


Next, it runs the command:

execve("/bin/cat", ["/bin/cat", "file"], ...) #which recall only overrides the userspace, the stuff we did up to now is safe

at init, cat now sees:
stdin(0) -> terminal
stdout(1) -> pipe write
stderr(2) -> screen often

However, it recieved argument "file"
so cat itself does
int fd = open(file)
and thereby makes it:

fd 0 -> keyboard
fd 1 -> pipe write
fd 2 -> stderr
fd 3 -> file

and then cat will read from file and write to stdout (the pipe write)
**NOTE** no need to dup2 and close to change what stdin is here, as for input it just goes by command line args



CONTINUING:
cat ~/.ssh/id_ed25119_linux_desktop | ssh bob@100.223.13.14 "cat >> ~/.ssh/authorized_keys"

Child process 2:

pid_t pid2 = fork()

after fork, it will 
dup2(fd[0],0)
close(fd[0])
close(fd[1])

so now its

fd 0 -> pipe read
fd 1 -> stdout
fd 2 -> stderr

now the key:
now, it will execute the cat

by default, its uses stdin when no command line args (lucky for us its pipe read)
and will have its stdout redirected to some file

Thus, will first open the file, call it file2

int fd = open(file2)

Then it will not fork because of the redirect, since already forked process for pipe

Then, it will change the stdout to fd 
dup2(fd,1)
close(fd)

Then it will execute 

execve(cat), which will, when no command line args do stdin to stdout, or pipe read to file2.




**NOTE** cannot do something like

echo < hello.txt > out.txt (not echo < "hello" > out.txt, bc will say no file called "hello" as input redirection
says open file and use it as programs stdin)

Doesnt work bc echo reads from command line args, so its the same as echo > out.txt
but generally, this just:

1:
Shell parses the command
It sees:
command: echo
arguments: ["echo", "hello"]
stdout redirection: out.txt

2:
Shell opens out.txt

Conceptually:

fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);

This creates the file if needed, or truncates it if it already exists.

3:
Shell forks

pid = fork();

Now there is a child process that will become echo.

4:
Child duplicates the file descriptor onto stdout

dup2(fd, STDOUT_FILENO);

Now:

fd 3  ---> out.txt
fd 1  ---> out.txt

File descriptor 1 (stdout) now points to out.txt.

5:
Child closes the extra file descriptor

close(fd);

Now:

fd 1 ---> out.txt

6:
Child calls execve

Something like:

execve("/usr/bin/echo",
       ["echo", "hello"],
       envp);

The echo program starts running. When it writes:

write(1, "hello\n", 6);

it goes into out.txt instead of the terminal.

7:
Parent shell waits

The shell waits for echo to finish, then displays the next prompt.



RECAP:

cat file.txt | cat >> file2.txt

1: create pipe for current process (shell) (kernel managed)
int pipefd[2];
pipe(pipefd)

2: fork
pid_t pid1 = fork()

3:in pid1
dup2(fd[1],1);
close(fd[0])
close(fd[1])

4: execve("/bin/cat",argv["/bin/cat","file.txt"],envp=["PATH=...])

5: int fd2open("file2.txt",RD_ONLY); //before the fork

6: now fork the second process, which will wait untill recieves input
pid_t pid2 = fork()

7: in pid2
dup2(fd[0],0)
dupe2(fd2,1)
close(fd[0])
close(fd[1])
close(fd2)

8: execve("/bin/cat",["/bin/cat"],["PATH=...])
state before execve:
0 -> pipe read
1 -> file2.txt
3 -> stderr
