# How do fork + exec work, also processes recap

Prolly get the gist about fork, execve, and processes, so we wont re-explain that, but will brief recap:

A **process** from an OS perspective is a collection of kernel-managed states

Process
├── CPU execution state (look at memory + stack for this)
│   ├── instruction pointer
│   ├── stack pointer
│   └── registers
│
├── virtual address space (covered in memory / stack / processes)
│   ├── program code
│   ├── global data
│   ├── heap
│   ├── stack
│   └── memory mappings
│
├── file descriptor table (covered in here + pipes_learn)
│   ├── fd 0
│   ├── fd 1
│   ├── fd 2
│   └── ...
│
├── process ID
├── credentials
├── signal state
├── working directory
└── many other kernel bookkeeping structures

Much of this, in linux lives in
```c
struct task_struct
```
(Kernel-Level struct, do NOT manipulate directly) (process defined in ELF, but for OS kept as struct)

With each struct being associated with:
task_struct
   |
   +--> virtual memory description
   |
   +--> file descriptor table
   |
   +--> scheduling state
   |
   +--> credentials
   |
   +--> signal information
   |
   +--> PID information

## User Mode vs Kernel Mode
(covered in intro to memory)

Loosely: some operations are not safe to allow the user to make. Exe: anything that is kernel-side is generally
unsafe to let the user do. Exe: mapping memory or reading kernel's memory 
(kernel memory is mapped into a process, but perms stored in the memory heirarchy enforce that it can only access it
when it supervisor-mode)

**supervisor-mode** the perms in the page table that enforces that cpu cannot read that memory unless in supervisor mode.
(aka kernel-mode)

**User Mode**: Priviledge Level 3

**Kernel Mode**: Priviledge Level 0

To do anything like:
- access a disk controller
- modify page tables
- create processes
- map arbitrary physical memory
- manipulate another process
- configure hardware
- directly modify kernel structures

**KEY** YOU CANNOT JUST DO IT YOURSELF! YOU MUST DO IT VIA A SYSTEM CALL, WHICH IS WHEN YOU ASK THE KERNEL TO DO IT 
FOR YOU USING PRE-EXISTING DEFINED FUNCTIONS KNOWN AS SYSTEM CALLS.

exe:
read (reading from a file to some buffer in the memory)
write
open/openat
close
mmap
fork/clone
execve
wait4
socket
connect

### Why do they look like functions?

**NOTE** these "look" like C functions, and indeed do exist as C functions, but in reality they are just a function
which can be run in assembly / machine code, and the C function is just defined in a library to make that low-level 
call under the hood, and get the return and work with it. THINK OF IT LIKE A WRAPPER FUNCTION THAT MAKES THE LOW-LEVEL 
SYSCALL AND THEN MAKES THE OUTPUT USABLE AT A C-LEVEL.

exe: write(fd, buf, count);
write(1, "hello\n", 6);

Conceptually:
your C code

    write(...)

       ↓

libc write() wrapper

       ↓

sets CPU registers according to syscall ABI

       ↓

executes CPU instruction:
    syscall

       ↓

CPU enters kernel mode

       ↓

Linux syscall handler

       ↓

kernel performs write

       ↓

kernel returns to user mode

       ↓

libc write() returns



your C program continues

### Thus, other languages can ALSO DO SYSCALLS
(same idea, the language just creates some function that serves as a "wrapper" for the actual syscall so it can fit
into the language's ecosystem)

CPU doesnt care about "the language", in fact it doesn't even know what that is. it only knows opcodes, data, and addresses. Everything else is just an abstraction (look at the C notes to know what I mean by abstraction)

It just wants to see the opcodes that facilitate the syscall (which we will explain shortly), THATS IT

### Ordinary Functions vs Syscalls
Given some normal C function:

```c
int add(int a, int b) {
    return a + b;
}

int x = add(2, 3);
```

This will be turned into machine/assembly code (roughly):
(note: edi is just the 4 LSBs of RDI register)
(recall rdi, rsi, rdx, rcx, r8, r9 are **caller-saved**, since its the caller's job to save it if it wants to)

mov edi, 2
mov esi, 3
call add
// edi and esi values NOT guarenteed to be saved (ie still 2, 3) (caller-saved)

BY CONTRAST:
```c
write(fd, void* buf, size_t bytes)
```

will LOOSELY become:
mov eax, 1 
mov edi, 1
mov rsi, buffer
mov edx, 6
syscall //this is the opcode for syscall, which is a literal opcode like mov or something

## Syscall Calling Convention

**NOTE** this is NOT the same as the sysv calling convention!!! (close tho)

For x86 sysvabi:

Item	Register
syscall number	rax //This is the number that corresponds to which syscall to execute: 0 is read, 1 is write, etc...
argument 1	rdi //first arg
argument 2	rsi //second arg
argument 3	rdx //third
argument 4	r10 //fourth **NOTE** its NOT rcx!
argument 5	r8 //fifth
argument 6	r9 //sixth
return value	rax 

Differences:
1: RAX is usually just the return register, but here it is overloaded to also be the register used for selecting
the syscall (also caller saved)

2: Notice that RCX is not used as an argument, instead uses r10. 
This is because RCX is already in use, used to store the stack return address after the syscall terminates (ie the current RIP, ie the next instruction to execute)
(RSP but for syscalls, since the RSP register is for that function (and the syscall is techincally a part of the function))

r11 is RFLAGS, but thats not too important


THUS:
```c
write(1, buf, 6);
```

BECOMES:
```assembly
mov rax, 1        ; syscall number: write
mov rdi, 1        ; fd (int)
mov rsi, buf      ; pointer (address)
mov rdx, 6        ; byte count (size_t, so basically uint)
syscall
```

## What does syscall opcode actually do?
(feel free to read more here: https://www.felixcloutier.com/x86/syscall)

CPU Before Syscall:

CPU
────────────────────────────────
RIP = address inside your program of the NEXT instruction
RSP = your user stack (pointer to the return address in the stack, when call ret, will go here, read the address, and load it into the rip to go back to the previous function call) (look at stack notes!!!) (RSP GIVES RECURSIVE FUNCTION)
registers = your values
privilege = user

During Call:

2 key registers:

**RCX**: the return address, where it should go after execution (usually just push RIP here before call)
**IA32_LSTAR MSR**: this is an OS-configured region of memory, holding the address of the next RIP needed to facilitate the system call, which RAX can help you determine based on the syscall number

1: change cpu mode from priviledge level 3 to 0
2: put RIP to RCX for return
3: set RIP to the address at IA32_LSTAR MSR and start executing


SO:
user:
    syscall
       |
       v

================ privilege boundary ================

kernel syscall entry
       |
       v
interpret syscall number
       |
       v
dispatch syscall handler (the part in IA32_LSTAR MSR)
       |
       v
perform requested operation
       |
       v
return path

================ privilege boundary ================

user:
    next instruction

### System Call Error handling
At the syscall level, error are represented as negative numbenrs:
-EBADF
-EFAULT
-ENOMEM
stored in RAX

But at the C level, its -1, and then inspected via errno:

write(...) == -1
errno (more on this in C learn), but its a thread global int, that can be string with strerror(errno)
refers to most recent error, so only one value

This conversion, between the negative nums in rax and the c behavior is done by libc
(its usually just inverse the minus, return -1, and set errno to the positive lol)


## File Descriptors in Processes

(again, this is covered in pipes_learn), but loosely, file descriptors, and the kernel-side file descriptor
table mapping descriptors to descriptions (structs) is PROCESS-DEPENDENT, not GLOBAL

each process has some table:
process
   |
   +-- fd table
        |
        +-- 0
        +-- 1
        +-- 2
        +-- 3

Where "3" is not the file, but an index into the calling/current process's descriptor table.

exe:
That process's FD table

0 ────→ terminal input 
1 ────→ terminal output
2 ────→ terminal error
3 ────→ kernel open-file object for foo.txt

ASIDE: FILE DESCRIPTOR VS DESCRIPTIONS:
In the process's description table, it is mapping ints to file descriptions, which are kernel-side structs
that describe the file, contains things like perms, size, offset, ptr to inode, ref count, etc.
These "files" dont have to be standard text files either, they can be:
a regular file
a terminal
a pipe
a TCP socket
a device like /dev/null

CRUTIALLY: this mapping is NOT fixed, you can allways change which file description is referenced by which number.
Very common, called "pipes and redirects" (prolly should read the aptly named file pipes_learn.md to learn more)



thus:
```c
write(3,...)
```
does:
current process
       ↓
fd table
       ↓
entry 3
       ↓
open file description

(To write your own syscall, have to recall that doesnt usually create an interrupt stack frame)
(thus, you are responsible for saving registers before the call)

(NOTE: IA32_LSTAR is a MSR, or **Model-Specific Register**, which means that it depends on the model)
Its address is: 0xC0000082
To write to it:
mov $0xC0000082, %ecx
wrmsr (special instruction that allows you to access it)

FOR NORMAL REGISTERS, READ SIMPLY VIA:
uint64_t rax;

asm volatile (
    "mov %%rax, %0"
    : "=r"(rax) //(rax) means choose some general purpose register to put it in, and save to rax
);
(asm volatile is gcc entension that lets you put assembly into c code directly)
asm = tells compiler this command is assembly, and to put it into the instruction area
volatile = dont assume this can be removed or moved just because the outputs arent obvious to you
useful for assembly code with side effects.

exe:

int x = 0;
while(x == 0){} //compiler will optimize this away, 

but with volatile:
volatile int x = 0; //it will check x EVERY TIME. Not guarenteed to not change
without, compiler may read x, and keep in a register, never re-reading x.
But, with volatile will keep rereading