////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

/* ScriptData
SDName: Wetlands
SD%Complete: 80
SDComment: Quest support: 1249
SDCategory: Wetlands
EndScriptData */

/* ContentData
npc_mikhail
npc_tapoke_slim_jahn
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"

/*######
## npc_tapoke_slim_jahn
######*/

enum eTapokeSlim
{
    QUEST_MISSING_DIPLO_PT11    = 1249,
    FACTION_ENEMY               = 168,
    SPELL_STEALTH               = 1785,
    SPELL_CALL_FRIENDS          = 16457,                    //summons 1x friend
    NPC_SLIMS_FRIEND            = 4971,
    NPC_TAPOKE_SLIM_JAHN        = 4962
};

class npc_tapoke_slim_jahn : public CreatureScript
{
public:
    npc_tapoke_slim_jahn() : CreatureScript("npc_tapoke_slim_jahn") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tapoke_slim_jahnAI(creature);
    }

    struct npc_tapoke_slim_jahnAI : public npc_escortAI
    {
        npc_tapoke_slim_jahnAI(Creature* creature) : npc_escortAI(creature) { }

        bool IsFriendSummoned;

        void Reset()
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
                IsFriendSummoned = false;
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 2:
                    if (me->HasStealthAura())
                        me->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                    SetRun();
                    me->setFaction(FACTION_ENEMY);
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING) && !IsFriendSummoned && GetPlayerForEscort())
            {
                for (uint8 i = 0; i < 3; ++i)
                    DoCast(me, SPELL_CALL_FRIENDS, true);

                IsFriendSummoned = true;
            }
        }

        void JustSummoned(Creature* summoned)
        {
            if (Player* player = GetPlayerForEscort())
                summoned->AI()->AttackStart(player);
        }

        void AttackedBy(Unit* pAttacker)
        {
            if (me->getVictim())
                return;

            if (me->IsFriendlyTo(pAttacker))
                return;

            AttackStart(pAttacker);
        }

        void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage, SpellInfo const*  /*p_SpellInfo*/)
        {
            if (HealthBelowPct(20))
            {
                if (Player* player = GetPlayerForEscort())
                {
                    if (player->IsPlayer())
                        CAST_PLR(player)->GroupEventHappens(QUEST_MISSING_DIPLO_PT11, me);

                    uiDamage = 0;

                    me->RestoreFaction();
                    me->RemoveAllAuras();
                    me->DeleteThreatList();
                    me->CombatStop(true);

                    SetRun(false);
                }
            }
        }
    };
};

/*######
## npc_mikhail
######*/

class npc_mikhail : public CreatureScript
{
public:
    npc_mikhail() : CreatureScript("npc_mikhail") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_MISSING_DIPLO_PT11)
        {
            Creature* pSlim = creature->FindNearestCreature(NPC_TAPOKE_SLIM_JAHN, 25.0f);

            if (!pSlim)
                return false;

            if (!pSlim->HasStealthAura())
                pSlim->CastSpell(pSlim, SPELL_STEALTH, true);

            if (npc_tapoke_slim_jahn::npc_tapoke_slim_jahnAI* pEscortAI = CAST_AI(npc_tapoke_slim_jahn::npc_tapoke_slim_jahnAI, pSlim->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), quest);
        }
        return false;
    }
};

#ifndef __clang_analyzer__
void AddSC_wetlands()
{
    new npc_tapoke_slim_jahn();
    new npc_mikhail();
}
#endif
