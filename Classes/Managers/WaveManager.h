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

    void update(float dt);          // 在 GameScene 的 update 中调用
    void startWave(int waveIndex);  // 开始第几波
    void stopSpawn();               // 停止刷怪（游戏结束/通关时）
    std::vector<Enemy*>& getAliveEnemies();  // 获取当前存活的敌人列表

    // 波次信息
    int getCurrentWave() const { return _currentWave; }
    int getTotalWaves() const { return _totalWaves; }
    int getKillCount() const { return _killCount; }
    bool isWaveActive() const { return _isSpawning || !_aliveEnemies.empty(); }
    bool isBossWave() const { return _isBossWave; }

    // 回调：波次清理完成时调用
    void setWaveClearedCallback(std::function<void(int)> callback);
    // 回调：所有波次完成（通关）时调用
    void setAllWavesClearedCallback(std::function<void()> callback);
    // 回调：Boss波开始
    void setBossWaveCallback(std::function<void(int)> callback);

private:
    void spawnEnemy();              // 生成一个敌人
    Enemy* createEnemyByType(int enemyType, const cocos2d::Vec2& pos, int waveLevel);
    void onWaveCleared();           // 当前波所有敌人被消灭
    void showWaveAnnouncement(int wave); // 显示波次公告
    int getEnemyCountForWave(int wave);  // 根据波次计算敌人数量

    Player* _player;
    cocos2d::Node* _parentLayer;   // 敌人添加到的层

    int _currentWave;               // 当前波次（从1开始）
    int _totalWaves;                // 总波次数（默认10波）
    int _enemiesToSpawn;            // 本波还需生成的敌人数
    float _spawnTimer;              // 生成计时器
    float _spawnInterval;           // 生成间隔（秒）
    bool _isSpawning;               // 是否正在生成

    std::vector<Enemy*> _aliveEnemies; // 当前存活的敌人列表
    int _totalEnemiesThisWave;      // 本波总敌人数
    int _enemiesSpawnedCount;       // 本波已生成的敌人数

    int _killCount;                 // 本局总击杀数
    bool _isBossWave;               // 当前波是否为Boss波
    bool _bossSpawned;              // Boss是否已生成

    float _waveDelayTimer;          // 波次间延迟计时器
    bool _waitingForNextWave;       // 是否在等待下一波

    // 回调函数
    std::function<void(int)> _waveClearedCallback;
    std::function<void()> _allWavesClearedCallback;
    std::function<void(int)> _bossWaveCallback;
};

#endif
