////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GARRISON_ENCHANTER_STUDY_HORDE_HPP_GARRISON
#ifndef CROSS
#define GARRISON_ENCHANTER_STUDY_HORDE_HPP_GARRISON

#include "../../../GarrisonScriptData.hpp"
#include "../../../GarrisonNPC.hpp"
#include "../../ProfessionBuilding.hpp"
#include "GarrisonMgr.hpp"

namespace MS { namespace Garrison
{
    //////////////////////////////////////////////////////////////////////////
    /// 79821 - Yukla Greenshadow                                         ////
    //////////////////////////////////////////////////////////////////////////
    namespace npc_YuklaGreenshadowAIData
    {
        extern InitSequenceFunction FnLevel1;
        extern InitSequenceFunction FnLevel2;
        extern InitSequenceFunction FnLevel3;

        extern char ScriptName[];
    }

    using npc_YuklaGreenshadow = ProfessionBuilding_SkillNPC<npc_YuklaGreenshadowAIData::ScriptName, SKILL_ENCHANTING, Quests::Horde_YourFirstEnchantingWorkOrder, 79821, &npc_YuklaGreenshadowAIData::FnLevel1, &npc_YuklaGreenshadowAIData::FnLevel2, &npc_YuklaGreenshadowAIData::FnLevel3>;

    //////////////////////////////////////////////////////////////////////////
    /// 79820 - Garra                                                     ////
    //////////////////////////////////////////////////////////////////////////
    namespace npc_GarraAIData
    {
        extern InitSequenceFunction FnLevel1;
        extern InitSequenceFunction FnLevel2;
        extern InitSequenceFunction FnLevel3;
    }

    class npc_Garra : public CreatureScript
    {
        public:
            /// Constructor
            npc_Garra();

            /// Called when a player opens a gossip dialog with the GameObject.
            /// @p_Player     : Source player instance
            /// @p_Creature   : Target GameObject instance
            virtual bool OnGossipHello(Player* p_Player, Creature* p_Creature) override;

            /// Called when a player selects a gossip item in the creature's gossip menu.
            /// @p_Player   : Source player instance
            /// @p_Creature : Target creature instance
            /// @p_Sender   : Sender menu
            /// @p_Action   : Action
            virtual bool OnGossipSelect(Player* p_Player, Creature* p_Creature, uint32 p_Sender, uint32 p_Action) override;

            /// Called when a CreatureAI object is needed for the creature.
            /// @p_Creature : Target creature instance
            virtual CreatureAI* GetAI(Creature* p_Creature) const override;

            /// Creature AI
            struct npc_GarraAI : public GarrisonNPCAI
            {
                /// Constructor
                npc_GarraAI(Creature* p_Creature);

                uint64 m_OwnerGuid = 0;

                void SetGUID(uint64 p_Guid, int32 p_Id) override;

                /// When the PlotInstance ID is set
                /// @p_BuildingID : Set plot instance ID
                virtual void OnSetPlotInstanceID(uint32 p_PlotInstanceID) override;

                /// When the building ID is set
                /// @p_BuildingID : Set building ID
                virtual void OnSetBuildingID(uint32 p_BuildingID) override
                {
                    m_OnPointReached.clear();

                    GarrBuildingEntry const* l_BuildingEntry = sGarrBuildingStore.LookupEntry(p_BuildingID);

                    if (!l_BuildingEntry)
                        return;

                    switch (l_BuildingEntry->Level)
                    {
                        case 1:
                            (npc_GarraAIData::FnLevel1)(this, me);
                            break;

                        case 2:
                            (npc_GarraAIData::FnLevel2)(this, me);
                            break;

                        case 3:
                            (npc_GarraAIData::FnLevel3)(this, me);
                            break;

                        default:
                            break;
                    }
                }
            };

    };

}   ///< namespace Garrison
}   ///< namespace MS

#endif  ///< GARRISON_ENCHANTER_STUDY_HORDE_HPP_GARRISON
#endif
