////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"

#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"

inline bool isStatic(MovementGenerator *mv)
{
    return (mv == &si_idleMovement);
}

void MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if (curr)
            DirectDelete(curr);
    }

    InitDefault();
}

// set new default movement generator
void MotionMaster::InitDefault()
{
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator(_owner->ToCreature());
        Mutate(movement == NULL ? &si_idleMovement : movement, MOTION_SLOT_IDLE);
    }
    else
    {
        Mutate(&si_idleMovement, MOTION_SLOT_IDLE);
    }
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if (curr && !isStatic(curr))
            delete curr;    // Skip finalizing on delete, it might launch new movement
    }
}

void MotionMaster::UpdateMotion(uint32 diff)
{
    if (!_owner)
        return;

    if (_owner->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED)) // what about UNIT_STATE_DISTRACTED? Why is this not included?
        return;

    ASSERT(!empty());

    _cleanFlag |= MMCF_UPDATE;
    if (!top()->Update(_owner, diff))
    {
        _cleanFlag &= ~MMCF_UPDATE;
        MovementExpired();
    }
    else
        _cleanFlag &= ~MMCF_UPDATE;

    if (_expList)
    {
        for (size_t i = 0; i < _expList->size(); ++i)
        {
            MovementGenerator* mg = (*_expList)[i];
            DirectDelete(mg);
        }

        delete _expList;
        _expList = NULL;

        if (empty())
            Initialize();
        else if (needInitTop())
            InitTop();
        else if (_cleanFlag & MMCF_RESET)
            top()->Reset(_owner);

        _cleanFlag &= ~MMCF_RESET;
    }

    // probably not the best place to pu this but im not really sure where else to put it.
    _owner->UpdateUnderwaterState(_owner->GetMap(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
}

void MotionMaster::DirectClean(bool reset)
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr) DirectDelete(curr);
    }

    if (empty())
        return;

    if (needInitTop())
        InitTop();
    else if (reset)
        top()->Reset(_owner);
}

void MotionMaster::DelayedClean()
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr)
            DelayedDelete(curr);
    }
}

void MotionMaster::DirectExpire(bool reset)
{
    if (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        DirectDelete(curr);
    }

    while (!empty() && !top())
        --_top;

    if (empty())
        Initialize();
    else if (needInitTop())
        InitTop();
    else if (reset)
        top()->Reset(_owner);
}

void MotionMaster::DelayedExpire()
{
    if (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        DelayedDelete(curr);
    }

    while (!empty() && !top())
        --_top;
}

void MotionMaster::MoveIdle()
{
    //! Should be preceded by MovementExpired or Clear if there's an overlying movementgenerator active
    if (empty() || !isStatic(top()))
        Mutate(&si_idleMovement, MOTION_SLOT_IDLE);
}

void MotionMaster::MoveRandom(float spawndist)
{
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        //sLog->outDebug(LOG_FILTER_GENERAL, "Creature (%llu) start moving random", _owner->GetGUID());
        Mutate(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_IDLE);
    }
}

void MotionMaster::MoveTargetedHome()
{
    Clear(false);

    if (_owner->GetTypeId() == TYPEID_UNIT && !_owner->ToCreature()->GetCharmerOrOwnerGUID())
    {
        //sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) targeted home", _owner->GetEntry(), _owner->GetGUID());
        Mutate(new HomeMovementGenerator<Creature>(), MOTION_SLOT_ACTIVE);
    }
    else if (_owner->GetTypeId() == TYPEID_UNIT && _owner->ToCreature()->GetCharmerOrOwnerGUID() != 0)
    {
        //sLog->outDebug(LOG_FILTER_GENERAL, "Pet or controlled creature (Entry: %u %llu) targeting home", _owner->GetEntry(), _owner->GetGUID());
        Unit* target = _owner->ToCreature()->GetCharmerOrOwner();
        if (target)
        {
            ///sLog->outDebug(LOG_FILTER_GENERAL, "Following %llu", target->GetGUID());
            Mutate(new FollowMovementGenerator<Creature>(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE), MOTION_SLOT_ACTIVE);
        }
    }
    else
    {
        sLog->outError(LOG_FILTER_GENERAL, "Player (%lu) attempt targeted home", _owner->GetGUID());
    }
}

void MotionMaster::MoveConfused()
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) move confused", _owner->GetGUID());
        Mutate(new ConfusedMovementGenerator<Player>(), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        //sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) move confused",
        //    _owner->GetEntry(), _owner->GetGUID());
        Mutate(new ConfusedMovementGenerator<Creature>(), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveChase(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    //_owner->ClearUnitState(UNIT_STATE_FOLLOW);
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%s) chase (%s)",
        ///    _owner->GetGUID(),
        ///    target->GetGUID());
        Mutate(new ChaseMovementGenerator<Player>(target, dist, angle), MOTION_SLOT_ACTIVE);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) chase %llu",
        ///    _owner->GetEntry(),
        ///    _owner->GetGUID(),
        ///    target->GetGUID());
        Mutate(new ChaseMovementGenerator<Creature>(target, dist, angle), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveFollow(Unit* target, float dist, float angle, MovementSlot slot)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    //_owner->AddUnitState(UNIT_STATE_FOLLOW);
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) follow (%llu)", _owner->GetGUID(), target->GetGUID());
        Mutate(new FollowMovementGenerator<Player>(target, dist, angle), slot);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) follow (%llu)", _owner->GetEntry(), _owner->GetGUID(), target->GetGUID());
        Mutate(new FollowMovementGenerator<Creature>(target, dist, angle), slot);
    }
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, bool generatePath)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) targeted point (Id: %u X: %f Y: %f Z: %f)", _owner->GetGUID(), id, x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath), MOTION_SLOT_ACTIVE);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) targeted point (ID: %u X: %f Y: %f Z: %f)",
        ///    _owner->GetEntry(), _owner->GetGUID(), id, x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MovePointWithRot(uint32 id, float x, float y, float z, float p_Orientation, bool generatePath)
{
    if (_owner->IsPlayer())
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (GUID: %u) targeted point (Id: %u X: %f Y: %f Z: %f)", _owner->GetGUIDLow(), id, x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, 0.0f, p_Orientation), MOTION_SLOT_ACTIVE);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u GUID: %u) targeted point (ID: %u X: %f Y: %f Z: %f)",
        ///    _owner->GetEntry(), _owner->GetGUIDLow(), id, x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, 0.0f, p_Orientation), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveLand(uint32 id, Position const& pos)
{
    float x, y, z;
    pos.GetPosition(x, y, z);

    ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u) landing point (ID: %u X: %f Y: %f Z: %f)", _owner->GetEntry(), id, x, y, z);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
    init.SetAnimation(Movement::ToGround);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveTakeoff(uint32 id, Position const& pos)
{
    float x, y, z;
    pos.GetPosition(x, y, z);

    ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u) landing point (ID: %u X: %f Y: %f Z: %f)", _owner->GetEntry(), id, x, y, z);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    init.SetAnimation(Movement::ToFly);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
}
void MotionMaster::MoveTakeoff(uint32 id, float p_X, float p_Y, float p_Z)
{
    Movement::MoveSplineInit init(_owner);
    init.MoveTo(p_X, p_Y, p_Z);
    init.SetAnimation(Movement::ToFly);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ)
{
    //this function may make players fall below map
    if (_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    if (speedXY <= 0.1f)
        return;

    float x, y, z;
    float moveTimeHalf = speedZ / Movement::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);

    _owner->GetNearPoint(_owner, x, y, z, _owner->GetObjectSize(), dist, _owner->GetAngle(srcX, srcY) + float(M_PI));

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    init.SetParabolic(max_height, 0);
    init.SetOrientationFixed(true);
    init.SetVelocity(speedXY);
    init.Launch();
    Mutate(new EffectMovementGenerator(0), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveJumpTo(float angle, float speedXY, float speedZ)
{
    //this function may make players fall below map
    if (_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    float x, y, z;

    float moveTimeHalf = speedZ / Movement::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    _owner->GetClosePoint(x, y, z, _owner->GetObjectSize(), dist, angle);
    MoveJump(x, y, z, speedXY, speedZ);
}

void MotionMaster::MoveJump(float x, float y, float z, float speedXY, float speedZ, float o, uint32 id)
{
    sLog->outDebug(LOG_FILTER_GENERAL, "Unit (GUID: %u) jump to point (X: %f Y: %f Z: %f)", _owner->GetGUIDLow(), x, y, z);

    if (speedXY <= 0.1f)
        return;

    float moveTimeHalf = speedZ / Movement::gravity;
    float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
    init.SetParabolic(max_height, 0);
    init.SetVelocity(speedXY);
    if (o != 10.0f)
        init.SetFacing(o);
    init.Launch();

    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveJump(Position const p_Pos, float p_SpeedXY, float p_SpeedZ, uint32 p_ID)
{
    MoveJump(p_Pos.m_positionX, p_Pos.m_positionY, p_Pos.m_positionZ, p_SpeedXY, p_SpeedZ, p_Pos.m_orientation, p_ID);
}

void MotionMaster::MoveJump(uint32 p_LocEntry, float p_SpeedXY, float p_SpeedZ, uint32 p_ID)
{
    WorldSafeLocsEntry const* l_Loc = sWorldSafeLocsStore.LookupEntry(p_LocEntry);
    if (l_Loc == nullptr)
        return;

    MoveJump(l_Loc->x, l_Loc->y, l_Loc->z, p_SpeedXY, p_SpeedZ, l_Loc->o, p_ID);
}

void MotionMaster::CustomJump(float x, float y, float z, float speedXY, float speedZ, uint32 id)
{
    speedZ *= 2.3f;
    speedXY *= 2.3f;

    if (speedXY <= 0.1f)
        return;

    float moveTimeHalf = speedZ / Movement::gravity;
    float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);
    max_height /= 15.0f;

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
    init.SetParabolic(max_height, 0);
    init.SetVelocity(speedXY);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::CustomJump(Unit* p_Target, float speedXY, float speedZ, uint32 id)
{
    if (p_Target == nullptr)
        return;

    float x, y, z;
    p_Target->GetPosition(x, y, z);

    speedZ *= 2.3f;
    speedXY *= 2.3f;

    if (speedXY <= 0.1f)
        return;

    float moveTimeHalf = speedZ / Movement::gravity;
    float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);
    max_height /= 15.0f;

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
    init.SetParabolic(max_height, 0);
    init.SetVelocity(speedXY);
    init.Launch();
    Mutate(new EffectMovementGenerator(id, 0, p_Target->GetGUID()), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveCirclePath(float x, float y, float z, float radius, bool clockwise, uint8 stepCount)
{
    float step = 2 * float(M_PI) / stepCount * (clockwise ? -1.0f : 1.0f);
    Position const& pos = { x, y, z, 0.0f };
    float angle = pos.GetAngle(_owner->GetPositionX(), _owner->GetPositionY());

    Movement::MoveSplineInit init(_owner);

    for (uint8 i = 0; i < stepCount; angle += step, ++i)
    {
        G3D::Vector3 point;
        point.x = x + radius * cosf(angle);
        point.y = y + radius * sinf(angle);

        if (_owner->IsFlying())
            point.z = z;
        else
            point.z = _owner->GetMap()->GetHeight(_owner->GetPhaseMask(), point.x, point.y, z);

        init.Path().push_back(point);
    }

    if (_owner->IsFlying())
    {
        init.SetFly();
        init.SetCyclic();
        init.SetAnimation(Movement::ToFly);
    }
    else
    {
        init.SetWalk(true);
        init.SetCyclic();
    }

    init.Launch();
}

void MotionMaster::MoveSmoothPath(uint32 pointId, G3D::Vector3 const* pathPoints, size_t pathSize, bool walk)
{
    Movement::PointsArray path(pathPoints, pathPoints + pathSize);

    Movement::MoveSplineInit init(_owner);
    init.MovebyPath(path);
    init.SetSmooth();
    init.SetWalk(walk);
    init.Launch();

    // This code is not correct
    // EffectMovementGenerator does not affect UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE
    // need to call PointMovementGenerator with various pointIds
    Mutate(new EffectMovementGenerator(pointId), MOTION_SLOT_ACTIVE);
    //Position pos(pathPoints[pathSize - 1].x, pathPoints[pathSize - 1].y, pathPoints[pathSize - 1].z);
    //MovePoint(EVENT_CHARGE_PREPATH, pos, false);
}

void MotionMaster::MoveSmoothFlyPath(uint32 p_PointID, G3D::Vector3 const* p_Path, size_t p_Size)
{
    Movement::MoveSplineInit l_Init(_owner);

    l_Init.SetSmooth();
    l_Init.SetFly();
    l_Init.SetUncompressed();

    for (uint32 l_Count = 0; l_Count < uint32(p_Size); ++l_Count)
        l_Init.Path().push_back(*p_Path++);

    l_Init.Launch();

    Mutate(new EffectMovementGenerator(p_PointID), MovementSlot::MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveSmoothFlyPath(uint32 p_PointID, Position const p_Position)
{
    Movement::MoveSplineInit l_Init(_owner);

    l_Init.SetSmooth();
    l_Init.SetFly();
    l_Init.SetUncompressed();
    l_Init.MoveTo(p_Position.m_positionX, p_Position.m_positionY, p_Position.m_positionZ, false, false);
    l_Init.Launch();

    Mutate(new EffectMovementGenerator(p_PointID), MovementSlot::MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveFall(uint32 id /*=0*/)
{
    // use larger distance for vmap height search than in most other cases
    float tz = _owner->GetMap()->GetHeight(_owner->GetPhaseMask(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (tz <= INVALID_HEIGHT)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f).",
        ///    _owner->GetMap()->GetId(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
        return;
    }

    // Abort too if the ground is very near
    if (std::fabs(_owner->GetPositionZ() - tz) < 0.1f)
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
        _owner->SetFall(true);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), tz, false);
    init.SetFall();
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed /*= SPEED_CHARGE*/, uint32 id /*= EVENT_CHARGE*/, bool generatePath /*= false*/)
{
    if (Impl[MOTION_SLOT_CONTROLLED] && Impl[MOTION_SLOT_CONTROLLED]->GetMovementGeneratorType() != DISTRACT_MOTION_TYPE)
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) charge point (X: %f Y: %f Z: %f)", _owner->GetGUID(), x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu charge point (X: %f Y: %f Z: %f)",
        ///    _owner->GetEntry(), _owner->GetGUID(), x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveCharge(PathGenerator const& path, float speed /*= SPEED_CHARGE*/)
{
    G3D::Vector3 dest = path.GetActualEndPosition();

    MoveCharge(dest.x, dest.y, dest.z, speed, EVENT_CHARGE_PREPATH);

    // Charge movement is not started when using EVENT_CHARGE_PREPATH
    Movement::MoveSplineInit init(_owner);
    init.MovebyPath(path.GetPath());
    init.SetVelocity(speed);
    init.Launch();
}

void MotionMaster::MoveSeekAssistance(float x, float y, float z)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog->outError(LOG_FILTER_GENERAL, "Player (GUID: %lu) attempt to seek assistance", _owner->GetGUID());
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) seek assistance (X: %f Y: %f Z: %f)",
        ///    _owner->GetEntry(), _owner->GetGUID(), x, y, z);
        _owner->AttackStop();
        _owner->ToCreature()->SetReactState(REACT_PASSIVE);
        Mutate(new AssistanceMovementGenerator(x, y, z), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog->outError(LOG_FILTER_GENERAL, "Player (%lu) attempt to call distract after assistance", _owner->GetGUID());
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) is distracted after assistance call (Time: %u)",
        ///    _owner->GetEntry(), _owner->GetGUID(), time);
        Mutate(new AssistanceDistractMovementGenerator(time), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) flee from (%llu)", _owner->GetGUID(),
        ///    enemy->GetGUID());
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) flee from (%s)%llu",
        ///    _owner->GetEntry(), _owner->GetGUID(),
        ///    enemy->GetGUID(),
        ///   time ? " for a limited time" : "");
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetGUID(), time), MOTION_SLOT_CONTROLLED);
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            ///sLog->outDebug(LOG_FILTER_GENERAL, "%s taxi to (Path %u node %u)", _owner->GetName(), path, pathnode);
            FlightPathMovementGenerator* mgen = new FlightPathMovementGenerator();
            mgen->LoadPath(_owner->ToPlayer());
            Mutate(mgen, MOTION_SLOT_CONTROLLED);
        }
        else
        {
            sLog->outError(LOG_FILTER_GENERAL, "%s attempt taxi to (not existed Path %u node %u)",
            _owner->GetName(), path, pathnode);
        }
    }
    else
    {
        sLog->outError(LOG_FILTER_GENERAL, "Creature (Entry: %u %lu attempt taxi to (Path %u node %u)",
            _owner->GetEntry(), _owner->GetGUID(), path, pathnode);
    }
}

void MotionMaster::MoveDistract(uint32 timer)
{
    if (Impl[MOTION_SLOT_CONTROLLED])
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Player (%llu) distracted (timer: %u)", _owner->GetGUID(), timer);
    }
    else
    {
        ///sLog->outDebug(LOG_FILTER_GENERAL, "Creature (Entry: %u %llu) (timer: %u)",
        ///    _owner->GetEntry(), _owner->GetGUID(), timer);
    }

    DistractMovementGenerator* mgen = new DistractMovementGenerator(timer);
    Mutate(mgen, MOTION_SLOT_CONTROLLED);
}

void MotionMaster::Mutate(MovementGenerator *m, MovementSlot slot)
{
    if (MovementGenerator *curr = Impl[slot])
    {
        Impl[slot] = NULL; // in case a new one is generated in this slot during directdelete
        if (_top == slot && (_cleanFlag & MMCF_UPDATE))
            DelayedDelete(curr);
        else
            DirectDelete(curr);
    }
    else if (_top < slot)
    {
        _top = slot;
    }

    Impl[slot] = m;
    if (_top > slot)
        _needInit[slot] = true;
    else
    {
        _needInit[slot] = false;
        m->Initialize(_owner);
    }
}

void MotionMaster::MovePath(uint32 path_id, bool repeatable)
{
    if (!path_id)
        return;
    //We set waypoint movement as new default movement generator
    // clear ALL movement generators (including default)
    /*while (!empty())
    {
        MovementGenerator *curr = top();
        curr->Finalize(*_owner);
        pop();
        if (!isStatic(curr))
            delete curr;
    }*/

    //_owner->GetTypeId() == TYPEID_PLAYER ?
        //Mutate(new WaypointMovementGenerator<Player>(path_id, repeatable)):
    Mutate(new WaypointMovementGenerator<Creature>(path_id, repeatable), MOTION_SLOT_IDLE);

    ///sLog->outDebug(LOG_FILTER_GENERAL, "%llu start moving over path (Id:%u, repeatable: %s)",
    ///    _owner->GetGUID(), path_id, repeatable ? "YES" : "NO");
}

void MotionMaster::MoveBackward(uint32 id, float x, float y, float z, float speed)
{
    if (_owner->IsPlayer())
        _owner->AddUnitMovementFlag(MOVEMENTFLAG_BACKWARD);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    init.SetOrientationInversed();
    init.Launch();
    if (speed > 0.0f)
        init.SetVelocity(speed);
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}


void MotionMaster::MoveRotate(uint32 time, RotateDirection direction)
{
    if (!time)
        return;

    Mutate(new RotateMovementGenerator(time, direction), MOTION_SLOT_ACTIVE);
}

void MotionMaster::propagateSpeedChange()
{
    /*Impl::container_type::iterator it = Impl::c.begin();
    for (; it != end(); ++it)
    {
        (*it)->unitSpeedChanged();
    }*/
    for (int i = 0; i <= _top; ++i)
    {
        if (Impl[i])
            Impl[i]->unitSpeedChanged();
    }
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
   if (empty())
       return IDLE_MOTION_TYPE;

   return top()->GetMovementGeneratorType();
}

MovementGeneratorType MotionMaster::GetMotionSlotType(int slot) const
{
    if (!Impl[slot])
        return NULL_MOTION_TYPE;
    else
        return Impl[slot]->GetMovementGeneratorType();
}

void MotionMaster::InitTop()
{
    top()->Initialize(_owner);
    _needInit[_top] = false;
}

void MotionMaster::DirectDelete(_Ty curr)
{
    if (isStatic(curr))
        return;
    curr->Finalize(_owner);
    delete curr;
}

void MotionMaster::DelayedDelete(_Ty curr)
{
    sLog->outFatal(LOG_FILTER_GENERAL, "Unit (Entry %u) is trying to delete its updating Movement Generator (Type %u)!", _owner->GetEntry(), curr->GetMovementGeneratorType());
    if (isStatic(curr))
        return;
    if (!_expList)
        _expList = new ExpireList();
    _expList->push_back(curr);
}

bool MotionMaster::GetDestination(float &x, float &y, float &z)
{
    if (_owner->movespline->Finalized())
        return false;

    G3D::Vector3 const& dest = _owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}
