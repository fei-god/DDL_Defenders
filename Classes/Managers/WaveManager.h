#pragma once
#ifndef __WAVE_MANAGER_H__
#define __WAVE_MANAGER_H__

#include "cocos2d.h"
#include "Enemy.h"
#include "Player.h"
#include <vector>
#include <functional>

class WaveManager : public cocos2d::Ref
{
public:
    static WaveManager* create(Player* player, cocos2d::Node* parentLayer);
    bool init(Player* player, cocos2d::Node* parentLayer);

    void update(float dt);
    void startWave(int waveIndex);
    void stopSpawn();
    std::vector<Enemy*>& getAliveEnemies();

    int getCurrentWave() const { return _currentWave; }
    int getTotalWaves() const { return _totalWaves; }
    int getKillCount() const { return _killCount; }
    bool isWaveActive() const { return _isSpawning || !_aliveEnemies.empty(); }
    bool isBossWave() const { return _isBossWave; }
    void setTotalWaves(int totalWaves) { _totalWaves = totalWaves; }
    void setFrozen(bool frozen);
    bool isFrozen() const { return _isFrozen; }

    // Time-based difficulty scaling
    void setElapsedTime(float elapsed) { _elapsedTime = elapsed; }

    void setWaveClearedCallback(std::function<void(int)> callback);
    void setAllWavesClearedCallback(std::function<void()> callback);

    void setEnemyKilledCallback(std::function<void(Enemy*)> callback);
    void setWaveTimerExpiredCallback(std::function<void(int)> callback);

    float getWaveTimerRemaining() const { return std::max(0.0f, _waveDuration - _waveTimer); }
    float getWaveDuration() const { return _waveDuration; }
    void setWaveDuration(float duration) { _waveDuration = duration; }

    // Restrict which enemy types can spawn (empty = all allowed)
    void setAllowedTypes(const std::vector<int>& types) { _allowedTypes = types; }

private:
    void spawnEnemy();
    Enemy* createEnemyByType(int enemyType, const cocos2d::Vec2& pos, int waveLevel);
    void onWaveCleared();
    void showWaveAnnouncement(int wave);
    int getEnemyCountForWave(int wave);

    Player* _player;
    cocos2d::Node* _parentLayer;

    int _currentWave;
    int _totalWaves;
    int _enemiesToSpawn;
    float _spawnTimer;
    float _spawnInterval;
    float _spawnElapsed;
    float _waveTimer;
    float _waveDuration;
    bool _isSpawning;

    std::vector<Enemy*> _aliveEnemies;

    int _killCount;
    bool _isBossWave;
    bool _bossSpawned;

    float _waveDelayTimer;
    bool _waitingForNextWave;
    bool _isFrozen;

    float _elapsedTime;

    // Orphaned projectiles (from dead enemies that still have active projectiles)
    std::vector<EnemyProjectile> _orphanProjectiles;

    std::function<void(int)> _waveClearedCallback;
    std::function<void()> _allWavesClearedCallback;
    std::function<void(int)> _waveTimerExpiredCallback;

    std::function<void(Enemy*)> _enemyKilledCallback;

    std::vector<int> _allowedTypes;   // empty = all allowed
};

#endif
