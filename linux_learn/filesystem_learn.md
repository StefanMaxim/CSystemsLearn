# Filesystem Organization

Linux follows the Filesystem Heirarchy Standard, which is built on 3 axes:



1: What is that file's role:

- Programs (binary executables)
- Configurations (how programs behave)
- Data (what programs run on)
- State (runtime-changing information like logs)

2: is it static or variable:
- Static = doesnt change unless you reinstall/update
- Variable = changes during runtime

3: who owns it
- System(OS) (installed by package manager)
- Admin(you) (installed manually)
- User (personal files)


## Master Map:

System Binaries = /bin or /usr/bin
Local/Admin Binaries = /usr/local or /opt
Config = /etc
Variable data = /var
User Data = /home
Runtime/System Internals = /proc or /sys or /run

## Layer 1: Root
root, or /, is both a directory and the namespace of the entire OS
(can access all files, processes, hardware, and even memory)

## Layer 2: Programs
/bin and /sbin hold the minimal, and essential system for the OS to boot and recover
(NO DEPENDENCIES ON OPTIONAL COMPONENTS)

/boot: holds the booting files like the kernel and stuff (only needed for boot, after start mostly irrelevant for day to day)
exe: /boot/vmlinuz

/bin:
Contains binary commands for use by all users (on most Linux systems, this directory is a shortcut to /usr/bin)

/sbin:
Contains more system-wide binaries, like nginx and fsck for networking/managing users 


exe:
/bin/bash
/bin/ls
/sbin/init


Asside on Minimality:
Absolute minimum:
kernel, in /boot/linuz
and an init process, PID 1 (init is a way to initialize processes)
(thats enough to boot into something, but pretty useless)

(READ SYSTEMD.MD FOR WHAT INIT PROCESS AND ALL THAT STUFF MEANS)

More reasonable minimum:
shell, (bash/sh) (shell = way to run commands)
basic utilities (coreutils) (coreutils = ways to manipulate files)
libraries /lib (holds required libraries)
init system: systemd
/etc configs



### /usr
/usr is the main system software tree
**NOTE** FOR MOST LINUX SYSTEM (USR-MERGE), sbin and bin are shortcuts to /usr/bin and /usr/sbin, like in Arch or RedHat linux, and UBUNTU
but on mac they are different (more on that later)

contains all non-essential, but standard OS software
managed by package managers (intended to be read-only in normal runtimes)
NOTE: package != executables
package managers handle packages:
exe: coreutils package has ls in it
/sbin/fsck in some other package
(however, only the packages installed by apt can be removed by it)

/usr/bin holds programs intended for normal users
exe:
/usr/bin/ls
/usr/bin/git
/usr/bin/python3
/usr/bin/vim



/usr/sbin holds system services / binary commands aka commands used for system administration
(often require sudo)
/usr/sbin/useradd
/usr/sbin/nginx
/usr/sbin/fsck
(less everyday tools like git and python and more system-wide things like network setup and user management)



(also USR-MERGED!, all libs /lib /lib64 /lib32)
/usr/lib stores libraries, which are .so/.a files or things that you can add to executables without having to re-type code
(the .a and .so files from maven_learn)
library = pre-written code that other programs can use (NOTE: /lib CONTAINS BOTH STATIC AND DYNAMICALLY LINKED LIBRARIES)
NOTE: /lib vs /lib64, lib64 used to store 64 bit library files whiel .lib for 32 on some systems, on others lib
stores both and /lib64 is just empty dir


2 main types:

shared libraries: .so
used at runtime, and dynamically loaded in

exe:
/usr/lib/x86_64-linux-gnu/libc.so.6 (standard c library, used by almost every program)

static libraries, or .a
used in compile time, code copied into executable

exe: use ldd to see which shared libraries a command is using

ldd /usr/bin/ls

otool -L /bin/ls (since again bin is often the same as /usr/bin nowadays)
(will give list of dylib or .so files its attached with)



/usr/share architecture independent data
means not tied to architecture type (x86_64 or ARM)
just data, not compiled
things programs use, not execute:
/usr/share/man        # manuals
/usr/share/icons      # icons
/usr/share/fonts      # fonts
/usr/share/doc        # documentation
/usr/share/locale     # translations


exe:
/usr/bin/git
/usr/bin/python3 (not needed for boot, but useful)
/usr/share/X11 (the windows server, at least part of it)



NOTES ON PACKAGE MANAGERS:

when you type: sudo apt install curl

curl, in this case, is a package, specifically a .deb for debian systems:
This is just an archive file, consisting of:
binaries, libraries, headers, install scripts, configs, docs

HOWEVER: install script is NOT for moving the packages to their respective spots in memory:
inside package, files are named with their indended location relative to the package:
exe:
user/bin/curl
usr/lib/libcurl.so
/usr/share/docs/curldocs

when installing, package manager just takes those files, and maps then to the root directory,
instead of the package:
usr/bin/curl -> /usr/bin/curl etc

BUT HOW DOES IT KNOW HOW TO DELETE PACKAGES:
inside, in var/lib/dpkg/info/curl.list
(apt calls dpkg under the hood)

What this does it it stores a list of all the files associated with the curl command:
/usr/bin/curl
/usr/share/docs/curldocs
/usr/share/man/man1/curl1.1.gz

and when uninstalling, it just goes through the list and removes all the files associated with the package in question


### /usr/local
This is executables installed by the admin, not the package manager or the OS on install
Structure mirrors /usr:
/usr/local/bin
/usr/local/lib
... (just where you put your own executables)

/usr/lib/libexec stores private, low-devel demons and system binaries, like the java_home on mac for changing 
java version


### /opt
Meant for isolated, third party software
(generally, self-contained)
not integrated into system heirarchy

/opt/maven
/opt/intellij


## Layer 3: Configuration (Control Plane)

/etc
contains only configs, ie files telling programs how to behave:

/etc/ssh/sshd_config
/etc/nginx/nginx.conf
/etc/environment

**NOTE** config here is global, user-dependent config goes elsewhere (usually a hidden file .config in userspace)


## Layer 4: Data and State
/var
meant for data that changes during regular operations:

logs:
/var/log/syslog
/var/log/nginx/access.log

or persistent app data:
/var/lib/mysql
/var/lib/docker

or even the cache:
/var/cache/apt


### Temporary Space
/temp
used for storing temporary, and non-persistent info
may be wiped at any time

### Runtime States
/run stores runtime only states
exe: PIDs, Sockets, Locks, etc (socket = networking CS180 notes, Lock = for syncronization, CS180 notes)

## Layer 5: User Domain

/home

lets each user get an isolated space:
/home/alice
/home/bob

INSIDE HOME DIRECTORY:
configs:
/home/alice/.config/
/home/alice/.bashrc 
(~/ is the same as home/<USER>/)

Or data:
~/projects
~/Downloads
~/Documents

## Layer 6: Kernel Interface (Confusing)
Here, they are not files per se, so much as interfaces to the kernel
(BEING AN OS, EVERYTHING IS A FILE, BUT THESE FILES MEAN SOMETHING MORE)

/proc
Virtual filesystem exposing kernel states:
/proc/cpuinfo
/proc/meminfo
/proc/<pid>/

/sys
Device and kernel control
(structured interface for hardware tuning)

/dev
Holds devices as files

(LINUX TREATES EVERYTHING AS A FILE INTERFACE)
/dev/sda     # disk
/dev/null    # sink
/dev/tty     # terminal




## Layer 7: Mount Points
/mnt
used for temporary mounts

/media
used for auto-mounted devices


## All together:

Static: /usr, /bin, /lib
Variable: /var /tmp /run

System: /usr /bin
Admin: /usr/local /opt
User: /home

Code: /usr /opt /usr/local
Config: /etc
Data: /var /home
Runtime: /run /proc






# MacOS Filesystem

Linux is Filesystem Heirarchy Standard
(binaries here, configs there, data there, and temp stuff there)

Apple still has many of those core Unix stuff, but much more apple-focused:
application bundles, library domains, APFS system/data separation, and stronger protection of system paths


KEY NOTES:

Applications: these are full directories trees that the finder considers as a single unit, one application
(.app file extension)

Resources Split by Domain:
System-Wide, local machine, network, and per-user (unlike linux's 3 focuses)

KEY NOTE: the core of the system is MUCH more isolated than linux, many core syslibs being read-only



## LOOSE HIGH-LEVEL ROADMAP


User home directory:
Linux: /home/ (home often empty on mac)
Macos: Users/

Installed GUI apps:
Linux: distro-specific, or /opt
Mac: /Applications

OS Files:
Linux:
/usr /bin /lib /etc
Macos: /System or /System/Library (protected)

Machine-Wide App Supporting Config:
Linux: /etc, /usr/share /var/lib
Mac: /Library

Per-user App Support:
Linux: .files, ~/.config ~/.local 
Macos: ~/Library

Mount Points:
Linux: /mnt /media
Macos: /Volumes

Local CLI/package manager tools:
Linux: /usr/local /opt
Macos: /usr/local /opt (this is the same)

Temp/runtime unix paths:
Linux: /temp /var /run
Macos: /temp /var /private (but with apple protection)

## Dive into /Applications

On Linux: Application = binaries + libraries + configs spread across FHS 
On Mac: bundles into a single application, .app

exe:
/Applications/Safari.app
/Applications/Xcode.app
/Applications/Visual Studio Code.app

Application are directories
Pkgs are files, basically installers:
They usually:
Copy apps into /Applications
Install files into /Library
Install command-line tools
Run setup scripts

dmg is disk image, file mounted by macos to appear like a temp drive
DMG usually contains TheAppInQuestion.app and Applications (shortcut)
you just copy the app into the Applications folder. On mac, applications are fully
separate.
pkgs are better when you need it to do stuff outside the app, like:
Installing files into /Library
Installing command-line tools into /usr/local/bin or similar
Installing frameworks, libraries, fonts, or drivers
Setting up launch daemons/services
Creating system configuration files
Running pre-install or post-install scripts

NOTE: DMG and PKG are not competitors, as DMG is just the payload container, 
it can contain just the .app and shortcut, or a pkg inside it.
Useful bc an app is a directory, so sending it alone is bad bc files could be altered, etc
instead, bundle it into a disk image.
Once done so, it can show a custom image letting the user simple drag the app
into the applications folder

Linux uses instead .tar.gz or .zip files for the dmg, and .deb or .rpm for 
handling packages.

PACKAGE:
A software package usually contains:

The program itself (executables)
Libraries it depends on
Configuration files
Metadata (name, version, author)
Instructions for installation/removal.

without it, you would have to manually copy over the files where they belong, this
is easier. (READ ABOVE, IT TELLS YOU HOW A PACKAGE KEEPS TRACK OF WHERE INSTALLATIONS GO)

(READ PACKAGES LEARN TO SEE HOW TO DO THIS)



**NOTE**
Application are not a single file, but have a directory structure like so:
MyApp.app/
  Contents/
    MacOS/
    Resources/
    Info.plist
    Frameworks/

On Linux, usually the executable lands in /usr/bin, and the rest is scattered throuought the filesystem
On Mac, apps are self-contained
This means:
app code, resources, icons, embedded frameworks, and metadata often ship together

That makes installing an app very easy, just drag it into the /Applications folder
It may never place a formal user-facing binary into /usr/bin at all

What is a framework:

At its core, a framework is:
1. A shared library, usually a dynamically linked on (.dylib)
2. Plus everything needed to use that library
unlike a simple .dylib, .frameworks have a directory structure:

MyFramework.framework/
├── Headers/
├── Modules/
├── Resources/
├── MyFramework (the binary)
└── Info.plist

Key Components:

Binary: the compiled code you link against
Headers: public interfaces (for objective c/ APIs)
Modules: metadata for Swift and modern import systems
Resouces: Images, storyboards, localization files, etc
Info.plist: metadata about teh framework (size, perms, etc)

Good for code reuse, modularity, encapsulation, and versioning (supports multiple versions in a single framework)

Dynamic vs Static Frameworks:
Dynamic = loaded at runtime + shared across files
Static = compiled into teh file

System Frameworks:
provide by Apple
/System/Library/Frameworks
exe:
AppKit for macos UIs
Foundation for basic data types, collections, etc
CoreData for object graph and persistence, useful for storage

How to use:
in Swift:
import Foundation
in Objective-C:
#include <Foundation/Foundation.h> //tells system to use teh frameworks exposed APIs in headers


Why is this better:
(refer to linking_intro.md for backround on headers, linking and the like)

it basically lets you have the header, the linked library, and all its resources in a single place
for c on linux, its a whole fiasco for linking when you import, but for mac its streamlined

in linux, given some .so file, there is no really good way to know how it works, making the framework model
much more appealing


## Now /System and /System/Library

BRIEF ASIDE: when deleting apps, be sure to check /Library and 
~/Library (root = global configs, system-wiude, home = for particular user) look for Application Support
Caches, and Preferences, Saved Application State, Logs, 
~/Library/Cookies
~/Library/HTTPStorages
~/Library/WebKit
~/Library/Containers
~/Library/Group Containers

Login items:
~/Library/LaunchAgents
/Library/LaunchAgents
/Library/LaunchDaemons

(lot of places to check)