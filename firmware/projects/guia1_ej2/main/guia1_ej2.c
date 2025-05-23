/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * Este proyecto hace titilar los leds 1 y 2 al mantener presionada las teclas 1 y 2 correspondientemente. 
 * También hace titilar el led 3 al presionar simultáneamente las teclas 1 y 2.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/03/2025 | Document creation		                         |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
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
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
    while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

    		break;
			case SWITCH_1 | SWITCH_2:
				LedOff(LED_1 | LED_2);
				LedToggle(LED_3);
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

			break;
		}
		
	   
	}

}



/*Retención*/

/*void app_main(void){
    uint8_t teclas;
    bool flag_switch1 = false;
    bool flag_switch2 = false;

    LedsInit();
    SwitchesInit();

    while(1) {
        teclas = SwitchesRead();

        if (teclas & SWITCH_1) {  // Usa bitwise AND para detectar si el botón está presionado
            flag_switch1 = !flag_switch1;
        }

        if (teclas & SWITCH_2) {  
            flag_switch2 = !flag_switch2;
        }

        // Lógica para controlar los LEDs
        if (flag_switch1 && !flag_switch2) {
            LedToggle(LED_2);
        } else if (flag_switch2 && !flag_switch1) {
            LedToggle(LED_1);
        }

	 	// Parpadeo del LED 3 (no depende de los switches)
        LedToggle(LED_3);
        
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }

}
*/