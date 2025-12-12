idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES mqtt nvs_flash esp_netif protocol_examples_common driver esp_wifi)