//Copyright 2015 <>< Charles Lohr, see LICENSE file.

// JA Remove comments later
// JA: Re-structured code to be per single output of WS chip rather than per RGB tripplet.
//   the WS IC is just a long shift register, it is *often* the case that each IC is driving a single
//   RGB LED (as per its design) and it is perfectly sensible to assume so for the web front end.  
//   In my opinion though the underlying code should be per single WS output (LED) rather than per 
//   tripplet (Pixel) It is possibe to have only monochrome LEDs or missuse the IC as a power 
//   LED PWM driver, through the magic of ebay and my soldering iron such things are available :-)

 
#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "espconn.h"
#include "mystuff.h"
#include "ws2812_i2s.h"
#include "commonservices.h"
#include <mdns.h>


#define procTaskPrio        0
#define procTaskQueueLen    1
#define FLASH_AFTER_N_SECONDS	2

static volatile os_timer_t some_timer;
static struct espconn *pUdpServer;
uint8_t last_leds[512*3];

int idle_counter=FLASH_AFTER_N_SECONDS;					// JA Sits at 0 when no UDP data has been received for N seconds

//JA: ptototypes, correct place ?
void ICACHE_FLASH_ATTR dmx_init();
void ICACHE_FLASH_ATTR dmx_send( uint8_t * buffer, uint16_t buffersize );


int number_of_leds=0;			// JA number of 8 bit brightness values in the last UDP packet received, 0 if no data yet or error

//int ICACHE_FLASH_ATTR StartMDNS();



// pointer, size
void ICACHE_FLASH_ATTR update_lights(void *p, int s)
{
	if (s>0)
	{
		ws2812_push( p, s );
		if ( SETTINGS.flag_send_DMX==TRUE ) 
		{
			if (idle_counter!=0) 				// Dont flash DMX fixture just the WS28XX LEDs 
				dmx_send (p, s);
		}
		else	uart0_sendStr("X");				// Uart TX not DMX so use for debug messages
	}
}



void user_rf_pre_init(void)
{
	//nothing.
}


char * strcat( char * dest, char * src )
{
	return strcat(dest, src );
}



//Tasks that happen all the time.
os_event_t    procTaskQueue[procTaskQueueLen];
static void ICACHE_FLASH_ATTR procTask(os_event_t *events)
{
	CSTick( 0 );
	system_os_post(procTaskPrio, 0, 0 );
}


// 10Hz timer
static void ICACHE_FLASH_ATTR myTimer_cb(void *arg)
{
	static int sc=0;					// slow counter
	static uint8 onoff=0;

	sc++;
	if (sc>5)						// Half second 
	{
		sc=0;
		if (idle_counter>0)
			idle_counter--;
		else
		{						// idle timer expired, flash first RGB LED at 1/2Hz
			~onoff;
			if (onoff==1)
			{
				last_leds[0]=64;
				last_leds[1]=64;
				last_leds[2]=64;
			}
			else
			{
				last_leds[0]=0;
				last_leds[1]=0;
				last_leds[2]=0;
			}
		}
	}
	CSTick( 1 );
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR
udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;

	if (SETTINGS.Firstled < len)
	{
		number_of_leds=len-SETTINGS.Firstled;
		ets_memcpy( last_leds, pusrdata+SETTINGS.Firstled, number_of_leds );
	}
	else	number_of_leds=0;				// If Firstled is beyond the packet end then its invalid

	update_lights( pusrdata+SETTINGS.Firstled, number_of_leds );
	idle_counter=FLASH_AFTER_N_SECONDS;
}

void ICACHE_FLASH_ATTR charrx( uint8_t c )
{
	//Called from UART.
}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	if (SETTINGS.flag_send_DMX==TRUE)
		dmx_init();							// Override uart settings with new ones
	else	uart0_sendStr("\r\n\033cesp8266 ws2812 driver\r\n");

//Uncomment this to force a system restore.
//	system_restore();

	CSSettingsLoad( 0 );
	CSPreInit();

        pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
	espconn_create( pUdpServer );
	pUdpServer->type = ESPCONN_UDP;
	pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pUdpServer->proto.udp->local_port = SETTINGS.UDP_port;
	espconn_regist_recvcb(pUdpServer, udpserver_recv);

	if( espconn_create( pUdpServer ) )
	{
		while(1) { uart0_sendStr( "\r\nFAULT\r\n" ); }
	}

	CSInit();

	SetServiceName( "ws2812" );
	AddMDNSName( "cn8266" );
	AddMDNSName( "ws2812" );
	AddMDNSService( "_http._tcp", "An ESP8266 Webserver", 80 );
	AddMDNSService( "_ws2812._udp", "WS2812 Driver", 7777 );
	AddMDNSService( "_cn8266._udp", "ESP8266 Backend", 7878 );

	//Add a process
	system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);

	//Timer example
	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *)myTimer_cb, NULL);
	os_timer_arm(&some_timer, 100, 1);

	ws2812_init();

	idle_counter=FLASH_AFTER_N_SECONDS;
	os_bzero(&last_leds,sizeof(last_leds));
	update_lights(&last_leds,sizeof(last_leds));

	printf( "Boot Ok.\n" );

	system_os_post(procTaskPrio, 0, 0 );
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void EnterCritical()
{
}

void ExitCritical()
{
}


