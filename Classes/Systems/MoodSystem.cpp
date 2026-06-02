#include "MoodSystem.h"

MoodSystem::MoodSystem()
    : currentMood(MoodType::Normal)
    , moodDuration(0.0f)
    , hasDuration(false)
{
}

MoodSystem::~MoodSystem()
{
}

void MoodSystem::initMoodSystem()
{
    currentMood = MoodType::Normal;
    moodDuration = 0.0f;
    hasDuration = false;
}

void MoodSystem::updateMood(float dt)
{
    if (!hasDuration)
    {
        return;
    }

    moodDuration -= dt;

    if (moodDuration <= 0.0f)
    {
        resetMood();
    }
}

void MoodSystem::changeMood(MoodType mood)
{
    currentMood = mood;
}

MoodType MoodSystem::getCurrentMood() const
{
    return currentMood;
}

float MoodSystem::getSpeedMultiplier() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return 1.0f;

    case MoodType::Focus:
        return 1.2f;

    case MoodType::Irritable:
        return 1.1f;

    case MoodType::Exhausted:
        return 0.7f;

    default:
        return 1.0f;
    }
}

float MoodSystem::getDamageMultiplier() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return 1.0f;

    case MoodType::Focus:
        return 0.9f;

    case MoodType::Irritable:
        return 1.2f;

    case MoodType::Exhausted:
        return 1.0f;

    default:
        return 1.0f;
    }
}

float MoodSystem::getCooldownMultiplier() const
{
    switch (currentMood)
    {
    case MoodType::Normal:
        return 1.0f;

    case MoodType::Focus:
        return 0.8f;

    case MoodType::Irritable:
        return 1.0f;

    case MoodType::Exhausted:
        return 1.2f;

    default:
        return 1.0f;
    }
}

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

    default:
        return "Unknown";
    }
}

void MoodSystem::setMoodDuration(float duration)
{
    moodDuration = duration;
    hasDuration = true;
}

void MoodSystem::resetMood()
{
    currentMood = MoodType::Normal;
    moodDuration = 0.0f;
    hasDuration = false;
}
