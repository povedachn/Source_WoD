////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef TRINITY_PETAI_H
#define TRINITY_PETAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Spell;

class PetAI : public CreatureAI
{
    public:

        explicit PetAI(Creature* c);

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature*);

        void KilledUnit(Unit* /*victim*/) override;
        void AttackStart(Unit* target) override;
        void MovementInform(uint32 moveType, uint32 data) override;
        void OwnerAttackedBy(Unit* attacker);
        void OwnerAttacked(Unit* target) override;
        void ReceiveEmote(Player* player, uint32 textEmote) override;

        // The following aren't used by the PetAI but need to be defined to override
        //  default CreatureAI functions which interfere with the PetAI
        //
        void MoveInLineOfSight(Unit* /*who*/) override { } // CreatureAI interferes with returning pets
        void MoveInLineOfSight_Safe(Unit* /*who*/) { } // CreatureAI interferes with returning pets
        void EnterEvadeMode() override { } // For fleeing, pets don't use this type of Evade mechanic

    private:
        bool _isVisible(Unit*) const;
        bool _needToStop(void);
        void _stopAttack(void);

        void UpdateAllies();

        TimeTracker i_tracker;
        std::set<uint64> m_AllySet;
        uint32 m_updateAlliesTimer;

        Unit* SelectNextTarget(bool allowAutoSelect) const;
        void HandleReturnMovement();
        void DoAttack(Unit* target, bool chase);
        bool CanAttack(Unit* target);
        void ClearCharmInfoFlags();
};
#endif
