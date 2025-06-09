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
 * |   Perifericos  |   ESP32   	|
 * |:--------------:|:--------------|
 * | sensor_pH      | 	CH1     	|
 * | sensor_riego 	| 	GPIO_20		|
 * | bomba_agua 	| 	GPIO_21		|
 * | bomba_pH_acido	| 	GPIO_22		|
 * | bomba_pH_basico| 	GPIO_23		|
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

 // PRIMER EJERCICIO: REGAR PLANTAS: El sistema está compuesto por una serie de recipientes con agua, una solución de 
 //  pH ácida y otra básica, un sensor de húmedad y uno de pH, y tres bombas
 //  peristálticas para los líquidos. Hay que encenderlas o apagarlas segun lo que 
 // devuelvan los sensores. 
 // Ademas hay que mostrar la informacion por UART.
 // Y mediente los switch 1 y 2 de la placa controlar si el sistema esta encendido
 //  o apagado.

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define PERIOD_SENSAR 3000000 //3s
#define PERIOD_MOSTRAR 5000000 //5s
#define PERIODO_SWITCH 40 
/*==================[internal data definition]===============================*/

TaskHandle_t sensar_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;
bool sistema = true;
bool bomba_agua = false;
bool bomba_acida = false;
bool bomba_basica = false;
float pH;

/*==================[internal functions declaration]=========================*/

void FuncTimerSensar(void* param){
    vTaskNotifyGiveFromISR(sensar_task_handle, pdFALSE);   
}

void FuncTimerMostrar(void* param){
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);   
}

static void SensarTask(void *pvParameter){

	// Primero convierto el valor de tensión en su equivalente en pH
	// luego verifico si es <6 -> prendo bomba básica
	// o si es >6.7 -> prendo bomba ácida
	// por último verifico si la humedad es 1 -> prendo bomba de agua
    
	uint16_t tension;
		
	while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    

		if(sistema){
			AnalogInputReadSingle(CH1, &tension);
            pH = (float)tension * (14.0/3.0) ;  // como el voltaje es de 0 a 3, y 0V es 0 pH, y 3V es 14 pH, el factor de conversion es 14/3
            if(pH < 6.0)
            {
                //prendo bobmba basica y apago la acida
                GPIOOn(GPIO_23);
                GPIOOff(GPIO_22);
                bomba_acida = false;
                bomba_basica = true;
            }
            else 
            if(pH > 6.7)
            {
                //prondo bomba acida y apago la basica
                GPIOOn(GPIO_22);
                GPIOOff(GPIO_23);
                bomba_basica = false;
                bomba_acida = true;
            }
            else 
            {
                // apago las dos bombas
                GPIOOff(GPIO_22);
                GPIOOff(GPIO_23);
                bomba_acida = false;
                bomba_basica = false;
            }
            
            // para la humedad si da 1 hay que prender la bomba si da 0 hay que apagarla
            bomba_agua = GPIORead(GPIO_20);    
            if (bomba_agua)
            {
                GPIOOn(GPIO_20);
            }
            else 
            {
                GPIOOff(GPIO_20);
            }            
        } 
	}
};


static void MostrarTask(void *pvParameter){
 	while (true)
	{
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        if (sistema)
        {
            UartSendString(UART_PC,"pH: ");
            UartSendString(UART_PC, (const char* ) UartItoa( pH, 10)); 

            if(bomba_agua)
            {
               UartSendString(UART_PC,", humedad correcta."); 
            }
            else 
            {
                UartSendString(UART_PC,", humedad incorrecta.");
            }
			UartSendString(UART_PC, "\r\n");

            if(bomba_acida)
            {
                UartSendString(UART_PC,"Bomba de pH acido encendida."); 
            }
            if(bomba_basica)
            {
                UartSendString(UART_PC,"Bomba de pH basico encendida."); 
            }

            if(bomba_agua)
            {
                UartSendString(UART_PC,"Bomba de agua encendida."); 
            }
			UartSendString(UART_PC, "\r\n");
        }
	}
}

static void TeclasTask(void *pvParameter){
	uint8_t teclas;
	while (true)
	{
		teclas = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
				sistema = true;
    		break;
    		case SWITCH_2:
				sistema = false;
    		break;
    	}
		vTaskDelay(PERIODO_SWITCH / portTICK_PERIOD_MS);
	}
};

/*==================[external functions definition]==========================*/
void app_main(void){

	//ANALÓGICO
	analog_input_config_t sensor_pH = {
		.input = CH1, 
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL
	};
	AnalogInputInit(&sensor_pH);
	AnalogOutputInit();

	//UART
	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 9600,		
		.func_p = NULL,			
		.param_p = NULL
	}; 
	UartInit(&miUart);

	//TAREAS
    xTaskCreate(&SensarTask, "Sensar", 4096, NULL, 5, &sensar_task_handle); 
    xTaskCreate(&MostrarTask, "Mostrar", 4096, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&TeclasTask, "Switch", 1024, NULL, 5, &teclas_task_handle);

	//TIMERS
    timer_config_t timer_sensar = {
        .timer = TIMER_A,
        .period = PERIOD_SENSAR,
        .func_p = FuncTimerSensar,
        .param_p = NULL
    };
    TimerInit(&timer_sensar);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = PERIOD_MOSTRAR,
        .func_p = FuncTimerMostrar,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

    TimerStart(timer_sensar.timer);
    TimerStart(timer_mostrar.timer);
}
/*==================[end of file]============================================*/


// SEGUNDO EJERCICIO: sistema de pesaje de camiones basado en la placa ESP-EDU.
// A través del sensor de ultrasonido HC-SR04 se mide la distancia del camión para ingresar a la balanza.
// Se calcula la velocidad y según su valor se encienden señales de advertencia.
// Cuando el vehiculo se detiene, se procede a pesarlo y se obtienen los valores a través de dos entradas 
// analógicas (son dos balanzas). Se informa a la PC la velocidad y el peso del camión, a través de la UART
// y se maneja desde la PC  (también a través de la UART) el control de una barrera.

/*!
* @section hardConn Hardware Connection
 *
 * |    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_3		|
 * | 	Trig	 	| 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	|
 * 
 * |    UART_PC     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  TX        | 	GPIO_16		|
 * | 	  RX        | 	GPIO_17		|
 * | 	  Gnd       | 	GND    	    |
 * 
 * |    Barrera     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Señal		| 	GPIO_1		|
 * | 	Gnd 	    | 	GND     	|
 *
 * |    Balanzas    |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Balanza 1	| 	CH1 		|
 * | 	Balanza 2	| 	CH2 		|  */

 /*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

#define PERIOD_DISTANCIA 100000 //100ms -> 10 muestras/seg
#define PERIOD_PESO_ADC 500 //5ms -> 200 muestras/seg

/*==================[internal data definition]===============================*/
TaskHandle_t distancia_task_handle = NULL;
TaskHandle_t adc_conversion_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;

float distancia_actual = 0.0;
float distancia_anterior = 0.0;
float tiempo = 0.10; //seg
float velocidad_max = 0.00;
float velocidad = 0.00;
float prom_peso_galga_1 = 0.00;
float prom_peso_galga_2 = 0.00;
float peso_total = 0.00;
uint16_t valor_galga_1 = 0;
uint16_t valor_galga_2 = 0;
float suma_galga1 = 0;
float suma_galga2 = 0;
uint8_t muestras_peso = 0;

/*==================[internal functions declaration]=========================*/

void FuncTimerDistancia(void* param){
    vTaskNotifyGiveFromISR(distancia_task_handle, pdFALSE);   
}

void FuncTimerConversionADC(void* param){
    vTaskNotifyGiveFromISR(adc_conversion_task_handle, pdFALSE);   
}

static void DistanciaTask(void *pvParameter){

	while(true){
		printf("Sensando\n");
		distancia_actual = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(PERIOD_DISTANCIA / portTICK_PERIOD_MS);
	}
}


static void LedsTask(void *pvParameter){
	while(true){
		printf("Leds\n");

		if(distancia_actual < 1000){
			velocidad = ((distancia_anterior - distancia_actual) / tiempo) / 100.0; // m/s

			if(velocidad > velocidad_max){
				velocidad_max = velocidad;
			}

			UartSendString(UART_PC, "Velocidad maxima: ");
			UartSendString(UART_PC, (char *)UartItoa(velocidad_max, 10));
			UartSendString(UART_PC, " m/s\r\n");

			if(velocidad > 800){
				LedOff(LED_1);
				LedOff(LED_2);
				LedOn(LED_3);
			}
			else if((velocidad < 800) && (velocidad > 0)){
				LedOff(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if(velocidad == 0){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);

				// Solo mostrar el peso si ya fue calculado (luego de 50 muestras)
				if(peso_total > 0){
					UartSendString(UART_PC, "Peso: ");
					UartSendString(UART_PC, (char *)UartItoa(peso_total, 10));
					UartSendString(UART_PC, " kg\r\n");

					// Una vez mostrado, reiniciar para no repetir
					peso_total = 0;
				}
			}
		} else {
			LedsOffAll();
			velocidad_max = 0.0; // Reiniciar para el próximo camión
		}

		distancia_anterior = distancia_actual;
		vTaskDelay(PERIOD_DISTANCIA / portTICK_PERIOD_MS);
	}
}


static void ADC_ConversionTask(void *pvParameter) {

	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		AnalogInputReadSingle(CH1, &valor_galga_1);
		AnalogInputReadSingle(CH2, &valor_galga_2);

		// Solo acumula si el camión está detenido -> factor de conversión 20000/3300
		if (velocidad == 0 && muestras_peso < 50) {
			float peso_galga1 = ((valor_galga_1 * 20000.0) / 3300.0);
			float peso_galga2 = ((valor_galga_2 * 20000.0) / 3300.0);

			suma_galga1 += peso_galga1;
			suma_galga2 += peso_galga2;
			muestras_peso++;
		}

		// Si ya hay 50 muestras, calcular promedio y dejarlo listo para imprimir
		if (muestras_peso == 50) {
			prom_peso_galga_1 = suma_galga1 / 50.0;
			prom_peso_galga_2 = suma_galga2 / 50.0;
			peso_total = prom_peso_galga_1 + prom_peso_galga_2;

			// Reiniciar acumuladores
			suma_galga1 = 0;
			suma_galga2 = 0;
			muestras_peso = 0;
		}
	}
}


void ControlBarrera(){
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);
    switch (tecla){
    case 'O':
        GPIOOn(GPIO_1);
        break;
	case 'C':
		GPIOOff(GPIO_1);
		break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	GPIOInit(GPIO_1, GPIO_OUTPUT);

	//ANALÓGICO
	analog_input_config_t config_ADC_CH1 = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC_CH1);
	
	analog_input_config_t config_ADC_CH2 = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC_CH2);

	//UART
	serial_config_t miUart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = ControlBarrera,
		.param_p = NULL
	};
	UartInit(&miUart);

	//TAREAS
    xTaskCreate(&DistanciaTask, "Sensar Distancia", 4096, NULL, 5, &distancia_task_handle); 
    xTaskCreate(&ADC_ConversionTask, "Sensar Peso", 4096, NULL, 5, &adc_conversion_task_handle);
	xTaskCreate(&LedsTask, "Leds", 4096, NULL, 5, &leds_task_handle);

	//TIMERS
    timer_config_t timer_distancia = {
        .timer = TIMER_A,
        .period = PERIOD_DISTANCIA,
        .func_p = FuncTimerDistancia,
        .param_p = NULL
    };
    TimerInit(&timer_distancia);

    timer_config_t timer_peso_adc = {
        .timer = TIMER_B,
        .period = PERIOD_PESO_ADC,
        .func_p = FuncTimerConversionADC,
        .param_p = NULL
    };
    TimerInit(&timer_peso_adc);

    TimerStart(timer_distancia.timer);
    TimerStart(timer_peso_adc.timer);
}
/*==================[end of file]============================================*/