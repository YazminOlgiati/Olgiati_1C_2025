/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Función que recibe un dato de 32 bits,  la cantidad de dígitos de salida y dos vectores de estructuras del tipo gpioConf_t. 
 * Uno  de estos vectores es igual al definido en el punto anterior y el otro vector mapea los puertos con el dígito 
 * del LCD a donde mostrar un dato:
 * Dígito 1 -> GPIO_19
 * Dígito 2 -> GPIO_18
 * Dígito 3 -> GPIO_9
 * La función deberá mostrar por display el valor que recibe. 
 * Reutilice las funciones creadas en el punto 4 y 5. 
 * Realice la documentación de este ejercicio usando Doxygen.
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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/