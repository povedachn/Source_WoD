////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

/* ScriptData
SDName: Boss_Houndmaster_Loksey
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum eEnums
{
    SAY_AGGRO                       = -1189021,
    SPELL_SUMMONSCARLETHOUND        = 17164,
    SPELL_BLOODLUST                 = 6742
};

class boss_houndmaster_loksey : public CreatureScript
{
public:
    boss_houndmaster_loksey() : CreatureScript("boss_houndmaster_loksey") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_houndmaster_lokseyAI (creature);
    }

    struct boss_houndmaster_lokseyAI : public ScriptedAI
    {
        boss_houndmaster_lokseyAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 BloodLust_Timer;

        void Reset()
        {
            BloodLust_Timer = 20000;
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoScriptText(SAY_AGGRO, me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (BloodLust_Timer <= diff)
            {
                DoCast(me, SPELL_BLOODLUST);
                BloodLust_Timer = 20000;
            }
            else BloodLust_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

#ifndef __clang_analyzer__
void AddSC_boss_houndmaster_loksey()
{
    new boss_houndmaster_loksey();
}
#endif
