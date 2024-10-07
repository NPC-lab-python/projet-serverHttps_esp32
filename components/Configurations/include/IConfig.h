#include <string>
#ifndef ICONFIG_H
#define ICONFIG_H

class IConfig {
public:
    virtual ~IConfig() = default;

    // Déclaration des méthodes d'accès comme fonctions virtuelles pures
    virtual const char* getConnecte() const = 0;
    virtual const char* getMetaDevice() const = 0;
    virtual const char* getMetaActivate() const = 0;
    virtual const char* getMetaPelletDelivered() const = 0;
    virtual const char* getPelletDelivered() const = 0;
    virtual const char* getBeamNosePoke() const = 0;
    virtual const char* getBeamPellet() const = 0;
    virtual const char* getSetter() const = 0;
    virtual const char* getReponses() const = 0;
};
class IConfigTmaze {
public:
    virtual ~IConfigTmaze() = default;

    // Déclaration des méthodes d'accès comme fonctions virtuelles pures
    virtual const char* getConnecte() const = 0;

    virtual const char* getPropertyServoRight() const = 0;
    virtual const char* getPropertyServoLeft() const = 0;
    virtual const char* getPropertyBeamRight() const = 0;
    virtual const char* getPropertyBeamLeft() const = 0;

    virtual const char* getMetaServoRight() const = 0;
    virtual const char* getMetaServoLeft() const = 0;
    virtual const char* getMetaBeamRight() const = 0;
    virtual const char* getMetaBeamLeft() const = 0;

    virtual const char* getServoRight() const = 0;
    virtual const char* getServoLeft() const = 0;
    virtual const char* getBeamRight() const = 0;
    virtual const char* getBeamLeft() const = 0;

    virtual const char* getSetter() const = 0;
    virtual const char* getReponses() const = 0;


    virtual std::string getHostname() const = 0;
    virtual std::string getUsername() const = 0;
    virtual std::string getPassword() const = 0;
    virtual std::string getClientID() const = 0;
    virtual std::string getTestamentTopic() const = 0;
};
#endif // ICONFIG_H
