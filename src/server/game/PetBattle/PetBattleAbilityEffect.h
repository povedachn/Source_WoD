////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __BATTLE_PET_ABILITY_EFFECT_HEADER__
#define __BATTLE_PET_ABILITY_EFFECT_HEADER__

#include "Common.h"

struct BattlePetAbilityEffectEntry;
class PetBattle;
class PetBattleAura;

enum BattlePetAbilityFlags
{
    BATTLEPET_ABILITY_FLAG_TRIGGER          = 1 << 0,
    BATTLEPET_ABILITY_FLAG_BUFF             = 1 << 1,
    BATTLEPET_ABILITY_TYPE_PASSIF           = 1 << 2,
    BATTLEPET_ABILITY_TYPE_SPECIAL_PASSIF   = 1 << 3,
    BATTLEPET_ABILITY_TYPE_DISPEL           = 1 << 4
};

enum PetBattleAbilityImplicitTarget
{
    PETBATTLE_TARGET_NONE,
    PETBATTLE_TARGET_CASTER,
    PETBATTLE_TARGET_TARGET,
    PETBATTLE_TARGET_CASTER_TEAM,
    PETBATTLE_TARGET_CASTER_TEAM_0,
    PETBATTLE_TARGET_CASTER_TEAM_1,
    PETBATTLE_TARGET_CASTER_TEAM_2,
    PETBATTLE_TARGET_TARGET_TEAM,
    PETBATTLE_TARGET_TARGET_TEAM_0,
    PETBATTLE_TARGET_TARGET_TEAM_1,
    PETBATTLE_TARGET_TARGET_TEAM_2,
    PETBATTLE_TARGET_ALL,
    PETBATTLE_TARGET_CASTER_TEAM_UNCONDITIONAL,
    PETBATTLE_TARGET_HEAD
};

struct PetBattleAbilityTurn
{
    uint32 ChainFailure;
};

struct PetBattleAbilityEffect
{
    // Constants
    BattlePetAbilityEffectEntry const* EffectInfo;
    PetBattleAbilityTurn* AbilityTurn;
    PetBattle* PetBattleInstance;
    uint32 AbilityID;
    uint32 Caster;
    std::vector<uint32> Targets;
    bool IsTriggered;

    // Temporary effect execution values
    uint32 Target;
    bool StopChain;
    uint32 Flags;
    bool ReportFailAsImmune;

    uint32 GetActiveOpponent();
    bool AddTarget(uint32 target);
    bool AddTarget(PetBattleAbilityImplicitTarget target);
    void SelectTargets();
    uint32 GetPetType();
    int32 CalculateDamage(int32 damage);
    int32 CalculateHeal(int32 heal);
    int32 CalculateHit(int32 accuracy);
    int32 GetHealth(uint32 target);
    int32 GetMaxHealth(uint32 target);
    int32 GetSpeed(uint32 target);
    bool SetHealth(uint32 target, int32 value);
    bool Damage(uint32 target, int32 damage, bool p_CantBeAvoidBlockedDodged = false);
    bool Heal(uint32 target, int32 heal);
    bool SetState(uint32 target, uint32 state, int32 value);
    int32 GetState(uint32 target, uint32 state);
    /// Modify pet battle state increment or decrement state by ModValue
    /// @p_Target   : Target Pet ID
    /// @p_StateID  : ID of the state to modify
    /// @p_ModValue : Value to add or sub
    /// @p_Apply    : Add or sub p_ModValue
    void ModState(uint32 p_Target, uint32 p_StateID, int32 p_ModValue, bool p_Apply = true);
    bool AddAura(uint32 target, int32 duration, int32 maxAllowed);
    PetBattleAura* GetAura(uint32 target, uint32 ability);
    void Trigger(uint32 target, uint32 ability);
    void Kill(uint32 target);

    bool Execute();

    bool HandleNull() { return false; }
    bool HandleDummy();

    bool HandleSetState();
    bool HandleDamage();
    bool HandleWitchingDamage();
    bool HandleStateDamage();
    bool HandleStopChainFailure();
    bool HandleHealPercentDealt();
    bool HandleHeal();
    bool HandleCatch();
    bool HandlePeriodicTrigger();
    bool HandleRampingDamage();
    bool HandleHealLastHitTaken();
    bool HandleRemoveAura();
    bool HandleModState();
    bool HandlePositiveAura();
    bool HandleHealState();
    bool HandleExtraAttackIfMoreFaster();
    bool HandleExtraHealIfLastHitKill();
    bool HandleSplittedDamage();
    bool HandleDamageNonLetal();
    bool HandleExtraAttackIfLessFaster();
    bool HandleHealOnSpecificWeather();
    bool HandleWeatherAura();
    bool HandleAuraCondAccuracyState();
    bool HandleCheckState();
    bool HandleCancellableAura();
    bool HandleWeatherDamage();
    bool HandleNegativeAura();
    bool HandlePeriodicPositiveTrigger();
    bool HandleControlAura();
    bool HandlePowerlessAura();
    bool HandleHealPercent();
    bool HandleHealPetType();
    bool HandleExtraAttack();
    bool HandleHealCasterPercentNotState();
    bool HandleDamagePercent();
    bool HandleDamageStateBonus();
    bool HandleDamageRuthless();
    bool HandleEqualizeLife();
    bool HandleDamageCasterPercent();
    bool HandleHealToggleAura();
    bool HandleInitialization();
    bool HandleResurect();
    bool HandleKill();
    bool HandleDamageToggleAura();
    bool HandleHealStateToggleAura();
    bool HandleResetState();
    bool HandleDamageHitState();
    bool HandleDamageAuraToggleAura();
    bool HandleHealthConsume();
    bool HandleSwap();
    bool HandleUpdatePriority();
    bool HandleSetHealthPercent();
    bool HandleLockActiveAbility();
    bool HandleAuraIfCasterFaster();
    bool HandleCleansing();
    bool HandleAuraState();
    bool HandleChangeCasterState();
    bool HandleSwapLow();
    bool HandleResetAuraDuration();
    bool HandleSwapHigh();
    bool HandleRedirectionAura();
    bool HandleCheckFailure();
    bool HandleTryRevive();
    bool HandleDamagePercentTaken();
    bool HandleDamageRange();
    bool HandleDamageWithBonus();
};

#endif
