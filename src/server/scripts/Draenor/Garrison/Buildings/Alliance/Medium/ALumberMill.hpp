////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GARRISON_LUMBER_MILL_ALLIANCE_HPP_GARRISON
#ifndef CROSS
#define GARRISON_LUMBER_MILL_ALLIANCE_HPP_GARRISON

#include "../../../GarrisonScriptData.hpp"
#include "../../../GarrisonNPC.hpp"
#include "GarrisonMgr.hpp"

namespace MS { namespace Garrison
{
    //////////////////////////////////////////////////////////////////////////
    /// 84248 - Justin Timberlord                                          ///
    //////////////////////////////////////////////////////////////////////////
    namespace npc_JustinTimberLordData
    {
        extern InitSequenceFunction FnLevel1;
        extern InitSequenceFunction FnLevel2;
        extern InitSequenceFunction FnLevel3;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class npc_JustinTimberLord : public CreatureScript
    {
        public:
            /// Constructor
            npc_JustinTimberLord();

            /// Called when a player opens a gossip dialog with the GameObject.
            /// @p_Player     : Source player instance
            /// @p_Creature   : Target creature instance
            virtual bool OnGossipHello(Player* p_Player, Creature* p_Creature) override;

            /// Called when a player selects a gossip item in the creature's gossip menu.
            /// @p_Player   : Source player instance
            /// @p_Creature : Target creature instance
            /// @p_Sender   : Sender menu
            /// @p_Action   : Action
            virtual bool OnGossipSelect(Player* p_Player, Creature* p_Creature, uint32 p_Sender, uint32 p_Action) override;

            /// Called when a player accepts a quest from the creature.
            /// @p_Player   : Source player instance
            /// @p_Creature : Target creature instance
            /// @p_Quest    : Accepted quest
            virtual bool OnQuestAccept(Player* p_Player, Creature* p_Creature, const Quest* p_Quest) override;

            /// Called when player asks from a creature gossip to create a shipment.
            /// @p_Player       : Player that created the shipment
            /// @p_Creature     : NPC that got the gossip
            virtual void OnShipmentCreated(Player* p_Player, Creature* p_Creature, uint32 p_BuildingID) override;

            /// Called when a CreatureAI object is needed for the creature.
            /// @p_Creature : Target creature instance
            CreatureAI* GetAI(Creature* p_Creature) const override;
    };

    class npc_JustinTimberLordAI : public SimpleSequenceCosmeticScriptAI<&npc_JustinTimberLordData::FnLevel1, &npc_JustinTimberLordData::FnLevel2, &npc_JustinTimberLordData::FnLevel3>
    {
        public:
            /// Constructor
            /// @p_Creature : AI Owner
            npc_JustinTimberLordAI(Creature* p_Creature);
    };
}   ///< namespace Garrison
}   ///< namespace MS

#endif  ///< GARRISON_LUMBER_MILL_ALLIANCE_HPP_GARRISON
#endif
