# Intro to storage, harddrives, and utilities regarding them

lsblk to see inside


## Storage on Mac

Physical SSD
│
├── GUID Partition Map
│
└── APFS Container
      │
      ├── Macintosh HD
      ├── Macintosh HD - Data
      ├── Recovery
      └── Any other APFS volumes

You might notice that mac creates a container ontop of the physical drive, and before the actual filesystem
called APFS Volumes


Older systems, like HFS or NTFS worked like this:

Disk
├── Partition 1 (100 GB)
├── Partition 2 (200 GB)
└── Partition 3 (500 GB)

Thus, if one partition became full, while the others had a lot of space, you would have to resize the partitions, which is
slow and risky

- Instead, the container owns all of the storage, and all the volumes share the available space dynamically.
This way, the system and your data can live on separate volumes while sharing the same free space. good for isolation, but still nice for dymanic space allocatin.

- Also, recovery volumes do not need fixed sizes, which is great to not waste space

- You can also create new volumes instantly without thinking about how to allocate the space




### Macintosh HD vs Macintosh HD - Data

HD stores read-only system files
HD - data stores applications, documents, and settings

Samsung 990 (Physical SSD)
└── Container disk5 (APFS Container)
    └── main_backup (APFS Volume)

(how it looks for me, for example)

to make more partitions, click the + icon and then add more partitions, (be sure to have selected the container)

can create volumes inside of volumes (however its misleading, as in reality it is adding another volume in the container)

