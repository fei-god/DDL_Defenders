#pragma once

#include <string>

/*
 * AudioManager
 * ------------------------------------------------------------
 * This class is the unified audio interface for the whole game.
 *
 * Other modules should NOT directly call cocos2d::AudioEngine.
 * They should call AudioManager methods, such as:
 *
 *     AudioManager::getInstance()->playKeyboardAttack();
 *     AudioManager::getInstance()->playEnemyDie();
 *     AudioManager::getInstance()->playVictory();
 *
 * Audio files should be placed under:
 *
 *     Resources/audio/
 *
 * In code, use paths like:
 *
 *     "audio/sfx_button_click.wav"
 */

class AudioManager
{
public:
    static AudioManager* getInstance();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // ------------------------------------------------------------
    // Resource management
    // ------------------------------------------------------------
    void preloadAll();
    void unloadAll();

    // ------------------------------------------------------------
    // Background music
    // ------------------------------------------------------------
    void playMenuBGM();
    void playGameBGM();
    void playBossBGM();

    void stopBGM();
    void pauseBGM();
    void resumeBGM();

    // ------------------------------------------------------------
    // UI / menu sound effects
    // ------------------------------------------------------------
    void playButtonClick();
    void playButtonHover();
    void playMenuOpen();
    void playMenuClose();
    void playPause();
    void playResume();

    // ------------------------------------------------------------
    // Player sound effects
    // ------------------------------------------------------------
    void playPlayerMove();
    void playPlayerHurt();
    void playPlayerDie();
    void playPlayerLevelUp();
    void playPlayerRankUp();

    // ------------------------------------------------------------
    // Weapon sound effects
    // ------------------------------------------------------------
    void playKeyboardAttack();
    void playLaserAttack();
    void playCoffeeAttack();
    void playWeaponUpgrade();

    // ------------------------------------------------------------
    // Enemy sound effects
    // ------------------------------------------------------------
    void playEnemySpawn();
    void playEnemyHit();
    void playEnemyDie();

    void playSleepyMonsterSpawn();
    void playPhoneMonsterSpawn();
    void playDDLMonsterSpawn();

    // ------------------------------------------------------------
    // Boss sound effects
    // ------------------------------------------------------------
    void playBossWarning();
    void playBossSpawn();
    void playBossHit();
    void playBossDie();

    // ------------------------------------------------------------
    // Game flow sound effects
    // ------------------------------------------------------------
    void playGameStart();
    void playGameOver();
    void playVictory();
    void playProgressComplete();

    // ------------------------------------------------------------
    // Mood system sound effects
    // ------------------------------------------------------------
    void playMoodFocus();
    void playMoodIrritable();
    void playMoodExhausted();

    // ------------------------------------------------------------
    // Environmental interaction sound effects
    // ------------------------------------------------------------
    void playBedEffect();
    void playSocketEffect();
    void playDeskEffect();

    // ------------------------------------------------------------
    // Volume control
    // volume range: 0.0f ~ 1.0f
    // ------------------------------------------------------------
    void setBGMVolume(float volume);
    void setSFXVolume(float volume);

    float getBGMVolume() const;
    float getSFXVolume() const;

    void muteAll();
    void unmuteAll();
    bool isMuted() const;

    // ------------------------------------------------------------
    // Generic low-level interfaces
    // Usually used only inside AudioManager or by advanced modules.
    // Prefer semantic methods such as playEnemyHit() in normal code.
    // ------------------------------------------------------------
    int playEffect(const std::string& path);
    int playEffect(const std::string& path, float volume);
    int playEffectWithCooldown(const std::string& path, int cooldownMs);
    int playBGM(const std::string& path, bool loop = true);

    void stopEffect(int audioId);
    void stopAllEffects();
    void stopAll();

private:
    AudioManager();

    float clampVolume(float volume) const;
    long long nowMilliseconds() const;

private:
    static AudioManager* instance_;

    int bgmAudioId_;
    std::string currentBGMPath_;

    float bgmVolume_;
    float sfxVolume_;

    bool muted_;
    bool bgmPaused_;
};
