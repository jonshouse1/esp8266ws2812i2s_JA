//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <commonservices.h>

extern uint8_t last_leds[512*3];
extern int number_of_leds;


int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;

	switch( pusrdata[1] )
	{
	case 'C': case 'c': //Custom command test
	{
		buffend += ets_sprintf( buffend, "CC" );
		return buffend-buffer;
	}

// JA Note: I dont entirely get what this is for so may now be broken
	case 'l': case 'L': //LEDs
	{
		int i, it = 0;
		buffend += ets_sprintf( buffend, "CL:%d:", number_of_leds );
//I get lost around here
// TODO: Fix me

		uint16_t toledsvals = number_of_leds/3;
		if( toledsvals > 600 ) toledsvals = 600;
		for( i = 0; i < toledsvals; i++ )
		{
			uint8_t samp = last_leds[it++];
			*(buffend++) = tohex1( samp>>4 );
			*(buffend++) = tohex1( samp&0x0f );
		}
		return buffend-buffer;
	}


	}
	return -1;
}
