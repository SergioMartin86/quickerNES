#pragma once

// CPU Byte Order Utilities
// Nes_Emu 0.7.0

inline unsigned get_be16( void const* p ) {
	return  ((unsigned char*) p) [0] * 0x100u +
			((unsigned char*) p) [1];
}
inline unsigned long get_be32( void const* p ) {
	return  ((unsigned char*) p) [0] * 0x01000000ul +
			((unsigned char*) p) [1] * 0x00010000ul +
			((unsigned char*) p) [2] * 0x00000100ul +
			((unsigned char*) p) [3];
}
inline void set_be16( void* p, unsigned n ) {
	((unsigned char*) p) [0] = (unsigned char) (n >> 8);
	((unsigned char*) p) [1] = (unsigned char) n;
}
inline void set_be32( void* p, unsigned long n ) {
	((unsigned char*) p) [0] = (unsigned char) (n >> 24);
	((unsigned char*) p) [1] = (unsigned char) (n >> 16);
	((unsigned char*) p) [2] = (unsigned char) (n >> 8);
	((unsigned char*) p) [3] = (unsigned char) n;
}

#define GET_LE16( addr )        (*(uint16_t*) (addr))
#define SET_LE16( addr, data )  (void) (*(uint16_t*) (addr) = (data))
#define SET_LE32( addr, data )  (void) (*(uint32_t*) (addr) = (data))

#ifndef GET_BE16
	#define GET_BE16( addr )        get_be16( addr )
	#define GET_BE32( addr )        get_be32( addr )
	#define SET_BE16( addr, data )  set_be16( addr, data )
	#define SET_BE32( addr, data )  set_be32( addr, data )
#endif