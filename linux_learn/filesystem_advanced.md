EXPLAIN THESE:
Firstly, they are NOT files
files meaning bytes stored in blocks, described by an inode (READ INODE NOTES)
in /dev /proc and /sys, the kernel fabricates their contents on demand
each entry backed by kernel code + data structures, not stored bytes

/dev is device nodes (special inodes -> kernel drivers)
unlike disk-backed inodes (disk contains inode table, kernel read inode from disk into inode struct, points to data block)
can also create special inodes that live ONLY in memory, aka synthetic inodes

How does this work:
the system you know, path -> inode -> blocks of data (from notes) is good for simple filesystem, but linux uses
virtual filesystem (VFS) that allows for /proc and /sys to work

3 Core Objects:
inode -> what this object is
metadata (perms, type), operations (what happens when you read/write)

dentry (directory entry)->this name refers to that inode
maps a filename "meminfo" -> inode
forms the directory tree

file -> this specific open instance
created when you call open()
tracks offset, flags, etc

What actually happens on file open:
exe: cat /home/user/file.txt

start from /, known to kernel
look up "home" in that directory
finds a dentry, which points to an inode
loads the inode (from disk if needed)
Repeat for user, then file.txt

while it is true that directories are stored as data blocks (the string -> inode mapping), kernel
does NOT require that.
each filesystem (ext4, procfs, sysfs, devtmpfs) implements functions like
lookup(parent_inode, name) -> dentry
read(file, buffer, size)
write(file, buffer, size)

kernel does not directly read directory blocks from disk, instead it asks teh filesystem what inode corresponds to 
this name.

what is a filesystem: its an abstraction used by the kernel. Its essentially a kernel module (or code) that implements
a set of operations (lookup,read,write,etc) over some backing stores

code: implements operations like lookup,read,write
data:
disk blocks (ext4), kernel memory (/proc),device drivers (/dev), network (NFS)
must have a clear way to mount it mount() -> returns superblock, representing the whole fielsytem instance
a root inode: starting point
operations (lookup, read, write, readdir lists directory entries)

note: /dev/sda1 is NOT a filesystem:
what it is literally(3 things):

1: the path, /dev/sda1, VFS objects
/ = root dentry
dev = dentry
sda1-> dentry
each dentry pointing to an inode

2: the inode for /dev/sda, created by devtmpfs/udev
contains file type (block device)
permissions
device number (THIS IS KEY)

dev_t rdev = MKDEV(8,1)



# Filesystem from Scratch:
PROJECT:
For example, create:

disk.img

and implement something like:

fs_format()
fs_create()
fs_open()
fs_read()
fs_write()
fs_seek()
fs_close()
fs_unlink()
fs_rename()

with a layout like:

+----------------------+
| Superblock           |
+----------------------+
| Free block bitmap    |
+----------------------+
| Inode bitmap         |
+----------------------+
| Inode table          |
+----------------------+
| Data blocks          |
|                      |
|                      |
+----------------------+

Start single-threaded.

Then make it thread-safe using:

pthread_mutex_t

and think carefully about what must be locked.

That combines three major 213 subjects:

File systems
      +
Memory/data structures
      +
Synchronization

This would prepare you extremely well for 15-410, because you would enter OS already understanding the basic filesystem abstraction instead of first encountering it there.