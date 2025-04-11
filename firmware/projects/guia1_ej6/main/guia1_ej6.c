/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Función que recibe un dato de 32 bits, la cantidad de dígitos de salida y 
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
 * | 27/03/2025 | Document creation		                         |
*
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
 *
 */

/* ===================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/** @def N_BITS
 * @brief Número de bits necesarios para representar un dígito BCD.
 */
#define N_BITS 4

/** @def LCD_DIGITS
 * @brief Número de dígitos del LCD.
 */
#define LCD_DIGITS 3 

/*==================[internal data definition]===============================*/

/**
 * @brief Estructura de configuración de un pin GPIO.
 */
typedef struct {
    gpio_t pin;
    io_t dir;
} gpioConfig_t;

/*==================[internal functions declaration]=========================*/

/** @fn void GPIOInit(gpio_t pin, io_t dir)
 * @brief Inicializa un pin GPIO con la dirección especificada.
 * @param[in] pin Pin GPIO a inicializar.
 * @param[in] dir Dirección del pin (entrada o salida).
 * @return
 */
void GPIOInit(gpio_t pin, io_t dir);

/** @fn void GPIOOn(gpio_t pin)
 * @brief Enciende un pin GPIO.
 * @param[in] pin Pin GPIO a encender.
 * @return
 */
void GPIOOn(gpio_t pin);

/** @fn void GPIOOff(gpio_t pin)
 * @brief Apaga un pin GPIO.
 * @param[in] pin Pin GPIO a apagar.
 * @return
 */
void GPIOOff(gpio_t pin);

/** @fn void GPIOstate(gpio_t pin, uint8_t state)
 * @brief Cambia el estado de un pin GPIO.
 * @param[in] pin Pin GPIO cuyo estado se cambiará.
 * @param[in] state Estado deseado.
 * @return
 */
void GPIOstate(gpio_t pin, uint8_t state);

/*==================[external functions definition]==========================*/

/** @fn int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief Convierte un número decimal en un arreglo BCD.
 * @param[in] data Número de entrada en formato decimal.
 * @param[in] digits Cantidad de dígitos a convertir.
 * @param[out] bcd_number Arreglo donde se almacena el resultado BCD.
 * @return int8_t 0 si fue exitoso, -1 si hubo error.
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	if (digits > 10) {
        return -1; 
    }
    // Inicializar el array de salida a 0
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
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


/** @fn void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config) 
 * @brief Muestra un dígito BCD en los pines GPIO configurados.
 * @param[in] digit Dígito en formato BCD a mostrar.
 * @param[in] gpio_config Vector de configuración de pines para cada bit BCD.
 * @return
 */
void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config) {
    for (uint8_t i = 0; i < N_BITS; i++) {
        GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
    }
    for (uint8_t i = 0; i < N_BITS; i++) {
            if ((digit & (1 << i)) == 0) {		//máscara (1 << i) y operación AND para determinar el estado de cada bit
                GPIOOff(gpio_config[i].pin);	// si apaga el GPIO si el bit es 0
            } else {
                GPIOOn(gpio_config[i].pin);		// se prende el GPIO si el bit es 1
            }
        }
}

/** @fn void displayNumberOnLCD(uint32_t data, gpioConfig_t *data_gpio_config, gpioConfig_t *digit_gpio_config)
 * @brief Muestra un número en el display LCD utilizando pines GPIO.
 * @param[in] data Número a mostrar en el display (máximo 3 dígitos).
 * @param[in] data_gpio_config Vector de configuración de los pines GPIO para los datos.
 * @param[in] digit_gpio_config Vector de configuración de los pines GPIO para los dígitos.
 * @return
 */
void displayNumberOnLCD(uint32_t data, gpioConfig_t *data_gpio_config, gpioConfig_t *digit_gpio_config) {
    uint8_t bcd_array[LCD_DIGITS];
    
    if (convertToBcdArray(data, LCD_DIGITS, bcd_array) != 0) {
        printf("Error: el número es demasiado grande para la cantidad de dígitos.\n");
        return;
    }

    // inicializar los pines del LCD
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        GPIOInit(digit_gpio_config[i].pin, digit_gpio_config[i].dir);
    }

    // mostrar cada dígito en el display
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        for (uint8_t j = 0; j < LCD_DIGITS; j++) {
            GPIOOff(digit_gpio_config[j].pin); // se apagan todos los dígitos
        }

        GPIOOn(digit_gpio_config[i].pin); //  pulso (on-off)

        BCDtoGPIO(bcd_array[i], data_gpio_config);
    }
}


/** @fn void app_main(void) 
 * @brief Función principal de la aplicación.
 * Configura los pines GPIO y muestra un número en el display LCD.
 * @return
 */
void app_main(void) {
    
    uint32_t number = 908;

	// vector de pines para cada bit BCD
    gpioConfig_t data_gpio_config[N_BITS] = {
        {GPIO_20, 1},
        {GPIO_21, 1},
        {GPIO_22, 1},
        {GPIO_23, 1}
    };
    
	// vector de pines para activar los dígitos en el LCD
    gpioConfig_t digit_gpio_config[LCD_DIGITS] = {
        {GPIO_19, 1}, // 1
        {GPIO_18, 1}, // 2
        {GPIO_9, 1}   // 3
    };

    displayNumberOnLCD(number, data_gpio_config, digit_gpio_config);
}
/*==================[end of file]============================================*/