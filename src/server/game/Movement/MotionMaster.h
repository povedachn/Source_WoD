////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef TRINITY_MOTIONMASTER_H
#define TRINITY_MOTIONMASTER_H

#include "Common.h"
#include "SharedDefines.h"
#include "Object.h"

class MovementGenerator;
class Unit;
class PathGenerator;

// Creature Entry ID used for waypoints show, visible only for GMs
#define VISUAL_WAYPOINT 1

// values 0 ... MAX_DB_MOTION_TYPE-1 used in DB
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE      = 0,                              // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE    = 1,                              // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE  = 2,                              // WaypointMovementGenerator.h
    MAX_DB_MOTION_TYPE    = 3,                              // *** this and below motion types can't be set in DB.
    ANIMAL_RANDOM_MOTION_TYPE = MAX_DB_MOTION_TYPE,         // AnimalRandomMovementGenerator.h
    CONFUSED_MOTION_TYPE  = 4,                              // ConfusedMovementGenerator.h
    CHASE_MOTION_TYPE     = 5,                              // TargetedMovementGenerator.h
    HOME_MOTION_TYPE      = 6,                              // HomeMovementGenerator.h
    FLIGHT_MOTION_TYPE    = 7,                              // WaypointMovementGenerator.h
    POINT_MOTION_TYPE     = 8,                              // PointMovementGenerator.h
    FLEEING_MOTION_TYPE   = 9,                              // FleeingMovementGenerator.h
    DISTRACT_MOTION_TYPE  = 10,                             // IdleMovementGenerator.h
    ASSISTANCE_MOTION_TYPE= 11,                             // PointMovementGenerator.h (first part of flee for assistance)
    ASSISTANCE_DISTRACT_MOTION_TYPE = 12,                   // IdleMovementGenerator.h (second part of flee for assistance)
    TIMED_FLEEING_MOTION_TYPE = 13,                         // FleeingMovementGenerator.h (alt.second part of flee for assistance)
    FOLLOW_MOTION_TYPE    = 14,
    ROTATE_MOTION_TYPE    = 15,
    EFFECT_MOTION_TYPE    = 16,
    NULL_MOTION_TYPE      = 17
};

enum MovementSlot
{
    MOTION_SLOT_IDLE,
    MOTION_SLOT_ACTIVE,
    MOTION_SLOT_CONTROLLED,
    MAX_MOTION_SLOT
};

enum MMCleanFlag
{
    MMCF_NONE   = 0,
    MMCF_UPDATE = 1, // Clear or Expire called from update
    MMCF_RESET  = 2  // Flag if need top()->Reset()
};

enum RotateDirection
{
    ROTATE_DIRECTION_LEFT,
    ROTATE_DIRECTION_RIGHT
};

// assume it is 25 yard per 0.6 second
#define SPEED_CHARGE    42.0f

class MotionMaster //: private std::stack<MovementGenerator *>
{
    private:
        //typedef std::stack<MovementGenerator *> Impl;
        typedef MovementGenerator* _Ty;

        void pop()
        {
            if (empty())
                return;

            Impl[_top] = NULL;
            while (!empty() && !top())
                --_top;
        }
        void push(_Ty _Val) { ++_top; Impl[_top] = _Val; }

        bool needInitTop() const
        {
            if (empty())
                return false;
            return _needInit[_top];
        }
        void InitTop();
    public:

        explicit MotionMaster(Unit* unit) : _expList(NULL), _top(-1), _owner(unit), _cleanFlag(MMCF_NONE)
        {
            for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
            {
                Impl[i] = NULL;
                _needInit[i] = true;
            }
        }
        ~MotionMaster();

        void Initialize();
        void InitDefault();

        bool empty() const { return (_top < 0); }
        int size() const { return _top + 1; }
        _Ty top() const
        {
            ASSERT(!empty());
            return Impl[_top];
        }
        _Ty GetMotionSlot(int slot) const
        {
            ASSERT(slot >= 0);
            return Impl[slot];
        }

        void DirectDelete(_Ty curr);
        void DelayedDelete(_Ty curr);

        void UpdateMotion(uint32 diff);
        void Clear(bool reset = true)
        {
            if (_cleanFlag & MMCF_UPDATE)
            {
                if (reset)
                    _cleanFlag |= MMCF_RESET;
                else
                    _cleanFlag &= ~MMCF_RESET;
                DelayedClean();
            }
            else
                DirectClean(reset);
        }
        void MovementExpired(bool reset = true)
        {
            if (_cleanFlag & MMCF_UPDATE)
            {
                if (reset)
                    _cleanFlag |= MMCF_RESET;
                else
                    _cleanFlag &= ~MMCF_RESET;
                DelayedExpire();
            }
            else
                DirectExpire(reset);
        }

        void MoveIdle();
        void MoveTargetedHome();
        void MoveRandom(float spawndist = 0.0f);
        void MoveFollow(Unit* target, float dist, float angle, MovementSlot slot = MOTION_SLOT_ACTIVE);
        void MoveChase(Unit* target, float dist = 0.0f, float angle = 0.0f);
        void MoveConfused();
        void MoveFleeing(Unit* enemy, uint32 time = 0);
        void MovePoint(uint32 id, Position const& pos, bool generatePath = true)
        {
            MovePoint(id, pos.m_positionX, pos.m_positionY, pos.m_positionZ, generatePath);
        }
        void MovePoint(uint32 id, G3D::Vector3 const& pos, bool generatePath = true)
        {
            MovePoint(id, pos.x, pos.y, pos.z, generatePath);
        }
        void MovePoint(uint32 id, float x, float y, float z, bool generatePath = true);
        void MovePointWithRot(uint32 id, float x, float y, float z, float p_Orientation = -1000.0f, bool generatePath = true);

        // These two movement types should only be used with creatures having landing/takeoff animations
        void MoveLand(uint32 id, Position const& pos);
        void MoveTakeoff(uint32 id, Position const& pos);
        void MoveTakeoff(uint32 id, float p_X, float p_Y, float p_Z);

        void MoveCharge(float x, float y, float z, float speed = SPEED_CHARGE, uint32 id = EVENT_CHARGE, bool generatePath = false);
        void MoveCharge(PathGenerator const& path, float speed = SPEED_CHARGE);
        void MoveCharge(Position const* p_Pos, float p_Speed = SPEED_CHARGE, uint32 p_ID = EVENT_CHARGE, bool generatePath = false)
        {
            MoveCharge(p_Pos->m_positionX, p_Pos->m_positionY, p_Pos->m_positionZ, p_Speed, p_ID, generatePath);
        }

        void MoveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ);
        void MoveJumpTo(float angle, float speedXY, float speedZ);
        void MoveJump(float x, float y, float z, float speedXY, float speedZ, float o = 10.0f, uint32 id = 0);
        void MoveJump(Position const p_Pos, float p_SpeedXY, float p_SpeedZ, uint32 p_ID = 0);
        void MoveJump(uint32 p_LocEntry, float p_SpeedXY, float p_SpeedZ, uint32 p_ID = 0);

        void CustomJump(float x, float y, float z, float speedXY, float speedZ, uint32 id = 0);
        void CustomJump(Unit* p_Target, float speedXY, float speedZ, uint32 id = 0);
        void MoveCirclePath(float x, float y, float z, float radius, bool clockwise, uint8 stepCount);
        void MoveSmoothPath(uint32 pointId, G3D::Vector3 const* pathPoints, size_t pathSize, bool walk);
        void MoveSmoothFlyPath(uint32 p_PointID, G3D::Vector3 const* p_Path, size_t p_Size);
        void MoveSmoothFlyPath(uint32 p_PointID, Position const p_Position);
        void MoveFall(uint32 id = 0);

        void MoveSeekAssistance(float x, float y, float z);
        void MoveSeekAssistanceDistract(uint32 timer);
        void MoveTaxiFlight(uint32 path, uint32 pathnode);
        void MoveDistract(uint32 time);
        void MovePath(uint32 path_id, bool repeatable);
        void MoveRotate(uint32 time, RotateDirection direction);

        void MoveBackward(uint32 id, float x, float y, float z, float speed = 0.0f);

        MovementGeneratorType GetCurrentMovementGeneratorType() const;
        MovementGeneratorType GetMotionSlotType(int slot) const;

        void propagateSpeedChange();

        bool GetDestination(float &x, float &y, float &z);
    private:
        void Mutate(MovementGenerator *m, MovementSlot slot);                  // use Move* functions instead

        void DirectClean(bool reset);
        void DelayedClean();

        void DirectExpire(bool reset);
        void DelayedExpire();

        typedef std::vector<_Ty> ExpireList;
        ExpireList* _expList;
        _Ty Impl[MAX_MOTION_SLOT];
        int _top;
        Unit* _owner;
        bool _needInit[MAX_MOTION_SLOT];
        uint8 _cleanFlag;
};
#endif
