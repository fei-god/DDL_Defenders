#pragma once
#pragma once
#ifndef __WAVE_MANAGER_H__
#define __WAVE_MANAGER_H__

#include "cocos2d.h"
#include "Enemy.h"
#include "Player.h"
#include <vector>

class WaveManager : public cocos2d::Ref
{
public:
    static WaveManager* create(Player* player, cocos2d::Node* parentLayer);
    bool init(Player* player, cocos2d::Node* parentLayer);

    void update(float dt);          // 需要在 GameScene 的 update 中调用
    void startWave(int waveIndex);  // 开始第几波
    void stopSpawn();               // 停止刷怪（游戏结束或通关时）
    std::vector<Enemy*>& getAliveEnemies();  // 获取当前活着的敌人列表，给武器和碰撞检测使用

private:
    void spawnEnemy();              // 生成一个敌人
    void onWaveCleared();           // 当前波次所有敌人都死亡后调用

    Player* _player;
    cocos2d::Node* _parentLayer;   // 用于添加敌人到场景中

    int _currentWave;               // 当前波次（从1开始）
    int _enemiesToSpawn;            // 本波还需要生成的敌人数量
    float _spawnTimer;              // 生成敌人的计时器
    float _spawnInterval;           // 生成间隔（秒）
    bool _isSpawning;               // 是否正在生成中

    std::vector<Enemy*> _aliveEnemies; // 当前存活的敌人列表（仅用于计数）
    int _totalEnemiesThisWave;          // 本波敌人总数
};
#endif
