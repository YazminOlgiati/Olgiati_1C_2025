/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Se mide distancia con un sensor de ultrasonido:
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * Mostrar el valor de distancia en cm utilizando el display LCD.
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
 * Refresco de medición: 1 s
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |   Periférico   |  Pin ESP32   	|
 * |:--------------:|:--------------|
 * | HC-SR04 Echo   | GPIO_X        |
 * | HC-SR04 Trigger| GPIO_X        |
 * | LED_1          | GPIO_X        |
 * | LED_2          | GPIO_X        |
 * | LED_3          | GPIO_X        |
 * | LCD Display    | GPIO_X        |
 * | TEC1           | GPIO_X        |
 * | TEC2           | GPIO_X        |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/04/2025 | Document creation		                         |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusiones]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]===================================*/

/*! @brief Período de refresco para la lectura del sensor (en milisegundos). */
#define CONFIG_PERIOD_1000 1000
/*! @brief Período de refresco para el control de LEDs (en milisegundos). */
#define CONFIG_PERIOD_100 100
/*! @brief Período de refresco para la lectura de las teclas (en milisegundos). */
#define CONFIG_PERIOD_10 10

/*! @brief Variable auxiliar para activar o desactivar la medición y el control de LEDs. */
bool encendido = true;
/*! @brief Variable auxiliar para mantener el último valor medido y congelar el estado de los LEDs. */
bool hold = false;
/*! @brief Almacena la distancia medida por el sensor en centímetros. */
float distancia = 0.00;

/*==================[internal data definition]===========================*/

/*! @brief Handle para la tarea de sensado de distancia. */
TaskHandle_t medir_task_handle = NULL;

/*! @brief Handle para la tarea de control de LEDs. */
TaskHandle_t leds_task_handle = NULL;

/*! @brief Handle para la tarea de lectura de teclas. */
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal functions declaration]=======================*/

/*!
 * @brief Tarea que lee la distancia usando el sensor ultrasónico HC-SR04.
 *
 * Esta tarea lee periódicamente la distancia medida por el sensor y la almacena
 * en la variable global `distancia`.
 *
 * @param[in] pvParameter Puntero a los parámetros de la tarea.
 */
static void MedirTask(void *pvParameter){
	while(true){
		//printf("Midiendo\n");
		if(encendido){
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(CONFIG_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

/*!
 * @brief Tarea que controla los LEDs según la distancia medida.
 *
 * Esta tarea enciende o apaga los LEDs dependiendo de la distancia almacenada
 * en la variable global `distancia`. También actualiza el display LCD.
 *
 * @param[in] pvParameter Puntero a los parámetros de la tarea.
 */
static void LedsTask(void *pvParameter){
	while(true){
		//printf("Leds\n");
		if(encendido){
			if(distancia < 10){
				LedsOffAll();
			}	
			else if(distancia >= 10 && distancia < 20){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if(distancia >= 20 && distancia < 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if (distancia > 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
			 if (!hold){
                LcdItsE0803Write(distancia);
            }
		}
		else{
			LcdItsE0803Off(); //apaga los tres dígitos del display ITS-E0803
			LedsOffAll();
		}
		vTaskDelay(CONFIG_PERIOD_100 / portTICK_PERIOD_MS);
	}
}

/*!
 * @brief Tarea que lee las teclas (TEC1 y TEC2).
 *
 * Esta tarea detecta el estado de las teclas y activa o desactiva la medición
 * (`activar`), o mantiene el valor medido (`hold`), según sea necesario.
 *
 * @param[in] pvParameter Puntero a los parámetros de la tarea.
 */
static void TeclasTask(void *pvParameter){
	uint8_t teclas;
	while(true){
		//printf("Teclas\n");
		teclas = SwitchesRead();
		switch (teclas){
		case SWITCH_1:
			encendido = !encendido;
			break;
		case SWITCH_2:
			hold = !hold;
			break;
		}
		vTaskDelay(CONFIG_PERIOD_10 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]========================*/

/*!
 * @brief Función principal de la aplicación.
 *
 * Esta función inicializa los periféricos y crea las tareas para el sensado de distancia,
 * control de LEDs y monitoreo de las teclas.
 */
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();

	xTaskCreate(&MedirTask, "Medir", 512, NULL, 5, &medir_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 5, &leds_task_handle);	
	xTaskCreate(&TeclasTask, "Teclas", 512, NULL, 5, &teclas_task_handle);
}
/*==================[end of file]============================================*/

// drivers/devices/src/hc_sr04.c
// #define US2CM		59		/* scale factor to conver pulse width to cm */

// En la foto se colocó un obejeto a 5cm del sensor,
// con el osciloscopio se ve que el pulso es de aparoximandamente 350ms, 
// este valor dividido 59 da como resultado 5,9cm (verificando la distancia medida).