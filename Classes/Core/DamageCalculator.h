#pragma once

#include <algorithm>

// 伤害计算单独拆出来，方便以后加暴击、护甲、状态加成。
class DamageCalculator
{
public:
    // rawDamage：武器或子弹的原始伤害
    // defense：敌人防御力
    // damageRate：伤害倍率，比如 1.2 表示增加 20%
    static int calculateFinalDamage(int rawDamage, int defense = 0, float damageRate = 1.0f)
    {
        // 先算倍率，再减防御。
        int damage = static_cast<int>(rawDamage * damageRate) - defense;

        // 至少造成 1 点伤害，避免防御太高导致永远打不动。
        damage = std::max(1, damage);

        return damage;
    }
};