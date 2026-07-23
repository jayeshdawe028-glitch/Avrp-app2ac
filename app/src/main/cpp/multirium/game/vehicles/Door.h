//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_DOOR_H
#define MULTIRIUM_DOOR_H

#include "../main.h"

class CDoor {
private:
    float m_fOpenAngle;
    float m_fClosedAngle;
    uint16 m_nDirn;
    uint8 m_nAxis;
    uint8 m_nDoorState;
    float m_fAngle;
    float m_fPrevAngle;
    float m_fAngVel;
};


#endif //MULTIRIUM_DOOR_H
