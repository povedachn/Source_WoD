////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

/* ScriptData
SDName: Westfall
SD%Complete: 90
SDComment: Quest support: 155, 1651
SDCategory: Westfall
EndScriptData */

/* ContentData
npc_daphne_stilwell
npc_defias_traitor
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"

/*######
## npc_daphne_stilwell
######*/

enum eEnums
{
    SAY_DS_START        = -1000293,
    SAY_DS_DOWN_1       = -1000294,
    SAY_DS_DOWN_2       = -1000295,
    SAY_DS_DOWN_3       = -1000296,
    SAY_DS_PROLOGUE     = -1000297,

    SPELL_SHOOT         = 6660,
    QUEST_TOME_VALOR    = 1651,
    NPC_DEFIAS_RAIDER   = 6180,
    EQUIP_ID_RIFLE      = 2511
};

class npc_daphne_stilwell : public CreatureScript
{
public:
    npc_daphne_stilwell() : CreatureScript("npc_daphne_stilwell") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_TOME_VALOR)
        {
            DoScriptText(SAY_DS_START, creature);

            if (npc_escortAI* pEscortAI = CAST_AI(npc_daphne_stilwell::npc_daphne_stilwellAI, creature->AI()))
                pEscortAI->Start(true, true, player->GetGUID());
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_daphne_stilwellAI(creature);
    }

    struct npc_daphne_stilwellAI : public npc_escortAI
    {
        npc_daphne_stilwellAI(Creature* creature) : npc_escortAI(creature) {}

        uint32 uiWPHolder;
        uint32 uiShootTimer;

        void Reset()
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                switch (uiWPHolder)
                {
                    case 7:
                        DoScriptText(SAY_DS_DOWN_1, me);
                        break;
                    case 8:
                        DoScriptText(SAY_DS_DOWN_2, me);
                        break;
                    case 9:
                        DoScriptText(SAY_DS_DOWN_3, me);
                        break;
                }
            }
            else
                uiWPHolder = 0;

            uiShootTimer = 0;
        }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            uiWPHolder = waypointId;

            switch (waypointId)
            {
                case 4:
                    SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING_NO_SHEATHE);
                    break;
                case 7:
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 8:
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037f, 1570.213f, 54.961f, 4.283f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 9:
                    me->SetSheath(SHEATH_STATE_RANGED);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836f, 1569.755f, 54.267f, 4.230f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697f, 1569.124f, 54.421f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237f, 1568.307f, 54.620f, 4.206f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037f, 1570.213f, 54.961f, 4.283f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.018f, 1570.738f, 54.828f, 4.220f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    break;
                case 10:
                    SetRun(false);
                    break;
                case 11:
                    DoScriptText(SAY_DS_PROLOGUE, me);
                    break;
                case 13:
                    SetEquipmentSlots(true);
                    me->SetSheath(SHEATH_STATE_UNARMED);
                    me->HandleEmoteCommand(EMOTE_STATE_USE_STANDING_NO_SHEATHE);
                    break;
                case 17:
                    player->GroupEventHappens(QUEST_TOME_VALOR, me);
                    break;
            }
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (me->Attack(who, false))
            {
                me->AddThreat(who, 0.0f);
                me->SetInCombatWith(who);
                who->SetInCombatWith(me);

                me->GetMotionMaster()->MoveChase(who, 30.0f);
            }
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->AI()->AttackStart(me);
        }

        void Update(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;

            if (uiShootTimer <= diff)
            {
                uiShootTimer = 1500;

                if (!me->IsWithinDist(me->getVictim(), ATTACK_DISTANCE))
                    DoCast(me->getVictim(), SPELL_SHOOT);
            }
            else
                uiShootTimer -= diff;
        }
    };
};

/*######
## npc_defias_traitor
######*/

#define SAY_START                   -1000101
#define SAY_PROGRESS                -1000102
#define SAY_END                     -1000103
#define SAY_AGGRO_1                 -1000104
#define SAY_AGGRO_2                 -1000105

#define QUEST_DEFIAS_BROTHERHOOD    155

class npc_defias_traitor : public CreatureScript
{
public:
    npc_defias_traitor() : CreatureScript("npc_defias_traitor") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
        {
            if (npc_escortAI* pEscortAI = CAST_AI(npc_defias_traitor::npc_defias_traitorAI, creature->AI()))
                pEscortAI->Start(true, true, player->GetGUID());

            DoScriptText(SAY_START, creature, player);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_defias_traitorAI(creature);
    }

    struct npc_defias_traitorAI : public npc_escortAI
    {
        npc_defias_traitorAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 35:
                    SetRun(false);
                    break;
                case 36:
                    DoScriptText(SAY_PROGRESS, me, player);
                    break;
                case 44:
                    DoScriptText(SAY_END, me, player);
                    player->GroupEventHappens(QUEST_DEFIAS_BROTHERHOOD, me);
                    break;
            }
        }

        void EnterCombat(Unit* who)
        {
            DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2), me, who);
        }

        void Reset() {}
    };
};

enum clodoEnum
{
    NPC_HOMELESS_STORMWIND_CITIZEN      = 42384,
    NPC_HOMELESS_STORMWIND_CITIZEN2     = 42386,
    NPC_WEST_PLAINS_DRIFTER             = 42391,
    KILL_CREDIT_WESTFALL_STEW           = 42617

};

class npc_westfall_stew : public CreatureScript
{
    public:
        npc_westfall_stew() : CreatureScript("npc_westfall_stew") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_westfall_stewAI (creature);
        }

        struct npc_westfall_stewAI : public ScriptedAI
        {
            npc_westfall_stewAI(Creature* creature) : ScriptedAI(creature) {}

            uint32 time;
            bool booltest;

            void Reset()
            {
                time = 30000;
                booltest = true;

                if (!me->ToTempSummon() || !me->ToTempSummon()->GetSummoner())
                    return;

                if (Player* invocer = me->ToTempSummon()->GetSummoner()->ToPlayer())
                {
                    std::list<Creature*> clodoList;
                    GetCreatureListWithEntryInGrid(clodoList, me, NPC_HOMELESS_STORMWIND_CITIZEN, 15.0f);

                    for (Creature* clodo : clodoList)
                    {
                        if (clodo->getStandState() != UNIT_STAND_STATE_SIT)
                        {
                            clodo->GetMotionMaster()->MoveFollow(me, 1, me->GetAngle(clodo));
                            clodo->SetStandState(UNIT_STAND_STATE_SIT);
                            invocer->KilledMonsterCredit(KILL_CREDIT_WESTFALL_STEW, 0);
                        }
                    }
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (booltest)
                {
                    if (time < diff)
                    {
                        std::list<Creature*> clodoList;
                        GetCreatureListWithEntryInGrid(clodoList, me, NPC_HOMELESS_STORMWIND_CITIZEN, 15.0f);

                        for (Creature* clodo : clodoList)
                        {
                            clodo->SetStandState(UNIT_STAND_STATE_STAND);
                            clodo->SetDefaultMovementType(RANDOM_MOTION_TYPE);
                        }

                        me->DespawnOrUnsummon();
                        booltest = false;
                    }
                    else
                        time -= diff;
                }
            }
        };
};

#ifndef __clang_analyzer__
void AddSC_westfall()
{
    new npc_daphne_stilwell();
    new npc_defias_traitor();
    new npc_westfall_stew();
}
#endif
