CPU executes instruction
        │
        ▼
Virtual address 0x401234
        │
        ▼
MMU uses CR3
        │
        ▼
Walk page tables
        │
        ▼
"Is virtual page 0x401000 mapped?"
        │
        ├── YES → get physical frame → access RAM
        │
        └── NO
              │
              ▼
         PAGE FAULT
              │
              ▼
        CPU enters kernel
              │
              ▼
       kernel's page-fault
          handler runs
              │
              ▼
   "Ah, this address belongs
    to a file-backed VMA."
              │
              ▼
   obtain the required file page
              │
              ▼
   physical page in RAM
              │
              ▼
   update page table:
   0x401000 → physical frame X
              │
              ▼
   return to userspace
              │
              ▼
   CPU retries the instruction
              │
              ▼
           SUCCESS


Also, whenever you change which process you are evaluating, it also changes the address in CR3 to be the new process's
virtual memory space

NOTE: doesnt switch when going from user mode to kernel mode, since kernel is actaully mapped in to process's address space.
(kernel usually has high virtual addresses, but low physical addresses)

High addresses
┌──────────────────────────┐
│ Kernel virtual memory    │  ← supervisor-only
│                          │
├──────────────────────────┤
│                         │
│      unused gap          │
│                         │
├──────────────────────────┤
│ User stack               │
│ ↓                        │
│                          │
│ mmap / shared libraries  │
│                          │
│ heap                     │
│ ↑                        │
│                          │
│ executable               │
└──────────────────────────┘
Low addresses

Syscall: CPU enters Kernel Mode, and can now look at and modify the kernel memory, which is useful for syscalls bc its cheaper

USER MODE
────────────────────────────
user code
user data
...
────────────────────────────
kernel mappings (inaccessible)
────────────────────────────

             syscall
                │
                ▼

KERNEL MODE
────────────────────────────
user code
user data
...
────────────────────────────
kernel mappings ← accessible
────────────────────────────


NOTE: these perms are not enforced by the VM_Area_struct, but rather the CPU's page table entries have perm bits that enforce this. (the heirarchy and translation is of course also a kernel-level resource, just like file handles and pipes and sockets)

(perm is called supervisor-only! memory is either mapped and valid or mapped but supervisor-only)
(also ofc encodes read/write and dirty/not bits and a bunch of other stuff)

the vm_area_struct is a kernel datastructure that describes virtual memory regions, but is it not what the CPU directly 
consults to evaluate the validity of memory accesses, CPU CONSULTS THE PAGE TABLE

user instruction
      │
      ▼
MMU walks page tables
      │
      ▼
PTE says "supervisor only"
      │
      ▼
CPU is currently in user mode
      │
      ▼
ACCESS DENIED → page fault

