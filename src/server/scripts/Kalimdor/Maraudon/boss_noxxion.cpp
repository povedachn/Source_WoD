////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

/* ScriptData
SDName: Boss_Noxxion
SD%Complete: 100
SDComment:
SDCategory: Maraudon
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_TOXICVOLLEY           = 21687,
    SPELL_UPPERCUT              = 22916
};

class boss_noxxion : public CreatureScript
{
public:
    boss_noxxion() : CreatureScript("boss_noxxion") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_noxxionAI (creature);
    }

    struct boss_noxxionAI : public ScriptedAI
    {
        boss_noxxionAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 ToxicVolleyTimer;
        uint32 UppercutTimer;
        uint32 AddsTimer;
        uint32 InvisibleTimer;
        bool Invisible;

        void Reset()
        {
            ToxicVolleyTimer = 7000;
            UppercutTimer = 16000;
            AddsTimer = 19000;
            InvisibleTimer = 15000;                            //Too much too low?
            Invisible = false;
        }

        void EnterCombat(Unit* /*who*/) {}

        void SummonAdds(Unit* victim)
        {
            if (Creature* Add = DoSpawnCreature(13456, float(irand(-7, 7)), float(irand(-7, 7)), 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 90000))
                Add->AI()->AttackStart(victim);
        }

        void UpdateAI(const uint32 diff)
        {
            if (Invisible && InvisibleTimer <= diff)
            {
                //Become visible again
                me->setFaction(14);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //Noxxion model
                me->SetDisplayId(11172);
                Invisible = false;
                //me->m_canMove = true;
            }
            else if (Invisible)
            {
                InvisibleTimer -= diff;
                //Do nothing while invisible
                return;
            }

            //Return since we have no target
            if (!UpdateVictim())
                return;

            //ToxicVolleyTimer
            if (ToxicVolleyTimer <= diff)
            {
                DoCastVictim(SPELL_TOXICVOLLEY);
                ToxicVolleyTimer = 9000;
            }
            else ToxicVolleyTimer -= diff;

            //UppercutTimer
            if (UppercutTimer <= diff)
            {
                DoCastVictim(SPELL_UPPERCUT);
                UppercutTimer = 12000;
            }
            else UppercutTimer -= diff;

            //AddsTimer
            if (!Invisible && AddsTimer <= diff)
            {
                //Interrupt any spell casting
                //me->m_canMove = true;
                me->InterruptNonMeleeSpells(false);
                me->setFaction(35);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                // Invisible Model
                me->SetDisplayId(11686);
                SummonAdds(me->getVictim());
                SummonAdds(me->getVictim());
                SummonAdds(me->getVictim());
                SummonAdds(me->getVictim());
                SummonAdds(me->getVictim());
                Invisible = true;
                InvisibleTimer = 15000;

                AddsTimer = 40000;
            }
            else AddsTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

#ifndef __clang_analyzer__
void AddSC_boss_noxxion()
{
    new boss_noxxion();
}
#endif
