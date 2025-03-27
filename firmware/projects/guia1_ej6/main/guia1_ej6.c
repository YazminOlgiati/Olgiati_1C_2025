/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Es una función que recibe un dato de 32 bits, la cantidad de dígitos de salida y 
 * dos vectores de estructuras del tipo gpioConf_t. Uno de estos vectores es igual 
 * a uno definido previamente y el otro vector mapea los puertos con el dígito del 
 * LCD a donde mostrar un dato:
 * Dígito 1 -> GPIO_19
 * Dígito 2 -> GPIO_18
 * Dígito 3 -> GPIO_9
 * La función muestra por display el valor que recibe.
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
 * | 30/08/2024 | Document creation		                         |
*
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
 *
 */

/* ===================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/
#define N_BITS 4
#define LCD_DIGITS 3 
/*==================[internal data definition]===============================*/
typedef struct {
    gpio_t pin;
    io_t dir;
} gpioConfig_t;

/*==================[internal functions declaration]=========================*/
/*==================[external functions definition]==========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	if (digits > 10) {
        return -1;  // Error: el número de dígitos solicitado es mayor al máximo permitido.
    }
    // Inicializar el array de salida a 0
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
    }
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[digits - i - 1] = data % 10;  // Guardar el dígito menos significativo
        data = data / 10;  // Eliminar el dígito menos significativo del dato
    }
    // Si después de convertir hay datos restantes en el número de entrada, 
    // significa que el número de dígitos era insuficiente
    if (data > 0) {
        return -1;  // Error: se necesitarían más dígitos para representar completamente el dato en BCD.
    }

    return 0;
}

void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config) {
    for (uint8_t i = 0; i < N_BITS; i++) {
        GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
    }
    for (uint8_t i = 0; i < N_BITS; i++) {
            if ((digit & (1 << i)) == 0) {		//máscara (1 << i) y operación AND para determinar el estado de cada bit
                GPIOOff(gpio_config[i].pin);	// Apagar el GPIO si el bit es 0
            } else {
                GPIOOn(gpio_config[i].pin);		// Encender el GPIO si el bit es 1
            }
        }
}

//Paso como parámetros el número de 32 bits, vector de estructuras con los pines de los bits BCD, vector de estructuras con los pines de los dígitos
void displayNumberOnLCD(uint32_t data, gpioConfig_t *data_gpio_config, gpioConfig_t *digit_gpio_config) {
    uint8_t bcd_array[LCD_DIGITS];
    
    // Convertir el número a formato BCD
    if (convertToBcdArray(data, LCD_DIGITS, bcd_array) != 0) {
        printf("Error: el número es demasiado grande para la cantidad de dígitos.\n");
        return;

    }

    // Inicializar los pines del LCD
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        GPIOInit(digit_gpio_config[i].pin, digit_gpio_config[i].dir);
    }

    // Mostrar cada dígito en el display
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        // Apagar todos los dígitos
        for (uint8_t j = 0; j < LCD_DIGITS; j++) {
            GPIOOff(digit_gpio_config[j].pin);
        }

        // Configurar pulso (On-Off)
        GPIOOn(digit_gpio_config[i].pin);

		// Mostrar el valor BCD del dígito
        BCDtoGPIO(bcd_array[i], data_gpio_config);
    }
}

void app_main(void) {
    
    uint32_t number = 908; // Número a mostrar en el display

	// Vector de pines para cada bit BCD
    gpioConfig_t data_gpio_config[N_BITS] = {
        {GPIO_20, 1},
        {GPIO_21, 1},
        {GPIO_22, 1},
        {GPIO_23, 1}
    };
    
	// Vector de pines para activar los dígitos en el LCD
    gpioConfig_t digit_gpio_config[LCD_DIGITS] = {
        {GPIO_19, 1}, // Dígito 1
        {GPIO_18, 1}, // Dígito 2
        {GPIO_9, 1}   // Dígito 3
    };

    displayNumberOnLCD(number, data_gpio_config, digit_gpio_config);
}
/*==================[end of file]============================================*/