/**
 * @file hc_sr04.c
 * @author Albano Peñalva
 * @brief Driver para el sensor HC-SR04 con soporte para múltiples instancias.
 * @version 0.2
 * @date 2025-05-27
 */

#include "hc_sr04.h"
#include "delay_mcu.h"

/*==================[macros and definitions]=================================*/
#define MAX_US     17700   /* máximo tiempo de distancia en us (300cm) */
#define MAX_CM     300     /* distancia máxima en cm */
#define MAX_INCH   118     /* distancia máxima en pulgadas */
#define US2CM      59      /* factor de conversión de us a cm */
#define US2INCH    150     /* factor de conversión de us a pulgadas */
#define WAIT_MAX   5900    /* tiempo máximo de espera por el echo (en us) */

/*==================[external functions definition]==========================*/

bool HcSr04Init(gpio_t echo, gpio_t trigger) {

  //  trigger_st = trigger;
//	echo_st = echo;

    GPIOInit(echo, GPIO_INPUT);
    GPIOInit(trigger, GPIO_OUTPUT);
    GPIOOff(trigger);  // Asegurarse de que esté en bajo al inicio
    return true;
}

uint16_t HcSr04ReadDistanceInCentimeters(gpio_t echo, gpio_t trigger) {
    uint16_t distance = 0, waiting = 0;

    GPIOOn(trigger);
    DelayUs(10);
    GPIOOff(trigger);

    // Esperar señal de subida en echo
    while (!GPIORead(echo)) {
        DelayUs(10);
        waiting += 10;
        if (waiting > WAIT_MAX) {
            return 0;
        }
    }

    // Medir duración del pulso en echo
    do {
        DelayUs(10);
        distance += 10;
        if (distance > MAX_US) {
            return MAX_CM;
        }
    } while (GPIORead(echo));

    return (distance / US2CM);
}

uint16_t HcSr04ReadDistanceInInches(gpio_t echo, gpio_t trigger) {
    uint16_t distance = 0, waiting = 0;

    GPIOOn(trigger);
    DelayUs(10);
    GPIOOff(trigger);

    while (!GPIORead(echo)) {
        DelayUs(10);
        waiting += 10;
        if (waiting > WAIT_MAX) {
            return 0;
        }
    }

    do {
        DelayUs(10);
        distance += 10;
        if (distance > MAX_US) {
            return MAX_INCH;
        }
    } while (GPIORead(echo));

    return (distance / US2INCH);
}

bool HcSr04Deinit(gpio_t echo, gpio_t trigger) {
    GPIODeinitPin(echo);
    GPIODeinitPin(trigger);
    return true;
}
