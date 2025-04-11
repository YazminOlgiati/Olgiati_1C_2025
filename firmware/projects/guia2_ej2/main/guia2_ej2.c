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

/*==================[inclusiones]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include <led.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "timer_mcu.h"

/*==================[macros and definitions]===================================*/

#define CONFIG_SENSOR_TIMERA 1000000
uint16_t distancia;
bool encendido = true;
bool hold = false;

/*==================[internal data definition]===========================*/
TaskHandle_t operar_distancia_task_handle = NULL;

/*==================[internal functions declaration]=======================*/

void FuncTimerA(void* param){
	vTaskNotifyGiveFromISR(operar_distancia_task_handle, pdFALSE);
}

static void OperarConDistancia(void *pvParameter){
    while(true){
		// La tarea esta en espera (bloqueada) hasta que reciba una notificación mediante ulTaskNotifyTake
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		
		if(encendido){
			// Medir distancia
			distancia = HcSr04ReadDistanceInCentimeters();

			// Manejar LEDs según la distancia
			if(distancia < 10){
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (distancia >= 10 && distancia < 20){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if(distancia >= 20 && distancia < 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if(distancia >= 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}

			// Mostrar distancia en la pantalla LCD si no está en hold
			if(hold){
				LcdItsE0803Write(distancia);
			}
		}
		else{
			// Apagar LEDs y pantalla LCD si on está apagado
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
			LcdItsE0803Off();
		}
		//vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}

static void Interrupciontecla1(void){
		
		encendido = !encendido;
}

static void Interrupciontecla2(void){
		
		hold = !hold;
}

/*==================[external functions definition]========================*/

void app_main(void){
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();
	SwitchesInit();
	// timer_config_t: configura el temporizador definiendo su periodo y la función que se llamará cuando el temporizador 
	//se dispare.
	timer_config_t timer_sensor = {
        .timer = TIMER_A,
        .period = CONFIG_SENSOR_TIMERA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_sensor);
	// Se habilitan interrupciones cuando se presionan las teclas SWITCH_1 y SWITCH_2 (desencadena OperarConTeclado)
	SwitchActivInt(SWITCH_1, &Interrupciontecla1, NULL );
	SwitchActivInt(SWITCH_2, &Interrupciontecla2, NULL );

	xTaskCreate(&OperarConDistancia, "OperarConDistancia", 2048, NULL, 5, &operar_distancia_task_handle);
	
	TimerStart(timer_sensor.timer);
}
/*==================[end of file]============================================*/