#include "WaveManager.h"
#include "SleepyMonster.h"
#include "DDLMonster.h"
#include "BossMonster.h"
#include "PhoneMonster.h"
#include "ThesisBoss.h"
#include "Core/AssetPaths.h"
#include "cocos2d.h"
#include <algorithm>
#include <cmath>

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
    _totalWaves = 1000000;
    _enemiesToSpawn = 0;
    _spawnTimer = 0.0f;
    _spawnInterval = 2.45f;
    _spawnElapsed = 0.0f;
    _waveTimer = 0.0f;
    _waveDuration = 30.0f;
    _isSpawning = false;
    _totalEnemiesThisWave = 0;
    _enemiesSpawnedCount = 0;
    _killCount = 0;
    _isBossWave = false;
    _bossSpawned = false;
    _waveDelayTimer = 0.0f;
    _waitingForNextWave = false;
    _isFrozen = false;

    return true;
}

int WaveManager::getEnemyCountForWave(int wave)
{
    int count = 8 + wave * 3;
    if (count > 42) count = 42;
    return count;
}

void WaveManager::startWave(int waveIndex)
{
    _currentWave = waveIndex;
    _enemiesSpawnedCount = 0;
    _waitingForNextWave = false;
    _waveDelayTimer = 0.0f;
    _spawnElapsed = 0.0f;
    _waveTimer = 0.0f;
    _bossSpawned = false;
    _isBossWave = (waveIndex > 0 && waveIndex % 3 == 0);
    _totalEnemiesThisWave = 0;
    _enemiesToSpawn = 0;
    _spawnTimer = 1.0f;
    _isSpawning = true;

    showWaveAnnouncement(_currentWave);
    CCLOG("Wave %d continuous spawning started.", _currentWave);
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
    Size visibleSize = Director::getInstance()->getVisibleSize();
    if (_parentLayer && _parentLayer->getContentSize().width > 0.0f &&
        _parentLayer->getContentSize().height > 0.0f)
    {
        visibleSize = _parentLayer->getContentSize();
    }

    std::string waveText;
    if (_isBossWave)
    {
        waveText = "Wave " + std::to_string(wave) + ": Thesis Boss";
    }
    else
    {
        waveText = "Wave " + std::to_string(wave);
    }

    auto label = Label::createWithSystemFont(waveText, "Arial", 36);
    Vec2 labelPos(visibleSize.width / 2, visibleSize.height / 2 + 100);
    if (_player)
    {
        labelPos = _player->getPosition() + Vec2(0, 120);
    }
    label->setPosition(labelPos);
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
        {
            std::string path = AssetPaths::resolve("art/monsters/sleepy_monster.png");
            enemy = SleepyMonster::create(path.empty() ? "enemy_sleepy.png" : path, pos, _player);
        }
        break;
    case 1: // DDLMonster - 冲锋敌人
        {
            std::string path = AssetPaths::resolve("art/monsters/ddl_monster.png");
            enemy = DDLMonster::create(path.empty() ? "enemy_ddl.png" : path, pos, _player);
        }
        break;
    case 2: // ThesisBoss
        {
            std::string path = AssetPaths::resolve("art/monsters/thesis_monster.png");
            if (path.empty())
            {
                path = AssetPaths::resolve("art/monsters/thesis_boss.png");
            }
            enemy = ThesisBoss::create(path.empty() ? "enemy_boss.png" : path, pos, _player, waveLevel);
        }
        break;
    case 3: // PhoneMonster
        {
            std::string path = AssetPaths::resolve("art/monsters/phone_monster.png");
            enemy = PhoneMonster::create(path.empty() ? "enemy_phone.png" : path, pos, _player);
        }
        break;
    }

    return enemy;
}

void WaveManager::spawnEnemy()
{
    if (!_player || !_player->isRoleAlive()) return;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    if (_parentLayer && _parentLayer->getContentSize().width > 0.0f &&
        _parentLayer->getContentSize().height > 0.0f)
    {
        visibleSize = _parentLayer->getContentSize();
    }
    Vec2 playerPos = _player->getPosition();
    float angle = CCRANDOM_0_1() * 2.0f * static_cast<float>(M_PI);
    float distance = 230.0f + CCRANDOM_0_1() * 200.0f;
    Vec2 spawnPos = playerPos + Vec2(std::cos(angle), std::sin(angle)) * distance;
    spawnPos.x = std::max(45.0f, std::min(visibleSize.width - 45.0f, spawnPos.x));
    spawnPos.y = std::max(45.0f, std::min(visibleSize.height - 45.0f, spawnPos.y));

    Enemy* enemy = nullptr;
    _enemiesSpawnedCount++;

    int type = 0;
    if (_isBossWave && !_bossSpawned && _waveTimer >= 6.0f)
    {
        type = 2;
        _bossSpawned = true;
        spawnPos = playerPos + Vec2(0.0f, 320.0f);
        spawnPos.x = std::max(80.0f, std::min(visibleSize.width - 80.0f, spawnPos.x));
        spawnPos.y = std::max(80.0f, std::min(visibleSize.height - 80.0f, spawnPos.y));
    }
    else
    {
        int randVal = rand() % 100;
        int ddlChance = std::min(34, 12 + _currentWave * 4);
        int phoneChance = std::min(30, 14 + _currentWave * 3);
        if (randVal < 48 - std::min(16, _currentWave * 2)) type = 0;
        else if (randVal < 48 - std::min(16, _currentWave * 2) + phoneChance) type = 3;
        else if (randVal < 92) type = 1;
        else type = 0;
    }
    enemy = createEnemyByType(type, spawnPos, _currentWave);

    if (enemy)
    {
        bool isBossEnemy = enemy->hasTag("Boss") || enemy->getObjectName() == "ThesisBoss" ||
            enemy->getObjectName() == "BossMonster";
        float targetSize = isBossEnemy ? 240.0f : 114.0f;
        Size enemySize = enemy->getContentSize();
        float targetScale = 1.0f;
        if (enemySize.width > 0.0f && enemySize.height > 0.0f)
        {
            targetScale = std::min(targetSize / enemySize.width, targetSize / enemySize.height);
        }

        enemy->setScale(targetScale * 0.1f);
        auto scaleUp = ScaleTo::create(0.3f, targetScale);
        enemy->runAction(scaleUp);

        _parentLayer->addChild(enemy, 10);  // 添加到游戏层

        auto nameLabel = Label::createWithSystemFont(enemy->getObjectName(), "Arial", 13);
        if (nameLabel)
        {
            nameLabel->setColor(isBossEnemy ? Color3B(255, 180, 90) : Color3B(235, 235, 245));
            nameLabel->enableOutline(Color4B(0, 0, 0, 210), 2);
            float labelY = enemySize.height * 0.5f + 16.0f / std::max(0.1f, targetScale);
            nameLabel->setPosition(Vec2(0.0f, labelY));
            nameLabel->setScale(1.0f / std::max(0.1f, targetScale));
            enemy->addChild(nameLabel, 20);
        }

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
    if (!_isSpawning)
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
            if (_enemyKilledCallback)
            {
                _enemyKilledCallback(enemy);
            }
            enemy->removeFromParent();
            _killCount++;
            it = _aliveEnemies.erase(it);
            continue;
        }

        // 更新敌人AI
        if (!_isFrozen)
        {
            enemy->updateEnemy(dt);
        }

        // 更新后再次检查
        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            if (_enemyKilledCallback)
            {
                _enemyKilledCallback(enemy);
            }
            enemy->removeFromParent();
            _killCount++;
            it = _aliveEnemies.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!_isFrozen)
    {
        _spawnElapsed += dt;
        _waveTimer += dt;
        _spawnTimer -= dt;

        if (_spawnTimer <= 0.0f)
        {
            int aliveCap = std::min(_isBossWave ? 34 : 30, 12 + _currentWave * 3);
            if (static_cast<int>(_aliveEnemies.size()) < aliveCap)
            {
                spawnEnemy();
            }

            float waveDifficulty = std::min(0.85f, (_currentWave - 1) * 0.12f);
            float linearPressure = std::min(0.55f, _waveTimer * 0.012f);
            float dynamicInterval = _spawnInterval - waveDifficulty - linearPressure;
            if (_isBossWave) dynamicInterval += 0.25f;
            if (dynamicInterval < 0.78f) dynamicInterval = 0.78f;
            _spawnTimer = dynamicInterval;
        }

        if (_waveTimer >= _waveDuration)
        {
            startWave(_currentWave + 1);
        }
    }
}

void WaveManager::onWaveCleared()
{
    _isSpawning = false;
    CCLOG("=== Wave %d CLEARED! (Total kills: %d) ===", _currentWave, _killCount);

    if (_waveClearedCallback)
        _waveClearedCallback(_currentWave);

    // Keep pressure continuous between waves.
    _waitingForNextWave = true;
    _waveDelayTimer = 0.8f;
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

void WaveManager::setEnemyKilledCallback(std::function<void(Enemy*)> callback)
{
    _enemyKilledCallback = callback;
}

void WaveManager::setFrozen(bool frozen)
{
    _isFrozen = frozen;
}

std::vector<Enemy*>& WaveManager::getAliveEnemies()
{
    return _aliveEnemies;
}
