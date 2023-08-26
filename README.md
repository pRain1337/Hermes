# Hermes

*Hermes was a master communicator, and thus he was able to persuade everyone, god or mortal being.*

## Table of contents
- [Intro](#intro)
- [Commands](#commands)
- [Examples](#examples)
  * [Dumping process memory](#dumping-process-memory)
  * [Reading LsaIso.exe memory](#reading-credential-guard-protected-memory)
- [Detection](#detection)
  * [MSR_SMI_COUNT](#msr_smi_count)
  * [Side channel cache detection](#side-channel-cache-detection)
  * [UEFI Image analysis](#uefi-image-analysis)
- [Repository Contents](#repository-contents)
  * [Hermes-SMM](#hermes-smm)
  * [Hermes-Client](#hermes-client)
- [Compiling](#compiling)
  * [First time setup](#first-time-setup)
  * [Building Hermes-SMM](#building-hermes-smm)
  * [Building Hermes-Client](#building-hermes-client)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
  * [How to patch PiSmmCpuDxeSmm](#how-to-patch-pismmcpudxesmm)
  * [No serial output after boot](#no-serial-output-after-boot)
- [Sources](#sources)
- [License](#License)
- 
## Intro

Hermes is a PoC demonstrating how a rootkit running in System Management Mode
(SMM) can be used by a user mode process to elevate it's own privileges higher than the kernel itself. 

If you want to read about SMM rootkits in general and how the previous version
of the [SMM rootkit](https://github.com/jussihi/SMM-Rootkit) was implemented,
please read the [blog post](https://jussihi.kapsi.fi/2022-09-08-smmrootkit/)
about it!

This version of the rootkit contains the basic components to interact with the windows kernel (find processes, get all modules of a process, get information about a process module and dump a memory range to file) or the basic memory procedures (read/write virtual & physical memory and convert virtual to physical memory).
It can be expanded to include automatic forensic actions as example.
**Supports:**
- Windows 11 22H2 - Windows 10 1809

As the usermode application itself does not interact with any of the processes or the memory itself as the smm rootkit is doing everything while the processor is in the system management mode, anti-viruses or even the kernel itself wont notice the behavior but only the effects that were done (as example, through DKOM).

Created by Jussi Hietanen (Aalto University, Espoo FI) and Diego Caminada
(HF-ICT, Muttenz CH).

## Commands

| Commad | Description | Input | Output |
|---|---|---|---|
| gd | Returns the directory base of the requested process  | Processname | Directory Base |
| gmd | Returns essential information of the requested module in a process | Processname & Modulename | Modulename & Size |
| gm | Returns all module names of the requested process  | Processname | Name of every processmodule |
| vr | Reads the memory at the requested virtual memory address | Source Virtual address, Directory Base & Size  | Memory read at the address |
| vw | Writes the supplied integer to the requested virtual memory address | Destination Virtual address, Directory Base, Size & Value | - |
| pr | Reads the memory at the requested physical memory address | Source Physical address, Directory Base & Size  | Memory read at the address |
| pw | Writes the supplied integer to the requested physical memory address | Destination Physical address, Directory Base, Size & Value | -  |
| vtp | Converts a virtual memory address to physical | Source Virtual address & Directory Base | Converted Physical address  |
| dm | Dumps the requested memory area | Source Virtual address, Directory Base, Size & File name | Memory read is written into the file |
| exit | Exits the client process | - | - |
| help | Displays the help about the commands | - | - |

## Examples
The following examples show basic usage of the smm rootkit.

### Dumping process memory
Hermes can be used to dump a memory range of a file, this is useful for reverse engineering of otherwise protected processes which cant be read normally or close themself as soon as they detect a debugger or reverse engineering toolkit.

The following video shows hermes in action dumping putty.exe:


https://github.com/pRain1337/Hermes/assets/26672236/58891cd8-1923-4247-88dd-1556a82d9e25



### Reading Credential Guard protected memory
_LsaIso.exe_ is the credential guard protected version of _Lsass.exe_ which stores and protects credentials.
Normal toolkits (as example cheat engine) are unable to read the virtual memory of _LsaIso.exe_ as the windows hypervisor is blocking their access, hermes fully bypasses this protection as it is not running under the hypervisor.

The following video shows hermes reading LsaIso.exe memory:


https://github.com/pRain1337/Hermes/assets/26672236/23bac20d-eb8a-49c0-b46a-753e42b08bf0



## Detection
The following examples show ideas and approaches to detect the activitys of SMM, but not in general malicious behavior.
Most of these could be evaded by utilizing additional components in the rootkit.

### MSR_SMI_COUNT
#### How it works
The MSR_SMI_COUNT increases everytime there is a active System management interrupt. For the best functionality, a rootkit would want to get regular execution which is not usual behavior.
By checking the counter, you can notice if a rootkit has enabled a timer and SMM gets more execution time than it would normally get.
For instructions on how to read the MSR_SMI_COUNT see [chipsec](https://github.com/chipsec/chipsec/tree/main) which also provides other useful information for analyzing UEFI/SMM.

#### How to prevent it
Spoofing a MSR is not easily done with SMM, the easiest approach would be to utilize a hypervisor to spoof the results of the MSR.
Using a hypervisor will ofcourse end in a lot of more flags.

### Side channel cache detection
#### How it works
Based on the idea of [Nick Peterson](https://twitter.com/nickeverdox/) which he presented in a [Tweet](https://web.archive.org/web/20211229210102/https://twitter.com/nickeverdox/status/1476295914423656456) to use side channels to detect reads of a memory. 
[H33p](https://github.com/h33p) has created a replication of this project in rust, to test this claim. 
The initial tweet only claimed detection from os or hypervisor, but we've tested it from smm and it was also able to detect it.
It basically works by checking the access time to a predefined memory location. As long as no other application touches the memory, the cache duration should stay the same.
The test itself can introduce false positives if anti-virus or similar applications are running on the system.
Old video with the private version of hermes (named atlas):


https://github.com/pRain1337/Hermes/assets/26672236/91ded9c8-983b-4741-bae4-32eb7125d279


- At 0:22 the virtual read function is used
- At 0:37 the physical read function is used

#### How to prevent it
This detection can be bypassed using the [control register 0](https://wiki.osdev.org/CR0#CR0).
By setting the _Cache disable_ and _Not-write through_ bit before performing a read and disabling these afterwards.
This results in a big performance hit as no caching is used anymore but the reads themselves wont be detected anymore.

### UEFI Image analysis
#### How it works
The SPI chip holds the image which contains the smm rootkit module, the chip itself can be read using manufacturer provided tools (as example afudos).
These images can then be analyzed using as example [UEFITool](https://github.com/LongSoft/UEFITool)

#### How to prevent it
Easiest way is to simply just block the reading of the SPI chip utilizing the protected range registers.
For more information about how to achieve this, check out the [x86-64 Intel Firmware Attack & Defense course](https://p.ost2.fyi/courses/course-v1:OpenSecurityTraining2+Arch4001_x86-64_RVF+2021_v1/about) by Xeno Kovah.

Blocking it ofcourse leaves a red flag, as this is not expected behavior.
A better looking approach would be utilizing SMM as a MitM to modify the read SPI data before it's passed to the user mode application.
This was already done by [TakahiroHaruyama](https://github.com/TakahiroHaruyama), for further information see [SpiMitm](https://github.com/TakahiroHaruyama/SpiMitm/).

## Repository Contents

### Hermes-SMM
Hermes-SMM is the server component which allows client applications to elevate
its rights.

### Hermes-Client
Hermes-Client is a interactive console application which can be used to
communicate with the smm rootkit.

## Compiling

Prerequisites: docker, git

### First time setup

**Please skip to "Building" if you've done the first time setup already.**

1. Inside this git repo, clone the EDK2 submodule, it will init the right tag
(vUDK2018) automatically:

```
git submodule update --init
```

2. Inside the `Hermes-SMM` -directory, run

```
docker build --no-cache -t edk-builder/edk-builder .
```

to build the `edk-builder` image.

3. Inside the `Hermes-SMM` -directory, start the docker by running:

```
docker run -it --privileged -v .:/root/ -u root -w /root edk-builder/edk-builder /bin/bash
```

Now inside the docker container, run

```
# cd edk2 
# make -C BaseTools
# . edksetup.sh
```

To set up the base tools. First time setup is done!

### Building Hermes-SMM

To build the Hermes SMM module, start the docker instance again in `Hermes-SMM`
directory with the following command:

```
docker run -it --privileged -v .:/root/ -u root -w /root edk-builder/edk-builder /bin/bash
```

Inside docker container, copy Hermes source files to the edk2 folder:

```
# cp -r Hermes-src/* edk2/
```

Still inside the running docker instance, you can proceed to building the
edk2's OVMF with SMM modules enabled

```
# cd edk2
# . edksetup.sh
# build -DSMM_REQUIRE
```

The resulting OVMF firmware will be inside `edk2/Build/OvmfX64/RELEASE_GCC5/FV`.

For running it on real hardware, you'll have to patch PiSmmCpuDxeSmm. Modern SMM protections setup by edk2 will produce a fault otherwise when accessing normal OS memory. Check out [How to patch PiSmmCpuDxeSmm](#how-to-patch-pismmcpudxesmm)

### Building Hermes-Client

To build the Hermes-Client you dont need docker, but visual studio.
Simply open the Solution file and compile, you might need platform/sdk/toolset on the project file first.
The default settings compile a x64-86 application for windows.

## Troubleshooting
This version was developed for and tested on the Intel z690 chipset, it should still work on older/newer chipsets as very few offsets are changed.
The rootkit itself also works in a virtualized environment, altough some of the native chipset feature it uses (SMI timer) are not available there.
If the rootkit is not working as expected, you can use the inbuilt serial functions to send text/numbers out. This even works, if the rootkit produces a General Exception which halts the system.

## FAQ

### How to patch PiSmmCpuDxeSmm

If you are trying to run this SMM rootkit on real hardware, you need to patch your motherboard's `PiSmmCpuDxeSmm` module from the UEFI firmware. You can mimick [our patch](https://github.com/jussihi/SMM-Rootkit/tree/master/SMM%20Rootkit/UefiCpuPkg) by 

Patching this variable initialization out and hard code the variable itself to 0 with your favorite disassembler (IDA or similar):
https://github.com/tianocore/edk2/blob/master/UefiCpuPkg/PiSmmCpuDxeSmm/X64/PageTbl.c#L352

Easiest way to find that function (SmmInitPageTable) is to search for the strings of the error messages:
https://github.com/tianocore/edk2/blob/master/UefiCpuPkg/Library/CpuExceptionHandlerLib/X64/ArchExceptionHandler.c#L265

Which is referenced multiple times in the SMI Page fault handler:
https://github.com/tianocore/edk2/blob/master/UefiCpuPkg/PiSmmCpuDxeSmm/X64/PageTbl.c#L1003

And the page fault handler is initialized in the same function as the variable initialization (SmmInitPageTable):
https://github.com/tianocore/edk2/blob/master/UefiCpuPkg/PiSmmCpuDxeSmm/X64/PageTbl.c#L442

### No serial output after boot

Sometimes the serial traffic is blocked as the operating system's own serial driver. This is at least the case in Windows systems when not booting inside a (QEMU/KVM) virtual machine.

#### There are two ways to get serial working on this sort of situation:

- Block the Operating System from loading the driver.

On GNU+Linux, you can disable the driver completely if one is loaded. On Windows systems, you might need to rename/delete the system's serial driver. The default path to the driver executable is  `C:\Windows\System32\drivers\serial.sys`

- Open an SSH client locally

You can also open the connection to the local serial port using your favorite serial client. At least on Windows this will prevent Windows' own driver from suppressing the serial output.

## Sources
Sources used while building the rootkit as inspiration and their usage:
Basic inspiration for this project - [SmmBackdoor](https://github.com/Cr4sh/SmmBackdoor)

Used for windows structures and memory functions - [vmread](https://github.com/h33p/vmread)

Interaction with the os and interacting with it - [pcillech](https://github.com/ufrisk/pcileech)

## License 
