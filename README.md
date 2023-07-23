# About

Support for a simple FAT-based filesystem and a short example that uses it.
The entire file system occupies 3kB and is contained in a file that acts as the entire disk. Disc consists of 30 clusters of 100 bytes each.

The first cluster contains only information about the disk: name, size, number of clusters and number of files.

The second cluster contains the FAT table for whole disk, where each byte denotes a cluster on the disk and contains the following information:
- 0x00: cluster is free.
- 0x01-0x29: indicates the next cluster in the chain. 
- 0xFF: the file is not continued to the next cluster.

The third cluster is called root directory and contains information about all files, their name, size and position in the file system.

Program contains following functions:
- mount(disc_name): opens the disc and puts everything into memory. If disc does not exist it creates new file and formats it.
- unmount(): saves the changes and destroys the handle.
- open(file_name): opens a file, if it does not exist, creates a new one. Returns a handle.
- write(file): writes data to the file.
- read(file): read data from file.
- delete(file): deletes the file from the file system.
- close(file): destroys the handle.

Subdirectories are not suported. Error is returned when the disk is full, i.e. when no new file can not be opened or written to an existing file. 
A file always takes up a whole cluster or more, the length of the file is recorded in the directory next to its name. 
If the root directory grows beyond the size of the cluster, it must also be possible to move it to a new cluster.

In example, 7 files are created that fill entire disc. Then second and fourth file are deleted and a new one is opened and written again until the disk is full.
