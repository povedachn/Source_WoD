////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

/** \file
    \ingroup world
*/

#include "WeatherMgr.h"
#include "Weather.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "AutoPtr.h"

namespace WeatherMgr
{

namespace
{
    typedef std::unordered_map<uint32, JadeCore::AutoPtr<Weather, ACE_Null_Mutex> > WeatherMap;
    typedef std::unordered_map<uint32, WeatherData> WeatherZoneMap;

    WeatherMap m_weathers;
    WeatherZoneMap mWeatherZoneMap;

    WeatherData const* GetWeatherData(uint32 zone_id)
    {
        WeatherZoneMap::const_iterator itr = mWeatherZoneMap.find(zone_id);
        return (itr != mWeatherZoneMap.end()) ? &itr->second : NULL;
    }
}

/// Find a Weather object by the given zoneid
Weather* FindWeather(uint32 id)
{
    WeatherMap::const_iterator itr = m_weathers.find(id);
    return (itr != m_weathers.end()) ? itr->second.get() : 0;
}

/// Remove a Weather object for the given zoneid
void RemoveWeather(uint32 id)
{
    // not called at the moment. Kept for completeness
    WeatherMap::iterator itr = m_weathers.find(id);

    if (itr != m_weathers.end())
        m_weathers.erase(itr);
}

/// Add a Weather object to the list
Weather* AddWeather(uint32 zone_id)
{
    WeatherData const* weatherChances = GetWeatherData(zone_id);

    // zone does not have weather, ignore
    if (!weatherChances)
        return NULL;

    Weather* w = new Weather(zone_id, weatherChances);
    m_weathers[w->GetZone()].reset(w);
    w->ReGenerate();
    w->UpdateWeather();

    return w;
}

void LoadWeatherData()
{
    uint32 oldMSTime = getMSTime();

    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT "
        // 1        2                   3                       4
        "zone, spring_rain_chance, spring_snow_chance, spring_storm_chance,"
        // 5                     6                   7
        "summer_rain_chance, summer_snow_chance, summer_storm_chance,"
        // 8                     9                  10
        "fall_rain_chance, fall_snow_chance, fall_storm_chance,"
        // 11                   12                  13
        "winter_rain_chance, winter_snow_chance, winter_storm_chance,"
        // 14
        "ScriptName FROM game_weather");

    if (!result)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 weather definitions. DB table `game_weather` is empty.");

        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 zone_id = fields[0].GetUInt32();

        WeatherData& wzc = mWeatherZoneMap[zone_id];

        for (uint8 season = 0; season < WEATHER_SEASONS; ++season)
        {
            wzc.data[season].rainChance  = fields[season * (MAX_WEATHER_TYPE-1) + 1].GetUInt8();
            wzc.data[season].snowChance  = fields[season * (MAX_WEATHER_TYPE-1) + 2].GetUInt8();
            wzc.data[season].stormChance = fields[season * (MAX_WEATHER_TYPE-1) + 3].GetUInt8();

            if (wzc.data[season].rainChance > 100)
            {
                wzc.data[season].rainChance = 25;
                sLog->outError(LOG_FILTER_SQL, "Weather for zone %u season %u has wrong rain chance > 100%%", zone_id, season);
            }

            if (wzc.data[season].snowChance > 100)
            {
                wzc.data[season].snowChance = 25;
                sLog->outError(LOG_FILTER_SQL, "Weather for zone %u season %u has wrong snow chance > 100%%", zone_id, season);
            }

            if (wzc.data[season].stormChance > 100)
            {
                wzc.data[season].stormChance = 25;
                sLog->outError(LOG_FILTER_SQL, "Weather for zone %u season %u has wrong storm chance > 100%%", zone_id, season);
            }
        }

        wzc.ScriptId = sObjectMgr->GetScriptId(fields[13].GetCString());

        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u weather definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void SendFineWeatherUpdateToPlayer(Player * p_Player)
{
    WorldPacket l_Data(SMSG_WEATHER, (4 + 4 + 4));
    l_Data << (uint32)WEATHER_STATE_FINE;
    l_Data << float(0.0f);
    l_Data.WriteBit(0);
    l_Data.FlushBits();

    p_Player->GetSession()->SendPacket(&l_Data);
}

void Update(uint32 diff)
{
    ///- Send an update signal to Weather objects
    WeatherMap::iterator itr, next;
    for (itr = m_weathers.begin(); itr != m_weathers.end(); itr = next)
    {
        next = itr;
        ++next;

        ///- and remove Weather objects for zones with no player
        // As interval > WorldTick
        if (!itr->second->Update(diff))
            m_weathers.erase(itr);
    }
}

} // namespace
