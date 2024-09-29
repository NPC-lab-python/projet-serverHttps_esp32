#ifndef CONFIGURATIONS_H
#define CONFIGURATIONS_H

#include "IConfig.h"

class Config : public IConfig
{
private:

    const char *connecte;
    const char *meta_device;
    const char *meta_activate;
    const char *meta_pellet_delivered;
    const char *pellet_delivered;
    const char *beam_nose_poke;
    const char *beam_pellet;
    const char *setter;
    const char *reponses;

public:
    // Constructeur avec initialisation de membres

    /*Config(const char *c, const char *md, const char *ma, const char *mpd, const char *pd,
           const char *bnp, const char *bp, const char *s, const char *r) : connecte(c), meta_device(md), meta_activate(ma), meta_pellet_delivered(mpd),
                                                                            pellet_delivered(pd), beam_nose_poke(bnp), beam_pellet(bp), setter(s), reponses(r) {}*/
    Config(const char *c, const char *md, const char *ma, const char *mpd, const char *pd,
           const char *bnp, const char *bp, const char *s, const char *r);
    Config(void);

    // Implémentation des méthodes d'accès
    const char *getConnecte() const override { return connecte; }
    const char *getMetaDevice() const override { return meta_device; }
    const char *getMetaActivate() const override { return meta_activate; }
    const char *getMetaPelletDelivered() const override { return meta_pellet_delivered; }
    const char *getPelletDelivered() const override { return pellet_delivered; }
    const char *getBeamNosePoke() const override { return beam_nose_poke; }
    const char *getBeamPellet() const override { return beam_pellet; }
    const char *getSetter() const override { return setter; }
    const char *getReponses() const override { return reponses; }
};

class ConfigTmaze : public IConfigTmaze
{
private:

    const char *connecte;

    const char *Property_ServoRight;
    const char *Property_ServoLeft;
    const char *Property_BeamRight;
    const char *Property_BeamLeft;

    const char *meta_ServoRight;
    const char *meta_ServoLeft;
    const char *meta_BeamRight;
    const char *meta_BeamLeft;

    const char *ServoRight;
    const char *ServoLeft;
    const char *BeamRight;
    const char *BeamLeft;

    const char *setter;
    const char *reponses;

public:
    // Constructeur avec initialisation de membres
    ConfigTmaze(const char *c, const char *PServoRight, 
                const char *PServoLeft, const char *PBeamRight, 
                const char *PBeamLeft,const char *mServoRight,
                const char *mServoLeft,const char *mBeamRight,
                const char *mBeamLeft,const char *SRight, const char *SLeft,
                const char *BRight,const char *BLeft,const char *s, const char *r);

    ConfigTmaze(void);

    // Implémentation des méthodes d'accès
    const char *getConnecte() const override { return connecte; }

    const char *getPropertyServoRight() const override { return Property_ServoRight; }
    const char *getPropertyServoLeft() const override { return Property_ServoLeft; }
    const char *getPropertyBeamRight() const override { return Property_BeamRight; }
    const char *getPropertyBeamLeft() const override { return Property_BeamLeft; }

    const char *getMetaServoRight() const override { return meta_ServoRight; }
    const char *getMetaServoLeft() const override { return meta_ServoLeft; }
    const char *getMetaBeamRight() const override { return meta_BeamRight; }
    const char *getMetaBeamLeft() const override { return meta_BeamLeft; }

    const char *getServoRight() const override { return ServoRight; }
    const char *getServoLeft() const override { return ServoLeft; }
    const char *getBeamRight() const override { return BeamRight; }
    const char *getBeamLeft() const override { return BeamLeft; }

    const char *getSetter() const override { return setter; }
    const char *getReponses() const override { return reponses; }
};
#endif // CONFIGURATIONS_H
