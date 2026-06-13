#pragma once
#ifndef __MOOD_SYSTEM_H__
#define __MOOD_SYSTEM_H__

#include "cocos2d.h"
#include <string>

// 情绪类型
// 这里在原来的基础上扩展，方便做接近 Brotato 的局内状态变化
enum class MoodType
{
    Normal,      // 普通：无明显加成或惩罚
    Focus,       // 专注：冷却更快，攻速更快，受伤略低
    Irritable,   // 烦躁：伤害更高，但受伤也更高
    Exhausted,   // 疲惫：速度下降，冷却变慢
    Excited,     // 兴奋：速度、攻速提高，但更脆
    Fear,        // 恐惧：速度提高，伤害下降
    Calm,        // 冷静：暴击、闪避略高
    Panic        // 慌乱：速度高，但冷却、受伤惩罚明显
};

class MoodSystem
{
public:
    MoodSystem();
    ~MoodSystem();

    // 初始化情绪系统
    void initMoodSystem();

    // 每帧更新情绪持续时间
    void updateMood(float dt);

    // 切换情绪
    void changeMood(MoodType mood);

    // 切换情绪，并设置持续时间
    void changeMood(MoodType mood, float duration);

    // 切换情绪，并设置持续时间和强度
    void changeMood(MoodType mood, float duration, float intensity);

    // 获取当前情绪
    MoodType getCurrentMood() const;

    // 获取上一个情绪
    MoodType getPreviousMood() const;

    // 判断当前是否为某种情绪
    bool isMood(MoodType mood) const;

    // 判断是否普通状态
    bool isNormal() const;

    // =========================
    // 情绪倍率
    // =========================

    // 获取情绪对应的速度倍率
    float getSpeedMultiplier() const;

    // 获取情绪对应的受伤倍率
    float getDamageMultiplier() const;

    // 获取情绪对应的技能冷却倍率
    float getCooldownMultiplier() const;

    // 输出伤害倍率
    float getOutgoingDamageMultiplier() const;

    // 攻速倍率
    float getAttackSpeedMultiplier() const;

    // 暴击率加成
    float getCritChanceBonus() const;

    // 闪避率加成
    float getDodgeChanceBonus() const;

    // 拾取范围倍率
    float getPickupRangeMultiplier() const;

    // 生命恢复倍率
    float getHpRegenMultiplier() const;

    // 材料获得倍率
    float getMaterialMultiplier() const;

    // =========================
    // 情绪显示
    // =========================

    // 获取情绪名字，方便 UI 显示或调试
    std::string getMoodName() const;

    // 获取中文情绪名字
    std::string getMoodChineseName() const;

    // 获取情绪描述
    std::string getMoodDescription() const;

    // 获取调试信息
    std::string getDebugInfo() const;

    // =========================
    // 持续时间
    // =========================

    // 设置情绪持续时间
    void setMoodDuration(float duration);

    // 获取剩余时间
    float getMoodDuration() const;

    // 获取初始持续时间
    float getMaxMoodDuration() const;

    // 获取持续时间百分比
    float getMoodDurationPercent() const;

    // 是否有持续时间限制
    bool hasMoodDurationLimit() const;

    // 恢复普通状态
    void resetMood();

    // =========================
    // 情绪强度
    // =========================

    // 情绪强度会影响倍率，默认 1.0
    // 例如 0.5 是弱情绪，1.5 是强情绪
    void setMoodIntensity(float intensity);
    float getMoodIntensity() const;

    // =========================
    // 情绪值系统
    // =========================

    // 情绪值可以被战斗事件改变，例如受伤、击杀、拾取、低血量
    void addMoodValue(float value);
    void reduceMoodValue(float value);
    void setMoodValue(float value);
    float getMoodValue() const;

    void setMaxMoodValue(float value);
    float getMaxMoodValue() const;

    float getMoodValuePercent() const;

    // 根据情绪值自动改变情绪
    // 例如情绪值高时进入 Focus 或 Excited，低时进入 Exhausted
    void updateMoodByValue();

    // =========================
    // 常用事件接口
    // =========================

    // 玩家受伤时调用
    void onPlayerDamaged();

    // 玩家击杀敌人时调用
    void onEnemyKilled();

    // 玩家拾取材料时调用
    void onMaterialPicked();

    // 玩家血量较低时调用
    void onLowHp();

    // 波次开始
    void onWaveStart();

    // 波次结束
    void onWaveEnd();

private:
    float clampFloat(float value, float minValue, float maxValue) const;
    float applyIntensity(float baseMultiplier) const;

private:
    MoodType currentMood;       // 当前情绪
    MoodType previousMood;      // 上一个情绪

    float moodDuration;         // 当前情绪剩余时间
    float maxMoodDuration;      // 当前情绪初始持续时间
    bool hasDuration;           // 当前情绪是否有持续时间限制

    float moodIntensity;        // 情绪强度，默认 1.0

    float moodValue;            // 情绪值
    float maxMoodValue;         // 最大情绪值
};

#endif
