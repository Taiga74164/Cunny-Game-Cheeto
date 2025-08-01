﻿#include "pch.h"
#include "PlayerStats.h"

#include <misc/cpp/imgui_stdlib.h>

namespace cheat::features
{
    PlayerStats::PlayerStats()
        : FeatureBase("Player Stats", "Modify your character's stats", FeatureSection::Player)
        , m_searchFilter("")
    {
        // Alternatively, you can find a better function to hook as long as it gets called every update
        HookManager::install(NewNormalAttackAction::Update(), hNewNormalAttackAction_Update);
    }

    void PlayerStats::draw()
    {
        ImGui::InputText("Search Stats", &m_searchFilter);
        ImGui::SameLine();

        if (ImGui::Button("Reset All"))
        {
            for (auto& pair : m_statValues)
            {
                pair.second = 0;
            }
        }

        if (ImGui::BeginChild("StatsList", ImVec2(0, 340), true))
        {
            for (const auto filteredStats = getFilteredStats(); auto stat : filteredStats)
            {
                const char* statName = magic_enum::enum_name(stat).data();

                if (!m_statValues.contains(stat))
                {
                    m_statValues[stat] = 0;
                }

                ImGui::InputInt(statName, &m_statValues[stat]);
            }
        }
        ImGui::EndChild();
    }

    std::vector<StatType_Enum> PlayerStats::getFilteredStats() const
    {
        std::vector<StatType_Enum> filtered;
        for (auto stat = StatType_Enum::MaxHP; stat < StatType_Enum::Max;
             stat = static_cast<StatType_Enum>(static_cast<int>(stat) + 1))
        {
            if (m_searchFilter.empty())
            {
                filtered.push_back(stat);
            }
            else
            {
                std::string statName = magic_enum::enum_name(stat).data();
                std::ranges::transform(statName, statName.begin(), tolower);
                std::string searchLower = m_searchFilter;
                std::ranges::transform(searchLower, searchLower.begin(), tolower);

                if (statName.find(searchLower) != std::string::npos)
                {
                    filtered.push_back(stat);
                }
            }
        }
        return filtered;
    }

    void PlayerStats::applyStats(BattleEntityStat* entityStat, BattleEntity* battleEntity) const
    {
        for (const auto& [fst, snd] : m_statValues)
        {
            if (snd != 0)
            {
                BattleEntityStatProcessor::Apply()(battleEntity->statProcessor(), fst);

                BattleEntityStat::SetValue()(entityStat, fst, static_cast<int64_t>(snd));
                auto currentStat = battleEntity->statProcessor()->CurrentStat();
                battleEntity->statProcessor()->InitialStat(currentStat);
                battleEntity->statProcessor()->DefaultStat(currentStat);
                if (fst == StatType_Enum::MaxHP)
                {
                    battleEntity->MaxHPCapGauge(snd);
                    battleEntity->statProcessor()->lastMaxHP(snd);
                }
            }
        }
    }

    void PlayerStats::hNewNormalAttackAction_Update(NewNormalAttackAction* _this, Battle* battle)
    {
        if (s_instance->m_enabled)
        {
            if (_this->Executer()->TacticEntityType() == TacticEntityType_Enum::Student)
            {
                // LOG_DEBUG("Executer: %s", _this->Executer()->Name()->ToString().c_str());
                if (const auto entityStat = _this->Executer()->get_CurrentStat()(_this->Executer());
                    entityStat != nullptr)
                {
                    s_instance->applyStats(entityStat, _this->Executer());
                }
            }
        }

        CALL_ORIGINAL(hNewNormalAttackAction_Update, _this, battle);
    }
}
