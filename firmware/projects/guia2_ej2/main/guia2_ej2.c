/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Se modifica la actividad del punto 1 de manera de utilizar 
 * interrupciones para el control de las teclas 
 * y el control de tiempos (Timers). 
 *
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
 * | 24/04/2025 | Document creation		                         |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
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

/** 
 * @def CONFIG_SENSOR_TIMER
 * @brief Período del temporizador en microsegundos para tomar una nueva medición de distancia. 
 */
#define CONFIG_SENSOR_TIMER 1000000 //1s

/** 
 * @var distancia
 * @brief Variable que almacena la distancia medida por el sensor ultrasónico en centímetros.
 */
uint16_t distancia;

/** 
 * @var encendido
 * @brief Indica si el sistema está encendido (true) o apagado (false).
 */
bool encendido = true;

/** 
 * @var hold
 * @brief Si está en true, mantiene congelado el valor mostrado en la pantalla LCD.
 */
bool hold = false;

/*==================[internal data definition]===========================*/

/** 
 * @var operar_distancia_task_handle
 * @brief Handle de la tarea que opera con la distancia medida.
 */
TaskHandle_t operar_distancia_task_handle = NULL;

/*==================[internal functions declaration]=======================*/

/** 
 * @fn void FuncTimerA(void* param)
 * @brief Función asociada al temporizador. Envía una notificación a la tarea que maneja la distancia.
 * @param[in] param no utilizado.
 */
void FuncTimerA(void* param){
	vTaskNotifyGiveFromISR(operar_distancia_task_handle, pdFALSE); /* Envía una notificación a la tarea*/
}


/** 
 * @fn static void OperarConDistancia(void *pvParameter)
 * @brief Tarea principal que mide la distancia y controla los LEDs y la pantalla LCD en base al valor leído.
 * @param[in] pvParameter no utilizado.
 */
static void OperarConDistancia(void *pvParameter){
    while(true){
		// La tarea esta en espera (bloqueada) hasta que reciba una notificación mediante ulTaskNotifyTake
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		
		if(encendido){
			// Medir distancia
			distancia = HcSr04ReadDistanceInCentimeters();
			LedsAllOff();
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
			if(!hold){
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

/** 
 * @fn void Interrupciontecla1(void)
 * @brief Interrupción asociada a la tecla 1. Cambia el estado de encendido/apagado del sistema.
 */
void Interrupciontecla1(void){
		
	encendido = !encendido;
}

/** 
 * @fn void Interrupciontecla2(void)
 * @brief Interrupción asociada a la tecla 2. Cambia el estado del modo hold para congelar/descongelar la pantalla.
 */
void Interrupciontecla2(void){
		
		hold = !hold;
}

/*==================[external functions definition]========================*/

/** 
 * @fn void app_main(void)
 * @brief Función principal de la aplicación. Inicializa periféricos, configura el temporizador, interrupciones y tareas.
 */
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();
	// timer_config_t: configura el temporizador definiendo su periodo 
	//y la función que se llamará cuando el temporizador se dispare.
	timer_config_t timer_sensor = {
        .timer = TIMER_A,
        .period = CONFIG_SENSOR_TIMER,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_sensor);
	// Se habilitan interrupciones cuando se presionan las teclas SWITCH_1 y SWITCH_2 (desencadena OperarConTeclado)
	SwitchActivInt(SWITCH_1, Interrupciontecla1, NULL);
	SwitchActivInt(SWITCH_2, Interrupciontecla2, NULL);

	xTaskCreate(&OperarConDistancia, "OperarConDistancia", 2048, NULL, 5, &operar_distancia_task_handle);
	
	TimerStart(timer_sensor.timer);
}
/*==================[end of file]============================================*/