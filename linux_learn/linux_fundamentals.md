# Linux Fundamentals


## File Commands

- ls  //lists all files in a directory
- ls -la  //lists all files, even the hidden ones .vimrc for example
- cd dir //changes cwd to dir
- cd //changes cwd to ~
- mkdir dir //creates new dir names dir in cwd
- rm file //removes file file from cwd
- rm -r dir //removes a dir
- rm -f file //forcefully removes a file
- rm -rf //forcefully removes a directory
- cp file1 file2 //copies files 1 to files 2
- cp -r dir1 dir2 //copies dir1 to dir2
- mv file1 file2 //moves the file1 to file2

### Tricky:
- ln -s file link //creates a symlink link to file
> What does that mean:  
> **Symbolic Link:** a special type of file that links to the location file, so when you write `cd link` it actually runs `cd file`

- stat file // prints out the metadata of the file to the stdout
- touch file //creates a file named file
- cat > file 
> What does this mean:  
> Usually, `cat` us used like `cat file` which will output the file to the stdout, or can be piped via `cat file | `
> In this case however, it will make it so anything you type in the terminal is auto redirected to the file

- more file // outputs the files
- head file //shows first 10 lines of teh file
- tail file //shows last 10 lines of the file

## Process Management

### What is a process?
A process is essentially a container for running a piece of software/program  
> **Program** = bytes on a disk (an executable file)  
> **Process** = that program in motion + all the other stuff needed to run it in motion

The parts of a process:

#### Its own Virtual Memory:
When a process is running, it has the illusion that is has access to limitless memory(from 0x0000 upward in memory)
**THAT SPACE CONTAINS:**  
- CODE (the machine instructions for that process)  
- Static Data (stuff like globals and constants for that program alone) (**REMEMBER**, to not want it have access to stuff from other programs)  
- Heap (grows as you malloc / allocate more space for that process)
- Stack (Contains Function call frames: return addresses, local variables, saved registers)

**WHY IS THIS MEMORY VIRTUAL**:
the process thinks its continuous, but in reality, the os keeps a table that maps the process's virtual memory addresses into actual memory addresses on the RAM. This way, the process can behave as it it were continuous when in reality
is is not.

#### CPU Execution State

> **NOTE** READ THE LINUX_PIPELINE.txt to learn more about the stack and instruction pointer!!!

When the OS switches between processes, it must keep track of where it must return to via the RSP and
what is it currently executing RIP

> **NOTE** while yes, the stack does contain a return address, call and ret are not the only time that discontinuities
occur!! thus, you have to save both RSP and the stack and RIP!!

exe:

suppose inside of foo, your instructions read

mov rax, [rbp-0x10]
add rax, 7
imul rax, rxb 

(note that by fetching the thing stored at that memory, your RIP will change to that memory, and so you will
have lost your current position in the code)
THE STACK DOESNT SOLVE THIS because you are not using call or ret here, just a simple memory read that will
make the chain of execution non-continuous!!!

THUS, you (as the process), must store:
1. THE RIP
2. THE RSP
3. Other Registers

(WAIT?!?! i thought the stack was part of ISA, meaning its tied to the hardware and process independant right?)
WRONG!!! because of the aformeantioned virtual memory, each process has its own stack!
infact, a process can have multiple threads each with their own stack


> **NOTE**
> Inside of a single process, there is a shared address space (virtual memory space) that is shared by all the threads
> BUT ALSO, a separate CPU register state where the RIP and RSP and stack are stored for each thread.



#### A Process ID and Metadata
Each process has a PID, or process ID that allows the computer to keep track of all of them

#### Open Handles to Resources
In linux, aside from your computer you can have many different devices
(essentially a wraper word for other files, drives, peripherals, all that stuff)

A program can often want to interact with these different devices, so it keeps a table of
file descriptors (because remember, everything is linux is treated as a file)

0 = stdout
1 = stdin
2 = stderr
but they can also refer to:
- files
- pipes
- terminals
- sockets (dont need to know the details)



### KEY DISTINCTION:

> **THREAD**
> a thread is just a continuous execution stream inside of a process (this is what has a stack and RIP and RSP)

> **PROCESS**
> memory address space + resource table


## File Permissions

## SSH

## Searching

### grep:

Loose overview:
grep "pattern" file.txt

grep "TODO" file1.txt file2.txt (SEARCH MULTIPLE FILES)
grep "TODO" * (checks all files in current dir)
grep -r "TODO" . (recursive check through directories)

grep -i (case-insensitive)
grep -n (show line number + file, often better than just l which shows filename)

grep -l "password" * (Shows only filenames matching)

grep -v "debug" app.log (KEY: shows lines NOT containing debug)
grep -c (counts matching lines)
grep -w "cat" file.txt (matches whole words, matches cat, but not category)
grep -x "cat" * (matches only lines that say cat and nothing else)

Useful combos:
grep -rin "error" .

grep -r "useState" --include="*.jsx" . (forces that files have certain extension)
grep -r "TODO" . --exclude-dir=node_modules (excludes certain directories)
grep -r "TODO" . --exclude="*.min.js" (excludes certain files)
(NO INCLUDE DIR OPTION) **NOTE** only works when looking at files/dirs! ls | grep doesnt work here

**NOW REGEX TIME!!!**

grep "^ERROR" app.log (means line must start with ERROR)

grep "done$" output.txt (means line must end with done)

grep "gr.y" file.txt (means matches a single character, can be gray, grey, doesnt matter)

grep -E "error|warning|failed" app.log (the | regex means can be either or)

grep -E "[0-9]+" file.txt (means can be any numbers from 0 to 9, + meaning 1 or more)

grep -E "[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}" file.txt
(matches email, kinda)

-F means LITERAL STRING, or FIXED STRING, WILL IGNORE THE . IN REGEX STUFF

**NOTE** GREP USES REGEX, NOT SHELL GLOBS!!!
"ssh*.service" doesnt work!! in regex it has different meaning




NOTE:
if you 
ls | grep -il"java" , it will return (standard input) since it doesnt have file-type context when piping

NOTE: the ls stuff will be interpreted as a series of paths,
ls | grep -ril "java", does work, and will search all of the directories outputted by ls recursively and list out
which files contain "java", case insensitive, in any form


rincvlxwE







### NOW ONTO FIND:
find searches for files/directories that matches the rules you specify

find WHERE CONDITIONS ACTION

find . -type f (searches for files in current directory, -type d for directories, -type l for symlinks)

-name "*.py" (searches for all .py files)

-size +100M (over 100 MB)
-mtime -1 (modified less than one day ago)

-o means OR

\(...\) used to group conditions together

-prune means do not search inside of matched directory
-print means print the matches

find . -type d \( \
  -name node_modules -o \
  -name .git -o \
  -name dist -o \
  -name build -o \
  -name .next -o \
  -name .venv -o \
  -name venv -o \
  -name __pycache__ -o \
  -name .pytest_cache -o \
  -name .mypy_cache \
\) -prune -print

-iname for case independent name, better if not sure

find /path/to/directory -maxdepth 1 -type f

maxdepth useful



## System Info

### ps
ps:
lists the current processes that are active on your computer.



## Compression
Compression + Combining happens via 2 methods:
Combine files = tar, compress = gzip


### tar
tar: tape archive, bundles multiple files + folders into one file, preserving directory perms and timestamps

tar -xzf filename.tar.gz //order is what you want to do (X) filter (gzip compression), and file

Action Tasks:
-c create archive, used to create new archives of files (combine them)
-x extract archive, used to create the normal directory structure from the tar.gz
-t lists the contents inside the file without extracting

File flag:
-f tells tar which file to use

Compression Flags
-z uses gzip, either for compression or decompression
-j bzip
-J xz compression (LZMA compression)

-v verbose

tar -czf archive.tar.gz /folder  (MUST BE CLEAR WITH THE EXTENSIONS, AS THEY ARE CONVENTION)
(can add more than just folder, and files are added in the order specified by the command list arg)

-r is like c, but new entries appended to the archive (uncompresssed archives only)
-u like r, but only adds it modification date newer than corresponding entry in the archive
(results is called TARBALL)

**KEY**
-C means change directory, which changes where it extracts files

-f means read the archive from or write the archive to the specified file.(changes use based on c vs x)
-f on c means write the new archive to whatever is after f
-f on x means read from whatever archive is listed after f
If NO -f SPECIFIED, -c WILL DEFAULT TO STDOUT (as in it is written to stdout, by default the terminal)
AND -x WILL DEFAULT TO STDIN (as in will decompress whatever file is in STDIN)
(RECALL -C IS CHANGE DIR, SO IT ALLWAYS OUTPUTS WHERE IS IT IS BY DEFAULT)

WILL DEFAULT TO STDOUT for -c AND STDIN for -x
cat archive.tar | tar -x works for example.

**A tar archive (tar) stores:**

filenames
directory structure
file contents (bytes)
basic metadata (permissions, timestamps, owner IDs)
(this makes it OS-indepentant, ie can compress and create on linux and decompress on mac)

Tarballs = Universal , tar is OS-based but abstracts away OS specific details

to work, it uses syscalls like open() read() write() mkdir() chmod() when on POSIX compliant systems like mac and linux


exe:

tar -<options> -f <archive-name> <files...>

tar -xzf filename.tar.gz 

tar -xzf backup.tar.gz -C /tmp

tar -cf archive.tar file1 file2 dir/

tar -czf archive.tar.gz /folder 

--strip=1 means remove the first directory element from the directory:

exe:
/dir
    /subdir1
        file1
    /subdir2
        file2

unzips to 
/subdir1
    file1
/subdir2
    file2









### GZIP
gzip: compresses a single file using the DEFLATE algorithm

gzip file //comrpesses it via gzip
gzip -d file //de-comrpesses the file
gzip -k file //**CRUTIAL** ZIP DELETES THE ORIGIONAL FILE, SO THIS MAKES SURE TO KEEP IT
gzip -c file //does not modify file, instead writes compressed version to stdout

gzip -c file >> file.gz  #THIS IS THE MAIN WAY TO CHANGE WHERE IT GOES, AS GZIP COMPRESSES IN PLACE
or -dc

-v verbose
-r recursive compression (USEFUL) (it compresses the files in a directory one by one)




## Network

### curl
curl:
curl, by default 
curl https://example.com
will
1: download the content (holds it in memory as a stream)
2: writes it to stdout
3: does not save it to a file


-o means write output to a file (like how -O which means to download as per that file name)
-O
-o- means to write to stdout instead (same as default, but more explicit)
-f fail silently on http errors (exe 404, will not output the html error page, instead exit with error)
-s silent mode (do not output what you are doing to stdout)
-S show error (in the even of an error, print it to stdout)
-L follow redirects (301, 302 re-direct)







## Installation

## Shortcuts




## Core System

### sudo

### systemctl
main way to interact with systemd (look at systemd.md for more info)

sudo systemctl poweroff -i
-i means ignore inhibitors, so it can shutdown even with others trying to block the shutdown, like other connected users





### apt
primary debian package manager (look at packages_learn.md for more info)

### dpkg
underlying debian package installer (look at packages_learn.md for more info)


## Generic

### man
- man (cmd) // checks the manual for how to use a command


### ls

ls to show files

-l is long format, for more info
-d means dont look inside directories
-i means show inodes
-h is human readable

73029703 -rw-r--r--@ 1 stefancelmare staff 26M Jul 30 23:25 file.txt

breakdown:
1. inode number
2. perms, meaning file, and the other stuff
3. link count
4. owner
5. group (staff is default group for regular users on mac)
6. 26M is size, or 26 megabytes
7. Last modified date
8. filename

whenever it shows l in the description, its a symlink


### single quotes, double quotes, and commmand substitution
- "" vs '' vs $() and how they interplay in commands like

Double QUotes:
they preserve spaces and most special characters, but still allow variable expansion and command substitution.

variable expansion:
name="Alice"
echo "Hello $name" outputs Hello Alice

command substitiion:
echo "Today is $(date)"
the $(date) gets executed

why quotes matter:

file="my file.txt" (NO SPACES)
rm $file becomes rm my file.txt, which is treated as 2 different arguments, with quotes, it becomes a single argument

rm "$file" will enforce that it is one single string, and delete "my file.txt", not the 2 files "my" and "file.txt"

**KEY**
file=my file.txt is parsed as:

file=my (string assignment, as everything is strings in the shell)
file.txt is to be executed!

**GLOBBING IS NOT PERFORMED INSIDE QUOTES**




Single Quotes:

everything is treated literally

name="Alice"
echo '$name' outputs $name

likewise
echo '$(date)' outputs $(date)


more on command substitution:
It 
1: runs a command and
2: replaces the entire expression with the commands output (KEY)

echo $(date)
might become:

echo Thu Jun 4 12:34:56 HST 2026

which then prints Thu Jun 4 12:34:56 HST 2026


another example:
files=$(ls)
files stores the output of ls

you can nest it via:

echo "$(whoami) is in $(pwd)"
george is in /home/george



















### eval vs exec
eval '$(/opt/homebrew/bin/brew shellenv)'

eval vs exec:

exec: replaces the terminal (the process as a whole) with a new process as specified by the ELF/Mach-O of another file. eval just evaluates a command
(note: if you specify a command via /, it will not execute it as a builtin, and instead create a new process for it)
echo is builtin, /bin/echo is not
NOTE: builtin zshrc in /etc/zshrc

### ln
ln: used to create hard links or symlinks in linux
ln source_file target_file

2 types of links: hard links and symlinks
hard link: the dentry directly references the inode number of the file you are referencing
(like . or ../file)

symlink: instead, a symlink is a file whose contents are a string representing the path to the file you
are linking. (path is stagnant, so moving file messes up the symlink)
(use man ln for more details)

-s creates symbolic link
-f if target exists, unlink it so link may occur. Will generally just remove the target
to let the linking work

-i writes error in stderr if target already exists
-h/-n if target is a symbolic link, do not follow it. useful when replacing a symlink that may point to a directory **KEY WHEN TARGET IS A SYMLINK TO DIR**

exe:
given dir/
and linkdir -> dir

if you run:

ln -s file linkdir, it will by default follow linkdir to dir/, and then create

dir/file -> file, not linkdir -> file!!! need ln -snf file linkdir to work!

**NOTE** run ls -ld outside of the directory, as if you cd'ed into it, it already evaluated
the symlink so ld -ld gives info on current dir.
ALSO, DO NOT PUT A TRAILING / HERE, SINCE IT WILL EVALUATE IT!!

TRAILING SLASH, IN THE WAY THAT THE KERNEL PARSES IT, MAKES IT EVALUATE THE LAST TOKEN OF THE PATH:

/usr/bin
has tokens "usr", "bin", which does not force shell to evaluate the last token beyond the fact
that is is a dentry. whereas /usr/bin/ forces it to resolve the last token, which will often
run the link anyways, and give you inaccurate results. (READ FILESYSTEM INTRO!!!)

loosely:
/usr/bin is the path to the file representing the directory(directory file)
/usr/bin/ is the path to get into the directory, forcing you to evaluate the bin dentry, which may be a symlink








### cat
cat: cat copies its input to its output.
if you type cat, and then type hello, it will echo back hello
(because reading from stdin(keyboard) and writing to stdout(terminal))

<<'EOF' is the here document, meaning pretend everything up to here is a document untill you reach a line containing EOF

so 
cat <<'EOF'
hello
world
EOF

is the same as cat from a document whose input was "hello \n world\n"
(EOF here is just a marker, it could be any other string you desire, with no quotes around it)

so cat [file1] [file2] means read those files and output then to stdout or whatever is your output

thus:

cat > ~/.ssh/.gitignore <<'EOF' redirects teh output to that file, its like an easy way to do it without vim!


### shell globs
shell globs are patterns that the shell expands into matching filenames
exe, given files:
apple.txt
apple.log
banana.txt
notes.md

"mv *pattern* destination/" is expanded by teh shell into "mv apple.txt apple.log destination/"

Common Shell globs:


'*' meaning 0 or more characters:
*.txt -> apple.txt banana.txt

'?' meaning exactly one character
file?.txt

[abc] meaning one character from a set
file[12].txt -> file1.txt file2.txt (**KEY**: if only file1 exists, no file2, it will just match the files that DO exist)

[a-z] meaning one character in a range
file[1-9] -> file1 file2 ... file 9 **KEY2** it matches characters, meaning fileo.txt is perfectly valid, even if o is not a number!

[!abc] meaning one character NOT in a range
file[!0].txt -> file(some character that is not 0).txt

More common ones
[[:digit:]] one digit
[[:alpha:]] one letter
[[:alnum::]] one letter or number
[[:lower:]] one lowercase letter
[[:upper:]] one uppercase letter
[[:space:]] one whitespace

**NOTE** the only mutli-character symbol for globs is *, which can be used rather liberally

```bash
report*2025*.csv
```
matches
report2025.csv
report_final_2025.csv
report-v2-2025-backup.csv

Double Star:

"results/**/*.npz" also matches and works, which matches to directories
IT MEANS MATCH 0 OR MORE DIRECTORIES RECURSIVELY
results/file.npz                  ✅
results/run1/file.npz             ✅
results/run1/epoch10/file.npz     ✅
results/a/b/c/d/file.npz          ✅

Single Star:

results/*/*.npz also works, but it enforces it must be only 1 directory.


KEY: These shell globs will be expanded by the shell, so if you dont want that, better to use them in quotes.
When inside quotes, the shell glob will be passed as is to the executable, not expanding it, letting it use it as
a argument.



### REGEX


## tree

tree displays a directory by recursing through it and printing ascii to stdout of its structure

## Shell History
CTRL-r lets you see shell history, displays: bck-i-search or (reverse-i-search)
typing "scp" will query bash history for scp commands
CTRL-r to iterate through results
CTRL-c to exit search