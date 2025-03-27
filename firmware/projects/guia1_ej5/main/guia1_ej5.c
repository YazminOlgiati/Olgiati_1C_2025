/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Función que recibe como parámetro un dígito BCD y un vector de estructuras del tipo gpioConf_t. 
 * Incluya el archivo de cabecera gpio_mcu.h.
 * Defina un vector que mapee los bits de la siguiente manera:
 * b0 -> GPIO_20
 * b1 ->  GPIO_21
 * b2 -> GPIO_22
 * b3 -> GPIO_23
 * La función deberá cambiar el estado de cada GPIO, a ‘0’ o a ‘1’, según el estado del bit correspondiente en el BCD 
 * ingresado. Ejemplo: b0 se encuentra en ‘1’, el estado de GPIO_20 debe setearse. 
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
	#include "gpio_mcu.h"
	/*==================[macros and definitions]=================================*/
	//const uint8_t N_BITS = 4;
	#define N_BITS 4
	/*==================[internal data definition]===============================*/
	struct gpioConfig_t
	{
		gpio_t pin;
		io_t dir;
	};
	/*==================[internal functions declaration]=========================*/
	void BCDtoGPIO(uint8_t digit,  struct gpioConfig_t *gpio_config){
		for(uint8_t i = 0; i < 4; i++){
			GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
		}
		for(uint8_t i = 0; i < N_BITS; i++){
			if((digit & 1 << i) == 0){			//máscara (1 << i) y operación AND para determinar el estado de cada bit
				GPIOOff(gpio_config[i].pin);	// Apagar el GPIO si el bit es 0		
			}
			else{
				GPIOOn(gpio_config[i].pin);		// Encender el GPIO si el bit es 1
			}
		}
	}
	/*==================[external functions definition]==========================*/
	void app_main(void){
		
		uint8_t digit = 6;
	
		struct gpioConfig_t config_pines[N_BITS];
	
		config_pines[0].pin = GPIO_20; //b0 = 0
		config_pines[1].pin = GPIO_21; //b1 = 1
		config_pines[2].pin = GPIO_22; //b2 = 1
		config_pines[3].pin = GPIO_23; //b3 = 0
	
		for(uint8_t i = 0; i < N_BITS; i++) 
		{
			config_pines[i].dir = 1; // dir = 1 indica que es una valor de salida
		}
	
		BCDtoGPIO(digit, config_pines);
	
	
	}
/*==================[end of file]============================================*/