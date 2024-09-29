
#include "GpioReader.h"
#include "GpioReaderpellet.h"
#include "WifiCpp.h"
#include "CPPMqtt.h"
#include "ControlMotor.h"
#include "freertos/semphr.h"
#include "Configurations.h"
#include "MqttEventHandlers.h"

// Définition des broches du moteur
#define MOTOR_STEP_PIN GPIO_NUM_13 // Exemple de broche de pas
#define MOTOR_DIR_PIN GPIO_NUM_12


const char *nameHost;
const char *iphostmqtt;
const char *usernamemqtt;
const char *clientIDmqtt;
const char *testament;
const char *passmqtt;

// const char *connecte;

// const char *Property_ServoRight;
// const char *Property_ServoLeft;
// const char *Property_BeamRight;
// const char *Property_BeamLeft;

// const char *meta_ServoRight;
// const char *meta_ServoLeft;
// const char *meta_BeamRight;
// const char *meta_BeamLeft;

// const char *ServoRight;
// const char *ServoLeft;
// const char *BeamRight;
// const char *BeamLeft;

// const char *setter;
// const char *reponses;