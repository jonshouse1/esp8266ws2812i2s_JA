#ESP8266 I2S WS2812 Driver with optional DMX512 

Modified version of ESP8266 WS2812 driver https://github.com/cnlohr/esp8266ws2812i2s
Really this should have been a fork but I am new to github


Lots of hacking here, do not try and use.

	Changes from Charles Lohr oroginal code:

		2nd LED (Green of first pixel) flashes and all other LEDs are off after 4 seconds of no UDP data reception

		Added optional DMX512 transmit on GPIO2 (untested)



	Todo:
		rename last_leds to just 'leds' or even 'lights'
		Work out how web front end works, add new variables in a "Light Settings" Tab
		Do the variables need to go inside UserData ?


	Bugs:
		When going idle the last_leds array seems to have crap in it



