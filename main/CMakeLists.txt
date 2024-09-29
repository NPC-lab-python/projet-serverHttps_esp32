idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_https_server ServoSCS15 WifiCpp CPPMqtt GpioReader ControlMotor GpioReaderpellet)
