# Intro to Linking

## Process Architecture

Key Types of Files:

ELF FILE:
Executable Linkable Format File (standard linux executables):



readelf -l /bin/bash (elfread lets you see the inside of an Executable):
otool for macOS,  because uses MachO files (-L for shared libs, -h for headers, -l for load commands, -tV disassemble)

---OUTPUT---------------------------------------------------------------------------
-rwxr-xr-x    1 root     root       519964 Jul  9  2001 /bin/bash

Elf file type is EXEC (Executable file)
Entry point 0x8059380
There are 6 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  PHDR           0x000034 0x08048034 0x08048034 0x000c0 0x000c0 R E 0x4
  INTERP         0x0000f4 0x080480f4 0x080480f4 0x00013 0x00013 R   0x1
      [Requesting program interpreter: /lib/ld-linux.so.2]
  LOAD           0x000000 0x08048000 0x08048000 0x79273 0x79273 R E 0x1000
  LOAD           0x079280 0x080c2280 0x080c2280 0x057e0 0x09bd0 RW  0x1000
  DYNAMIC        0x07e980 0x080c7980 0x080c7980 0x000e0 0x000e0 RW  0x4
  NOTE           0x000108 0x08048108 0x08048108 0x00020 0x00020 R   0x4

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .interp 
   02     .interp .note.ABI-tag .hash .dynsym .dynstr .gnu.version .gnu.version_r .rel.got .rel.bss .rel.plt .init .plt .text .fini .rodata 
   03     .data .eh_frame .ctors .dtors .got .dynamic .bss 
   04     .dynamic 
   05     .note.ABI-tag 
---OUTPUT---------------------------------------------------------------------------

FORMAT OF THE FILE:
+------------------------+
| ELF Header             |
+------------------------+
| Program Header Table   |  (optional)
+------------------------+
| Section 1              |
+------------------------+
| Section 2              |
+------------------------+
| ...                    |
+------------------------+
| Section Header Table   |  (optional)
+------------------------+






ELF Header:  Header for entire file, holds archtecture, endianness, and entry point to program
exe fields:
unsigned char e_ident[16];   // magic bytes, class, endianness
Elf64_Half    e_type;         // executable, shared library, relocatable...
Elf64_Half    e_machine;      // x86-64, ARM, RISC-V...
Elf64_Word    e_version;
Elf64_Addr    e_entry;        // entry point
Elf64_Off     e_phoff;        // program header offset
Elf64_Off     e_shoff;        // section header offset
Elf64_Word    e_flags;
Elf64_Half    e_ehsize;
Elf64_Half    e_phentsize;
Elf64_Half    e_phnum;
Elf64_Half    e_shentsize;
Elf64_Half    e_shnum;
Elf64_Half    e_shstrndx;





Program Header Table: This points to the different segments that need to be loaded in (mapped into the process)
(this is used by the kernel + dynamic loader) (if static, then kernel does all the mapping)

Each entry describes a section to load:
PT_LOAD
PT_DYNAMIC
PT_INTERP (this is the stuff for the interpreter / dynamic linker)
PT_NOTE
PT_PHDR
PT_TLS
PT_GNU_STACK
PT_GNU_RELRO

and each entry contains:
p_type
p_offset
p_vaddr
p_paddr
p_filesz
p_memsz
p_flags
p_align

(more on that below)

Program Sections:
Your executable is split into different parts for the different types of data that must be stored there:

.text where you actual executable machine instructions go, like the instructions that the CPU will execute
.rodata stores read-only initialized data like strings and global constants (see globals_statics_learn.md)
.data stored initialized globals + statics (see globals_statics_learn.md)
.bss stored uninitialized globals and statics (will be initialized to 0 when the process starts)(MORE LATER)

exe:
suppose you have

int x = 5;
int y;
int main() {
    printf("Hello");
    return 1;
}

the print and return flow of code are in .text
the "Hello" string in rodata
the x variable in .data
and the y variable in .bss

(THESE WILL MAKE SENSE LATER)
.got Global Offset Table, for storing temporary memory
.symtable (hold information used to locate and relocate symbols and references)
.strtab (string table, holds the strings used by the program)
.plt (holds the PTL table used to resolve symbols dynamically linked via the loader)
.dynamic (holds key info for dynamic linking, like which libs are needed)


Section Header Table: At the bottom, and points to the individual sections of the ELF file,
like .text, .rodata, etc









So, to reiterate:

The Section Header Table at the bottom holds pointers to the sections of the file like
.text and .bss 

The Program Header Table at the near top holds pointers to the different segments of the file
(SEGMENT = SIMILAR SECTIONS SQUISHED TOGETHER TO MAKE LOADING INTO THE PROCESS EASIER)

Kernel only used the Program Headers for loading, since it loads by segment, not by section:

Each program header, aka PT_LOAD describes a part of memory to map into the process (read the processes notes to understand what I mean by mapping files into memory)

For each PT_LOAD entry, Kernel:
1. Creates the VMA struct for it
2. Sets Perms
3. descides the file backing

EXE:
loadable sections:

Segment 1 (r-x)
- .text
- .rodata

Segment 2 (rw-)
- .data
- .bss

On the file, it is organized literally as:

PT_LOAD:
    offset = 0x0000
    vaddr  = 0x400000
    filesz = ...
    memsz  = ...
    flags  = r-x

PT_LOAD:
    offset = ...
    vaddr  = ...
    filesz = ...
    memsz  = ...
    flags  = rw-

(NOTE: notice how it doesnt say anything about the sections, like it doesnt say .text in reality)
(just uses addresses, offsets, and amount of memory, to highlight the sections of the ELF file that need to get
loaded into the process)

offset = offset from teh start of the ELF file (on the harddrive, static)
vaddr = virtual address that it should be mapped to (this is how the VMA struct knows where it goes)
filesz = file size, aka the number of bytes FROM THAT OFFSET that must be mapped into memory
memsz = memory size, aka the number of bytes that must be mapped into memory IN TOTAL
flags = just stuff like perms

This is used by the kernel when mapping to memory and creating the VMA structs to index in process memory:
mmap(file, offset,1000 bytes);





KEY: FILE-BACKED MEMORY VS NON FILE-BACKED MEMORY

certain things you want in the process must be file-backed, meaning that the memory in the process must
be "backed" or determined by a file, like, for example, the .data section that holds integers

in the .data on the ELF file:

int x = 7 (obviously in bytes, not c, but there are 4 bytes here that we WANT to be in memory)
thus, the .data section for this file MUST be file backed by the elf file, specifically the .data section of
the elf file, hense the mmap command use

HOWEVER, NOT ALL MEMORY SECTIONS MUST BE FILE BACKED:
exe:
.bss, for our int y;, there is NO data that is backing it, its literally 0!
instead of having it backed by a file, it is ANONYMOUSLY BACKED, meaning on loading, the loader just
loads in a random zero-ed page of memory, making all of that memory 0
NOTE: it doesnt mean it stays 0, just that on the mapping for a VMA struct, it will point to a random zeroed
file. 

Specifically, for bss, looks like:
filesz = 1000
memsz = 2000, meaning that the first 1000 bytes are mmaped over, and the remaining 1000 are anonymously backed

Again:
after loading in, Kernel looses track of which section is which! only knows the data stored in the VMA struct:

start address (virtual ofc)
end address (virt)
permissions (r/w/x)
file backing (if any)
offset into file (if file-backed, allows it to be backed by a particular part of a fiel, not the whole thing)

NOTE: aside from just hte file, loader also mmaps part of the kernel into the process to speed up syscalls and inter-process communication, but its a good practice to not break into kernelspace.


## More on Libraries

(to first get a loose understanding, please read filesystem_learn to get whats going on here)

First thing to make clear, "libraries" in linux are executables too! they also obey the ELF format, just
dont have as clear of a start point like regular executables.

Library: a piece of software meant to be re-used between programs without the need for re-writing

2 main types of libraries:

Statically Linked:
contains compiled code (ELF), gets copied into your code during link-time

Dynamically LInked:
contains compiled code (ELF), gets linked, but never copied into your code. Instead, it is loaded into the 
process as a separate file (with its own separate segments) and references to its functions are resolved by
the loader during runtime

### Brief example, to illustrate

Start with some code:

```c
#include<stdio.h>

int main()
{
    printf("Hello");
    return 1;
}
```

Ever wondered how this works? We clearly never defined printf, or even declared it for that matter, 
yet when you run this it works fine. HOW?

Part 1: Header file

when you #include<stdio.h>, it LITERALLY copies the header to the top of the file

the header, as you've seen before, looks something like this (simplified):

#infdef STDIO_H
#define STDIO_H
int printf(const char* format, ...);
#endif

so, when you #include, it literally just copies over the declaration to the top of your c file during compilation:
(notice pre-processor flags gone from both, and just pretend this is binary)

int printf(const char* format, ...);

int main()
{
    printf("Hello");
    return 0;
}

Okay... that makes sense, when you do gcc -c main.c -o main.o, it creates the object file that looks like the
binary version of the pseudocode above (following ELF format of course), but then when you write:

gcc main.o -o app // it creates a magical executable that suddently works! how is this possible






SECRET:

even though printf is not defined in our main.o is IS defined in a library somewhere, specifically
libc.so. Thus, gcc must somehow be using the definition of printf defined in the library there, and resolving
any calls to "printf" in our own main.o by routing them to use the library. 

How does it do this:
using the linker! but the implementation depends on the type of library





Static Library:
A static library is, again, a compiled ELF file that we want to be copied into our executable DURING LINK-TIME

exe:
suppose we have some static library mylib.a, which contains the 

void myfunc (int val1, int val2); definition that we want to invoke from main.o.

this library, and libraries in general are JUST ARCHIVES OF MULTIPLE .o FILES DEFINING STUFF
(if you look inside of libmylib.a, you will find):

1. A bunch of .o files used to define functionality
2. A symbol table: a table which maps symbols to the place in the file where they are defined
exe:
myfunc -> offset 0x019283109 in the file
(symbol meaning any function or variable or something that a file doesnt know how to resolve, like 
main.o's printf)

During Linking:
the linker will

1: Merge the sections of the different static object files it is linking
(takes the .text from main.o, and the .text from libmylib.a's files and combines them into one large file)

2: Resolves Symbols:
(looks in main.c, sees that it need some symbol "printf", checks the symbol tables of all of the existing
libraries, find the symbol, and then connects them, making the address of the function call be the one in 
libmylib.a)

3: assigns final addresses
multiple different files, each with their own offsets from their own top, now combined into a single file,
using only the absolute addresses/offsets from the new single file's top.





Dynamic Libraries:
still an archive of ELF files, but this time to not just copy them into the main binary.
Instead, map them in separatly into the process using mmap.
(if you go to the top, you saw the PT_LOAD segments that would be mapped in?
its just like that except for the library itself)

When linking with dynamically loaded libraries, instead of copying and merging,
linker makes note of which libraries must be loaded into the process later at runtime.

This is done via the DT_NEEDED entry table:
basically a table saying which shared libraries must be there.
During runtime, the loader will then search for them in standard paths (/usr/lib or /usr/local/lib)
or in special user-specified paths.
Then it will mmap in the required library segments, along with the other segments of the final ELF executable

Also, in the metadata for the ELF file, the linker will make note that it needs a dynamic loader, usually
ld-linux, whose job it is to load in those required lbraries into memory and resolve the calls to their
methods.
(use readelf -l /usr/bin/curl | grep interpreter to see which one is used)

Full Process:

compile via gcc, gcc internally calls ld(the linux's linker) which links the static and dynamic libraries with the final ELF file,

Then during runtime, run program, kernel sees in the ELF header that is needs dynamic loader, invokes ld-linux
the standard dynamic loader, loads required .so libraries

### Altogether Intercommunication

exe:
program calls printf, defined in libc
loader finds printf in libc's symbol table
and patches the call site (via PLT and GOT)

Problem:
when you are compiling/linking your main.c, and it sees printf("Hello")
compiler does NOT know where printf will be loaded into memory, 
or which version of libc will be used, etc..

Instead of hard-coding addresses, generates placeholder call mechanism (PLT + GOT)

GOT: Global Offset Table
table of pointers
each entry is address of some global variable or function
INTIALLY: entries are empty or point to resolver stub
LATER: filled with real addresses

PLT: Procedure Linkage Table
a way of indirectly calling a function via the GOT
instead of calling printf(), you call printf@PLT

when you call that PLT entry:
1. looks up the GOT entry hoping to find the actual address of printf
2. if not resolved yet, jumps to resolver
3. if resolved, jump to real function

full calling:

printf("Hello");
->
call printf@plt

function PLT stub runs:

print@PLT:
    jmp *GOT[printf]
    push relocation_index
    jmp resolver

(WHAT DOES THIS MEAN)

Every external function like printf has:
1. slot in GOT
2. relocation index
3. PLT entry

GOT and PLT explained, but relocation index is just

printf 1
scanf 2
strlen 3
(basically a mapping between the strings for the symbol name to indicies, making them easier to operate on)

SO, what the resolution stub does is:

On first call:
jumps to the GOT entry of printf, however because this is the first time you are using it, it doesnt
hold an absolute address inside the process and instead has the address to the next line in the stub

From there, it will push the relocation index to the resolver.
(the resolver is just a part of the dynamic loader whose job is to resovlve symbols)
by giving it the symbols relocation index, the resolver now knows

1: symbol name (printf)
it wil then search the loaded shared objects, specifically their symbol tables to try to find
the desired symbol. Once it finds the real addess of teh function, it writes it to the GOT slot(called patching the GOT slot), and then jumps to the real function

On future calls:
the first like just jumps to the GOT table entry, which now contains the actual address of that symbol, which
is then resolved and can be called repeatedly




IN CONCLUSION:
when you call, ./hello, instead it looks at its INTERP field, finds the interpreter specified (ld_linux.so.2)
and actually passes your executable to it.

This will then:
reads the executable
finds required shared libraries (ones listed via DT_NEEDED)
maps them into memory (libc.so, etc.)
performs relocations
resolves symbols
calls constructors (linking_intro for this)
finally jumps to your program's entry point (_start)

## Now, for practice:

