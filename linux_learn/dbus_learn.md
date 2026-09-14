#### Aside on D-Bus
D-Bus means desktop bus, and its an inter-process communicaiton system (IPC) allowing 
programs and services to communicate without knowing each other's PID.

Instead of apps communicating through custom sockets or files(which need pid), dbus provides standard message bus.
exe:
network manager tells apps that network connection changed
power manager informs programs that battery is low
service exposes methods that other programs can call

**KEY** this way, clients dont need to know a PID, and can simply send messaged to a well
known D-Bus name like org.example.MyService

SYSTEM:
1. Services publish/expose interfaces
2. Clients call methods or recieve signals
3. The bus daemon routes the messages

Relation to systemd:
whereas we, the user, interact with systemd via systemctl, it also exposes its functionality
over d-bus so processes can more easily communicate with it.

EXE:
when you systemctl start nginx
systemctl doesnt manipulate services directly!
instead, it sends a D-Bus request to systemd daemon (PID 1), asking it to start the service:
systemctl
      │
      ▼
    D-Bus
      │
      ▼
systemd (PID 1)
      │
      ▼
Starts nginx.service

D-Bus Activation:
instead of running all services all the time:
1. service registers a D-Bus name
2. The service is not running
3. A client sends a message to that name
4. D-Bus (often integrated with systemd) automatically starts that service
5. message is delivered

THis saves boot time.

EXE2:
suppose application want to reboot computer.
Instead of running reboot, send D-Bus message like:

Method:
org.freedesktop.login1.Manager.Reboot()

D-Bus activated Services:
[Service]
Type=dbus #tells systemd to wait untill the service acquires the specific D-Bus name
**NOTE** thus, it isnt just waiting for process to start, but for it to reguster a unique
name of the D-Bus before it can consider the service started. 



**BusName**=org.example.MyService #registers the D-Bus name the service must own
ExecStart=/usr/bin/myservice # Once that name appears, systemd considers teh service started 

**NOTE 2**
Notice, the D-Bus names look like URLs!
Thats because they reverse DNS names, made this way to guarentee global uniqueness!
EXE:
imagine if every dev could name their service "Settings" on the D-Bus, that would be a pain!
Instead, everybody uses a name based on a domain they own
freedesktop.org is org.freedesktop.*, etc (hense the reverse part)
(same convection for java package names com.google.common, org.apache.http)
That being said, every D-Bus connection gets a unique name automatically
exe:
:1.4
:1.27, assigned by D-Bus daemon

A process can then request a well-known name **RequestName("")**
That being said, it isnt enforced by some online DNS, just the local D-Bus daemon.
Processes use teh well known name, not the random assigned one, which is just for D-Bus internals.

##### D-Bus Commands
Structure is:
Bus name: who you are talking to
Object path: which object inside that service
Interface: what API that object exposes

Service (bus name)
    ↓
Object
    ↓
Interface
    ↓
Method


1. List bus names for all processes:
busctl list 

2. 
// Tells you what objects does this service expose
busctl tree org.freedesktop.systemd1

returns somethign like

/
└─/org
   └─/org/freedesktop
      └─/org/freedesktop/systemd1
         ├─unit
         ├─job
         └─manager
DBus lets processes expose mant objects each with their own path

**KEY**
This may LOOK like a directory tree, but thats just semantics.
Everything there is just objects, but organized this way to make things easier.
kind of like how you can have urls:
https://example.com/api
https://example.com/api/users
https://example.com/api/posts
but reperesent it as:
/
└── api
    ├── users
    └── posts

OBJECT PATHS ARE JUST IDENTIFIERS, MADE THIS WAY FOR ORGANIZATIONAL PURPOSES
THE /, /org, /org/freedesktop/, IS LIKELY JUST THERE FOR ORGANIZATIONAL PURPOSES, ALLOWING
BUSCTL TO WALK THE CHILDREN UNTILL REACHING THE DESIRED OBJECT.
CAN CHECK VIA busctl introspect org.freedesktop.systemd1 /org

gclocal@gclocal-MS-7E59:~$ busctl introspect org.freedesktop.systemd1 /org
NAME                                TYPE      SIGNATURE RESULT/VALUE FLAGS
org.freedesktop.DBus.Introspectable interface -         -            -    
.Introspect                         method    -         s            -    
org.freedesktop.DBus.Peer           interface -         -            -    
.GetMachineId                       method    -         s            -    
.Ping                               method    -         -            -    
org.freedesktop.DBus.Properties     interface -         -            -    
.Get                                method    ss        v            -    
.GetAll                             method    s         a{sv}        -    
.Set                                method    ssv       -            -    
.PropertiesChanged                  signal    sa{sv}as  -            -    

This shows that /org is indeed an object, and has methods like .Introspect, .Ping, and
.Get and .Set, which are general D-Bus interfaces, not systemd specific.
Systemd just chose to register objects at those paths, even though they dont expose
any systemd functionality.

Kind of like with a web server:
for /api/users, can choose to make pages for /, /api, but not necessary

Why do this?
D-Bus specs define an interface called org.freedesktop.DBus.Introspectable,
an object implementing it returns an XML describing itself and its immediate children
by putting an introspectable object at /org, can recurse through it

**KEY**:
How does Busctl know what to display for tree?
It finds it out recursively, as the process's dont register it with the bus daemon.
It calls Introspect on /, gets an XML of its child nodes, and recursively goes through it
building the tree.
**CRUTIAL: ITS HARD TO SEE AT FIRST, BUT THE VERY LAST ROW OF THE INTROSPECT XML SAYS:**
```bash
 </signal>\n </interface>\n <node name=\"LogControl1\"/>\n <node name=\"systemd1\"/>\n</node>\n"
 ```
 notice: node name= systemd1, given this is teh /org/freedesktop node.
 IT DOES SHOW A LIST OF "child" NODES AT THE BOTTOM, USED FOR INTROSPECTION, SINCE AGAIN DBUS ONLY KNOWS NAME


**KEY 2: UNLIKE THE DBUS NAME:**
The / naming is enforced though. So given some directory /foo, with child bar, its child's object path is /foo/bar. ONLY RULE IS THAT MUST START FROM / SO IT KNOWS WHERE TO RECURSE FROM






3. busctl introspect

busctl introspect org.freedesktop.systemd1 /org/freedesktop/systemd1

This means "tell me everyting about this object"
could respond with somethgin like:

INTERFACE
org.freedesktop.systemd1.Manager

METHODS:
StartUnit()
StopUnit()
RestartUnit()
Reload()
ListUnits()

PROPERTIES:
SIGNALS:

**NOTE**
Notice the types of methods you can have:
NAME                                TYPE      SIGNATURE RESULT/VALUE FLAGS
org.freedesktop.DBus.Introspectable interface -         -            -    
.Introspect                         method    -         s            -    
org.freedesktop.DBus.Peer           interface -         -            -    
.GetMachineId                       method    -         s            -    
.Ping                               method    -         -            -    
org.freedesktop.DBus.Properties     interface -         -            -    
.Get                                method    ss        v            -    
.GetAll                             method    s         a{sv}        -    
.Set                                method    ssv       -            -    
.PropertiesChanged                  signal    sa{sv}as  -            -  

This is the class definition of an object.
Says:
1. What interfaces object implements
2. What methods you can call
3. What signals it emits, and 
4. what properties it exposes

Interfaces: like C++ class, such that everything that follows untill the next
interface belongs to it. ALL METHODS ARE PART OF INTERFACES. Method calls are allways
Service, Object, Interface, Method



exe: interface org.freedesktop.DBus.Peer has methods .GetMachineId, and .Ping.
leading . means it belonds to the interface.

Can have INTERFACE, METHOD, PROPERTY, SIGNAL

Signature is method signature, ss meaning 2 strings
Return type is return type
Flags is stuff like read only




4. busctl call
busctl call \
org.freedesktop.systemd1 \
/org/freedesktop/systemd1 \
org.freedesktop.systemd1.Manager \
ListUnits

means:
"Call the ListUnits method on the org.freedesktop.systemd1.Manager interface of the object at
/org/freedesktop/systemd1 owned by the service org.freedesktop.systemd1

Argument	                            Meaning
org.freedesktop.systemd1	            Which service (bus name) (not necessarily 1 per process, more on that later)
/org/freedesktop/systemd1	            Which object
org.freedesktop.systemd1.Manager	    Which interface
ListUnits	                            Which method

Analogous to:

HTTP	            D-Bus
Server	            Bus name
URL path	        Object path
API namespace	    Interface
Endpoint/function	Method


Inspect systemd service:
busctl tree org.freedesktop.systemd1

View available methods:
busctl introspect org.freedesktop.systemd1 \
/org/freedesktop/systemd1

Call a method:
busctl call org.freedesktop.systemd1 \
/org/freedesktop/systemd1 \
org.freedesktop.systemd1.Manager \
ListUnits

busctl call SERVICE OBJECT_PATH INTERFACE METHOD [SIGNATURE [ARGUMENTS...]]
exe:Echo(in s message) → s

busctl call service object interface Echo s "Hello"
busctl call service object interface Configure ib 30 true

**COMMON PARAMETERS**
s is argument type, "Hello" is actual argument
ib is argumetn types, integer then boolean
then 30 and true are the actual argments


##### D-Bus Full walkthrough


**KEY CORRECTION**
the D-Bus name is techically attached to a connection, not a PID:

SUppose process has PID 5000

Connects to the D-Bus Daemon via Unix-domain socket:

service process PID 5000
        |
        | Unix socket connection
        v
D-Bus daemon

D-Bus daemon assigns connection a unique name, like :1.42

Then process request a well-known name RequestName(""):
com.example.Calculator

if works then daemon records:
well-known name                 owning connection
---------------------------------------------------
com.example.Calculator    ->    :1.42

THIS IS HOW A SINGLE PROCESS COULD THEORETICALLY OPEN MULTIPLE D_BUS CONNECTIONS
PID 5000
├── connection A -> :1.42
└── connection B -> :1.57



NOW: after process owns that name (more specifically a socket of that process)
Suppose you want to expose this D-Bus API:

Bus name:
    com.example.Calculator

Object path:
    /com/example/Calculator

Interface:
    com.example.Calculator

Method:
    Add(int a, int b) -> int


The code for this is as follows:

1. Define teh C function that implements the method

static int method_add(
    sd_bus_message *message,
    void *userdata,
    sd_bus_error *error)
{
    int a;
    int b;

    sd_bus_message_read(message, "ii", &a, &b); #reads the 2 integer arguments

    int result = a + b; # adds them

    return sd_bus_reply_method_return(message, "i", result); #returns result as D-Bus reply
}

2. THEN, IT DESCRIPES THE INTERFACE WITH A V-TABLE:
THE LIBRARY MUST KNOW WHICH D-BUS METHODS EXIST AND WHICH C FUNCTION IMPLEMENTS THEM

static const sd_bus_vtable calculator_vtable[] = {
    SD_BUS_VTABLE_START(0),

    SD_BUS_METHOD(
        "Add",          // D-Bus method name
        "ii",           // input signature
        "i",            // output signature
        method_add,     // C callback
        0
    ),

    SD_BUS_VTABLE_END
};

3. FINALLY, IT REGISTERS THE OBJECT WITH THE LOCAL SD-BUS CONNECTION:

sd_bus *bus; #there is already a connected sd_bus object

//register the object
sd_bus_add_object_vtable(
    bus,
    NULL,
    "/com/example/Calculator",
    "com.example.Calculator",
    calculator_vtable,
    calculator_object
);


Local registration table inside PID 5000:

Object path:
    /com/example/Calculator

Interface:
    com.example.Calculator

Methods:
    Add -> C function method_add()

User data:
    pointer to calculator_object

4. Now, the simple dispatch loop
for (;;) {
    sd_bus_process(bus, NULL);
    sd_bus_wait(bus, (uint64_t)-1);
}

Now, when another process calls the method, 

The D-Bus daemon routes the message to the connection that owns com.example.Calculator.
sd-bus receives the message on that connection.
sd-bus looks in its local registration table.
It finds the object path /com/example/Calculator.
It finds the interface com.example.Calculator.
It finds the method Add.
It invokes the registered C function:
method_add(message, calculator_object, error);
The callback computes the result and sends the reply back to the caller.

(LIKELY HAVE TO DO AN EXAMPLE)

NOTE: these functions are likely in teh systemd library (libsystemd)
/usr/lib/aarch64-linux-gnu/systemd/libsystemd-shared-259.so

(This is where libc is, for example)