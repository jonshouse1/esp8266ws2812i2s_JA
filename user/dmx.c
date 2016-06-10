// Placeholder for DMX512 

#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "espconn.h"


#define DMX_USIZE	512


//TODO: 
	// initialise UART with 250k baud
	// send break, mark after break then N bytes of brightness data

void ICACHE_FLASH_ATTR dmx_init()
{
}


// Clip at brightness value 512 as that is the maxmim DMX universe size, though 513 bytes are used in the 
// dmx packet, the first one nearly always being 0
void ICACHE_FLASH_ATTR dmx_send( uint8_t * buffer, uint16_t buffersize )
{
	int i;
	int ch=0;

	if (buffersize<DMX_USIZE)
		ch=buffersize;
	else	ch=DMX_USIZE;				// less than or equal to 512 values
		
	for (i=0;i<buffersize;i++)
	{
		// dmx send
	}
}

