idf_component_register(SRCS "GpioReader.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer TaskNotifier)
