#
#  Inna Hoover Brown-STUDIO
#  Copyright 2023 Inna Hoover Brown-studio SARL
#  All Rights Reserved.
#

file(GLOB_RECURSE sources_Ashran OutdoorPvP/Ashran/*.hpp OutdoorPvP/Ashran/*.cpp OutdoorPvP/Ashran/*.hxx)

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${sources_Ashran}
  OutdoorPvP/OutdoorPvPTF.cpp
  OutdoorPvP/OutdoorPvPSI.cpp
  OutdoorPvP/OutdoorPvPSI.h
  OutdoorPvP/OutdoorPvPZM.cpp
  OutdoorPvP/OutdoorPvPNA.cpp
  OutdoorPvP/OutdoorPvPHP.cpp
  OutdoorPvP/OutdoorPvPTF.h
  OutdoorPvP/OutdoorPvPHP.h
  OutdoorPvP/OutdoorPvPZM.h
  OutdoorPvP/OutdoorPvPNA.h
)

message("  -> Prepared: Outdoor PVP Zones")
