#include "Configurations.h"
/*const char *Config::connecte = "sasha/01/F01/$connected";
const char *Config::meta_device = "sasha/01/F01/feeder_1/$meta";
const char *Config::meta_activate = "sasha/01/F01/feeder_1/activate/$meta";
const char *Config::meta_pellet_delivered = "sasha/01/F01/feeder_1/pellet_delivered/$meta";
const char *Config::pellet_delivered = "sasha/01/F01/feeder_1/pellet_delivered";
const char *Config::beam_nose_poke = "sasha/01/F01/feeder_1/nose_poke";
const char *Config::beam_pellet = "sasha/01/F01/feeder_1/beam_pellet";
const char *Config::setter = "sasha/01/F01/feeder_1/activate/set";
const char *Config::reponses = "1";

Config config1("sasha/01/F01/$connected", 
"sasha/01/F01/feeder_1/$meta", 
"sasha/01/F01/feeder_1/activate/$meta", 
"sasha/01/F01/feeder_1/pellet_delivered/$meta",
 "sasha/01/F01/feeder_1/pellet_delivered", 
 "sasha/01/F01/feeder_1/nose_poke", 
 "sasha/01/F01/feeder_1/beam_pellet", 
 "sasha/01/F01/feeder_1/activate/set", "1");

Config config2("sasha/02/F02/$connected", 
"sasha/02/F02/feeder_2/$meta", 
"sasha/02/F02/feeder_2/activate/$meta", 
"sasha/02/F02/feeder_2/pellet_delivered/$meta",
 "sasha/02/F02/feeder_2/pellet_delivered", 
 "sasha/02/F02/feeder_2/nose_poke", 
 "sasha/02/F02/feeder_2/beam_pellet", 
 "sasha/02/F02/feeder_2/activate/set", "1");

Config config3("sasha/03/F03/$connected", 
"sasha/03/F03/feeder_3/$meta", 
"sasha/03/F03/feeder_3/activate/$meta", 
"sasha/03/F03/feeder_3/pellet_delivered/$meta",
 "sasha/03/F03/feeder_3/pellet_delivered", 
 "sasha/03/F03/feeder_3/nose_poke", 
 "sasha/03/F03/feeder_3/beam_pellet", 
 "sasha/03/F03/feeder_3/activate/set", "1");
 */
Config::Config(){}

Config::Config(const char* c, const char* md, const char* ma, const char* mpd, const char* pd,
            const char* bnp, const char* bp, const char* s, const char* r) :
            connecte(c), meta_device(md), meta_activate(ma), meta_pellet_delivered(mpd),
            pellet_delivered(pd), beam_nose_poke(bnp), beam_pellet(bp), setter(s), reponses(r) {}

ConfigTmaze::ConfigTmaze(){}

ConfigTmaze::ConfigTmaze(const char *c, const char *PServoRight, 
                const char *PServoLeft, const char *PBeamRight, 
                const char *PBeamLeft,const char *mServoRight,
                const char *mServoLeft,    const char *mBeamRight,
                const char *mBeamLeft,const char *SRight, const char *SLeft,
                const char *BRight,const char *BLeft,const char *s, const char *r) :
                connecte(c), Property_ServoRight(PServoRight), 
                Property_ServoLeft(PServoLeft), Property_BeamRight(PBeamRight), 
                Property_BeamLeft(PBeamLeft),meta_ServoRight(mServoRight),
                meta_ServoLeft(mServoLeft),meta_BeamRight(mBeamRight),
                meta_BeamLeft(mBeamLeft),ServoRight(SRight),ServoLeft(SLeft),
                BeamRight(BRight),BeamLeft(BLeft),setter(s), reponses(r) {}