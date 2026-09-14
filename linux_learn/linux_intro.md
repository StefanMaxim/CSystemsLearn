# Linux 

Linux is realy GNU + Linux

Kernel = /boot/vmlinuz
userland stuff = part GNU + other tools

Main GNU tools:

1. Core Command-Line utilities:
ls, cp, grep, etc

2. The Shell
/bin/bash

3. Compiler Toolchain
GNU Compliler Collection (gcc)
GNU bin utils (ls, as)
GNU Debugger (gdb)

4. Core Libraries
GNU C Library (glibc, the standard c library used by most programs)
-lc or #include stdio.h,stdlib.h, math.h,string.h, errno.h, 

5. System Utilities and core utils:
coreutils, findutils, diffutils, tar, etc


What is NOT GNU:
init system, systemd (not made by GNU, init system/service management)

device management, /dev for device files/interfaces
use udev(now part of systemd)

log-in/session management:
login/agentty (from util-linux)

Filesystem+Disk Utilities:
mount,fdisk (from util-linux)

Networking Stack:;
iproute2, for ip command
NetworkManager, for DHCP and the like
DHCP client via dhclient

package manager: apt dnf packman

Graphical System:

Display Server:
Xorg for X11
Wayland alternative

Desktop Environment:
GNOME/KDE

Bootloader, OpenSSL, and much more...

All of these are compiled by usually your operating system





Aside on SystemD:
Boot Order:

1. Firmware (BIOS/UEFI) (usually burned into rom, low level for hardware control)
2. Bootloader (GRUB)
3. Kernel (linux kernel)
4. Init System (systemd)
5. Everything else (services, login, networks, etc)

## Shell vs Terminal

Shell = zsh or bash, a program that lets you interpret the messages you write

Terminal = a program that allows you to interact with a computer using text.

Order:
You
  │
  ▼
Terminal
  │
  ▼
Shell (e.g., bash or zsh)
  │
  ▼
Operating System
  │
  ▼
Output
  │
  ▼
Terminal displays the result