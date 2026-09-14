# general nvidia structure:

GPU chip
   ↓
GPU module
   ↓
HGX motherboard/platform
   ↓
DGX server
   ↓
NVL rack
   ↓
AI datacenter


## 1: GPU

these are the actual AI accelerator chip
(think)
B200
B300
H100
H200

## 2: HGX
This is nvidia's server platform that holds multiple GPUs together
A typical HGX board contains:

8 GPUs
NVLink connections between them
High-speed switches (PCIe switches, allows multiple GPUS to share access to a single or multiple CPUs)
Power delivery
Cooling interfaces

CPU
 │
PCIe x16
 │
PCIe Switch
├── GPU 1
├── GPU 2
├── GPU 3
└── GPU 4
The PCIe Switch:
Expands the number of PCIe devices that can be connected.
Routes PCIe packets between devices.
Doesn't increase the total bandwidth to the CPU—it shares the available upstream bandwidth.

In most cases, it uses the NVSwitch, a specialized switch that connects mulitple gpus vs NV Link
GPU0 ─┐
GPU1 ─┤
GPU2 ─┤
GPU3 ─┼── NVSwitch
GPU4 ─┤
GPU5 ─┤
GPU6 ─┤
GPU7 ─┘
(results in higher gpu-to-gpu bandwidth than PCIe)


InfinityBand and Ethernet Switches:

WHen multiple GPU servers are connected in a cluster, it often is done via high-speed network switches rather than PCIe or NVLink switches. (GPU server just means beefy computer)

FINAL DIAGRAM:

Switch type	                    Connects	            Typical use
PCIe switch	                    CPU ↔ GPUs/SSDs	        Expands PCIe connectivity inside a server
NVSwitch	                    GPU ↔ GPU	            High-bandwidth communication within a multi-GPU server
InfiniBand/Ethernet switch	    Server ↔ Server	        Connects GPU servers into large AI clusters

HGX EXample:
HGX B300

+-------------------------------+
| GPU | GPU | GPU | GPU         |
| GPU | GPU | GPU | GPU         |
+-------------------------------+

8 Blackwell Ultra GPUs
Connected with NVLink

These HGXs are often bought by server manufacturers (OEMs or original equipment manufacturers) like ASUS, Dell, HPE, SuperMicro to make the full servers

**NOTE** the HGX is not a server motherboard, as it doest come with the parts expected to be in a motherboard:
Motherboard
├── CPU(s)
├── RAM
├── PCIe slots
│   ├── GPU
│   ├── GPU
│   └── NIC
└── Storage

Instead, it is usually called a GPU baseboard or GPU subsystem

## 3: DGX

Instead of selling just teh gpu motherboard, NVIDIA also sells the whole machine
DGX:

CPU
RAM
Storage
Networking
Power
Cooling
HGX board
Operating software

The DGX of course also already includes the HGX platform

## Grace 
Grace is nvidia's arm-based CPU, made to pair with the gpu

Grace CPU
      │
NVLink-C2C
      │
Blackwell GPU

This combination is called a superchip
exe:
GB200
GB300

NOTE: they arent just CPUS, but rather CPU + GPU integrated on a single platform
GB300 Superchip

+-------------+
| Grace CPU   |
+-------------+
       ||
 NVLink-C2C
       ||
+-------------+
| Blackwell   |
| Ultra GPU   |
+-------------+

## NVL
connects many GPUS together under a high-bandwidth fabric

GB300 NVL72

72 GPUs
36 Grace CPUs
Thousands of GB/s of NVLink bandwidth

Blackwell GPU
        ↓
8 GPUs on an HGX board
        ↓
HGX board inside a DGX server
        ↓
Multiple DGX servers linked by NVLink and InfiniBand
        ↓
NVL rack
        ↓
AI cluster

OR, FOR GRACE-BLACKWELL CHIPS

GB300 Superchip
        ↓
72 Superchips
        ↓
GB300 NVL72 rack
        ↓
AI factory