//Copyright 2015 <>< Charles Lohr, see LICENSE file.

// JA modified
//	Second LED (first pixel, green) flashes when no UDP data has been recieved for 4 seconds
//	Added template for DMX send on TX pin 
 
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
#define FLASH_AFTER_N_SECONDS	3 * 2					// Unit is really half seconds so multiply by 2

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
void ICACHE_FLASH_ATTR update_lights(uint8_t *p, int s)
{
	//printf("update_lights: size=%d  [0]=%d\t[1]=%d\t[2]=%d\t[3]=%d\n",s,p[0],p[1],p[2],p[3]);
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
	if (sc>4)						// Half second 
	{
		sc=0;
		if (idle_counter>0)
			idle_counter--;
		else
		{						// idle timer expired, flash first RGB LED at 1/2Hz
			os_bzero(&last_leds,sizeof(last_leds));	// No idea why the array has crap in it at this point
			onoff=onoff^1;
			if (onoff==1)
			{
				//last_leds[0]=16;
				last_leds[1]=16;
				//last_leds[2]=16;
			}
			else
			{
				last_leds[0]=0;
				last_leds[1]=0;
				last_leds[2]=0;
			}
			update_lights( last_leds, sizeof(last_leds) );
		}
	}
	CSTick( 1 );
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR
udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;

SETTINGS.Firstled=0;
	if (SETTINGS.Firstled < len)
	{
		number_of_leds=len-SETTINGS.Firstled;
		ets_memcpy( last_leds, pusrdata+SETTINGS.Firstled, number_of_leds );
	}
	else	number_of_leds=0;				// If Firstled is beyond the packet end then its invalid

	update_lights( last_leds, number_of_leds );
	idle_counter=FLASH_AFTER_N_SECONDS;
}

void ICACHE_FLASH_ATTR charrx( uint8_t c )
{
	//Called from UART.
}

void user_init(void)
{
	int i;
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(10000);
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
	//pUdpServer->proto.udp->local_port = SETTINGS.UDP_port;
	pUdpServer->proto.udp->local_port = 7777;
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
	update_lights((uint8_t*)&last_leds,sizeof(last_leds));

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


