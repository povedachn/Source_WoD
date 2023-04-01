////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef TRINITY_REACTORAI_H
#define TRINITY_REACTORAI_H

#include "CombatAI.h"

class Unit;

class ReactorAI : public AggressorAI
{
    public:

        explicit ReactorAI(Creature* c) : AggressorAI(c) {}

        void MoveInLineOfSight(Unit*) {};

        static int Permissible(const Creature*);
};
#endif

