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
    _spawnInterval = 2.0f;
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
    _elapsedTime = 0.0f;

    return true;
}

int WaveManager::getEnemyCountForWave(int wave)
{
    // Increased spawn counts: base of 15 + wave*6, cap raised to 80
    int count = 15 + wave * 6;
    if (count > 80) count = 80;
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
    _parentLayer->addChild(label, 100);

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
    case 0: // SleepyMonster
        {
            // Use new root-level sprite
            std::string path = AssetPaths::resolve("art/monsters/sleepymonster.png");
            if (path.empty()) path = AssetPaths::resolve("art/monsters/sleepy_monster.png");
            enemy = SleepyMonster::create(path.empty() ? "enemy_sleepy.png" : path, pos, _player, waveLevel);
        }
        break;
    case 1: // DDLMonster
        {
            std::string path = AssetPaths::resolve("art/monsters/DDLmonster.png");
            if (path.empty()) path = AssetPaths::resolve("art/monsters/ddl_monster.png");
            enemy = DDLMonster::create(path.empty() ? "enemy_ddl.png" : path, pos, _player, waveLevel);
        }
        break;
    case 2: // ThesisBoss
        {
            std::string path = AssetPaths::resolve("art/monsters/ThesisBoss.png");
            if (path.empty()) path = AssetPaths::resolve("art/monsters/thesis_boss.png");
            if (path.empty()) path = AssetPaths::resolve("art/monsters/thesis_monster.png");
            enemy = ThesisBoss::create(path.empty() ? "enemy_boss.png" : path, pos, _player, waveLevel);
        }
        break;
    case 3: // PhoneMonster
        {
            std::string path = AssetPaths::resolve("art/monsters/phonemonster.png");
            if (path.empty()) path = AssetPaths::resolve("art/monsters/phone_monster.png");
            enemy = PhoneMonster::create(path.empty() ? "enemy_phone.png" : path, pos, _player, waveLevel);
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
        // Apply time-based difficulty scaling
        float timeFactor = Enemy::getTimeScaleFactor(_elapsedTime);
        if (timeFactor > 1.0f)
        {
            int scaledHp = static_cast<int>(enemy->getMaxHp() * timeFactor);
            enemy->setMaxHp(scaledHp);
            enemy->setHp(scaledHp);
            enemy->setAttackDamage(static_cast<int>(enemy->getAttackDamage() * timeFactor));
            enemy->setSpeed(enemy->getSpeed() * (1.0f + (timeFactor - 1.0f) * 0.5f));
        }

        bool isBossEnemy = enemy->hasTag("Boss") || enemy->getObjectName() == "ThesisBoss" ||
            enemy->getObjectName() == "BossMonster";

        // Smaller monster sizes: 60px for regular, 120px for boss
        float targetSize = isBossEnemy ? 120.0f : 60.0f;
        Size enemySize = enemy->getContentSize();
        float targetScale = 1.0f;
        if (enemySize.width > 0.0f && enemySize.height > 0.0f)
        {
            targetScale = std::min(targetSize / enemySize.width, targetSize / enemySize.height);
        }

        // Spawn animation: start tiny and scale up
        enemy->setScale(targetScale * 0.1f);
        auto scaleUp = ScaleTo::create(0.3f, targetScale);
        auto startAnim = CallFunc::create([enemy]() {
            enemy->startIdleAnimation();
        });
        enemy->runAction(Sequence::create(scaleUp, startAnim, nullptr));

        _parentLayer->addChild(enemy, 10);

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

    if (_player == nullptr || !_player->isRoleAlive())
    {
        stopSpawn();
        return;
    }

    // Update all alive enemies
    for (auto it = _aliveEnemies.begin(); it != _aliveEnemies.end(); )
    {
        Enemy* enemy = *it;

        if (enemy == nullptr)
        {
            it = _aliveEnemies.erase(it);
            continue;
        }

        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            if (enemy->isRoleAlive())
            {
                enemy->die();
            }
            // Move remaining projectiles to orphan list
            auto& projs = enemy->getProjectiles();
            for (auto& p : projs)
            {
                if (p.active) _orphanProjectiles.push_back(p);
            }
            projs.clear();
            if (_enemyKilledCallback)
            {
                _enemyKilledCallback(enemy);
            }
            enemy->removeFromParent();
            _killCount++;
            it = _aliveEnemies.erase(it);
            continue;
        }

        if (!_isFrozen)
        {
            enemy->updateEnemy(dt);
        }
        else
        {
            enemy->updateHurtCooldown(dt);
        }

        if (!enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            // Move remaining projectiles to orphan list
            auto& projs = enemy->getProjectiles();
            for (auto& p : projs)
            {
                if (p.active) _orphanProjectiles.push_back(p);
            }
            projs.clear();
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

    // --- Process orphan projectiles (from dead enemies) ---
    for (auto& proj : _orphanProjectiles)
    {
        if (!proj.active || !proj.node) continue;
        proj.elapsed += dt;
        if (proj.elapsed >= proj.lifetime)
        {
            proj.active = false;
            if (proj.node)
            {
                proj.node->runAction(Sequence::create(FadeOut::create(0.15f), RemoveSelf::create(), nullptr));
                proj.node = nullptr;
            }
            continue;
        }
        proj.position += proj.direction * proj.speed * dt;
        if (proj.node)
        {
            proj.node->setPosition(proj.position);
            proj.node->setRotation(proj.node->getRotation() + dt * 180.0f);
        }
        // Check collision with player
        if (_player && _player->isRoleAlive())
        {
            float dist = proj.position.distance(_player->getPosition());
            if (dist < proj.radius + 25.0f)
            {
                _player->takeDamage(proj.damage);
                proj.active = false;
                if (proj.node)
                {
                    proj.node->runAction(Sequence::create(FadeOut::create(0.1f), RemoveSelf::create(), nullptr));
                    proj.node = nullptr;
                }
            }
        }
    }
    _orphanProjectiles.erase(
        std::remove_if(_orphanProjectiles.begin(), _orphanProjectiles.end(),
            [](const EnemyProjectile& p) { return !p.active; }),
        _orphanProjectiles.end());

    // --- Process enemy projectiles ---
    Size visibleSize = Director::getInstance()->getVisibleSize();
    if (_parentLayer && _parentLayer->getContentSize().width > 0.0f)
    {
        visibleSize = _parentLayer->getContentSize();
    }
    float projOffscreenMargin = 60.0f;

    for (auto* enemy : _aliveEnemies)
    {
        if (enemy && enemy->isRoleAlive())
        {
            enemy->updateProjectiles(dt);

            // Check projectile-player collision
            if (_player && _player->isRoleAlive())
            {
                Vec2 playerPos = _player->getPosition();
                for (auto& proj : enemy->getProjectiles())
                {
                    if (!proj.active || !proj.node) continue;

                    // Off-screen cleanup
                    if (proj.position.x < -projOffscreenMargin ||
                        proj.position.y < -projOffscreenMargin ||
                        proj.position.x > visibleSize.width + projOffscreenMargin ||
                        proj.position.y > visibleSize.height + projOffscreenMargin)
                    {
                        proj.active = false;
                        proj.node->runAction(Sequence::create(FadeOut::create(0.15f), RemoveSelf::create(), nullptr));
                        proj.node = nullptr;
                        continue;
                    }

                    float dist = proj.position.distance(playerPos);
                    if (dist < proj.radius + 25.0f) // 25px player hit radius
                    {
                        // Hit the player!
                        _player->takeDamage(proj.damage);

                        // === Dramatic impact explosion ===
                        auto hitFx = DrawNode::create();
                        hitFx->setPosition(proj.position);

                        // Expanding shockwave ring
                        hitFx->drawSolidCircle(Vec2::ZERO, 28.0f, 0, 16, Color4F(1.0f, 0.25f, 0.05f, 0.7f));
                        hitFx->drawSolidCircle(Vec2::ZERO, 18.0f, 0, 12, Color4F(1.0f, 0.5f, 0.15f, 0.8f));
                        hitFx->drawSolidCircle(Vec2::ZERO, 8.0f, 0, 8, Color4F(1.0f, 0.9f, 0.6f, 0.9f));

                        // Radial sparks flying outward
                        for (int i = 0; i < 12; i++)
                        {
                            float a = i * M_PI / 6;
                            Vec2 pt = Vec2(cosf(a), sinf(a)) * 15.0f;
                            hitFx->drawSolidCircle(pt, 3.5f, 0, 4, Color4F(1.0f, 0.8f, 0.3f, 0.85f));
                        }
                        // Additional scattered debris
                        for (int i = 0; i < 8; i++)
                        {
                            float a = (CCRANDOM_0_1()) * 2 * M_PI;
                            float r = 10.0f + (CCRANDOM_0_1()) * 18.0f;
                            Vec2 pt = Vec2(cosf(a), sinf(a)) * r;
                            hitFx->drawDot(pt, 2.0f + (CCRANDOM_0_1()) * 3.0f,
                                Color4F(1.0f, 0.7f, 0.2f, 0.7f));
                        }

                        _parentLayer->addChild(hitFx, 200);
                        auto expand = ScaleTo::create(0.4f, 2.8f);
                        auto fade = FadeOut::create(0.4f);
                        hitFx->runAction(Sequence::create(
                            Spawn::create(expand, fade, nullptr),
                            RemoveSelf::create(), nullptr));

                        // Deactivate projectile
                        proj.active = false;
                        if (proj.node)
                        {
                            proj.node->runAction(Sequence::create(
                                FadeOut::create(0.1f),
                                RemoveSelf::create(),
                                nullptr));
                            proj.node = nullptr;
                        }
                    }
                }
                enemy->cleanupProjectiles();
            }
        }
    }

    if (!_isFrozen)
    {
        _spawnElapsed += dt;
        _waveTimer += dt;
        _spawnTimer -= dt;

        if (_spawnTimer <= 0.0f)
        {
            // Increased alive cap: scales faster with waves
            int aliveCap = std::min(_isBossWave ? 55 : 48, 18 + _currentWave * 5);
            if (static_cast<int>(_aliveEnemies.size()) < aliveCap)
            {
                spawnEnemy();
            }

            // Faster spawn intervals over time (was 0.85/0.55/0.65 min)
            float waveDifficulty = std::min(1.0f, (_currentWave - 1) * 0.14f);
            float linearPressure = std::min(0.65f, _waveTimer * 0.016f);
            float dynamicInterval = _spawnInterval - waveDifficulty - linearPressure;
            if (_isBossWave) dynamicInterval += 0.2f;
            // Minimum spawn interval reduced from 0.65 to 0.45
            if (dynamicInterval < 0.45f) dynamicInterval = 0.45f;
            _spawnTimer = dynamicInterval;
        }

        if (_waveTimer >= _waveDuration)
        {
            onWaveCleared();
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
