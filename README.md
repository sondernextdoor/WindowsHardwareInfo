# Windows-Hardware-Info
A project written in C++ to get hardware info on a Windows PC. Interfaces with the Windows Management Instrumentation (WMI) service to query hardware info of interest and provides a basic command line interface. 

Retrieves the following:

Disks
-------------

Serial Number

Model

Interface Type

Drive Letter

Size

Free Space

Media Type

IsBootDrive 


System Management BIOS (SMBIOS)
-------------
Serial Number

Manufacturer

Product

Version


GPUs
-------------
Name

Driver Version

Resolution

Refresh Rate


CPU
-------------
Processor Id

Manufacturer

Name

Number of Cores

Number of Logical Processors


Network Adapters
-------------
Name

MAC Address


System
------------
Name

IsHypervisorPresent

OS Version

OS Name

OS Architecture

OS SerialNumber


Physical Memory
-------------
Part Number


Registry
-------------
Computer Hardware Id


Command Line
-------------
The command line interface is independent from the hardware logic and is only used to demonstrate functionality. All relevant hardware logic is in hwid.h

Valid Commands:

"disk" - prints all disk info

"smbios" - prints all SMBIOS info

"gpu" - prints all GPU info

"cpu" - prints all CPU info

"network" - prints all network adapter info

"system" - prints all system info

"physicalmemory" - prints all physical memory info

"registry" - prints all registry info

"all" - prints all info

"help" - opens this webpage on the default browser

"exit" - exits the program


Each of the above commands (except "all", "help" and "exit") can be used to retrieve fields from the hardware component they specify. To do so, the "get" keyword must follow one of the above commands. Valid field(s) should be specified after "get"

For example, "disk get serialnumber" would retrieve the disk serial number(s), while "disk get serialnumber, model" would retrieve the disk serial number(s), as well as the model(s).

Valid fields are as follows:

Disk:

`serialnumber,
model,
interface,
driveletter,
size,
freespace,
mediatype,
isbootdrive`


SMBIOS:

`serialnumber,
manufacturer,
product,
version`


GPU:

`name,
driverversion,
resolution,
refreshrate`


CPU:

`processorid,
manufacturer,
name,
cores,
threads`


Network:

`name,
mac`


System:

`name,
ishypervisorpresent,
osversion,
ostitle,
osarchitecture,
osserialnumber`


Physical Memory:

`partnumber`


Registry:

`computerhardwareid`
