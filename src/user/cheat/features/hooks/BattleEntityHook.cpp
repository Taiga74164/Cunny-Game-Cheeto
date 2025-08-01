﻿#include "pch.h"
#include "BattleEntityHook.h"

#include "user/cheat/features/combat/OneHitKill.h"
#include "user/cheat/features/player/GodMode.h"

namespace cheat::features
{
    BattleEntityHook::BattleEntityHook()
        : FeatureBase("Battle Entity", "Hooks for BattleEntity features",
                      FeatureSection::Hooks)
    {
        HookManager::install(BattleEntity::AddHitPoint(), hBattleEntity_AddHitPoint);
    }

    int64_t BattleEntityHook::hBattleEntity_AddHitPoint(BattleEntity* _this, int64_t value)
    {
        auto ohk = OneHitKill::getInstance();
        auto gm = GodMode::getInstance();

        if (ohk->isEnabled())
        {
            if (_this->TacticEntityType() != TacticEntityType_Enum::Student)
            {
                _this->HitPoint(1);
                _this->HitPointBefore(1);
            }
        }

        if (gm->isEnabled())
        {
            if (_this->TacticEntityType() == TacticEntityType_Enum::Student)
            {
                _this->Damaged(nullptr); // So OnDamaged event won't be invoked
                _this->HitPoint(0);
                _this->HitPointBefore(0);
                return 0;
            }
        }

        return CALL_ORIGINAL(hBattleEntity_AddHitPoint, _this, value);
    }
}
