#pragma once
#ifndef __DESK_LAMP_LASER_H__
#define __DESK_LAMP_LASER_H__

#include "Weapon.h"

class DeskLampLaser : public Weapon
{
public:
    static DeskLampLaser* create(Player* owner);
    bool initDeskLampLaser(Player* owner);
    virtual void fire() override;
};

#endif
