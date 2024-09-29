idf_component_register(SRCS "ControlMotor.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES MotorDriver TaskNotifier OutPutTTL)