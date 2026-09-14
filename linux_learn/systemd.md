NOTE: init systems like systemd do NOT start processes
Process creation is done by the kernel in responce to systemcalls like execve() and fork()



instead, is it the first process to be started (called init) that manages the system:

1: Bootstraps the system, meaning it lauches essential services like networking and login on start
2: Manages services (service = backround, long running process hosted on server, by daemon, and handled by init system) can start, stop, and manage
3: Handles dependencies by ensuring that services start in the correct order,
like how webserver needs networking (dependencies as in needs them running to run something else)
4: Managing the running system: handles task scheduling, devices, and the like, but ess


exe:
say you install nginx on Linux and want to run a webserver with it

without systemd:
1: mmust manually start nginx every time you boot
2: make sure neetworking is ready first before running it
3: restart it yourself if it crashes
thats tedious 

instead:
sudo systemctl, which interacts with systemd to make it run by itself

for something like nginx:
FILE LIKE THIS:
[Unit]
Description=NGINX Web Server #Human-readable description of the service
After=network.target #This means dont start NGINX untill AFTER network.target has been initialized

[Service]
ExecStart=/usr/sbin/nginx #Command systemd should run to start the service, interally systemd asks kernel to create new process using fork() and execve()
Restart=always #if nginx exits unexpectedly, restart it automatically

[Install]
WantedBy=multi-user.target #Whenever system reaches the normal multi-user operating state, 
THIS SERVICE SHOULD ALSO BE STARTED. REACHING TARGET = SYSTEMD IS WORKING TO MATCH THE TARGET BY
STARTING REQUIRED SERVICES.

exe: mutli-user.target includes:
sshd.service
nginx.service
mysql.service


**TARGET** a target in systemd is a particular system state or milestone
most common is **multi-user.target**, which means "System should have booted enough that multiple users can log in and normal system services should be running"
Boot begins
      │
      ▼
Basic system initialized
      │
      ▼
Networking available
      │
      ▼
multi-user.target
      │
      ├── SSH starts
      ├── NGINX starts
      ├── MySQL starts
      └── Cron starts


**SERVICE FILES**
this information, about the
[Unit]
[Service]
[Install]
are part of the service files, like ssh.service

They are found in 2 primary locations:

1. /usr/lib/systemd/system/
OR
2. /lib/systemd/system/
and custom services live in
3. /etc/systemd/system/

can view with
**systemctl** cat nginx
OR
find file via systemctl status nginx
Loaded: loaded (/usr/lib/systemd/system/nginx.service; enabled)
and then cat that file or view it however

**NOTE** Due to Usr Merge, ITS THE SAME PLACE!!! there are now 2 places
/usr/lib/systemd and /etc/systemd, depending on internal vs external software.




## SERVICE FILE BREAKDOWN
SSH Actual .service file:

[Unit] # describes service and dependencies
Description=OpenBSD Secure Shell server # Human-readable description of the service

Documentation=man:sshd(8) man:sshd_config(5) # points to documentation manual for sshd and sshd_config

After=network.target nss-user-lookup.target auditd.service #dont start untill after these are up
**KEY** this doesnt force them to start, just says wait untill they do. Use Requires=  or Wants= for that

ConditionPathExists=!/etc/ssh/sshd_not_to_be_run #a condition that the file must not exist,
(! means not), so if file exists, will not start ssh

[Service] #everything about the actual daemon
EnvironmentFile=-/etc/default/ssh # loads environment variables from here, - means ignore it if it doesnt exist. **NOTE** $SSHD_OPTS comes from here

ExecStartPre=/usr/sbin/sshd -t # run this before starting, -t means test the configuration for syntax errors

ExecStart=/usr/sbin/sshd -D $SSHD_OPTS # this is teh real command, -D means dont Daemonize. **NOTE** sshd normally forks itself into the backround, but systemd doesnt want that so it can supervise it. Otherwise, it will start up, fork, parent process exits, and child process detaches from the terminal and continues running in the backround. (thats traditional Unix
Daemon behavior) (read linux_intro.md for more on terminals and forking away and stuff)

ExecReload=/usr/sbin/sshd -t # when someone runs systemctl reload ssh, first checks config

ExecReload=/bin/kill -HUP $MAINPID # next, it config valid, send SIGHUP to main sshd process, which means reload your config without disconnecting users, MAINPID filled in systemd

**NOTES ON KILL** CONTRARY TO NAME, KILL IS USED FOR SENDING SIGNALS TO PROCESSES.
/bin/kill [signal] PID
/bin/kill -SIGNAL PID
so /bin/kill -HUP PID, sends SIGHUP to PID 1234 (like how -lc is really libc)
-HUP originally meant that terminal connection was closed, but today can be interpreted as 
reload configs, reopne logs, refhresh state without exiting, depends on program.

KillMode=process # when stopping process, treat as proces (control-group default, process, mixed, none) process means only kill the main sshd process, not every child process.
Important because every SSH login is a child process, so stopping the daemon doesnt immediatley kill logged in users


Restart=on-failure #if crashes, restart it, but dont restart if intentionally stopped
RestartPreventExitStatus=255 # one exception: if exit with status 255, dont restart it. 255 means fatar startup errors like config problem

Type=notify # how systemd knows service is ready, types include
simple, forking, oneshot, notify, idle.
notify means the service sends a message to systemd saying "im ready", untill then, systemd
considers it "starting"




RuntimeDirectory=sshd # creates /run/sshd before starting, which disappears on reboot. Useful for runtime files like sockets or PID-related state


RuntimeDirectoryMode=0755 # perms for runtime dir, 
0 means owner, then read = 4 write = 2 ex = 1
7 = rwx (owner)
5 = rx (group)
5 = rx (others)

[Install]
WantedBy=multi-user.target #when system reaches that target, thats when you start the service.
How it works:
this practically means "symlink this service file such that multi-user.target wants it"

CREATES THIS SYMLINK ON ENABLE
/etc/systemd/system/multi-user.target.wants/ssh.service
    -> /lib/systemd/system/ssh.service
**NOTE** when you write systemctl status ssh or cat ssh, you see that it says
triggered by ssh.socket, meaning it is socket activated. THus, it is not in the multi-user.target.wants or sockets.target.wants like you would expect.

systemctl is-enabled ssh

ANSWER: they are in /etc/systemd/system! reason the socket and service are both enabled is that
this way it can claim port 22 and give it to the service later for ease.


Alias=sshd.service # creates an alias for it, which may coincide with the actual name in some
distros. EXE: some call it ssh.server, but people expect sshd.service, so this makes both work:
systemctl enable ssh
systemctl enable sshd

gclocal@gclocal-MS-7E59:/usr/lib/systemd/system$ ls -ld ssh.service
-rw-r--r-- 1 root root 561 Mar 12 07:14 ssh.service

gclocal@gclocal-MS-7E59:/usr/lib/systemd/system$ ls -ld sshd.service 
lrwxrwxrwx 1 root root 11 Apr 27 17:15 sshd.service -> ssh.service

can add in ls -i to see inode to see if its teh same



## SOCKET FILE BREAKDOWN
And the socket:
[Unit]
Description=OpenBSD Secure Shell server socket #human readable
Before=sockets.target ssh.service #start this socket before sockets.target, and ssh.service.
Useful when using socket activation, since systemd want the listing socket ready before it starts the ssh daemon

ConditionPathExists=!/etc/ssh/sshd_not_to_be_run #again only runs if the file not present

[Socket]
ListenStream=0.0.0.0:22 ## listens for TCP connection on IPv4 port 22.
**NOTE** 0.0.0.0 means every IPv4 address on this machine, equivalent to
192.168.1.5:22
10.0.0.3:22
127.0.0.1:22
ALL AT ONCE

ListenStream=[::]:22 # listens on IPv6 port 22, where [::] is the IPv6 equivalent of 
0.0.0.0, meaning every IPv6 address



BindIPv6Only=ipv6-only #normally, on Linux, IPv6 sockets can sometimes accept IPv4 connections
IPv4 mapped Ipv6 addresses, but this option says don tdo that, keep the IPv6 socket strictly 
IPv6. Thus, you end up with 2 sockets


Accept=no ## **VERY IMPORTANT** 
There are 2 modes:
Accept=no
means one service handles all incoming connections:
systemd
     │
 listens on :22
     │
     ▼
starts ssh.service
     │
     ▼
sshd accepts every client
(One long-running daemon)

Accept=yes
means each incoming connection gets its own service instance

client 1
      ▼
sshd@1.service

client 2
      ▼
sshd@2.service

client 3
      ▼
sshd@3.service

OpenSSH uses Accept=no because it is designed to handle multiple connections simultaneously

FreeBind=yes #Normally, a program can only bind to an IP address that already exists on teh machine.
exe:
Machine has 192.168.1.10
trying to bind 192.168.1.20 would fail, FreeBind=yes tells systemd bind anyways, even if that 
address doesnt exist yet

This is useful if:

networking isn't fully configured yet,
the IP will appear later,
the address is managed by DHCP,
the address is a floating IP in a high-availability setup.


**ASIDE ON SOCKETS**
READ SOCKETS_LEARN.MD






[Install]
WantedBy=sockets.target #symlinks this files into teh want sections of that socket like before
RequiredBy=ssh.service # this is teh inverse of Requires=, when the service is enabled, systemd
knows that it requires the socket to be enabled as well.
This is mostly for systemctl enable/disable, not so that ssh.service starts because ssh.socket is active.





Socket Activation:
When teh socket is enabled:
Boot
 │
 ▼
systemd
 │
 ▼
ssh.socket starts
 │
 ▼
Port 22 is open
 │
 ▼
(No sshd process running yet)

Then, when a client connects, 
Laptop
   │
SSH to port 22
   │
   ▼
systemd receives the connection
   │
   ▼
Starts ssh.service
   │
   ▼
Passes the already-open listening socket to sshd
   │
   ▼
sshd begins handling SSH sessions

**NOTE** generally, ssh is enabled the traditional way via systemctl enable ssh, not
this other method. Nonetheless, it keeps the socket up so that it can pass the existing listening socket to the service later.


## TARGET FILE BREAKDOWN
And the target are:

multi-user.target

      # SPDX-License-Identifier: LGPL-2.1-or-later
      #
      #  This file is part of systemd.
      #
      #  systemd is free software; you can redistribute it and/or modify it
      #  under the terms of the GNU Lesser General Public License as published by
      #  the Free Software Foundation; either version 2.1 of the License, or
      #  (at your option) any later version.

[Unit]
Description=Multi-User System  #user-generated description of the file
Documentation=man:systemd.special(7) # location for the documentation
Requires=basic.target #
Conflicts=rescue.service rescue.target
After=basic.target rescue.service rescue.target
AllowIsolate=yes
















































## INTERPLAY:

When A Linux Machine boots, it roughly goes :

firmware
  -> bootloader
      -> Linux kernel
          -> optional initramfs/initrd userspace (more on that later)
              -> real root filesystem
                  -> /sbin/init

where **/sbin/init** is usually a symlink to systemd.
since systemd is first userspace process, it runs as PID 1, making it the system init process, whose job is to bring up
and maintain userspace services.


Very early, systemd does some built-in setup like:
set up basic API filesystems like /proc, /sys, /dev
configure basic things like hostname/loopback
install signal handlers
initialize logging connections, etc...

But then it descides, "What is the initial unit I should try to activate?"

By default, that is default.target, but it can be overridden via a kernel command line option like
systemd.unit=rescue.target

**KEY** thus, boot is start PID 1, then figure out how to activate default.target, not how to read all service files.

Here is sample of default.target:

      #  SPDX-License-Identifier: LGPL-2.1-or-later
      #
      #  This file is part of systemd.
      #
      #  systemd is free software; you can redistribute it and/or modify it
      #  under the terms of the GNU Lesser General Public License as published by
      #  the Free Software Foundation; either version 2.1 of the License, or
      #  (at your option) any later version.

[Unit]
Description=Graphical Interface
Documentation=man:systemd.special(7)
Requires=multi-user.target #HUH? REASON IS THAT DEFAULT.TARGET IS THE DESTINATION, NOT THE STARTING POINT!
Wants=display-manager.service
Conflicts=rescue.service rescue.target
After=multi-user.target rescue.service rescue.target display-manager.service
AllowIsolate=yes

default.target
        |
        v
multi-user.target
   /      |      \
network  sshd   getty


### Aside on Units:
A Unit is a systemd internal object for something it can manage
exe:
sshd.service          a service process
multi-user.target     a grouping/synchronization point
sockets.target        a grouping of sockets
dbus.socket           a listening socket
home.mount            a mounted filesystem
dev-sda.device        a kernel device object

a **UNIT FILE** is an INI-style that describes those unit objects, .service, .socket, .target are such files
**NOTE** a Unit file is not the unit itself, but rather the config used to construct the in-memory unit object

Systemd units can come from many different places:
/usr/lib/systemd/system/     package-provided units
/etc/systemd/system/         admin overrides/custom units
/run/systemd/system/         runtime units
/run/systemd/generator*/     generated units
transient D-Bus-created units
kernel/device state

this is called the **Load Path** essentially where systemd finds its units, with files in earlier
path locations overriding ones in later paths


WHen systemd want ssh.service, it conceptually runs:
load_unit("sshd.service"):
    find main unit file in load path
    read [Unit], [Service], [Install], etc.
    read drop-ins, such as sshd.service.d/*.conf
    read dependency symlink directories:
        sshd.service.wants/
        sshd.service.requires/
    apply implicit dependencies
    apply default dependencies
    create/update in-memory Unit object

Result: in memory object with properties
Name=sshd.service
Type=service
LoadState=loaded
ActiveState=inactive
SubState=dead
Wants=...
Requires=...
Before=...
After=...
ExecStart=...
Restart=...

**NOTE** Load just means take from disk, and make a memory object, not START or ENABLE

systemctl list-units only lists ones referenced directly or indirectly in teh dependency graph, not all on disk

### Unit States

Every unit has a current state, exe:

inactive #currently stopped
activating #in the process of starting
active #currently started
deactivating #in the process of starting
failed

The High-Level state is called **ACTIVE** and the more precise type-specific state is called **SUB**
exe:
ACTIVE=active
SUB=running

**KEY**
Thus, a unit object has a **current reality**:
"sshd.service is currently active/running"
But when you ask systemd to do something, you create a **requested transition**
exe:
please start sshd.service
please stop sshd.service
please restart sshd.service
That requested transition is a **job**


### Jobs
A job is a systemd internal record that says "move this unit towards a desired state"
exe:
start job for sshd.service
stop job for nginx.service
restart job for postgresql.service
reload job for systemd-journald.service

Think of a job like:
struct Job {
    Unit *unit;          // pointer to the unit it modifies
    JobType type;        // START, STOP, RESTART, RELOAD, ...
    JobState state;      // waiting, running, done, failed, canceled, ...
    JobResult result;    // success, dependency, timeout, skipped, ...
}

**KEY** A unit can have a job attached to it, called the unit's **job property**
If a unit contains a currently scheduled or executing job, the property contains it jobID and job object path, 
else the job ID is 0.

**systemctl show ssh.service -p Job**

systemctl show ssh.service -p Job --value
then 
systemctl list-jobs

Or over D-Bus
busctl introspect org.freedesktop.systemd1 \
    /org/freedesktop/systemd1/job/123

**NOTE** Systemctl show gives only the JobId, but D-Bus via busctl you will see the full object path (LOOK INTO LATER)

Thus, when you run systemctl start sshd.service, systemctl does not itself start sshd, but
rather it talks to systemd PID 1 and asks it to start the unit via the D-Bus API, which 
defines a method StartUnit(), as enqueing a start job and possibly depending jobs. 
It returns a job object, and clients must watch that job if they want the final result.

systemctl start sshd.service
    -> send StartUnit("sshd.service", "replace") to PID 1
        -> PID 1 creates Job(type=start, unit=sshd.service)

KEY IDEA: when you start a service, it does not do it right away, instead it just 
creates the job object for what you want to do.

READ DBUS LEARN FOR MORE INFO


### Transactions

A **transaction** is a temporary set of jobs systemd builds before committing them to 
the real job queue

exe:
suppose you run

systemctl start myapp.service

which contains:
[Unit]
Requires=postgresql.service
Wants=redis.service
After=postgresql.service redis.service

1. From here, systemd starts with an initial requested job:

START myapp.service,

2. Then, it expands Dependencies
myapp Requires postgresql -> add START postgresql.service
myapp Wants redis         -> add START redis.service

**NOTE** this expansion is infinitely recursive. IE EVERY DEPENDENCY OF THE DEPENDENCIES IS
CONTAINED, SO LITERALLY EVERY SINGLE UNIT THAT MUST BE STARTED BEFORE IS INCLUDED IN THE
TRANSACTION.

3. Then it adds ordering edges
postgresql.service START must finish before myapp.service START begins
redis.service START must finish before myapp.service START begins

4. Creates the full temporary transaction

Jobs:
    J1 = START myapp.service
    J2 = START postgresql.service
    J3 = START redis.service

Requirement edges:
    myapp.service requires postgresql.service
    myapp.service wants redis.service

Ordering edges:
    postgresql.service -> myapp.service
    redis.service      -> myapp.service

Or in graph form:
START postgresql.service ─┐
                          ├── must complete before ──▶ START myapp.service
START redis.service ──────┘

**KEY**
Requires and Wants decide what jobs are in the transaction
(They are the unit dependency edges causing other jobs to be pulled in)
(WANTS IS JUST LESS STRONG THAN REQUIRES, IN THAT WILL STILL START WITHOUT IT ON THE EVENT
THAT IT FAILS. REQUIRES FORCES IT TO BE STARTED FIRST, AND IF THE DEPENDENCY STOPS SO TOO DOES THE SERVICE) BindsTo is even stronger, as it also ties lifetime.

After and Before decide ordering among jobs already in the transaction
(They decide the ordering edges that constrain when jobs may be run, but only matter
if both relevant jobs exist)
(can be redundant, ie one service's before is that services after, but this is when you 
cannot edit the other service, for isolation)






### Transaction Builds
Model:

manager_start_unit(unit, mode):
    create empty transaction T
    add_job(T, START, unit, required=true)

    recursively expand requirement dependencies:
        if START A and A Requires B:
            add START B as required
        if START A and A Wants B:
            add START B as wanted/optional
        if START A and A Requisite B:
            require B to already be active; do not start it
        if START A and A Conflicts B:
            add STOP B

    add ordering constraints:
        if A After B and both A and B have jobs:
            job(B) must run before job(A)
        if A Before B and both A and B have jobs:
            job(A) must run before job(B)

    merge with existing queued jobs #THE ACTUAL JOB QUEUE
    detect conflicts
    detect ordering cycles
    drop optional jobs if needed to make transaction valid
    fail transaction if required jobs cannot be reconciled
    commit transaction into real job queue

exe:
Current job queue:

START sshd
START nginx
STOP cups

You run systemctl restart nginx

It then creates a new transaction in memory:
STOP nginx
START nginx

However, it doesn't just append these to the queue, it tries to merge them correctly.
It might replace the old job, merge into one, cancel one, reject the new transaction, etc

This is why systemd can start many things at once:
it keeps asking: which jobs have no unmet ordering pre-requisites, and those jobs are runnable

exe:
A.service
B.service
C.service After=A.service
D.service After=A.service

Transaction:
START A
START B
START C
START D

Ordering:
A -> C
A -> D

Execution:
time 0: start A and B in parallel
time 1: A finishes starting
time 2: start C and D in parallel


### After= low level
[Unit]
After=network.target

This creates an ordering edge stored on the unit object.
"this unit is ordered after network.target"

**KEY**
No job is pulled from this, so if it only says after=network.target, but no
Requires, BindTo, or Wants, then transaction contains only
START myapp.service

Exe:

A wants B, B requires C.
If C doesnt work, then B exlcuded, but A succeeds (Unsure)

If C fails, then B which requires C and after will fail, then A waits for B to start bc after
but on B fail, A is allowed to start

**KEY** AFter also determines ordering during transaction merges, which happen to create the
final job queue

### Finished Starting

The KEY is that it is unit-type specific

When systemd says A must start AFTER B, means A's job cannot begin untill after
B's job is considered complete, however what that means depends on teh unit's type

For a target:
target start job is complete when the target itself becomes active

For a socket:
complete when systemd created/listened on the socket

For a .mount:
complete when mount operation is done

For a service:
depends on type
type=simple means started immediately after it forks the service process, even before 
execve is envoked

type=exec means after execve succeeds

type=oneshot means waist untill the main process exist(IE whatever ExecStart runs, waits 
for exit status 0)

type=notify means service sends signal READY=1, send via the sd_notify() API from libsystemd
sd_notify(0, "READY=1");
Trick is that when systemd sees type=notify, it creates a notify socket (Unix Datagram Socket)
and exports its address via the environment
(**KEY IDEA**)

NOTIFY_SOCKET=/run/systemd/notify
NOTIFY_SOCKET=@/org/freedesktop/systemd1/notify //alternative, via an abstract socket

And process inherits it via execve()
argv = ...
envp = {
    ...
    NOTIFY_SOCKET=/run/systemd/notify
}

And internally, sd_notify() creates its own socket
int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);

lastly it sends 1 datagram:
READY=1
STATUS=Listening on port 8080
MAINPID=12345
via sendmsg() and sendto()

### Wants low level
[Unit]
Wants=redis.service

Adds a requirement-type dependency edge

A wants B.

This way, when a start job for A is added to a transaction, a start job for B is also
tried to be added.

But, wants is weak, so if it fails, it doesnt fail the whole transaction

Also, without any order specified, starts both simultaneously (ie both runnable)
so good to order

[Unit]
Wants=redis.service
After=redis.service
MEANS:
Pull redis into the transaction.
If redis is being started, start me after redis.
But if redis fails, I may still start.

### Requires low level
Also pulls job, but is stronger
[Unit]
Requires=postgresql.service

Transaction:
START myapp.service
START postgresql.service

KEY:
If myapp.service is activated,
postgresql.service is also activated.

If postgresql.service is explicitly stopped/restarted,
myapp.service is also stopped/restarted.

If postgresql.service fails to activate, and myapp.service has After=postgresql.service,
then myapp.service will not be started.

### Before low level
Just like After, but it says that this unit must be started before
other units
Again, it does not pull units into the transaction, just defined ordering edges.

NOTE:
normally, you do not write 
Before=mutli-user.target, 
instead use WantedBy=multi-user.target, which creates a symlink for the service
and puts it in multi-user.target.wants directory, either in /etc or /usr/lib

**SPECIAL BEHAVIOR**
target units automatically complement their Wants= and Requires= with 
After=, unless DefaultDependencies=no is set

This means that the service is normally started before the target is considered reached, not 
after or at the same time.



[Install]
WantedBy=multi-user.target

does not mean "Star this after multi-user.target"
but rather it means: 
"When mutli-user.target is the goal, pull this service into that target's transaction

### WantedBy low level
WantedBy is not runtime dependency!

[Install]
WantedBy=multi-user.target

is used by: systemctl enable my.service, not my normal transaction execution!
**NOTE** enabling vs disabling means it just creates this symlink, which dictates the
.wants/ of the listed unity to the current unit.

When you enable the unit, systemd creates a symlink like:
/etc/systemd/system/multi-user.target.wants/my.service
    -> /usr/lib/systemd/system/my.service

EFFECTIVELY, WHEN ENABLED, SYSTEMD SEES:
multi-user.target Wants=my.service

WHY DO THIS: because it lets you add it on later without editing the 
multi-user.target file.
WantedBy and RequiredBy work the same way, lets you create Wants and Requires depencencies
for units you cannot actively control / are already active


### RequiredBy low level
This is the same as WantedBy, but stronger

[Install]
RequiredBy=some.target

means

When enabled, create:
some.target.requires/my.service

WantedBy creates a .wants/ symlink, and RequiredBy creates a .requires/ symlink

**KEY** but AGAIN this isnt a runtime thing, so it doesnt enforce it when you start
this program that those are started at all. when enabled, it just adds it to teh wants of the other, but if its already running, no difference

### All Options

After/Before: Determines ordering in a transaction and during transaction merges. Does not pull jobs into a transaction.
Runtime property. Ordering edges

Wants/Requires: Determines the dependency/requirement edges in a transaction. Does not determine order. Wants means can work without, Requires means cannot work without

WantedBy/RequiredBy: used to affect the Wants/Requires of other units without editing their
files. Both will symlink the primary unit to the specified unit's .wants/ and .requires/
directories respectively. Not a runtime property, only done when enabling and disabling units
(enable = create symlink, disable = remove) enabling = effectively writing
Wants=my.service in the specified unit, disabling means removing that.
Key functionality: when used with a target, implied After= for all units unless DefaultDependency=no.


### Boot as a transaction

At boot, systemd activates 
default.target, which usually

default.target -> graphical.target
or default.target -> multi-user.target

default.target:
[Unit]
Description=Graphical Interface
Documentation=man:systemd.special(7)
Requires=multi-user.target #implied after
Wants=display-manager.service #implied after
Conflicts=rescue.service rescue.target #MEANS THESE UNITS CANNOT BE ACTIVE TOGETHER
After=multi-user.target rescue.service rescue.target display-manager.service
AllowIsolate=yes

(THIS IS A SYMLINK TO graphical.target)


Thus, on Boot:

1. Systemd starts with

START default.target (initial job)

2. It builds the transaction for the job before queueing it
Then, it reads its dependencies recursively and constructs the graph,
looking at its depencency symlinks ,creating effective dependencies

Then it recursively expands
START multi-user.target
START sshd.service
START cron.service
START networking.service
...

The real boot becomes a large graph:
systemd starts default.target

default.target
  -> multi-user.target
      -> sshd.service
      -> cron.service
      -> NetworkManager.service
      -> getty.target
          -> getty@tty1.service
      -> basic.target
          -> sockets.target
          -> timers.target
          -> paths.target
          -> sysinit.target

**NOTE** can view with 
systemctl list-dependencies --all default.target
and boot transaction jobs
systemctl list-jobs

exe:

multi-user.target
wants myapp.service and sshd.service

myapp.service
reqs postgresql.service
wants redis.service
after both
notify

postgresql.service
type notify

redis.service
type=simple

1. systemd creates job START default.target

2. Expansion, looks at Wants, Requires, and its .wants and .requires
multi-user.target Wants=myapp.service
    -> add START myapp.service

multi-user.target Wants=sshd.service
    -> add START sshd.service

myapp.service Requires=postgresql.service
    -> add START postgresql.service as required

myapp.service Wants=redis.service
    -> add START redis.service as optional


3. Ordering
myapp After=postgresql
    -> START postgresql before START myapp

myapp After=redis
    -> START redis before START myapp

target default dependency:
multi-user.target Wants=myapp
    -> START myapp before START multi-user.target

target default dependency:
multi-user.target Wants=sshd
    -> START sshd before START multi-user.target

4. Assemble job queues
Jobs:
START postgresql.service
START redis.service
START myapp.service
START sshd.service
START multi-user.target
ordering:
postgresql.service -> myapp.service
redis.service      -> myapp.service
myapp.service      -> multi-user.target
sshd.service       -> multi-user.target

5. Make final job queue
time 0:
    start postgresql.service
    start redis.service
    start sshd.service

time 1:
    redis.service Type=simple: considered started after fork
    sshd.service considered started according to its unit Type=

time 2:
    postgresql.service Type=notify sends READY=1
    postgresql start job completes

time 3:
    start myapp.service

time 4:
    myapp.service Type=notify sends READY=1
    myapp start job completes

time 5:
    multi-user.target becomes active

### Conflicts

[Unit]
Conflicts=rescue.target

means this unit and rescue.target should not be active together.
If systemd starts one, it schedules a stop job for the other

**KEY** again however, doesn't speficy ordering, just that a stop job must be scheduled
Need Before= or After= to force it to stop it before starting this one

[Unit]
Conflicts=old.service
After=old.service

### Shutdown:

if startup order is 
postgresql.service -> myapp.service
because
myapp.service After=postgresql.service

Then shutdown order is
stop myapp.service first
stop postgresql.service second

### Sockets
Sockets used for event-driven jobs
exe:
myapp.socket
[Socket]
ListenStream=8080

[Install]
WantedBy=sockets.target

This means sockets.target wants myapp.socket, and After= is implied,
Thus, at boot:
sockets.target wants myapp.socket
systemd starts myapp.socket
systemd opens/listens on port 8080

NO MYAPP.SERVICE RUNNING YET

LATER:
client connects to port 8080
kernel queues connection
systemd sees activity on myapp.socket
systemd enqueues START myapp.service

### Default dependencies

**KEY IDEA**
if you write a simple service file:

[Service]
ExecStart=/usr/local/bin/foo

Systemd automatically adds:
After=basic.target
Conflicts=shutdown.target

depending on your unit type

Implicit Dependencies: allways added depending on your unit type/config
Default Dependencies: added unless DefaultDependencies=no

**FOR TARGETS, A CRUTIAL DEFAULT DEPENDENCY IS**
If target T Wants=A or Requires=A,
then target T is automatically ordered After=A,
unless disabled/overridden.
SAME FOR WantedBy and RequiredBy WHEN DEALING WITHA TARGET, BUT NEED TO ENABLE FOR IT TO WORK!


### systemctl enable vs systemctl start

systemctl start my.service meahs

Ask PID 1 to create a START job now (via the D-Bus Start-Unit, which adds it to staging
area aka the transaction and then once dependencies expanded, then adds jobs to queue)

systemctl enable my.service means
"modify filesystem symlinks so this unit is pulled into future transactions, according
to its WantedBy and RequiredBy


### Whole SystemD model in one precise picture:
PID 1 systemd manager
│
├── Unit objects
│     ├── sshd.service
│     │     ├── current state: active/running
│     │     ├── dependencies: After=network.target, WantedBy=multi-user.target, ...
│     │     └── type-specific config: ExecStart=..., Restart=...
│     │
│     ├── multi-user.target
│     │     ├── current state: active
│     │     └── dependencies: Wants=sshd.service, Wants=cron.service, ...
│     │
│     └── dbus.socket
│           ├── current state: active/listening
│           └── triggers dbus.service
│
├── Job queue
│     ├── START postgresql.service
│     ├── START redis.service
│     ├── START myapp.service
│     └── START multi-user.target
│
└── Transaction builder
      ├── expands Wants=/Requires=/Conflicts=
      ├── applies Before=/After=
      ├── checks conflicts/cycles
      ├── merges with existing jobs
      └── commits runnable jobs


Init Order defined by graph:

Requirement dependencies:
    decide which jobs are included.

Ordering dependencies:
    decide which included jobs must wait for which other included jobs.

Unit type readiness:
    decides when a job counts as complete.

Targets:
    provide named milestones and grouping.

Enablement symlinks:
    decide what gets pulled into boot goals.

Activation mechanisms:
    sockets, devices, timers, paths, D-Bus, etc. can add jobs later.

Final Version:
Systemd boot is not a script.
It is repeated graph solving.

1. Pick goal unit, usually default.target.
2. Load that unit and recursively load units it depends on.
3. Convert desired state changes into jobs.
4. Expand requirement dependencies into more jobs.
5. Apply ordering dependencies between those jobs.
6. Validate/merge the transaction.
7. Run every job whose ordering prerequisites are satisfied.
8. As jobs complete, unlock more jobs.
9. During runtime, new events can enqueue new jobs.
























































## Boot Process

At boot:
systemd starts (first user-space program after the kernel)
it sees should reach multi-user.target
that target included nginx, so nginx shoudl run

check dependencies, see After=network.target, systemd waits untill networking is ready

starts nginx look at ExecStart
keeps track of it (PID, status, logs)

if it crashes, Restart=always means it will restart 


NOTE: DO NOT INTEACT WITH SYSTEMD DIRECTLY, INSTEAD USE SYSTEMCTL

sudo systemctl enable --now nginx //enable means enable it ON BOOT, --now means start 
sudo systemctl disable nginx
sudo systemctl start nginx
sudo systemctl stop nginx
sudo systemctl restart nginx
sudo systemctl status nginx

so the absolute minimum is:

1: kernel (non-negotiable)
manages memory, CPU, devices
provides system calls (fork exec, etc)
is the ONLY THING that can actually create processes (AVAILABLE IN GLIBC, with headers
unistd, fcnctl, etc)
(without it, nothing runs)

2: The first process (PID 1)
after booting, kernel does something crutial:
run this program as the first userspace process
(usually systemd, or on older systems SysVinit)
why is some first process necessary:
because after kernel initializes, it asks:
"okay im ready, now i need to run a userspace program"
then, it tries to execute a program as PID1, by default /sbin/init
or whatever you specify with init=...


WHY THIS SEPARATON:
Linux works on separation of Kernelspace and Userspace
Kernel used to manage mem, and create proceses, but does not descide what to do after initializing.

Userspace is everything else, normal programs, which descide what to run, when to run it, and how to react to failures

This is indended, bc if kernel descided what to do next, it would lead to it being hardcoded: start this, then do that
then that... which is bad because less customizability. 

The ONLY thing you knwo it will do is start whatever the init process is, init=... usually /sbin/init. 
that program because PID 1.

PID 1 is the very first process. All other processes either come from it, either directly or indirectly.
Thus, PID1 is responsible for

1: Building the System, desciding
- what programs should run
- in what order
- under what conditions
(exe: start networking start ssh, start a web server like nginx, all done via syscalls in the process of PID1)

2: it cleans up processes
when process dies, it becomes a zombie
its parents must "reap" it (collect its exit status)
if the parent dies, the process is adopted by PID1

thus, PID1 must clean up orphaned processes for the entire system

Note: you can do just kernel, and make PID1 be /bin/sh.
so the kernel is running, and the shell is PID1 (valid)
however, nothing else happens unless you do it

cannot just make anything PID 1, like /bin/git or something
then, kernel runs process, it terminales, returns exit status, but that goes to kernel, so its done(will kernel panick without any task in PID1)



wait, what is all that orphan-zombie stuff:

NOTE: every process, except the very first, has a parent:


PID 1 (init/systemd)
 ├── bash
 │    ├── python
 │    └── vim
 └── sshd

processes are created using:

fork() -> duplicate process
**KEY** modern kernels use copy-on-write, meaning the forked process shares the same underlying memory pages untill a 
write occurs, because its usually not worth copying over all of that process's memory bc its often followed by execve
which deletes it all anyways.

execve() -> replaces the processes's memory (all of it) with the new layout as specified in the executable
**KEY** it does NOT replace the entire process, only that process's userspace! the kernel space of that process
remains, which is the one that contains stuff like the kernel file descriptor table!
HOWEVER: each process has its own file descriptor table, so its global in visibility, but local in scope

so everything forms a tree rooted at PID1



When a process finishes, it doesn't disappear instantly. 

Instead:
1. it stops running, giving up the memory it claimed on startup
2. The kernel keeps a tiny record of
    - exit status (success/failure) (0 is success, 1 is failure)
    - some accounting info
leftover process is called a Zombie Process

Why do they exist:
because parent might want to know:
1. Did my child success
2. what exit code did it return

thus, kernel waits untill parent says:
okay, ive read the result, you can fully remove it

**KEY**
(done via wait() and waitpid())

Running -> Exits -> Zombie -> Reaped -> Gone

Zombie is not running, or using CPU, just an entry in teh process table.
**NOTE** Zombie process's do not keep its RAM allocation (code, heap, stack, etc got freed)
but it does occupy kernel bookkeeping like its PID, exit status, and resource usage stats
PROCESS BECOMES A ZOMBIE VIA exit(status) syscall

**PROBLEM**
what if parent never calls wait()?
then the zombie just stays there


## Orphans

what if:
bash (parent)
 └── python (child still running)

 and bash exits? NOW python IS AN ORPHAN

 kernel now re-assigns the orphan to PID 1

thus: PID 1 must reap all orphaned children when they exit.


programs like bash dont expect random orphaned children
dont sit in a look calling wait
and often exit quickly

That means they fail to reap zombies or exit, which crashees the system

systemd or SysVinit:
constantly mobitor child processes
call wiat appropriately
clean up zombies
manage entire process groups


FULL PROCESS LIFECYLE:

1: process calls exit(status) or return from main() (which effectively calls exit())
2: kernel frees process's userspace resources, and marks as zombie
3: parent process is notified via SIGCHLD
4: parent calls wait() waitpid() or related function
5: kernel returns the exist status to the parent and removes the zombie from process table















































## Full example:

full chain from boot -> systemd -> python script walkthrough

0: Bootloader -> kernel load (before linux runs)

1. Firmware (BIOS/UEFI) runs
BIOS is legacy or UEFI for modern system (basically its software baked into your hardware)
it initializes CPU, Ram, basic hardware
and finds a bootable device (disk, USB, etc)

2. Loads a bootloader (GRUB)
BIOS loads the first-stage bootloader from the disk (MBR)
UEFI loads an EFI executable from the EFI System Partition

Bootloader loads:
/boot/vmlinuz (compressed linux kernel) into memory

initramfs (initial RAM filesystem) into memory(DOES NOT RUN, JUST LOADS)

passes kernel commands (root=, quiet, etc, and location of initramfs)
kernel command line (eg init=/usr/lib/systemd/systemd) prepared by the bootloader, and passed
to the kernel.
then bootloader jumps into kernel entry point (handing over control to the kernel)


1: Kernel Starts (no processes yet)

Cpu is in kernel mode
no user processes exist
no scheduler is initialized
memory manager is up

Now, kernel initializes enough of itself:
sets up memory management, scheduler, interrupts
detects hardware and loads built-in drivers for CPU, RAM, etc

CRUTIALLY:
kernel mounts initramfs as a temporary root (initramfs is loaded first by the bootloader, but the kernel runs it itself
as a temp filesystem)
prepares for first userspace execution
(NOT a process yet, just mounts it as a temporary place)
then, it executed /init from the initramfs
**KEY** MORE CLEARLY, IT DECOMPRESSES THE ARCHIVE FILE INITRAMFS INTO AN IN-MEMORY FILESYSTEM
AND THIS FILESYSTEM BECOMES THE KERNEL'S TEMPORARY ROOT FILESYSTEM
THUS: IT IS A MINI FILESYSTEM MOUNTED AS TEMP ROOT UNTILL YOU MOUNT THE ACTUAL ROOT FILESYSTEM
/
├── init
├── bin
├── sbin
├── lib
└── ...


What is initramfs?
it is tiny, temporary userspace whose job is to prepare the REAL root filesystem
/init might be:
1. a shell script (Busy-Box-stye)
2. systemd itself running in initramfs mode

what it does:
loads necessary kernel modules (disk, RAID, LVM, crypto)
discovers hardware needed to acces root filesystem
decrypts disks (if using LUKS)
assemble RAID or LVM volumes (storage)
Mounts the REAL root filesystem (/dev/sda2) (MOUNTING = MAKE CONTENTS OF STORAGE DEVICE ACCESSIBLE VIA PATH, RELATIVE TO ROOT)


**Aside on mounting**:
wait, isnt /dev/sda a mount already?
NO: is it not a filesystem, but just raw access to a disk partition.

cat /dev/sda1 returns binary garbage, not filenames because there is no filesystem interpretation happening
mounting takes the raw disk data:
filesystem struction (ext4, xfs)
Metadata (inodes, directories, etc)
Raw data blocks
AND INTERPRETS THEM INTO A FILESYSTEM STRUCTURE

then, pivot to the "real" init process:



2: Kernel creates PID 1 (systemd)
calls:
argc = number of arguments
argv = string of arguments to the command, in this case systemd
envp = environment pointer, or an array of string containing environment variable (PATH, USER) of the OS
**EXPLANATION**
argv is of type char**, pointer to pointer of character, or really 2d array, where each element is a string:

./program hello world
argv[0] = ./program
argv[1] = hello
argv[2] = world

argc = 3

if your environment variables (ie your shell environment) is
PATH=/usr/bin:/bin
HOME=/home/alice
USER=alice
SHELL=/bin/bash

then envp is:
envp[0] = "PATH=/usr/bin:/bin"
envp[1] = "HOME=/home/alice"
... (FORMATR IS KEY=value)

WHERE DOES ENVP COME FROM:
when you login, os creates your login process (desktop session) with an initial environment, often dictated
by ~/.zshrc, or ~/.bashrc and such

can view this with env or printenv.

when you run the command:

./program hello world
under the hood, the systemcall 
execve("./program",argv,envp) is envoked, with the arrays argv and envp being

argv = {
    "./program",
    "hello",
    "world",
    NULL
};

envp = {
    "PATH=/usr/bin:/bin",
    "HOME=/home/alice",
    "USER=alice",
    ...
    NULL
};

which is given to the kernel by the shell




execve("/usr/lib/systemd/systemd",argc,envp)

so kernel -> execve(systemd) -> PID 1
(now, systemd running as PID 1)


3: Systemd boot sequence (high level)

3.1: systemd initialized itself:
NOTE: systemd builds dependency graph of units and executes them in paralle when possible (read above)

sets up loggin (journald)
mounts essential filesystems (mount -t proc proc /proc, mount -t sysfs sys /sys, mount -t dev /dev)
starts cgroups heirarchy (user heriarchy for login and stuff)

3.2: sysyemd starts services:

NOTE: Systemd organized everything into UNITS:
.service -> daemons
.mount -> filesystemd
.socket -> sockets (for networking)
.target -> grouping (runlevels)
.timer -> scheduled tasks

CRUTIALLY, SYSTEMD BUILDS DEPENDENCY TREE AND STARTS PARALLEL EXECUTION OF CREATING / STARTING THESE UNITS

for each service, exe: sshd, networking, etc

Step A: fork

pid = fork(); //NOW, THERE ARE 2 PROCESSES, Parent: systemd PID1, child, service setup process

Step B: in child -> exec()
execve("/usr/sbin/sshd",argv,envp)
Now, child becomes sshd process
still managed by systemd

Step C: parent tracks it
systemd records:
PID of child, service metadata, and restart policy



4: Systemd event loop (simplified)

systemd continuosly does things like:

waitpid(-1,&status,WNOHANG);
pid -1 means to wait for any child process that has terminated
&status is a pointer to an integer where the child's exit status will be stored if child is reaped
WNOHANG means to return immediately if no child exited, instead of waiting and blocking the parent process

Return value: >0 means child exited, adn will return value of its PID
0 means WNOHANG was used, so child still running no exit
-1 means error occured

this is likely in a loop:
while (waitpid(-1, &status, WNOHANG) > 0) {
    // Child reaped, status holds exit info
}

this is how it reaps zombie processes, detects crashes, and restarts services if needed


5: You log in -> shell starts

when you open a terminal or login:

fork(); //from systemd PID1
execve("/bin/bash",argv,envp); //inside child

so chain:
systemd -> bash (your shell)


6: Run a python script:
python myscript.py

this triggers:

6.1 shell fork:
pid = fork();

systemd -> bash -> (new clone child)

6.2
replace child with python
exevce("/usr/bin/python",["python3","myscript.py"],envp);

now child is python interpreter

systemd -> bash -> python



7: Python runs your script:
bash -> python3 -> running scropt

(inside python, it may fork() for mutliprocessing or exec() other programs, os.exec)

exe:
os.system("ls") -> internal fork() execve("/bin/sh",["sh","-c","ls"],envp);


8: Python Potential Subprocesses

subprocess.run(["echo","hello"])

becomes
fork(); //forks from python
execve("/usr/bin/echo",["echo',"hello"],envp);


9: Process exit flow (CRUTIAL)
python exits
exit(status);
//return vs exit: return returns from teh function, exit terminates the whole program and passes its exit status up the 
chain

kernel now marks old python as zombie:
exited + zombie untill parent reaps it

AT THE SAME TIME:
Bash reaps it:
waitpid(child_pid,&status,0);

now its fully removed


**NOTE**
if it parent (bash) dies:
kernel re-parents it to PID 1:

then, systemd eventually
waitpid(child,&status, ...) and cleans it up



11: FULL CHAIN

BIOS/UEFI
Bootloader (GRUB)
Kernel (/boot/vmlinuz)
execve("/sbin/init"); -> PID 1
systemd forks/exec services
login shell (bash)
bash fork/exec
python3 myscript
fork/exec subprocesses



KEY: how to invoke system calls.