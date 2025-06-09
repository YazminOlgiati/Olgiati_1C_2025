/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * El sistema "Estacion de Monitoreo" está compuesto por un sensor de humedad y temperatura DHT11 y un 
 * sensor analógico de radiación. Indica el riesgo de nevada cuando la humedad es superior al 85% y la 
 * temperatura está entre 0 y 2°C, muestreando cada 1 segundo. También indica cuando la radiación se 
 * encuentra por encima de 40mR/h, muestreando cada 5 segundos. Para estas tres situaciones se enciende
 * un led de distinto color. Además el sistema se enciende con la tecla 1 y se apaga con la tecla 2.
 *  
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * 
 * |    UART_PC     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  TX        | 	GPIO_16		|
 * | 	  RX        | 	GPIO_17		|
 * | 	  Gnd       | 	GND    	    |
 * 
 * |    dht11       |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Señal		| 	GPIO_1/CH1	|
 * | 	Gnd 	    | 	GND     	|
 * 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/06/2025 | Document creation		                         |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/

/**
 * @brief Período de muestreo para temperatura y humedad (1 segundo)
 */
#define PERIOD_TyH 1000000 

/**
 * @brief Período de muestreo para radiación (5 segundos)
 */
#define PERIOD_RAD_ADC 5000000 

/**
 * @brief Período de muestreo para lectura de teclas (50 ms)
 */
#define PERIODO_SWITCH 50

/*==================[internal data definition]===============================*/

/** Tarea para sensar temperatura y humedad */
TaskHandle_t sensarTyH_task_handle = NULL;
/** Tarea para sensar radiación */
TaskHandle_t sensarRad_ADC_task_handle = NULL;
/** Tarea para manejar LEDs */
TaskHandle_t leds_task_handle = NULL;
/** Tarea para lectura de teclas */
TaskHandle_t teclas_task_handle = NULL;

/** Bandera que indica si el sistema está encendido */
bool sistema_encendido = false;
/** Variable que almacena la temperatura en ºC */
float temperatura = 0.00;
/** Variable que almacena la humedad en % */
float humedad = 0.00;
/** Variable que almacena la radiación en mR/h */
float radiacion = 0.00;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función del timer de temperatura y humedad
 * Notifica a la tarea correspondiente si el sistema está encendido.
 * @param param No utilizado
 */
void FuncTimerSensarTyH(void* param){
    if (sistema_encendido) { //verifico si primero si el sistema está encendido
        vTaskNotifyGiveFromISR(sensarTyH_task_handle, pdFALSE);
    }
}

/**
 * @brief Función del timer de radiación
 * Notifica a la tarea correspondiente si el sistema está encendido.
 * @param param No utilizado
 */
void FuncTimerSensarRadADC(void* param){
    if (sistema_encendido) {
        vTaskNotifyGiveFromISR(sensarRad_ADC_task_handle, pdFALSE);
    }
}

/**
 * @brief Tarea que lee temperatura y humedad desde el sensor DHT11
 * Si se detectan valores que indican riesgo de nevada, se informa por UART.
 * @param pvParameter No utilizado
 */
static void SensarTyHTask(void *pvParameter){
	   while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if(sistema_encendido){
			if (dht11Read(&humedad, &temperatura)) { //humedad en % y temperatura en °C
            char texto[100];

			// humedad mayor a 85%, y temperatura entre 0 y 2°C -> riesgo de nevada
            if (humedad > 85 && temperatura >= 0 && temperatura <= 2) {
                printf(texto, sizeof(texto),
                    "Temperatura: %.1fºC - Húmedad: %.1f%% - RIESGO DE NEVADA\r\n",
                    temperatura, humedad);
            } else {
                printf(texto, sizeof(texto),
                    "Temperatura: %.1fºC - Húmedad: %.1f%%\r\n",
                    temperatura, humedad);
            }
            UartSendString(UART_PC, texto);
        	}

		}
        
    }
}


/**
 * @brief Tarea que lee y convierte el valor analógico de radiación
 * Si la radiación supera los 40 mR/h, se informa por UART.
 * @param pvParameter No utilizado
 */
static void SensarRadADCTask(void *pvParameter){
    uint16_t tension;

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (sistema_encendido) {
			//lectura y conversión
            AnalogInputReadSingle(CH1, &tension);
            radiacion = (float)tension * (100.0 / 3300.0);  // 3.3V -> 100 mR/h

            char texto[50];

			// radiación mayor a 40mR/h -> riesgo ambiente
            if (radiacion > 40) {
                printf(texto, sizeof(texto),
                    "Radiación %.1fmR/h - RADIACIÓN ELEVADA\r\n", radiacion);
            } else {
                printf(texto, sizeof(texto),
                    "Radiación %.1fmR/h\r\n", radiacion);
            }

            UartSendString(UART_PC, texto);
        }
    }
}

/**
 * @brief Tarea que gestiona los LEDs según el estado ambiental
 * LED rojo para riesgo de nevada.  
 * LED amarillo para radiación elevada.  
 * LED verde para condiciones normales.
 * @param pvParameter No utilizado
 */
static void LedsTask(void *pvParameter){
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(500));

        if (!sistema_encendido) {
            GPIOOff(GPIO_1);
            GPIOOff(GPIO_2);
            GPIOOff(GPIO_3);
        }

		//Led verde 1 - Led amarillo 2 - Led rojo 3
        if (humedad > 85 && temperatura >= 0 && temperatura <= 2) { 
            GPIOOn(GPIO_3);  
            GPIOff(GPIO_2);
            GPIOOff(GPIO_1);
        } else if (radiacion > 40) {
            GPIOOn(GPIO_2); 
            GPIOOff(GPIO_3);
            GPIOOff(GPIO_1);
        } else {
            GPIOOn(GPIO_1); 
            GPIOOff(GPIO_2);
            GPIOOff(GPIO_3);
        }
    }
}

/**
 * @brief Tarea que controla el encendido y apagado del sistema mediante teclas
 * SWITCH_1: enciende el sistema  
 * SWITCH_2: apaga el sistema
 * @param pvParameter No utilizado
 */
static void TeclasTask(void *pvParameter){
	uint8_t teclas;
	while (true)
	{
		teclas = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
				sistema_encendido = true;
    		break;
    		case SWITCH_2:
				sistema_encendido = false;
    		break;
    	}
		vTaskDelay(PERIODO_SWITCH / portTICK_PERIOD_MS);
	}
};


/*==================[external functions definition]==========================*/

/**
 * @brief Función principal del programa.
 */
void app_main(void){
	
	//INICIALIZACIÓN PERIFÉRICOS
	dht11Init(GPIO_22);
	LedsInit();
	GPIOInit(GPIO_1, GPIO_OUTPUT);
    GPIOInit(GPIO_2, GPIO_OUTPUT);
    GPIOInit(GPIO_3, GPIO_OUTPUT);


	//SENSOR ANALÓGICO
	analog_input_config_t config_ADC_CH1 = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC_CH1);
	AnalogOutputInit();


	//UART
	serial_config_t miUart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&miUart);


	//TAREAS
    xTaskCreate(&SensarTyHTask, "Sensar Temperatura y Humedad", 4096, NULL, 5, &sensarTyH_task_handle); 
    xTaskCreate(&SensarRadADCTask, "Sensar Radiacion", 4096, NULL, 5, &sensarRad_ADC_task_handle);
	xTaskCreate(&LedsTask, "Leds", 4096, NULL, 5, &leds_task_handle);
	xTaskCreate(&TeclasTask, "Teclas", 4096, NULL, 5, &teclas_task_handle);


	//TIMERS
    timer_config_t timer_tyh = {
        .timer = TIMER_A,
        .period = PERIOD_TyH,
        .func_p = FuncTimerSensarTyH,
        .param_p = NULL
    };
    TimerInit(&timer_tyh);

    timer_config_t timer_rad_adc = {
        .timer = TIMER_B,
        .period = PERIOD_RAD_ADC,
        .func_p = FuncTimerSensarRadADC,
        .param_p = NULL
    };
    TimerInit(&timer_rad_adc);

    TimerStart(timer_tyh.timer);
    TimerStart(timer_rad_adc.timer);
}
/*==================[end of file]============================================*/

