For programs like C, 3 parts:

Header File, .h, which is used during compile only!
it declares functions, used by the compiler to verify that everything is good

Static Library, .a:
contains compiled code, gets copied into your code during link-time

Dynamic Library:
contains compiled code, gets linked, but is loaded dynamically during runtime


### Full Process for Linkage:


Start with some code:

#include <stdio.h>

int main() {
    printf("Hello\n");
    return 0;
}

This is defined in some header file, stdio.h

int printf(const char *format, ...);


2: when you #include ..., it LITERALLY copies the header to the top of the file, conceptually
this tells the compiler that such a function exists, and what its intended behavior is(no linking)

3: Now onto compilation
gcc main.c -c
checks if printf is used correctly, generates the object file main.o with teh headers pre-pended
(still no idea how printf works)

4:Now, linking
gcc main.o
(didnt specify where printf lives, but still works?)
Reason: because gcc automatically links the c standard library! specifically glibc
that include /lib/libc.so /libc.a

secretly, gcc main.o => gcc main.o -lc (meaning link with libc)
(the printf is acutally implemented in libc.so somewhere, with symbol printf)

However, what about using something that is not linked by default:

#include<math.h>
double x = sqrt(5);

does not automatically link libm.so, so need to include via -lm (link math.so)

For custom libraries:

#include <foo.h>
-lfoo to load the foo dynamic/static library



How to make your own Libraries:

First, make the header (what users see)

mylib.h:

#ifndef MYLIB_H
#define MYLIB_H

void hello();

#endif


Then, implement it:

mylib.c:

#include <stdio.h>
#include "mylib.h"

void hello() {
    printf("Hello from my library");
}

#### ar
Now, make the library:

Statically linked:

gcc -c mylib.c -o mylib.o

ar rcs libmylib.a mylib.o (Created static library)

(this used linux archiving tool ar to create a static library)
ar = archiver program, used to bundle object files into a single static archive (a static library)
r = replace/isert files into archive: if libmylib.a exists replace /update it
c = create archive if doesnt exist + suppresses warning
s = indexes the archive (builds symbol table so linkers can find functions faster)
libmylib.a is the output file, by convention .a on linux
libmylib.o is the compiled object file being added to the library


(AR = used to make library archives, TAR = used to make tape archives)

symbolic table:
index of everything defined in an object file or library, 
where a symbol is just the name the linker cares abt, usually:
function names (add, print_result)
or global variables
so table says:
add -> defined here
helper -> defined here

static library is just a bunch of .o files bundled together (ARCHIVE = BUNDLE FILES TOGETHER)
if left alone, linker would have to search through all the .o files for a given function, and 
see if it contains the desired symbol

instead, via ar rcs libmylib.a mylib.o, the s flag means to create a symbol table to speed up linkage




Dynamically Linked:

compile with position-independent encoding:

gcc -fPIC -c mylib.c -o mylib.o 

-c means compile only, no linking
-fPIC means generate Positional Independant Encoding:
normally, compiled code can contain absolute addresses :jump to 0x102911281
PIC means use relative addressing (from RIP or something) [rip + offset_to_global]
Or Global Offset Table (GOT) (read process notes for more info) [rip + offset_to_GOT_entry]
basically, a table of addresses for global variables and external symbols (entities the linker must resolve, like functions in .so/.a, global variables, and external references)
dynamic linker resolves symbols and writes their actual addresses in at runtime into the table

gcc -shared -o libmylib.so mylib.o 




Step 3: Use said Library

main.c

#include "mylib.h"

int main() {
    hello();
    return 0;
}

Compile + Link:

Static Linking Example:
gcc main.c -L. -lmylib -o app

-L. means look in current directory for libraries
-lmylib -> find libmylib.a and link it

Dynamic linking

gcc main.c -L. -lmylib -o app
(same command, but prefers .so if possible)

Step 5:
KEY FOR RUNTIME:
when running
./app, might get: 
"error while loading shared libraries: libmylib.so: cannot open shared object file"
reason: OS doesnt know where to look for the library (its not in /lib or /usr/lib)
(NOTE: header file in /usr/include on Linux or /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/stdio.h)

Fixes:

Temporary Fix: make the path clear:
LD_LIBRARY_PATH=. 
./app

Make path clear to linker

gcc main.c -L. -lmylib -Wl,-rpath=. -o app

-Wl, means send the following to the linker, where each comma separates another argument
-rpath=.  means at runtime, look at current directory for paths

thus:
-Wl,-rpath,. -> -Wl -rpath . (passes second two as arguments to linker)


More permanent fix:
sudo cp libmylib.so /usr/lib //copies it to the directory full of libraries
sudo ldconfig //runs the tool that updates the shared library cache. scans /lib /usr/lib and registers those libs
so the linker can easily find them

this changes your linking command to make it much smaller

gcc main.o -lmylib -o app (no -L. because /usr/lib is standard, and no -Wl,-rpath=. because no need)

NOTE: might be safer to put in /usr/local/lib, as /usr/lib meant for package managers, not good to put user
files there



Lastly about the header:
-I. used to tell it where to find the headers that it needs for the main, ie whenever a #include pops up





NOTES on the linker, ld:

the GCC command does more than just:

ld \
  -o app \
  main.o aux.o \
  -L. -lmylib \
  -lc \
  -dynamic-linker /lib64/ld-linux-x86-64.so.2

(the pure linker version)

(ld = linker, ld-linux is the dynamic loader/linker)

it also adds:
crt1.o
crti.o
crtbegin.o
crtend.o
crtn.o
without these, main() is never properly called, constructors/destructors wont work, 
(constructor = functions that run before main, destructor = functions that run after main)

this is what it really is calling:
ld \
  -o app \
  /usr/lib/x86_64-linux-gnu/crt1.o \
  /usr/lib/x86_64-linux-gnu/crti.o \
  /usr/lib/gcc/x86_64-linux-gnu/13/crtbegin.o \
  main.o aux.o \
  -L. -lmylib \
  -lc -lgcc -lgcc_s \
  /usr/lib/gcc/x86_64-linux-gnu/13/crtend.o \
  /usr/lib/x86_64-linux-gnu/crtn.o \
  -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
  -rpath .


Reason:

if recall from assembly, CPU starts at the _start flag/entrypoint and goes down 1-by-1
the start must:
1: set up the runtime
2: call main (argc,argv,envp)
3: handle program exit

what do all of the listed files above do:

1: crt1.o
C Runtime Startup Part 1:
creates entrypoint, _start
does setup, then calls:
__libc_start_main(main, argc, argv, ...) , eventually calling your main method

(crt1 is the normal dynamically linked version, Scrt1 is the PIE executable, or position independent encoding)
(fPIC)
rcrt1 is static PIE

2: crti.o
defined .init and .fini sections, used before and after main

what is initialization:
setting up global + static variables 
linkage = does a variable link, or refer to teh same entity across different parts of the program.

exe:

External Linkage:
//file1.c
int x = 5; //global variable

//file2.c
extern int x; here, x refers to the same variable aross files, hense external linkage
NOTE: extern declares without defining

Internal Linkage:
only visible inside particular file
//file2.c
static int x = 5;
y is private to file2.c, even if others try to extern it, not visible

No Linkage:
name is unique to its scope

void func(){
    int x = 5; //x exists here only inside of this function
}

Summary:
global just means its outside of any function definition. can be static or non-static. 
global static: scope = file-wide, lifetime = program wide, internal linkage
global non-static: scope = program-wide, lifetime = program wide, external linkage 
local static: scope = in function defined, lifetime is program-wide, no linkage
local non-static: scope = function defined, lifetime is function, linkage is no linkage




3: crtn.o




back to buisness:

okay, so what does a linker actually do:

the linker works with object files. (main.o crt1.o crti.o, etc)

when you link via gcc main.o -o app 

it really runs:
ld crt1.o crti.o crtbegin.o main.o ... crtend.o crtn.o -lc

What that means:
each object file has sections:

code (.text)
data (.data)
symbols (names like main, prinf)
relocation info

1. The linker merges the sections:
main.o, crt1.o, and libc pieces into a single .text second for the executable (statically or dynamically)

2. resolves symbols:
int main() { printf("hi"); }
main.o says it needs printf, linker searches for it, find it in libc, and linker connects then

3. Assigns final addresses

before: main is at offset 0x0
after: main is ax 0x41092091


How linking works:

library = just bundled archive of .o files

ar t lib.a => returns main.o auxilary.o

so linking a static library is just combining it with the origional executable together(as stated above)


Dynamic Libraries:
libc.so

here, does not copy it into the executable, instead makes note that this is needed during runtime


how does GCC pass in teh files:
crt1.o crti.o crtbegin.o ... crtend.o crtn.o
(.o files so linked like any other standard object file)

(gcc -v main.c will show you this)

(base) stefancelmare@pal-nat186-14-203 library_practice % gcc -v main.c
Apple clang version 21.0.0 (clang-2100.0.123.102)
Target: arm64-apple-darwin25.4.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
 "/Library/Developer/CommandLineTools/usr/bin/clang" -cc1 -triple arm64-apple-macosx26.0.0 -Wundef-prefix=TARGET_OS_ -Wdeprecated-objc-isa-usage -Werror=deprecated-objc-isa-usage -Werror=implicit-function-declaration -emit-obj -dumpdir a- -disable-free -clear-ast-before-backend -disable-llvm-verifier -discard-value-names -main-file-name main.c -mrelocation-model pic -pic-level 2 -mframe-pointer=non-leaf -fno-strict-return -ffp-contract=on -fno-rounding-math -funwind-tables=1 -fobjc-msgsend-selector-stubs -fno-sized-deallocation -target-sdk-version=26.4 -fvisibility-inlines-hidden-static-local-var -fno-modulemap-allow-subdirectory-search -fdefine-target-os-macros -fno-assume-unique-vtables -fno-modulemap-allow-subdirectory-search -enable-tlsdesc -target-cpu apple-m1 -target-feature +v8.5a -target-feature +aes -target-feature +altnzcv -target-feature +ccdp -target-feature +ccpp -target-feature +complxnum -target-feature +crc -target-feature +dotprod -target-feature +flagm -target-feature +fp-armv8 -target-feature +fp16fml -target-feature +fptoint -target-feature +fullfp16 -target-feature +jsconv -target-feature +lse -target-feature +neon -target-feature +pauth -target-feature +perfmon -target-feature +predres -target-feature +ras -target-feature +rcpc -target-feature +rdm -target-feature +sb -target-feature +sha2 -target-feature +sha3 -target-feature +specrestrict -target-feature +ssbs -target-abi darwinpcs -debugger-tuning=lldb "-fdebug-compilation-dir=/Users/stefancelmare/Library/Mobile Documents/com~apple~CloudDocs/Home/Computer Science/CSToolsLearn/linux_learn/library_practice" -target-linker-version 1266.8 -v "-fcoverage-compilation-dir=/Users/stefancelmare/Library/Mobile Documents/com~apple~CloudDocs/Home/Computer Science/CSToolsLearn/linux_learn/library_practice" -resource-dir /Library/Developer/CommandLineTools/usr/lib/clang/21 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -I/usr/local/include -internal-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/local/include -internal-isystem /Library/Developer/CommandLineTools/usr/lib/clang/21/include -internal-externc-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -internal-externc-isystem /Library/Developer/CommandLineTools/usr/include -internal-iframework /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks -internal-iframework /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/SubFrameworks -internal-iframework /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/Library/Frameworks -Wno-elaborated-enum-base -ferror-limit 19 -fmessage-length=151 -stack-protector 1 -fstack-check -mdarwin-stkchk-strong-link -fblocks -fencode-extended-block-signature -fregister-global-dtors-with-atexit -fgnuc-version=4.2.1 -fskip-odr-check-in-gmf -fmax-type-align=16 -fcommon -fcolor-diagnostics -clang-vendor-feature=+disableNonDependentMemberExprInCurrentInstantiation -fno-odr-hash-protocols -clang-vendor-feature=+enableAggressiveVLAFolding -clang-vendor-feature=+revert09abecef7bbf -clang-vendor-feature=+thisNoAlignAttr -clang-vendor-feature=+thisNoNullAttr -clang-vendor-feature=+disableAtImportPrivateFrameworkInImplementationError -Wno-error=allocator-wrappers -fdwarf2-cfi-asm -o /var/folders/n1/0nnckscj36xfsjzb_6z3y0ym0000gn/T/main-061d40.o -x c main.c

**NOTE** the above part is just the compilation, with all of its flags in place. calling clang to compile main.c and produce main.o in /var/folders/n1/0nnckscj36xfsjzb_6z3y0ym0000gn/T/main-061d40.o. -cc1 means clang does the real compilation


Key things to note in this massive command:
-main-file-name main.c (tells clang the main file name)
-x c (tells it the language of compilation)
-o ...main-090.o //where the output is stored
-emit-obj means to generate the .o file in teh directory

Also some target hardware+abi to use

issysroot MacOS.sdk

SDK = Software Development Kit:
its basically a reference kit that describes the OS to programmers(rather a directory that neatly has all the stuff needed)
what APIs exist
what functions are available
what behavior is allowed
includes headers, like stdio.h and stdlib.h
system libraries, what you link against later

NOTE: LOOK MORE INTO THIS, WHAT THE SDK IS AND HOW IT WORKS
from the looks of it, it has nothing in the /usr/bin section except some configs, but the include is full
(bin and libexec is different, the include is full of stuff)
the frameworks seem to still be there.


Linux parallel:
/usr/include contains teh system headers, which houses stdlib.h and stuff (SDK like)

c standard library (glibc) actual library containing implementaiton for the printf and stuff

kernel interface headers: linux/syscall.h, linux/socket.h


KEY DIFFERENCE:
For MacOS, one clear SDK, built by Apple, comes with Xcode and command line tools, 
For linux, headers + libc + tools installed separately, comes from distro, more flexible
(linux has -I, -L, and -Wl for passing to linker libraries)
(macos has it all under the sdk)
(-framework COCOA for adding frameworks)
(NOTE: rpath is the RUNTIME LOADER, not linker, -L is linker)
DYNAMIC LOADER / LOADER dyld on mac ld on linux!!!

-isysroot /MacOSX.sdk
-I/usr/include (where the headers live)

remember SDK doesn't hold all the files there, just has an organized view to them, making it easy to find.
versioned too, so can make code for older version of mac when not backwards compatible.
JUST CD INTO THE SDK TO SEE HOW ITS FORMATTED. ONLY USEFUL FOR COMPILE-TIME VIEW




clang -cc1 version 21.0.0 (clang-2100.0.123.102) default target arm64-apple-darwin25.4.0
ignoring nonexistent directory "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/local/include"
ignoring nonexistent directory "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/Library/Frameworks"
#include "..." search starts here:
#include <...> search starts here:
 /usr/local/include
 /Library/Developer/CommandLineTools/usr/lib/clang/21/include
 /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
 /Library/Developer/CommandLineTools/usr/include
 /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks (framework directory)
 /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/SubFrameworks (framework directory)
End of search list.

 "/Library/Developer/CommandLineTools/usr/bin/ld" -demangle -lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib -no_deduplicate -dynamic -arch arm64 -platform_version macos 26.0.0 26.4 -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -mllvm -enable-linkonceodr-outlining -o a.out -L/usr/local/lib /var/folders/n1/0nnckscj36xfsjzb_6z3y0ym0000gn/T/main-061d40.o -lSystem /Library/Developer/CommandLineTools/usr/lib/clang/21/lib/darwin/libclang_rt.osx.a

 This is the linking step, as seen from /usr/bin/ld. says:
 1: create an executable titles a.out
 2: for arm64
 3: use the temporary object file
 4: link agains libSystem
 5: link against clang runtime support

what to note here:
-lSystem:
contains
libc
libm
pthread
system call wrappers
startup/runtime support

(printf, malloc, exit, all defined here)


what about libclang?
its clang's compiler runtime
provided helper routines for:
integer arithmetic helpers
sanitizer/runtime hooks
stack protection support
builtins
low-level compiler support functions

(similar to gcc's libgcc)



 Notes here:
 on macos, gcc doesnt actually use gcc, but rather apple's clang

 what was:
 crt1.o crti.o crtbegin.o main.o -lc -lgcc crtend.o crtn.o on linux's gcc
 main.o -lSystem libclang_rt.osx.a on Apple's clang



 Final notes on teh Loader:

 unlike with .a files, which are just copied in, dynamic libraries are not just tossed in, instead make note of it being
 needed:

 .dylib or .so files
 when you 
 -lSystem or -lc

linker does not copy over libc code.
instead in Mach-O stored load command
LC_LOAD_DYLIB

can see with 

otool -L a.out, ldd a.out
(/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1356.0.0))

On code start:

Brief aside on what this all means:

for proceses (add to notes)

In ELF file, has sections like:
.text
.data
.rodata
.bss
(used by compiler and linker)

The kernel uses the program headers, like PT_LOAD, which describe a part of memory to map in

for each PT_LOAD entry, Kernel:
1. creates teh VMA struct
2. sets perms
3. descided the file backing


loadable sections:

exe:
Segment 1 (r-x):
- .text
- .rodata (read-only data, constants string literals etc)

Segment 2 (rw-):
- .data
- .bss

so kernel just sees:
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

(no mention of sections)


each loadable segment has 2 sections:

filesz - size of the file
memsz - size of teh memory to load in
for .data, its file-backed, so kernel does:

mmap(file,offset,1000 bytes)

for bss however, not file backed:
filesz = 1000
memsz = 2000

(first 1000 bytes from file, rest from anonymous memory (zeroed))

so, it maps 1000 bytes from file, rest zeroed


Thus, after loading it into memory, Kernel looses track on which is which:
VMA structs only have:

start address
end address
permissions (r/w/x)
file backing (if any)
offset into file (if file-backed)

nothing about .data or .bss (in runtime, its just bytes in memory)



