## after created the 2 files, stlength.c and streverse.c

make static library

gcc -c stlength.c streverse.c
ar rcs libstutils.a stlength.o streverse.o

dynamic library:
gcc -c -fPIC stlength.c streverse.c
gcc -shared stlength.o streverse.o -o libstutils2.so


Key note here:
streverse uses #import "stlength.h", but it wasnt provided to it allways

right now, you have the libraries, but they are clunky, so there must be a clean API header the users can 
use to include it, and let the OS handle the rest if its in the right path or tell the linker to do it itself

Library Internals:
stlength.c
streverse.c 
are meant to be hidden

Exposed API Header:
mylib.h
mylib2.h
is meant to be seen.

exe:

stutils.h (static one)

#ifndef STUTILS_H
#define STUTILS_H

#include "stlength.h"

int stlenth(char* str);

char* streverse(char* str);

#endif



## Next steps: organizing the library
While library is literally just the .so file, it is often packaged alongside its other code objects
(kinda like a framework on mac):

1. Public Interface (the header files, what users should include)
2. Implementation (/src, the .c and internal .h files)
3. Build Artifacts (the .so and .a files )

good library format:

mylib/
├── include/
│   └── mylib.h        ← PUBLIC API (what users include)
│
├── src/
│   ├── stlength.c
│   ├── streverse.c
│   └── internal.h     ← optional private helpers
│
├── build/
│   ├── libmylib.a
│   └── libmylib.so
│
├── Makefile
└── README.md

(this is just good developer practices, in reality):

.a and .so files go in /usr/local/lib
.h goes it /usr/local/include

On install, just create makefile rules for getting all the files where they gotta go:

install:
	mkdir -p /usr/local/include
	mkdir -p /usr/local/lib
	cp include/mylib.h /usr/local/include/
	cp libmylib.so /usr/local/lib/
	cp libmylib.a /usr/local/lib/
	ldconfig //this wil update the loader config to see the new libraries



FOR MACOS:
a littel different:

for static libraries, exactly the same, down to teh commands

for dynamic, mac uses .dylib, not .so
gcc -dynamiclib (instead fo shared), rest is exactly the same

remember to tell MACOS where it is:
DYLD_LIBRARY_PATH=.


makefiles must include:
build
clean
test (optional but very useful)
install
uninstall


HOW TO TURN INTO A FULLY-FLEDGED PACKAGE:















## Notes on Header files:

but all the header files are c headers, no? so what if another program want to use them?

header files are moreso a contract, rather than implementation
it just says that there exists some function that behaves this way
(this type of input, this type of output), actual implementaion in some .so or .a file

Other langauges like Python have FFI, or Foreign Function Interface, which lets you use c functions directly:

import ctypes
libc = ctypes.CDLL("libc.so.6")
libc.printf(b"Hello from C via Python!\n")

works bc of ABI, makes inter-language communication clear
(functions ahve specific symbol names exe:printf, and clear calling conventions)

headers is guidance for a correct call, just saying the way the function is formatted:
include function prototype:
name, parameters, return type (Signature = how compiler sees it, doesnt have return type)

prototype used to validate your calls
signature (in c, uses only function name) = used to differentiate between functions by compiler

No header file = compiler will try to guess if that function is correct => corrupting registers or bad preformance


EXE:
there is a great library on device, with a .h file in /usr/local/include and a implementaiton in /usr/local/lib, but
I am writing in python/java. how to I access it?

IDEA: Dont import the header! just call teh compiled .so using the ABI and re-create teh header's information
in your own language!

Python:

suppose your header is:
int add (int a, int b);

Step 1: load the shared library:

import ctypes
lib = ctypes.CDLL("/usr/local/lib/libmylib.so")

Step 2: re-create the header:
lib.add.argtypes = [ctypes.c_int, ctypes.c_int]
lib.add.restype = ctypes.c_int

Step 3: Call it
result = lib.add(2, 3)
print(result)  # 5



Java:
java is stricter, and uses something called JNI (Java Native Interface)

given int add (int a, int b);

Step 1: declare a native java method:

public class MyLib {
    static {
        System.load("/usr/local/lib/libmylib.so");
    }

    public native int add(int a, int b);
}

Step 2: java requires a thin wrapper around the method:


#include <jni.h>

JNIEXPORT jint JNICALL Java_MyLib_add(JNIEnv *env, jobject obj, jint a, jint b) {
    return add(a, b); // call your real C function
}

Step 3: call from Java

MyLib lib = new MyLib();
System.out.println(lib.add(2, 3));


For most tasks like this, Java did the hard work:
Your Java code
   ↓
Java standard library (Java)
   ↓
JNI layer (C/C++)
   ↓
libc / system calls (C ABI)
   ↓
Kernel