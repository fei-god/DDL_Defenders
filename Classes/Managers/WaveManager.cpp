#include "WaveManager.h"
#include "SleepyMonster.h"
#include "DDLMonster.h"
#include "BossMonster.h"
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
    _totalWaves = 10;              // 总共10波
    _enemiesToSpawn = 0;
    _spawnTimer = 0.0f;
    _spawnInterval = 1.5f;
    _isSpawning = false;
    _totalEnemiesThisWave = 0;
    _enemiesSpawnedCount = 0;
    _killCount = 0;
    _isBossWave = false;
    _bossSpawned = false;
    _waveDelayTimer = 0.0f;
    _waitingForNextWave = false;

    return true;
}

int WaveManager::getEnemyCountForWave(int wave)
{
    // 每波敌人数量 = 3 + wave * 2，上限25个
    int count = 3 + wave * 2;
    if (count > 25) count = 25;
    return count;
}

void WaveManager::startWave(int waveIndex)
{
    _currentWave = waveIndex;
    _enemiesSpawnedCount = 0;
    _waitingForNextWave = false;
    _waveDelayTimer = 0.0f;

    // 清理上一波残留
    _aliveEnemies.clear();
    _bossSpawned = false;

    // Boss波：每5波出现一次
    _isBossWave = (waveIndex % 5 == 0);

    if (_isBossWave)
    {
        // Boss波：Boss + 少量小兵
        _totalEnemiesThisWave = 4 + waveIndex / 2;  // 随波次增加小兵数量
        if (_totalEnemiesThisWave > 15) _totalEnemiesThisWave = 15;
        CCLOG("=== BOSS WAVE %d! ===", waveIndex);
        if (_bossWaveCallback)
            _bossWaveCallback(waveIndex);
    }
    else
    {
        _totalEnemiesThisWave = getEnemyCountForWave(waveIndex);
    }

    _enemiesToSpawn = _totalEnemiesThisWave;
    _spawnTimer = 1.0f;  // 第一只敌人1秒后生成
    _isSpawning = true;

    showWaveAnnouncement(waveIndex);

    CCLOG("Wave %d started: %d enemies (Boss:%s)",
        waveIndex, _totalEnemiesThisWave, _isBossWave ? "YES" : "NO");
}

void WaveManager::stopSpawn()
{
    _isSpawning = false;
    _enemiesToSpawn = 0;
}

void WaveManager::showWaveAnnouncement(int wave)
{
    // 在屏幕上显示波次公告
    // 使用Label显示，持续2秒后消失
    auto visibleSize = Director::getInstance()->getVisibleSize();

    std::string waveText;
    if (_isBossWave)
    {
        waveText = "!!! BOSS WAVE " + std::to_string(wave) + " !!!";
    }
    else
    {
        waveText = "Wave " + std::to_string(wave);
    }

    auto label = Label::createWithSystemFont(waveText, "Arial", 36);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 100));
    label->setOpacity(0);
    _parentLayer->addChild(label, 100);  // 高层级确保显示在最前

    // 淡入 → 停留 → 淡出 → 移除
    auto fadeIn = FadeIn::create(0.3f);
    auto delay = DelayTime::create(1.5f);
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = RemoveSelf::create();
    label->runAction(Sequence::create(fadeIn, delay, fadeOut, remove, nullptr));
}

Enemy* WaveManager::createEnemyByType(int enemyType, const Vec2& pos, int waveLevel)
{
    Enemy* enemy = nullptr;

    switch (enemyType)
    {
    case 0: // SleepyMonster - 基础敌人
        enemy = SleepyMonster::create("enemy_sleepy.png", pos, _player);
        break;
    case 1: // DDLMonster - 冲锋敌人
        enemy = DDLMonster::create("enemy_ddl.png", pos, _player);
        break;
    case 2: // BossMonster
        enemy = BossMonster::create("enemy_boss.png", pos, _player, waveLevel);
        break;
    }

    // 根据波次给敌人增加属性加成（难度递增）
    if (enemy && enemyType != 2)  // Boss有自己的属性计算
    {
        float hpScale = 1.0f + (waveLevel - 1) * 0.1f;     // 每波HP+10%
        float atkScale = 1.0f + (waveLevel - 1) * 0.08f;   // 每波攻击+8%
        float spdScale = 1.0f + (waveLevel - 1) * 0.03f;   // 每波速度+3%

        int scaledHp = static_cast<int>(enemy->getMaxHp() * hpScale);
        enemy->setMaxHp(scaledHp);
        enemy->setHp(scaledHp);

        int scaledAtk = static_cast<int>(enemy->getAttackDamage() * atkScale);
        enemy->setAttackDamage(scaledAtk);

        float scaledSpd = enemy->getSpeed() * spdScale;
        enemy->setSpeed(scaledSpd);
    }

    return enemy;
}

void WaveManager::spawnEnemy()
{
    if (!_player || !_player->isRoleAlive()) return;

    // 随机生成位置（屏幕边缘）
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos;
    int edge = rand() % 4;
    switch (edge)
    {
    case 0: // 左边
        spawnPos = Vec2(-20, rand() % (int)visibleSize.height);
        break;
    case 1: // 右边
        spawnPos = Vec2(visibleSize.width + 20, rand() % (int)visibleSize.height);
        break;
    case 2: // 下边
        spawnPos = Vec2(rand() % (int)visibleSize.width, -20);
        break;
    case 3: // 上边
    default:
        spawnPos = Vec2(rand() % (int)visibleSize.width, visibleSize.height + 20);
        break;
    }

    Enemy* enemy = nullptr;
    _enemiesSpawnedCount++;

    if (_isBossWave && !_bossSpawned)
    {
        // Boss波：最后一个生成的必然是Boss
        bool isLastEnemy = (_enemiesToSpawn <= 0);

        if (isLastEnemy)
        {
            // Boss从屏幕顶部中央出现
            spawnPos = Vec2(visibleSize.width / 2, visibleSize.height + 50);
            enemy = createEnemyByType(2, spawnPos, _currentWave);  // Boss
            _bossSpawned = true;
            CCLOG("BOSS spawned at wave %d!", _currentWave);
        }
        else
        {
            // Boss波小兵：DDLMonster为主
            int type = (rand() % 100 < 60) ? 1 : 0;  // 60% DDL, 40% Sleepy
            enemy = createEnemyByType(type, spawnPos, _currentWave);
        }
    }
    else
    {
        // 普通波：根据波次决定敌人类型比例
        int randVal = rand() % 100;

        if (_currentWave <= 2)
        {
            // 前2波：只有SleepyMonster（让玩家适应）
            enemy = createEnemyByType(0, spawnPos, _currentWave);
        }
        else if (_currentWave <= 4)
        {
            // 3-4波：30% DDLMonster, 70% SleepyMonster
            if (randVal < 30)
                enemy = createEnemyByType(1, spawnPos, _currentWave);
            else
                enemy = createEnemyByType(0, spawnPos, _currentWave);
        }
        else
        {
            // 5波以后：50% DDL, 50% Sleepy
            if (randVal < 50)
                enemy = createEnemyByType(1, spawnPos, _currentWave);
            else
                enemy = createEnemyByType(0, spawnPos, _currentWave);
        }
    }

    if (enemy)
    {
        // 给敌人添加出现时的缩放动画
        enemy->setScale(0.1f);
        auto scaleUp = ScaleTo::create(0.3f, (_isBossWave && _bossSpawned) ? 1.5f : 1.0f);
        enemy->runAction(scaleUp);

        _parentLayer->addChild(enemy, 10);  // 添加到游戏层
        _aliveEnemies.push_back(enemy);
    }
    else
    {
        CCLOG("ERROR: Failed to create enemy!");
        _enemiesSpawnedCount--;
    }
}

void WaveManager::update(float dt)
{
    // 如果在等待下一波，计时
    if (_waitingForNextWave)
    {
        _waveDelayTimer -= dt;
        if (_waveDelayTimer <= 0.0f)
        {
            _waitingForNextWave = false;
            if (_currentWave < _totalWaves)
            {
                startWave(_currentWave + 1);
            }
            else
            {
                // 所有波次完成！
                CCLOG("ALL WAVES CLEARED! Victory!");
                if (_allWavesClearedCallback)
                    _allWavesClearedCallback();
            }
        }
        return;
    }

    if (!_isSpawning && _aliveEnemies.empty())
    {
        return;
    }

    // 检查玩家是否存活
    if (_player == nullptr || !_player->isRoleAlive())
    {
        stopSpawn();
        return;
    }

    // 更新所有存活的敌人
    for (auto it = _aliveEnemies.begin(); it != _aliveEnemies.end(); )
    {
        Enemy* enemy = *it;

        if (enemy == nullptr)
        {
            it = _aliveEnemies.erase(it);
            continue;
        }

        // 检查敌人是否死亡或非活跃
        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            // 如果敌人还活着但从场景移除了，执行死亡逻辑
            if (enemy->isRoleAlive())
            {
                enemy->die();
            }
            enemy->removeFromParent();
            _killCount++;
            it = _aliveEnemies.erase(it);
            continue;
        }

        // 更新敌人AI
        enemy->updateEnemy(dt);

        // 更新后再次检查
        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            enemy->removeFromParent();
            _killCount++;
            it = _aliveEnemies.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 生成新敌人
    if (_enemiesToSpawn > 0)
    {
        _spawnTimer -= dt;

        if (_spawnTimer <= 0.0f)
        {
            spawnEnemy();
            _enemiesToSpawn--;

            // 动态生成间隔：敌人越多间隔越短（最少0.8秒）
            float dynamicInterval = 1.8f - (_totalEnemiesThisWave - _enemiesToSpawn) * 0.05f;
            if (dynamicInterval < 0.8f) dynamicInterval = 0.8f;
            _spawnTimer = dynamicInterval;
        }
    }

    // 判断当前波是否完成
    if (_enemiesToSpawn <= 0 && _aliveEnemies.empty())
    {
        onWaveCleared();
    }
}

void WaveManager::onWaveCleared()
{
    _isSpawning = false;
    CCLOG("=== Wave %d CLEARED! (Total kills: %d) ===", _currentWave, _killCount);

    if (_waveClearedCallback)
        _waveClearedCallback(_currentWave);

    // 波次间延迟3秒
    _waitingForNextWave = true;
    _waveDelayTimer = 3.0f;
}

void WaveManager::setWaveClearedCallback(std::function<void(int)> callback)
{
    _waveClearedCallback = callback;
}

void WaveManager::setAllWavesClearedCallback(std::function<void()> callback)
{
    _allWavesClearedCallback = callback;
}

void WaveManager::setBossWaveCallback(std::function<void(int)> callback)
{
    _bossWaveCallback = callback;
}

std::vector<Enemy*>& WaveManager::getAliveEnemies()
{
    return _aliveEnemies;
}
