/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 0
#define TOGGLE 2

/*==================[internal data definition]===============================*/

typedef struct 
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;       //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   //indica el tiempo de cada ciclo
} leds;


/*==================[internal functions declaration]=========================*/
 
void funcion_leds(leds*mis_leds){

	switch (mis_leds->mode)
	{
	case ON:
		LedOn(mis_leds->n_led);
		break;
	
	case OFF:
		LedOff(mis_leds->n_led);
		break;

	case TOGGLE:
		for(size_t i=0; i < mis_leds->n_ciclos; i++){
			LedToggle(mis_leds->n_led);
		

			for(size_t j=0; j < mis_leds->periodo; j++){
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		
		}

		break;
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){

	LedsInit();

	leds led_1, led_2, led_3;
	led_1.n_led = LED_1;
	led_2.n_led = LED_2;
	led_3.n_led = LED_3;

	led_1.mode = ON;
	led_2.mode = OFF;

	led_3.mode = TOGGLE;
	led_3.n_ciclos = 10;
	led_3.periodo = 500;

	funcion_leds(&led_1);
	funcion_leds(&led_2);
	funcion_leds(&led_3);

	
}
/*==================[end of file]============================================*/