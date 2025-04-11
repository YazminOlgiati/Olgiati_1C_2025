/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Función que recibe un dato de 32 bits,  la cantidad de dígitos de salida y un puntero a un arreglo 
 * donde se almacene los n dígitos. La función deberá convertir el dato recibido a BCD, 
 * guardando cada uno de los dígitos de salida en el arreglo pasado como puntero.
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
 * | 27/03/2025 | Document creation		                         |
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

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
    // retorna -1 si hay error
	if (digits > 10) {
        return -1; 
    }
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0; // inicialización en 0
    }
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[digits - i - 1] = data % 10;  // dígito menos significativo
        data = data / 10;  // se borra el dígito menos significativo
    }
    // Si después de convertir hay datos restantes en el número de entrada, 
    // significa que el número de dígitos era insuficiente
    if (data > 0) {
        return -1;
    }

    return 0;
}


/*==================[external functions definition]==========================*/

void app_main(void){
	uint8_t bcd_array[5];
    uint32_t number = 549;
    uint8_t digits = 3;

    int8_t result = convertToBcdArray(number, digits, bcd_array);

    if (result == 0) {
        printf("Conversion exitosa: ");
        for (uint8_t i = 0; i < digits; i++) {
            printf("%d ", bcd_array[i]);
        }
        printf("\n");
    } else {
        printf("Conversion fallida.\n");
    }
}
/*==================[end of file]============================================*/