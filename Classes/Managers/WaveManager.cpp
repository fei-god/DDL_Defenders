#include "WaveManager.h"
#include "SleepyMonster.h"
#include "DDLMonster.h"
#include "cocos2d.h"

USING_NS_CC;

WaveManager* WaveManager::create(Player* player, Node* parentLayer)
{
    WaveManager* mgr = new WaveManager();
    if (mgr && mgr->init(player, parentLayer))
    {
        mgr->autorelease();
        return mgr;
    }
    CC_SAFE_DELETE(mgr);
    return nullptr;
}

bool WaveManager::init(Player* player, Node* parentLayer)
{
    _player = player;
    _parentLayer = parentLayer;
    _currentWave = 0;
    _enemiesToSpawn = 0;
    _spawnTimer = 0.0f;
    _spawnInterval = 1.5f;
    _isSpawning = false;
    _totalEnemiesThisWave = 0;
    return true;
}

void WaveManager::startWave(int waveIndex)
{
    _currentWave = waveIndex;
    // 根据波次决定生成哪些敌人
    // 基础数量：波次 * 2 个敌人，上限10
    int baseCount = 3 + waveIndex / 2;
    if (baseCount > 12) baseCount = 12;
    _totalEnemiesThisWave = baseCount;
    _enemiesToSpawn = baseCount;
    _spawnTimer = 0.0f;
    _isSpawning = true;

    // 清空旧的存活记录（理论上已经全死了）
    _aliveEnemies.clear();

    CCLOG("Wave %d started, will spawn %d enemies", waveIndex, _enemiesToSpawn);
}

void WaveManager::stopSpawn()
{
    _isSpawning = false;
    _enemiesToSpawn = 0;
}

std::vector<Enemy*>& WaveManager::getAliveEnemies()
{
    return _aliveEnemies;
}

void WaveManager::spawnEnemy()
{
    if (!_player || !_player->isRoleAlive()) return;

    // 随机生成位置：在屏幕边缘
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos;
    int edge = rand() % 4;
    if (edge == 0) // 左边
        spawnPos = Vec2(0, rand() % (int)visibleSize.height);
    else if (edge == 1) // 右边
        spawnPos = Vec2(visibleSize.width, rand() % (int)visibleSize.height);
    else if (edge == 2) // 下边
        spawnPos = Vec2(rand() % (int)visibleSize.width, 0);
    else // 上边
        spawnPos = Vec2(rand() % (int)visibleSize.width, visibleSize.height);

    Enemy* enemy = nullptr;
    // 每波第 5 个敌人固定为 DDLMonster，或者随机
    bool isSpecial = (_totalEnemiesThisWave - _enemiesToSpawn) % 5 == 0;
    if (isSpecial || (_currentWave >= 3 && rand() % 100 < 40))
    {
        enemy = DDLMonster::create("enemy_ddl.png", spawnPos, _player);
    }
    else
    {
        enemy = SleepyMonster::create("enemy_sleepy.png", spawnPos, _player);
    }

    if (enemy)
    {
        _parentLayer->addChild(enemy);
        _aliveEnemies.push_back(enemy);
        // 监听敌人死亡事件（简单轮询，也可以使用回调）
    }
    else
    {
        CCLOG("Failed to create enemy!");
    }
}

void WaveManager::update(float dt)
{
    if (!_isSpawning)
    {
        return;
    }

    if (_player == nullptr || !_player->isRoleAlive())
    {
        stopSpawn();
        return;
    }

    // 第1步：更新当前已经生成出来的敌人
    for (auto it = _aliveEnemies.begin(); it != _aliveEnemies.end(); )
    {
        Enemy* enemy = *it;

        if (enemy == nullptr)
        {
            it = _aliveEnemies.erase(it);
            continue;
        }

        // 如果敌人已经死亡或非激活，就从场景中移除，并从_aliveEnemies列表中删除
        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            enemy->removeFromParent();
            it = _aliveEnemies.erase(it);
            continue;
        }

        // 敌人的移动、攻击冷却、追踪玩家都在updateEnemy(dt)中完成。
        enemy->updateEnemy(dt);

        // 更新之后再判断一次。
        // 因为敌人可能在这一帧被其他逻辑设置为死亡或非激活。
        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            enemy->removeFromParent();
            it = _aliveEnemies.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 第2步：按间隔继续刷怪
    if (_enemiesToSpawn > 0)
    {
        _spawnTimer -= dt;

        if (_spawnTimer <= 0.0f)
        {
            spawnEnemy();
            _enemiesToSpawn--;
            _spawnTimer = _spawnInterval;
        }
    }

    // 第3步：判断这一波是否结束
    if (_enemiesToSpawn <= 0 && _aliveEnemies.empty())
    {
        onWaveCleared();
    }
}

void WaveManager::onWaveCleared()
{
    _isSpawning = false;
    CCLOG("Wave %d cleared!", _currentWave);
    // 自动进入下一波（延迟 3 秒）
    auto delay = DelayTime::create(3.0f);
    auto call = CallFunc::create([this]() {
        // 进入下一波，可以增加一些奖励
        startWave(_currentWave + 1);
        });
    _parentLayer->runAction(Sequence::create(delay, call, nullptr));
}