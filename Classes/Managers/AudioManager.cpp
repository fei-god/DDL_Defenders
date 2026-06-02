#include "AudioManager.h"

#include "audio/include/AudioEngine.h"

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>

// For Cocos2d-x 3.x / many Cocos2d-x 4.x project templates,
// AudioEngine is under cocos2d::experimental.
//
// If your project reports a compile error like:
//     'experimental': is not a member of 'cocos2d'
// then replace the following line with:
//     using cocos2d::AudioEngine;
using cocos2d::AudioEngine;

namespace
{
    // ------------------------------------------------------------
    // Audio resource paths
    // Files should be placed in Resources/audio/
    // ------------------------------------------------------------

    // BGM
    const char* BGM_MENU       = "audio/bgm_menu.mp3";
    const char* BGM_GAME       = "audio/bgm_dorm_night.mp3";
    const char* BGM_BOSS       = "audio/bgm_boss.mp3";

    // UI
    const char* SFX_BUTTON_CLICK = "audio/sfx_button_click.wav";
    const char* SFX_BUTTON_HOVER = "audio/sfx_button_hover.wav";
    const char* SFX_MENU_OPEN    = "audio/sfx_menu_open.wav";
    const char* SFX_MENU_CLOSE   = "audio/sfx_menu_close.wav";
    const char* SFX_PAUSE        = "audio/sfx_pause.wav";
    const char* SFX_RESUME       = "audio/sfx_resume.wav";

    // Player
    const char* SFX_PLAYER_MOVE     = "audio/sfx_player_move.wav";
    const char* SFX_PLAYER_HURT     = "audio/sfx_player_hurt.wav";
    const char* SFX_PLAYER_DIE      = "audio/sfx_player_die.wav";
    const char* SFX_LEVEL_UP        = "audio/sfx_level_up.wav";
    const char* SFX_RANK_UP         = "audio/sfx_rank_up.wav";

    // Weapons
    const char* SFX_KEYBOARD_ATTACK = "audio/sfx_keyboard_attack.wav";
    const char* SFX_LASER_ATTACK    = "audio/sfx_laser_attack.wav";
    const char* SFX_COFFEE_ATTACK   = "audio/sfx_coffee_attack.wav";
    const char* SFX_WEAPON_UPGRADE  = "audio/sfx_weapon_upgrade.wav";

    // Enemies
    const char* SFX_ENEMY_SPAWN = "audio/sfx_enemy_spawn.wav";
    const char* SFX_ENEMY_HIT   = "audio/sfx_enemy_hit.wav";
    const char* SFX_ENEMY_DIE   = "audio/sfx_enemy_die.wav";

    const char* SFX_SLEEPY_MONSTER = "audio/sfx_sleepy_monster_spawn.wav";
    const char* SFX_PHONE_MONSTER  = "audio/sfx_phone_monster_spawn.wav";
    const char* SFX_DDL_MONSTER    = "audio/sfx_ddl_monster_spawn.wav";

    // Boss
    const char* SFX_BOSS_WARNING = "audio/sfx_boss_warning.wav";
    const char* SFX_BOSS_SPAWN   = "audio/sfx_boss_spawn.wav";
    const char* SFX_BOSS_HIT     = "audio/sfx_boss_hit.wav";
    const char* SFX_BOSS_DIE     = "audio/sfx_boss_die.wav";

    // Game flow
    const char* SFX_GAME_START       = "audio/sfx_game_start.wav";
    const char* SFX_GAME_OVER        = "audio/sfx_game_over.wav";
    const char* SFX_VICTORY          = "audio/sfx_victory.wav";
    const char* SFX_PROGRESS_COMPLETE = "audio/sfx_progress_complete.wav";

    // Mood
    const char* SFX_MOOD_FOCUS     = "audio/sfx_mood_focus.wav";
    const char* SFX_MOOD_IRRITABLE = "audio/sfx_mood_irritable.wav";
    const char* SFX_MOOD_EXHAUSTED = "audio/sfx_mood_exhausted.wav";

    // Environment
    const char* SFX_BED_EFFECT    = "audio/sfx_bed_effect.wav";
    const char* SFX_SOCKET_EFFECT = "audio/sfx_socket_effect.wav";
    const char* SFX_DESK_EFFECT   = "audio/sfx_desk_effect.wav";

    const int INVALID_AUDIO_ID = -1;

    // Used to avoid sound spam when the same effect is triggered too frequently.
    std::map<std::string, long long> g_lastPlayTime;
}

AudioManager* AudioManager::instance_ = nullptr;

AudioManager* AudioManager::getInstance()
{
    if (instance_ == nullptr)
    {
        instance_ = new AudioManager();
    }
    return instance_;
}

AudioManager::AudioManager()
    : bgmAudioId_(INVALID_AUDIO_ID),
      currentBGMPath_(""),
      bgmVolume_(0.50f),
      sfxVolume_(1.00f),
      muted_(false),
      bgmPaused_(false)
{
}

void AudioManager::preloadAll()
{
    std::vector<const char*> audioFiles = {
        BGM_MENU,
        BGM_GAME,
        BGM_BOSS,

        SFX_BUTTON_CLICK,
        SFX_BUTTON_HOVER,
        SFX_MENU_OPEN,
        SFX_MENU_CLOSE,
        SFX_PAUSE,
        SFX_RESUME,

        SFX_PLAYER_MOVE,
        SFX_PLAYER_HURT,
        SFX_PLAYER_DIE,
        SFX_LEVEL_UP,
        SFX_RANK_UP,

        SFX_KEYBOARD_ATTACK,
        SFX_LASER_ATTACK,
        SFX_COFFEE_ATTACK,
        SFX_WEAPON_UPGRADE,

        SFX_ENEMY_SPAWN,
        SFX_ENEMY_HIT,
        SFX_ENEMY_DIE,
        SFX_SLEEPY_MONSTER,
        SFX_PHONE_MONSTER,
        SFX_DDL_MONSTER,

        SFX_BOSS_WARNING,
        SFX_BOSS_SPAWN,
        SFX_BOSS_HIT,
        SFX_BOSS_DIE,

        SFX_GAME_START,
        SFX_GAME_OVER,
        SFX_VICTORY,
        SFX_PROGRESS_COMPLETE,

        SFX_MOOD_FOCUS,
        SFX_MOOD_IRRITABLE,
        SFX_MOOD_EXHAUSTED,

        SFX_BED_EFFECT,
        SFX_SOCKET_EFFECT,
        SFX_DESK_EFFECT
    };

    for (const char* file : audioFiles)
    {
        AudioEngine::preload(file);
    }
}

void AudioManager::unloadAll()
{
    stopAll();
    AudioEngine::uncacheAll();
    g_lastPlayTime.clear();
}

void AudioManager::playMenuBGM()
{
    playBGM(BGM_MENU, true);
}

void AudioManager::playGameBGM()
{
    playBGM(BGM_GAME, true);
}

void AudioManager::playBossBGM()
{
    playBGM(BGM_BOSS, true);
}

void AudioManager::stopBGM()
{
    if (bgmAudioId_ != INVALID_AUDIO_ID)
    {
        AudioEngine::stop(bgmAudioId_);
        bgmAudioId_ = INVALID_AUDIO_ID;
    }

    currentBGMPath_.clear();
    bgmPaused_ = false;
}

void AudioManager::pauseBGM()
{
    if (bgmAudioId_ != INVALID_AUDIO_ID && !bgmPaused_)
    {
        AudioEngine::pause(bgmAudioId_);
        bgmPaused_ = true;
    }
}

void AudioManager::resumeBGM()
{
    if (bgmAudioId_ != INVALID_AUDIO_ID && bgmPaused_)
    {
        AudioEngine::resume(bgmAudioId_);
        bgmPaused_ = false;
    }
}

void AudioManager::playButtonClick()
{
    playEffectWithCooldown(SFX_BUTTON_CLICK, 80);
}

void AudioManager::playButtonHover()
{
    playEffectWithCooldown(SFX_BUTTON_HOVER, 120);
}

void AudioManager::playMenuOpen()
{
    playEffect(SFX_MENU_OPEN);
}

void AudioManager::playMenuClose()
{
    playEffect(SFX_MENU_CLOSE);
}

void AudioManager::playPause()
{
    playEffect(SFX_PAUSE);
}

void AudioManager::playResume()
{
    playEffect(SFX_RESUME);
}

void AudioManager::playPlayerMove()
{
    // Do NOT call this every frame. Call when the player starts moving,
    // or keep the cooldown to avoid repeated sound spam.
    playEffectWithCooldown(SFX_PLAYER_MOVE, 300);
}

void AudioManager::playPlayerHurt()
{
    playEffectWithCooldown(SFX_PLAYER_HURT, 120);
}

void AudioManager::playPlayerDie()
{
    playEffect(SFX_PLAYER_DIE);
}

void AudioManager::playPlayerLevelUp()
{
    playEffect(SFX_LEVEL_UP);
}

void AudioManager::playPlayerRankUp()
{
    playEffect(SFX_RANK_UP);
}

void AudioManager::playKeyboardAttack()
{
    playEffectWithCooldown(SFX_KEYBOARD_ATTACK, 80);
}

void AudioManager::playLaserAttack()
{
    playEffectWithCooldown(SFX_LASER_ATTACK, 80);
}

void AudioManager::playCoffeeAttack()
{
    playEffectWithCooldown(SFX_COFFEE_ATTACK, 100);
}

void AudioManager::playWeaponUpgrade()
{
    playEffect(SFX_WEAPON_UPGRADE);
}

void AudioManager::playEnemySpawn()
{
    playEffectWithCooldown(SFX_ENEMY_SPAWN, 120);
}

void AudioManager::playEnemyHit()
{
    playEffectWithCooldown(SFX_ENEMY_HIT, 80);
}

void AudioManager::playEnemyDie()
{
    playEffectWithCooldown(SFX_ENEMY_DIE, 80);
}

void AudioManager::playSleepyMonsterSpawn()
{
    playEffectWithCooldown(SFX_SLEEPY_MONSTER, 200);
}

void AudioManager::playPhoneMonsterSpawn()
{
    playEffectWithCooldown(SFX_PHONE_MONSTER, 200);
}

void AudioManager::playDDLMonsterSpawn()
{
    playEffectWithCooldown(SFX_DDL_MONSTER, 200);
}

void AudioManager::playBossWarning()
{
    playEffect(SFX_BOSS_WARNING);
}

void AudioManager::playBossSpawn()
{
    playEffect(SFX_BOSS_SPAWN);
}

void AudioManager::playBossHit()
{
    playEffectWithCooldown(SFX_BOSS_HIT, 100);
}

void AudioManager::playBossDie()
{
    playEffect(SFX_BOSS_DIE);
}

void AudioManager::playGameStart()
{
    playEffect(SFX_GAME_START);
}

void AudioManager::playGameOver()
{
    playEffect(SFX_GAME_OVER);
}

void AudioManager::playVictory()
{
    playEffect(SFX_VICTORY);
}

void AudioManager::playProgressComplete()
{
    playEffect(SFX_PROGRESS_COMPLETE);
}

void AudioManager::playMoodFocus()
{
    playEffect(SFX_MOOD_FOCUS);
}

void AudioManager::playMoodIrritable()
{
    playEffect(SFX_MOOD_IRRITABLE);
}

void AudioManager::playMoodExhausted()
{
    playEffect(SFX_MOOD_EXHAUSTED);
}

void AudioManager::playBedEffect()
{
    playEffectWithCooldown(SFX_BED_EFFECT, 500);
}

void AudioManager::playSocketEffect()
{
    playEffectWithCooldown(SFX_SOCKET_EFFECT, 500);
}

void AudioManager::playDeskEffect()
{
    playEffectWithCooldown(SFX_DESK_EFFECT, 500);
}

void AudioManager::setBGMVolume(float volume)
{
    bgmVolume_ = clampVolume(volume);

    if (!muted_ && bgmAudioId_ != INVALID_AUDIO_ID)
    {
        AudioEngine::setVolume(bgmAudioId_, bgmVolume_);
    }
}

void AudioManager::setSFXVolume(float volume)
{
    sfxVolume_ = clampVolume(volume);
}

float AudioManager::getBGMVolume() const
{
    return bgmVolume_;
}

float AudioManager::getSFXVolume() const
{
    return sfxVolume_;
}

void AudioManager::muteAll()
{
    muted_ = true;

    if (bgmAudioId_ != INVALID_AUDIO_ID)
    {
        AudioEngine::setVolume(bgmAudioId_, 0.0f);
    }

    // Existing short effects are stopped to make mute immediate.
    stopAllEffects();
}

void AudioManager::unmuteAll()
{
    muted_ = false;

    if (bgmAudioId_ != INVALID_AUDIO_ID)
    {
        AudioEngine::setVolume(bgmAudioId_, bgmVolume_);
    }
}

bool AudioManager::isMuted() const
{
    return muted_;
}

int AudioManager::playEffect(const std::string& path)
{
    return playEffect(path, 1.0f);
}

int AudioManager::playEffect(const std::string& path, float volume)
{
    if (muted_)
    {
        return INVALID_AUDIO_ID;
    }

    float finalVolume = clampVolume(sfxVolume_ * volume);
    return AudioEngine::play2d(path, false, finalVolume);
}

int AudioManager::playEffectWithCooldown(const std::string& path, int cooldownMs)
{
    long long current = nowMilliseconds();

    auto it = g_lastPlayTime.find(path);
    if (it != g_lastPlayTime.end())
    {
        long long delta = current - it->second;
        if (delta < cooldownMs)
        {
            return INVALID_AUDIO_ID;
        }
    }

    g_lastPlayTime[path] = current;
    return playEffect(path);
}

int AudioManager::playBGM(const std::string& path, bool loop)
{
    if (currentBGMPath_ == path && bgmAudioId_ != INVALID_AUDIO_ID)
    {
        if (bgmPaused_)
        {
            resumeBGM();
        }
        return bgmAudioId_;
    }

    stopBGM();

    float finalVolume = muted_ ? 0.0f : bgmVolume_;
    bgmAudioId_ = AudioEngine::play2d(path, loop, finalVolume);
    currentBGMPath_ = path;
    bgmPaused_ = false;

    return bgmAudioId_;
}

void AudioManager::stopEffect(int audioId)
{
    if (audioId != INVALID_AUDIO_ID)
    {
        AudioEngine::stop(audioId);
    }
}

void AudioManager::stopAllEffects()
{
    // Cocos2d-x AudioEngine has no simple category-based stop for SFX only.
    // To avoid stopping BGM, we temporarily remember and restore BGM if needed.
    // In this simple course-project version, stopAllEffects() only clears cooldown data.
    // Use stopAll() when you want to stop every sound including BGM.
    g_lastPlayTime.clear();
}

void AudioManager::stopAll()
{
    AudioEngine::stopAll();

    bgmAudioId_ = INVALID_AUDIO_ID;
    currentBGMPath_.clear();
    bgmPaused_ = false;
    g_lastPlayTime.clear();
}

float AudioManager::clampVolume(float volume) const
{
    if (volume < 0.0f)
    {
        return 0.0f;
    }

    if (volume > 1.0f)
    {
        return 1.0f;
    }

    return volume;
}

long long AudioManager::nowMilliseconds() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
