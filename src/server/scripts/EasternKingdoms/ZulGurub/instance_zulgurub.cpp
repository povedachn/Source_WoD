////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptPCH.h"
#include "zulgurub.h"

static const DoorData doordata[] =
{
    {GO_VENOXIS_EXIT,           DATA_VENOXIS,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_MANDOKIR_EXIT1,         DATA_MANDOKIR,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_MANDOKIR_EXIT2,         DATA_MANDOKIR,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_MANDOKIR_EXIT3,         DATA_MANDOKIR,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_MANDOKIR_EXIT4,         DATA_MANDOKIR,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_MANDOKIR_EXIT5,         DATA_MANDOKIR,           DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_ZANZIL_EXIT,            DATA_ZANZIL,             DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_KILNARA_EXIT,           DATA_KILNARA,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {0,                         0,                       DOOR_TYPE_ROOM,     BOUNDARY_NONE}
};

class instance_zulgurub : public InstanceMapScript
{
    public:
        instance_zulgurub() : InstanceMapScript("instance_zulgurub", 859) { }
        
        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_zulgurub_InstanceMapScript(map);
        }

        struct instance_zulgurub_InstanceMapScript : public InstanceScript
        {
            instance_zulgurub_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(EncounterCount);
                LoadDoorData(doordata);
                venoxisGUID     = 0;
                mandokirGUID    = 0;
                kilnaraGUID     = 0;
                zanzilGUID      = 0;
                jindoGUID       = 0;
                hazzarahGUID    = 0;
                renatakiGUID    = 0;
                wushoolayGUID   = 0;
                grilekGUID      = 0;
                uiBosses        = 0;
            }
    
            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                   case NPC_VENOXIS:
                      venoxisGUID = creature->GetGUID();
                      break;
                   case NPC_MANDOKIR:
                      mandokirGUID = creature->GetGUID();
                      break;
                   case NPC_KILNARA:
                      kilnaraGUID = creature->GetGUID();
                      break;
                   case NPC_ZANZIL:
                      zanzilGUID = creature->GetGUID();
                      break;
                   case NPC_JINDO:
                      jindoGUID = creature->GetGUID();
                      break;
                   case NPC_HAZZARAH:
                      hazzarahGUID = creature->GetGUID();
                      break;
                   case NPC_RENATAKI:
                      renatakiGUID = creature->GetGUID();
                      break;
                   case NPC_WUSHOOLAY:
                      wushoolayGUID = creature->GetGUID();
                      break;
                   case NPC_GRILEK:
                      grilekGUID = creature->GetGUID();
                      break;
                   default:
                      break;
                }
            }
            
            void OnGameObjectCreate(GameObject* pGo)
            {
                switch (pGo->GetEntry())
                {
                    case GO_VENOXIS_EXIT:
                    case GO_MANDOKIR_EXIT1:
                    case GO_MANDOKIR_EXIT2:
                    case GO_MANDOKIR_EXIT3:
                    case GO_MANDOKIR_EXIT4:
                    case GO_MANDOKIR_EXIT5:
                    case GO_ZANZIL_EXIT:
                    case GO_KILNARA_EXIT:
                        AddDoor(pGo, true);
                        break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (state == DONE)
                {
                    uiBosses++;
                    SaveToDB();
                }

                return true;
            }

            uint32 GetData(uint32 type)
            {
                if (type == DATA_BOSSES)
                    return uiBosses;
                return 0;
            }
            uint64 GetData64(uint32 type)
            {
                switch (type)
                {
                    case DATA_VENOXIS:
                        return venoxisGUID;
                    case DATA_MANDOKIR:
                        return mandokirGUID;
                    case DATA_KILNARA:
                        return kilnaraGUID;
                    case DATA_ZANZIL:
                        return zanzilGUID;
                    case DATA_JINDO:
                        return jindoGUID;
                    case DATA_HAZZARAH:
                        return hazzarahGUID;
                    case DATA_RENATAKI:
                        return renatakiGUID;
                    case DATA_WUSHOOLAY:
                        return wushoolayGUID;
                    case DATA_GRILEK:
                        return grilekGUID;
                    default:
                        break;

                }

                return 0;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "Z G " << uiBosses << " " << GetBossSaveData();

                OUT_SAVE_INST_DATA_COMPLETE;
                return saveStream.str();
            }

            void Load(char const* str)
            {
                if (!str)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(str);

                char dataHead1, dataHead2;

                std::istringstream loadStream(str);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'Z' && dataHead2 == 'G')
                {
                    loadStream >> uiBosses;
                    if (uiBosses > 5)
                        uiBosses = 0;
                    for (uint8 i = 0; i < EncounterCount; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;

                        SetBossState(i, EncounterState(tmpState));
                    }
                }
                else
                    OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        protected:
             uint64 venoxisGUID;
             uint64 mandokirGUID;
             uint64 kilnaraGUID;
             uint64 zanzilGUID;
             uint64 jindoGUID;
             uint64 hazzarahGUID;
             uint64 renatakiGUID;
             uint64 wushoolayGUID;
             uint64 grilekGUID;
             uint32 uiBosses;
        };
};

#ifndef __clang_analyzer__
void AddSC_instance_zulgurub()
{
    new instance_zulgurub();
}
#endif
