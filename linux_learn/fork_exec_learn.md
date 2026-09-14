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

       ↓

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
// edi and esi values NOT guarenteed to be saved (ie still 2, 3)

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
the syscall

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
RSP = your user stack (pointer to the return address in the stack, when call ret, will go here, read the address, and load it into the rip to go back to the previous function call) (look at stack notes!!!)
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
