# Project 1: 
## Concurrency and Message Passing

'n' worker processes read a binary file and sort the 64 bit unsigned positive integers in a parallel manner.
To merge results, we are utilizing posix messaging queues.

## Simple Kernel Module

We can program our own kernel module easily and insert it into the Linux kernel. Our kernel module is inserted we can perform any privileged action as our program is running in the kernel mode. We are inspecting a process with the given ID in this module.
To compile a kernel module, you need to have linux headers in your machine. Its not that hard just google "Installing linux headers" and stackoverflow will probably do its job.


## Usage

### CSORT
_let n be the number of concurrent processes you want to run._  
_input file should be in binary format, you can generate a simple binary file with binary_generator.c_  
_modify it as you wish_  
```bash
gcc -o csort csort.c
csort <n> <input_file> <output_file>
``` 

### SIMPLE KERNEL MODULE
Root access is required.
```bash
make #compile kernel module  
insmod modpcb.ko param_pid=XXX #to insert our lovely module and specify a process id to inspect, (change XXX)   
dmesg | tail #to see what our lovely kernel module says when inserted  
rmmod modpcb #lets remove it from the kernel  
dmesg | tail #to see what our cute kernel module says when leaving us :(  
```


