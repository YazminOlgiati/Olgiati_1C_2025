/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora el puerto serie. 
 * Envíe los datos de las mediciones para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:
 * 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
 * Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 * Con las teclas “O” (encerder) y “H” (mantener), replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_2		|
 * | 	PIN_X	 	| 	GPIO_3		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 08/05/2025 | Document creation		                         |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingnerieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
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

/*==================[macros and definitions]=================================*/
/** 
* @def REFRESH_DISPLAY
* @brief Período de refresco para la medición del sensor de ultrasonido.*/
#define REFRESH_MEDICION 1000000 //1s

/** 
* @def REFRESH_DISPLAY
* @brief Período de refresco para la actualización del display. 
*/
#define REFRESH_DISPLAY 1000000 //1s

/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea encargada de sensar la distancia.
 * Esta variable guarda el identificador de la tarea de FreeRTOS que se encarga de leer la distancia desde el sensor de ultrasonido.
 */
TaskHandle_t Sensar_task_handle = NULL;

/**
 * @brief Handle de la tarea encargada de mostrar la distancia.
 * Esta variable guarda el identificador de la tarea de FreeRTOS que se encarga de actualizar los LEDs, el display y enviar la distancia por puerto serie.
 */
TaskHandle_t Mostrar_task_handle = NULL;

/** 
 * @var distancia
 * @brief Variable que almacena la distancia medida por el sensor ultrasónico en centímetros.
 */
uint16_t distancia = 0;

/** 
 * @var distancia
 * @brief Variable auxiliar para activar o desactivar la medición y el control de LEDs.
 */
bool encendido = true;

/** 
 * @var distancia
 * @brief Variable auxiliar para mantener el último valor medido y congelar el estado de los LEDs.
 */
bool hold = false;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void escribirDistanciaEnPc
 * @brief Enviar la distancia medida al puerto serie
 * Esta función envía el valor de la distancia al puerto serie con el formato especificado en la consigna.
 */
void escribirDistanciaEnPc(){
    UartSendString(UART_PC, "distancia ");
    UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
    UartSendString(UART_PC, " cm\r\n");
}

/**
 * @fn void sensarTask()
 * @brief Tarea para sensar la distancia
 * Esta tarea se activa por una interrupción de temporizador. Cuando está activada, mide la distancia con el sensor de ultrasonido.
 */
void sensarTask(){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido){
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @fn void mostrarTask()
 * @brief Tarea para mostrar la distancia
 * Esta tarea se activa por otra interrupción de temporizador. Controla los LEDs y el display LCD para mostrar la distancia medida.
 */
void mostrarTask(){
    while (true){
        // La tarea esta en espera (bloqueada) hasta que reciba una notificación mediante ulTaskNotifyTake
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido){
            if (distancia < 10){
                LedsOffAll();
            }
            else if ((distancia > 10) & (distancia < 20)){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if ((distancia > 20) & (distancia < 30)){
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
            escribirDistanciaEnPc();
        }
        else{
            LcdItsE0803Off();
            LedsOffAll();
        }
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

/**
 * @fn void TeclasOnHold()
 * @brief Función de interrupción para los comandos enviados desde el puerto serie
 * Esta función lee los comandos 'O' y 'H' desde el puerto serie, replicando la funcionalidad de las teclas 1 y 2.
 */
void TeclasOnHold(){
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);
    switch (tecla){
    case 'O':
        encendido = !encendido;
        break;
	case 'H':
		hold = !hold;
		break;
	}
}

/**
 * @fn void FuncTimerSensar()
 * @brief Función de temporizador para la tarea de sensado
 * Activa la tarea de sensado utilizando una notificación.
 */
void FuncTimerSensar(){
    vTaskNotifyGiveFromISR(Sensar_task_handle, pdFALSE); 
}

/**
 * @fn void FuncTimerMostrar()
 * @brief Función de temporizador para la tarea de mostrar
 * Activa la tarea de mostrar utilizando una notificación.
 */
void FuncTimerMostrar(){
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}


/*==================[external functions definition]==========================*/
/**
 * @fn void app_main(void)
 * @brief Función principal
 * Inicializa los periféricos, configura las teclas con interrupciones, los temporizadores, y las tareas de FreeRTOS para el control del sensor de ultrasonido y la visualización de la distancia.
 */
void app_main(void){
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    serial_config_t ControlarUart =
        {
            .port = UART_PC,
            .baud_rate = 115200,
            .func_p = TeclasOnHold,
            .param_p = NULL,
        };
    UartInit(&ControlarUart);
    

    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESH_MEDICION,
        .func_p = FuncTimerSensar,
        .param_p = NULL};
    TimerInit(&timer_medicion);


    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = REFRESH_DISPLAY,
        .func_p = FuncTimerMostrar,
        .param_p = NULL};
    TimerInit(&timer_mostrar);
 

    SwitchActivInt(SWITCH_1, Interrupciontecla1, NULL);
    SwitchActivInt(SWITCH_2, Interrupciontecla2, NULL);

    xTaskCreate(&sensarTask, "sensar", 512, NULL, 5, &Sensar_task_handle);
    xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &Mostrar_task_handle);
}
/*==================[end of file]============================================*/