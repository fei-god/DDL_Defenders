#pragma once
#ifndef __MOOD_SYSTEM_H__
#define __MOOD_SYSTEM_H__

#include "cocos2d.h"

enum class MoodType
{
    Normal,      // 普通
    Focus,       // 专注
    Irritable,   // 烦躁
    Exhausted    // 疲惫
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

    // 获取当前情绪
    MoodType getCurrentMood() const;

    // 获取情绪对应的速度倍率
    float getSpeedMultiplier() const;

    // 获取情绪对应的受伤倍率
    float getDamageMultiplier() const;

    // 获取情绪对应的技能冷却倍率
    float getCooldownMultiplier() const;

    // 获取情绪名字，方便 UI 显示或调试
    std::string getMoodName() const;

    // 设置情绪持续时间
    void setMoodDuration(float duration);

    // 恢复普通状态
    void resetMood();

private:
    MoodType currentMood;     // 当前情绪
    float moodDuration;       // 当前情绪剩余时间
    bool hasDuration;         // 当前情绪是否有持续时间限制
};

#endif

