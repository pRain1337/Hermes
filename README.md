# Hermes

## Introduction
*Hermes was a master communicator, and thus he was able to persuade everyone, god or mortal being.*

## Table of contents
- [Intro](#intro)
- [Repository Contents](#repository-contents)
- [Compiling](#compiling)

## Intro

Hermes is PoC demonstrating how a System Management Mode (SMM) rootkit can be used to elevate context using smm. 

If you want to read about SMM rootkits in general and how the previous version of the [SMM rootkit](https://github.com/jussihi/SMM-Rootkit) was implemented, please read the [blog post](https://jussihi.kapsi.fi/2022-09-08-smmrootkit/) about it!

For a more practical application of the rootkit, check out [Apollo](https://github.com/pRain1337/Apollo) which does not require any running component on the operating system.

Created by Jussi Hietanen (Aalto University, Espoo FI) and Diego Caminada (HF-ICT, Muttenz CH).

## Commands

| Commad | Description | Input | Output |
|---|---|---|---|
| gD | Returns the directory base of the requested process  | Processname | Directory Base |
| gMD | Returns essential information of the requested module in a process | Processname & Modulename | Modulename & Size |
| gM | Returns all module names of the requested process  | Processname | Name of every processmodule |
| vR | Reads the memory at the requested virtual memory address | Source Virtual address, Directory Base & Size  | Memory read at the address |
| vW | Writes the supplied integer to the requested virtual memory address | Destination Virtual address, Directory Base, Size & Value | - |
| pR | Reads the memory at the requested physical memory address | Source Physical address, Directory Base & Size  | Memory read at the address |
| pW | Writes the supplied integer to the requested physical memory address | Destination Physical address, Directory Base, Size & Value | -  |
| vTp | Converts a virtual memory address to physical | Source Virtual address & Directory Base | Converted Physical address  |
| dM | Dumps the requested memory area | Source Virtual address, Directory Base, Size & File name | Memory read is written into the file |
| exit | Exits the client process | - | - |
| help | Displays the help about the commands | - | - |

## Examples

### Evading HVCI

## Detection

### MSR_SMI_COUNT

### Side channel cache detection

### UEFI Image analysis

## Repository Contents

### Hermes-SMM
Hermes-SMM is the server component which allows client applications to elevate its rights.

### Hermes-Client
Hermes-Client is a interactive console application which can be used to communicate with the smm rootkit.

## Compiling

## Sources