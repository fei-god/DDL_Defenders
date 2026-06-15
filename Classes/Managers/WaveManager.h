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

    void update(float dt);          // 鍦?GameScene 鐨?update 涓皟鐢?
    void startWave(int waveIndex);  // 寮€濮嬬鍑犳尝
    void stopSpawn();               // 鍋滄鍒锋€紙娓告垙缁撴潫/閫氬叧鏃讹級
    std::vector<Enemy*>& getAliveEnemies();  // 鑾峰彇褰撳墠瀛樻椿鐨勬晫浜哄垪琛?

    // 娉㈡淇℃伅
    int getCurrentWave() const { return _currentWave; }
    int getTotalWaves() const { return _totalWaves; }
    int getKillCount() const { return _killCount; }
    bool isWaveActive() const { return _isSpawning || !_aliveEnemies.empty(); }
    bool isBossWave() const { return _isBossWave; }
    void setTotalWaves(int totalWaves) { _totalWaves = totalWaves; }
    void setFrozen(bool frozen);
    bool isFrozen() const { return _isFrozen; }

    // 鍥炶皟锛氭尝娆℃竻鐞嗗畬鎴愭椂璋冪敤
    void setWaveClearedCallback(std::function<void(int)> callback);
    // 鍥炶皟锛氭墍鏈夋尝娆″畬鎴愶紙閫氬叧锛夋椂璋冪敤
    void setAllWavesClearedCallback(std::function<void()> callback);
    // 鍥炶皟锛欱oss娉㈠紑濮?
    void setBossWaveCallback(std::function<void(int)> callback);
    void setEnemyKilledCallback(std::function<void(Enemy*)> callback);

private:
    void spawnEnemy();              // 鐢熸垚涓€涓晫浜?
    Enemy* createEnemyByType(int enemyType, const cocos2d::Vec2& pos, int waveLevel);
    void onWaveCleared();           // 褰撳墠娉㈡墍鏈夋晫浜鸿娑堢伃
    void showWaveAnnouncement(int wave); // 鏄剧ず娉㈡鍏憡
    int getEnemyCountForWave(int wave);  // 鏍规嵁娉㈡璁＄畻鏁屼汉鏁伴噺

    Player* _player;
    cocos2d::Node* _parentLayer;   // 鏁屼汉娣诲姞鍒扮殑灞?

    int _currentWave;               // 褰撳墠娉㈡锛堜粠1寮€濮嬶級
    int _totalWaves;                // 鎬绘尝娆℃暟锛堥粯璁?0娉級
    int _enemiesToSpawn;            // 鏈尝杩橀渶鐢熸垚鐨勬晫浜烘暟
    float _spawnTimer;              // 鐢熸垚璁℃椂鍣?
    float _spawnInterval;           // 鐢熸垚闂撮殧锛堢锛?
    float _spawnElapsed;
    float _waveTimer;
    float _waveDuration;
    bool _isSpawning;               // 鏄惁姝ｅ湪鐢熸垚

    std::vector<Enemy*> _aliveEnemies; // 褰撳墠瀛樻椿鐨勬晫浜哄垪琛?
    int _totalEnemiesThisWave;      // 鏈尝鎬绘晫浜烘暟
    int _enemiesSpawnedCount;       // 鏈尝宸茬敓鎴愮殑鏁屼汉鏁?

    int _killCount;                 // 鏈眬鎬诲嚮鏉€鏁?
    bool _isBossWave;               // 褰撳墠娉㈡槸鍚︿负Boss娉?
    bool _bossSpawned;              // Boss鏄惁宸茬敓鎴?

    float _waveDelayTimer;          // 娉㈡闂村欢杩熻鏃跺櫒
    bool _waitingForNextWave;       // 鏄惁鍦ㄧ瓑寰呬笅涓€娉?
    bool _isFrozen;

    // 鍥炶皟鍑芥暟
    std::function<void(int)> _waveClearedCallback;
    std::function<void()> _allWavesClearedCallback;
    std::function<void(int)> _bossWaveCallback;
    std::function<void(Enemy*)> _enemyKilledCallback;
};

#endif

