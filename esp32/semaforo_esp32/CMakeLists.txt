cmake_minimum_required(VERSION 3.5)

# ----------------------------------------------------------------------------
# ESTA LÍNEA ES EL "PUENTE"
# Le dice al compilador: "Busca la librería de conexión WiFi en la carpeta
# de ejemplos de instalación de ESP-IDF, porque yo estoy fuera de ella".
# ----------------------------------------------------------------------------
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(semaforo_esp32)