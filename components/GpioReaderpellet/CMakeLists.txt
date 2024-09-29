idf_component_register(SRCS "GpioReaderpellet.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer TaskNotifier)
