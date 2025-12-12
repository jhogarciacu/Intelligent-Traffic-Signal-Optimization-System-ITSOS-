/*
 * PROYECTO: Nodo Semáforo IoT (ESP-IDF)
 * DESCRIPCIÓN: Recibe comandos MQTT para controlar GPIOs
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h" // Librería auxiliar del ejemplo para conectar WiFi fácil
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

static const char *TAG = "SEMAFORO_ESP32";

// ============================================================
// 1. CONFIGURACIÓN DE PINES (AJUSTA SI ES NECESARIO)
// ============================================================
// Semáforo Norte-Sur

#define PIN_NS_ROJO     0
#define PIN_NS_AMARILLO 1
#define PIN_NS_VERDE    2   // (Cuidado: El IO2 suele ser el LED azul de la placa)

// Semáforo Este-Oeste
#define PIN_EO_ROJO     4
#define PIN_EO_AMARILLO 5
#define PIN_EO_VERDE    6

// Máscara para configurar todos los pines juntos
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<PIN_NS_ROJO) | (1ULL<<PIN_NS_AMARILLO) | (1ULL<<PIN_NS_VERDE) | \
                              (1ULL<<PIN_EO_ROJO) | (1ULL<<PIN_EO_AMARILLO) | (1ULL<<PIN_EO_VERDE))
// Tópico MQTT
#define TOPIC_COMANDOS "semaforos/comandos"

// ============================================================
// 2. FUNCIONES DE HARDWARE
// ============================================================

void configurar_gpios(void)
{
    // Configuración básica de pines como salida
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // Iniciar todo apagado
    gpio_set_level(PIN_NS_ROJO, 0); gpio_set_level(PIN_NS_AMARILLO, 0); gpio_set_level(PIN_NS_VERDE, 0);
    gpio_set_level(PIN_EO_ROJO, 0); gpio_set_level(PIN_EO_AMARILLO, 0); gpio_set_level(PIN_EO_VERDE, 0);
    
    ESP_LOGI(TAG, "GPIOs configurados correctamente.");
}

void aplicar_estado(const char *data, int len)
{
    // 1. Seguridad: Apagar todo primero
    gpio_set_level(PIN_NS_ROJO, 0); gpio_set_level(PIN_NS_AMARILLO, 0); gpio_set_level(PIN_NS_VERDE, 0);
    gpio_set_level(PIN_EO_ROJO, 0); gpio_set_level(PIN_EO_AMARILLO, 0); gpio_set_level(PIN_EO_VERDE, 0);

    // 2. Comparar comando recibido
    if (strncmp(data, "NS_GREEN", len) == 0) {
        gpio_set_level(PIN_NS_VERDE, 1);
        gpio_set_level(PIN_EO_ROJO, 1);
        ESP_LOGI(TAG, "ACCION: Verde Norte-Sur");
    } 
    else if (strncmp(data, "NS_YELLOW", len) == 0) {
        gpio_set_level(PIN_NS_AMARILLO, 1);
        gpio_set_level(PIN_EO_ROJO, 1);
        ESP_LOGI(TAG, "ACCION: Amarillo Norte-Sur");
    }
    else if (strncmp(data, "EO_GREEN", len) == 0) {
        gpio_set_level(PIN_EO_VERDE, 1);
        gpio_set_level(PIN_NS_ROJO, 1);
        ESP_LOGI(TAG, "ACCION: Verde Este-Oeste");
    }
    else if (strncmp(data, "EO_YELLOW", len) == 0) {
        gpio_set_level(PIN_EO_AMARILLO, 1);
        gpio_set_level(PIN_NS_ROJO, 1);
        ESP_LOGI(TAG, "ACCION: Amarillo Este-Oeste");
    }
    else if (strncmp(data, "ALL_RED", len) == 0) {
        gpio_set_level(PIN_NS_ROJO, 1);
        gpio_set_level(PIN_EO_ROJO, 1);
        ESP_LOGI(TAG, "ACCION: Todo Rojo");
    }
    else {
        ESP_LOGW(TAG, "Comando desconocido: %.*s", len, data);
    }
}

// ============================================================
// 3. LOGICA MQTT
// ============================================================

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Conectado! Suscribiendo a %s...", TOPIC_COMANDOS);
        esp_mqtt_client_subscribe(client, TOPIC_COMANDOS, 0);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Mensaje recibido (Len=%d): %.*s", event->data_len, event->data_len, event->data);
        aplicar_estado(event->data, event->data_len);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "Error MQTT");
        break;
        
    default:
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://172.20.10.6:1883",
        // Cuando uses tu Gateway local, cambia esto por: "mqtt://192.168.1.XX:1883"
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

// ============================================================
// 4. MAIN
// ============================================================

void app_main(void)
{
    ESP_LOGI(TAG, "[INICIO] Sistema Semáforo ESP32 arrancando...");

    // 1. Configurar LEDs
    configurar_gpios();

    // 2. Inicializar NVS (Necesario para WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 3. Conectar a WiFi (Usa config de menuconfig)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect()); // Esta función viene del ejemplo original

    // 4. Iniciar MQTT
    mqtt_app_start();
}