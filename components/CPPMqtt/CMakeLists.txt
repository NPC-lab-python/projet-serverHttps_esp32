set(SOURCES CPPMqtt.cpp)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "include"
                    REQUIRES CPPGpio mqtt TaskNotifier MqttEventHandlers)
                    