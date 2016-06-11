// DMX512 on GPIO2

#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "espconn.h"


#define DMX_USIZE	512


void ICACHE_FLASH_ATTR dmx_init()
{
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
        uart_div_modify(1, UART_CLK_FREQ/250000);
        WRITE_PERI_REG(UART_CONF0(1), (STICK_PARITY_DIS)|(TWO_STOP_BIT << UART_STOP_BIT_NUM_S)| \
                        (EIGHT_BITS << UART_BIT_NUM_S));
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
		

        //BREAK
        gpio_output_set(0, BIT2, BIT2, 0);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
        os_delay_us(150);

        //MARK
        gpio_output_set(BIT2, 0, BIT2, 0);
        os_delay_us(54);

        //START CODE + DMX DATA
        uart_tx_one_char(1, buffer[0]);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
        os_delay_us(54);

        //DMX data
        for (i = 1; i < ch; i++)			// ch-1 maybe 
        {
                uart_tx_one_char(1, buffer[i]);
                os_delay_us(54);
        }
}

