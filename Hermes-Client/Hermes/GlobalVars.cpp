#include "GlobalVars.h"

/*
 *   Defines if debug output should be printed out
 *
*/ 
bool debug = false; 

/*
 *   Memory identifier used by smm to find the communication buffer
 *
*/
unsigned char identifier[160] = {
0x0f, 0x2b, 0x34, 0x7f, 0x58, 0x40, 0x22, 0x61, 0x9c, 0xcc, 0x0f, 0xfa, 0x44, 0x0f, 0x59, 0x61,
0x40, 0x8b, 0x37, 0x6e, 0x8f, 0xe9, 0x1d, 0x2d, 0x81, 0x12, 0x65, 0x48, 0x6b, 0x8b, 0xcb, 0x0c,
0xa8, 0x25, 0x6a, 0xac, 0x9e, 0x66, 0xe3, 0x5d, 0x14, 0x24, 0x19, 0x45, 0x75, 0xab, 0xe8, 0xa1,
0x5a, 0x89, 0x99, 0xbe, 0xf9, 0xcc, 0x71, 0x81, 0x98, 0xe1, 0xce, 0x07, 0x9b, 0xc8, 0x59, 0x49,
0xa7, 0x5b, 0xfe, 0xa4, 0x05, 0x6a, 0xa6, 0x70, 0xd3, 0xe9, 0x0e, 0x85, 0xb3, 0xb1, 0x43, 0xde,
0xdc, 0xef, 0x3b, 0xfe, 0x0d, 0x20, 0xf1, 0xc6, 0x65, 0x92, 0x20, 0x97, 0xad, 0xa7, 0xcc, 0x32,
0x46, 0x05, 0x5c, 0xc9, 0xf2, 0xa1, 0xb8, 0x79, 0x92, 0x34, 0x03, 0xa4, 0x17, 0x20, 0x12, 0x0b,
0x35, 0x85, 0x81, 0x98, 0xe8, 0x76, 0xf5, 0x61, 0x0b, 0x7c, 0xe7, 0xfc, 0xcf, 0x97, 0x88, 0x81,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // this

/*
 *   Identifiers used in the packets to communicate with smm.
 *
*/
uint16_t start_identifier = 0xf345;
uint16_t end_identifier = 0xa13f;