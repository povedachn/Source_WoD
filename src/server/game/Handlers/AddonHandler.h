////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ADDONHANDLER_H
#define __ADDONHANDLER_H

#include "Common.h"
#include "Config.h"
#include "WorldPacket.h"

class AddonHandler
{
    /* Construction */
    friend class ACE_Singleton<AddonHandler, ACE_Null_Mutex>;
    AddonHandler();

    public:
        ~AddonHandler();
                                                            //build addon packet
        bool BuildAddonPacket(WorldPacket* Source, WorldPacket* Target);
};
#define sAddOnHandler ACE_Singleton<AddonHandler, ACE_Null_Mutex>::instance()
#endif

