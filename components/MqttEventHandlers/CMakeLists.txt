idf_component_register(SRCS "MqttEventHandlers.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES mqtt freertos Configurations ServoSCS15)