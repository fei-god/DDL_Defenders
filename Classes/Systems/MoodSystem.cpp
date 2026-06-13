#include "MoodSystem.h"

#include <sstream>
#include <algorithm>

MoodSystem::MoodSystem()
    : currentMood(MoodType::Normal)
    , previousMood(MoodType::Normal)
    , moodDuration(0.0f)
    , maxMoodDuration(0.0f)
    , hasDuration(false)
    , moodIntensity(1.0f)
    , moodValue(50.0f)
    , maxMoodValue(100.0f)
{
}

MoodSystem::~MoodSystem()
{
}

void MoodSystem::initMoodSystem()
{
    currentMood = MoodType::Normal;
    previousMood = MoodType::Normal;

    moodDuration = 0.0f;
    maxMoodDuration = 0.0f;
    hasDuration = false;

    moodIntensity = 1.0f;

    moodValue = 50.0f;
    maxMoodValue = 100.0f;
}

void MoodSystem::updateMood(float dt)
{
    if (hasDuration)
    {
        moodDuration -= dt;

        if (moodDuration <= 0.0f)
        {
            resetMood();
        }
    }

    // 情绪值缓慢回到中间值，避免一直处于极端状态
    float middleValue = maxMoodValue * 0.5f;

    if (moodValue > middleValue)
    {
        moodValue -= dt * 1.0f;

        if (moodValue < middleValue)
        {
            moodValue = middleValue;
        }
    }
    else if (moodValue < middleValue)
    {
        moodValue += dt * 1.0f;

        if (moodValue > middleValue)
        {
            moodValue = middleValue;
        }
    }
}

void MoodSystem::changeMood(MoodType mood)
{
    previousMood = currentMood;
    currentMood = mood;
}

void MoodSystem::changeMood(MoodType mood, float duration)
{
    changeMood(mood);
    setMoodDuration(duration);
}

void MoodSystem::changeMood(MoodType mood, float duration, float intensity)
{
    changeMood(mood);
    setMoodDuration(duration);
    setMoodIntensity(intensity);
}

MoodType MoodSystem::getCurrentMood() const
{
    return currentMood;
}

MoodType MoodSystem::getPreviousMood() const
{
    return previousMood;
}

bool MoodSystem::isMood(MoodType mood) const
{
    return currentMood == mood;
}

bool MoodSystem::isNormal() const
{
    return currentMood == MoodType::Normal;
}

// =========================
// 情绪倍率
// =========================

float MoodSystem::getSpeedMultiplier() const
{
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Normal:
        multiplier = 1.0f;
        break;

    case MoodType::Focus:
        multiplier = 1.15f;
        break;

    case MoodType::Irritable:
        multiplier = 1.10f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.70f;
        break;

    case MoodType::Excited:
        multiplier = 1.25f;
        break;

    case MoodType::Fear:
        multiplier = 1.30f;
        break;

    case MoodType::Calm:
        multiplier = 1.05f;
        break;

    case MoodType::Panic:
        multiplier = 1.35f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getDamageMultiplier() const
{
    // 这里表示“受到伤害倍率”
    // 小于 1 表示更抗打，大于 1 表示更脆
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Normal:
        multiplier = 1.0f;
        break;

    case MoodType::Focus:
        multiplier = 0.90f;
        break;

    case MoodType::Irritable:
        multiplier = 1.20f;
        break;

    case MoodType::Exhausted:
        multiplier = 1.15f;
        break;

    case MoodType::Excited:
        multiplier = 1.15f;
        break;

    case MoodType::Fear:
        multiplier = 1.10f;
        break;

    case MoodType::Calm:
        multiplier = 0.95f;
        break;

    case MoodType::Panic:
        multiplier = 1.30f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getCooldownMultiplier() const
{
    // 冷却倍率：越小越快
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Normal:
        multiplier = 1.0f;
        break;

    case MoodType::Focus:
        multiplier = 0.80f;
        break;

    case MoodType::Irritable:
        multiplier = 0.95f;
        break;

    case MoodType::Exhausted:
        multiplier = 1.25f;
        break;

    case MoodType::Excited:
        multiplier = 0.85f;
        break;

    case MoodType::Fear:
        multiplier = 1.10f;
        break;

    case MoodType::Calm:
        multiplier = 0.90f;
        break;

    case MoodType::Panic:
        multiplier = 1.20f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getOutgoingDamageMultiplier() const
{
    // 这里表示“玩家造成伤害倍率”
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Normal:
        multiplier = 1.0f;
        break;

    case MoodType::Focus:
        multiplier = 1.10f;
        break;

    case MoodType::Irritable:
        multiplier = 1.25f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.80f;
        break;

    case MoodType::Excited:
        multiplier = 1.15f;
        break;

    case MoodType::Fear:
        multiplier = 0.85f;
        break;

    case MoodType::Calm:
        multiplier = 1.05f;
        break;

    case MoodType::Panic:
        multiplier = 0.90f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getAttackSpeedMultiplier() const
{
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Normal:
        multiplier = 1.0f;
        break;

    case MoodType::Focus:
        multiplier = 1.25f;
        break;

    case MoodType::Irritable:
        multiplier = 1.10f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.75f;
        break;

    case MoodType::Excited:
        multiplier = 1.30f;
        break;

    case MoodType::Fear:
        multiplier = 0.90f;
        break;

    case MoodType::Calm:
        multiplier = 1.05f;
        break;

    case MoodType::Panic:
        multiplier = 1.15f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getCritChanceBonus() const
{
    switch (currentMood)
    {
    case MoodType::Focus:
        return 0.05f * moodIntensity;

    case MoodType::Calm:
        return 0.08f * moodIntensity;

    case MoodType::Irritable:
        return 0.03f * moodIntensity;

    case MoodType::Exhausted:
        return -0.05f * moodIntensity;

    case MoodType::Panic:
        return -0.03f * moodIntensity;

    default:
        return 0.0f;
    }
}

float MoodSystem::getDodgeChanceBonus() const
{
    switch (currentMood)
    {
    case MoodType::Focus:
        return 0.03f * moodIntensity;

    case MoodType::Fear:
        return 0.06f * moodIntensity;

    case MoodType::Calm:
        return 0.04f * moodIntensity;

    case MoodType::Exhausted:
        return -0.05f * moodIntensity;

    case MoodType::Panic:
        return -0.08f * moodIntensity;

    default:
        return 0.0f;
    }
}

float MoodSystem::getPickupRangeMultiplier() const
{
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Focus:
        multiplier = 1.10f;
        break;

    case MoodType::Calm:
        multiplier = 1.15f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.80f;
        break;

    case MoodType::Panic:
        multiplier = 0.90f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getHpRegenMultiplier() const
{
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Calm:
        multiplier = 1.25f;
        break;

    case MoodType::Focus:
        multiplier = 1.10f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.70f;
        break;

    case MoodType::Irritable:
        multiplier = 0.85f;
        break;

    case MoodType::Panic:
        multiplier = 0.70f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

float MoodSystem::getMaterialMultiplier() const
{
    float multiplier = 1.0f;

    switch (currentMood)
    {
    case MoodType::Focus:
        multiplier = 1.10f;
        break;

    case MoodType::Calm:
        multiplier = 1.10f;
        break;

    case MoodType::Excited:
        multiplier = 1.05f;
        break;

    case MoodType::Exhausted:
        multiplier = 0.90f;
        break;

    default:
        multiplier = 1.0f;
        break;
    }

    return applyIntensity(multiplier);
}

// =========================
// 情绪显示
// =========================

std::string MoodSystem::getMoodName() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return "Normal";

    case MoodType::Focus:
        return "Focus";

    case MoodType::Irritable:
        return "Irritable";

    case MoodType::Exhausted:
        return "Exhausted";

    case MoodType::Excited:
        return "Excited";

    case MoodType::Fear:
        return "Fear";

    case MoodType::Calm:
        return "Calm";

    case MoodType::Panic:
        return "Panic";

    default:
        return "Unknown";
    }
}

std::string MoodSystem::getMoodChineseName() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return "普通";

    case MoodType::Focus:
        return "专注";

    case MoodType::Irritable:
        return "烦躁";

    case MoodType::Exhausted:
        return "疲惫";

    case MoodType::Excited:
        return "兴奋";

    case MoodType::Fear:
        return "恐惧";

    case MoodType::Calm:
        return "冷静";

    case MoodType::Panic:
        return "慌乱";

    default:
        return "未知";
    }
}

std::string MoodSystem::getMoodDescription() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return "No special mood effect.";

    case MoodType::Focus:
        return "Move faster, attack faster, cooldown shorter, take slightly less damage.";

    case MoodType::Irritable:
        return "Deal more damage, but take more damage.";

    case MoodType::Exhausted:
        return "Move slower, attack slower, cooldown longer.";

    case MoodType::Excited:
        return "Move and attack much faster, but take more damage.";

    case MoodType::Fear:
        return "Move faster and dodge more, but deal less damage.";

    case MoodType::Calm:
        return "Higher critical chance, dodge chance, pickup range, and regeneration.";

    case MoodType::Panic:
        return "Move very fast, but cooldown and damage taken become worse.";

    default:
        return "Unknown mood.";
    }
}

std::string MoodSystem::getDebugInfo() const
{
    std::ostringstream oss;

    oss << "[MoodSystem]"
        << " mood=" << getMoodName()
        << " value=" << moodValue << "/" << maxMoodValue
        << " duration=" << moodDuration
        << " intensity=" << moodIntensity
        << " speedMul=" << getSpeedMultiplier()
        << " damageTakenMul=" << getDamageMultiplier()
        << " cooldownMul=" << getCooldownMultiplier()
        << " outgoingDamageMul=" << getOutgoingDamageMultiplier();

    return oss.str();
}

// =========================
// 持续时间
// =========================

void MoodSystem::setMoodDuration(float duration)
{
    if (duration <= 0.0f)
    {
        moodDuration = 0.0f;
        maxMoodDuration = 0.0f;
        hasDuration = false;
        return;
    }

    moodDuration = duration;
    maxMoodDuration = duration;
    hasDuration = true;
}

float MoodSystem::getMoodDuration() const
{
    return moodDuration;
}

float MoodSystem::getMaxMoodDuration() const
{
    return maxMoodDuration;
}

float MoodSystem::getMoodDurationPercent() const
{
    if (!hasDuration || maxMoodDuration <= 0.0f)
    {
        return 0.0f;
    }

    return moodDuration / maxMoodDuration;
}

bool MoodSystem::hasMoodDurationLimit() const
{
    return hasDuration;
}

void MoodSystem::resetMood()
{
    previousMood = currentMood;
    currentMood = MoodType::Normal;

    moodDuration = 0.0f;
    maxMoodDuration = 0.0f;
    hasDuration = false;

    moodIntensity = 1.0f;
}

// =========================
// 情绪强度
// =========================

void MoodSystem::setMoodIntensity(float intensity)
{
    moodIntensity = clampFloat(intensity, 0.1f, 3.0f);
}

float MoodSystem::getMoodIntensity() const
{
    return moodIntensity;
}

// =========================
// 情绪值系统
// =========================

void MoodSystem::addMoodValue(float value)
{
    moodValue += value;
    moodValue = clampFloat(moodValue, 0.0f, maxMoodValue);
}

void MoodSystem::reduceMoodValue(float value)
{
    moodValue -= value;
    moodValue = clampFloat(moodValue, 0.0f, maxMoodValue);
}

void MoodSystem::setMoodValue(float value)
{
    moodValue = clampFloat(value, 0.0f, maxMoodValue);
}

float MoodSystem::getMoodValue() const
{
    return moodValue;
}

void MoodSystem::setMaxMoodValue(float value)
{
    maxMoodValue = std::max(1.0f, value);

    if (moodValue > maxMoodValue)
    {
        moodValue = maxMoodValue;
    }
}

float MoodSystem::getMaxMoodValue() const
{
    return maxMoodValue;
}

float MoodSystem::getMoodValuePercent() const
{
    if (maxMoodValue <= 0.0f)
    {
        return 0.0f;
    }

    return moodValue / maxMoodValue;
}

void MoodSystem::updateMoodByValue()
{
    float percent = getMoodValuePercent();

    if (percent >= 0.85f)
    {
        changeMood(MoodType::Excited, 6.0f, 1.2f);
    }
    else if (percent >= 0.70f)
    {
        changeMood(MoodType::Focus, 6.0f, 1.0f);
    }
    else if (percent <= 0.15f)
    {
        changeMood(MoodType::Exhausted, 6.0f, 1.2f);
    }
    else if (percent <= 0.30f)
    {
        changeMood(MoodType::Fear, 5.0f, 1.0f);
    }
    else
    {
        if (currentMood != MoodType::Normal && !hasDuration)
        {
            resetMood();
        }
    }
}

// =========================
// 常用事件接口
// =========================

void MoodSystem::onPlayerDamaged()
{
    reduceMoodValue(8.0f);

    // 受伤后有概率进入恐惧或慌乱，这里直接按情绪值判断
    if (getMoodValuePercent() < 0.25f)
    {
        changeMood(MoodType::Panic, 3.0f, 1.0f);
    }
}

void MoodSystem::onEnemyKilled()
{
    addMoodValue(5.0f);

    if (getMoodValuePercent() > 0.75f)
    {
        changeMood(MoodType::Excited, 4.0f, 1.0f);
    }
}

void MoodSystem::onMaterialPicked()
{
    addMoodValue(1.0f);
}

void MoodSystem::onLowHp()
{
    reduceMoodValue(5.0f);

    if (getMoodValuePercent() < 0.35f)
    {
        changeMood(MoodType::Fear, 4.0f, 1.0f);
    }
}

void MoodSystem::onWaveStart()
{
    changeMood(MoodType::Focus, 3.0f, 1.0f);
}

void MoodSystem::onWaveEnd()
{
    changeMood(MoodType::Calm, 5.0f, 1.0f);
}

// =========================
// 工具函数
// =========================

float MoodSystem::clampFloat(float value, float minValue, float maxValue) const
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}

float MoodSystem::applyIntensity(float baseMultiplier) const
{
    // 让情绪强度影响倍率，但不直接把倍率乘爆
    // 例如 baseMultiplier = 1.2, intensity = 1.5
    // 最终为 1 + (0.2 * 1.5) = 1.3
    float result = 1.0f + (baseMultiplier - 1.0f) * moodIntensity;

    if (result < 0.1f)
    {
        result = 0.1f;
    }

    return result;
}
