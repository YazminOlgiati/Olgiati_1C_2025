/*! @mainpage Proyecto integrador
 *
 * @section genDesc General Description
 *
 * Proyecto que utiliza un sensor ultrasónico HC-SR04 y un servo SG90 para abrir/cerrar una puerta.
 *
 * @section hardConn Hardware Connection
 *
 * |    Componente     |   ESP32 GPIO   |
 * |:-----------------:|:--------------:|
 * |  Trigger (HC-SR04)|    GPIO_5      |
 * |  Echo (HC-SR04)   |    GPIO_13     |
 * |  Servo (SG90)     |    GPIO_4      |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                     |
 * |:----------:|:--------------------------------|
 * | 22/05/2025 | Creación del proyecto           |
 *
 * @author Yazmin Olgiati (yazmin.olgiati@ingenieria.uner.edu.ar)
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "servo_sg90.h"
#include "ble_mcu.h"
#include "neopixel_stripe.h"
#include <string.h>

/*==================[macros and definitions]===============================*/
/*==================[macros and definitions]===============================*/

#define PERIODO_SENSOR_US        2000000   // 2 segundos
#define TIEMPO_CIERRE_US         5000000   // 5 segundos

#define CONFIG_BLINK_PERIOD      500

/*==================[internal data definition]============================*/

TaskHandle_t sensor_task_handle = NULL;
TaskHandle_t cierre_task_handle = NULL;

gpio_t gpio_trigger_1 = GPIO_2;
gpio_t gpio_echo_1 = GPIO_3;
gpio_t gpio_trigger_2 = GPIO_19;
gpio_t gpio_echo_2 = GPIO_20;
gpio_t gpio_servo = GPIO_9;

static timer_config_t timer_cierre;

/*==================[Funciones internas]==================*/

// Timer A → notifica a SensorTask
void SensorTimerCallback(void *param) {
    vTaskNotifyGiveFromISR(sensor_task_handle, pdFALSE);
}

// Timer B → notifica a CierreTask
void CierreTimerCallback(void *param) {
    vTaskNotifyGiveFromISR(cierre_task_handle, pdFALSE);
}

void SensorTask(void *pvParameters) {
    bool puerta_abierta = false;

    


    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // espera Timer A


        HcSr04Init(gpio_echo_1, gpio_trigger_1);
        uint16_t distancia1 = HcSr04ReadDistanceInCentimeters();

        HcSr04Init(gpio_echo_2, gpio_trigger_2);
        uint16_t distancia2 = HcSr04ReadDistanceInCentimeters();

        //uint16_t distancia1 = HcSr04ReadDistanceInCentimeters(gpio_echo_1, gpio_trigger_1);
        //uint16_t distancia2 = HcSr04ReadDistanceInCentimeters(gpio_echo_2, gpio_trigger_2);

        //printf("Distancia: %u cm\n", distancia1);

        if (distancia1 > 0 && distancia1 < 5 && !puerta_abierta) {
            ServoMove(SERVO_0, 90);
            BleSendString("Puerta abierta\n");
            puerta_abierta = true;
            printf("Distancia: %u cm\n", distancia1);
            // reinciar Timer B
            TimerStop(timer_cierre.timer);
            TimerStart(timer_cierre.timer);

        } else if (distancia2 > 0 && distancia2 < 5 && puerta_abierta) {
            ServoMove(SERVO_0, -90);
            BleSendString("Puerta cerrada\n");
            puerta_abierta = false;
            printf("Distancia: %u cm\n", distancia2);

            TimerStop(timer_cierre.timer); // Cancelar cierre automático
        }
    }
}

static void CierreTask(void *pvParameter) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Espera Timer B
        ServoMove(SERVO_0, -90);
        BleSendString("Puerta cerrada automáticamente\n");
    }
}

/*==================[app_main]============================*/

void app_main(void) {
    // Inicialización de sensores y servo
    HcSr04Init(gpio_echo_1, gpio_trigger_1);
    HcSr04Init(gpio_echo_2, gpio_trigger_2);
    ServoInit(SERVO_0, gpio_servo);

    // Configurar timer A (detección periódica)
    timer_config_t timer_sensor = {
        .timer = TIMER_A,
        .period = PERIODO_SENSOR_US,
        .func_p = SensorTimerCallback,
        .param_p = NULL
    };
    TimerInit(&timer_sensor);
    TimerStart(timer_sensor.timer);  

    // Configurar timer B (cierre automático)
    timer_cierre.timer = TIMER_B;
    timer_cierre.period = TIEMPO_CIERRE_US;
    timer_cierre.func_p = CierreTimerCallback;
    timer_cierre.param_p = NULL;
    TimerInit(&timer_cierre);        // se arranca desde SensorTask


    xTaskCreate(&SensorTask, "SensorTask", 2048, NULL, 5, &sensor_task_handle);
    xTaskCreate(&CierreTask, "CierreTask", 2048, NULL, 5, &cierre_task_handle);

    // BLE
    uint8_t blink = 0;
    static neopixel_color_t color;
    ble_config_t ble_config = {
        "ESP_YAZ",
        NULL
    };
    NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    NeoPixelAllOff();
    BleInit(&ble_config);

    // LED de estado BLE
    while (1) {
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch (BleStatus()) {
            case BLE_OFF:
                NeoPixelAllOff();
                break;
            case BLE_DISCONNECTED:
                if (blink % 2)
                    NeoPixelAllColor(NEOPIXEL_COLOR_ROSE);
                else
                    NeoPixelAllOff();
                blink++;
                break;
            case BLE_CONNECTED:
                NeoPixelAllColor(NEOPIXEL_COLOR_GREEN);
                break;
        }
    }
}
/*==================[end of file]============================================*/

/* 

//PRUEBA DEL MOVIMIENTO DEL SERVO

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo_sg90.h"
#include "gpio_mcu.h"

#define SERVO_GPIO GPIO_4  // Cambiá este valor si usás otro pin

void app_main(void) {
    // Inicializo el servo en el pin especificado
    ServoInit(SERVO_0, SERVO_GPIO);

    while (1) {
        printf("Moviendo a -90° (ángulo mínimo)\n");
        ServoMove(SERVO_0, -90);
        vTaskDelay(pdMS_TO_TICKS(2000));  // Espera 2 segundos

        printf("Moviendo a 0° (centro)\n");
        ServoMove(SERVO_0, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));  // Espera 2 segundos

        printf("Moviendo a 90° (ángulo máximo)\n");
        ServoMove(SERVO_0, 90);
        vTaskDelay(pdMS_TO_TICKS(2000));  // Espera 2 segundos
    }
}

*/