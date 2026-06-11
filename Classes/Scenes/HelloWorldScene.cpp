/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "CollisionManager.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    // 初始化游戏
    initGame();
    initInput();
    initUI();

    // 注册每帧更新
    this->scheduleUpdate();

    return true;
}

void HelloWorld::initGame()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // === 创建玩家 ===
    // 玩家属性：HP 100, 速度 150, 防御 5
    _player = Player::create(
        "Player",
        "player.png",                          // 玩家图片（需要在Resources中有）
        Vec2(visibleSize.width / 2, visibleSize.height / 2),
        100,        // maxHp - 玩家血量
        150.0f,     // baseSpeed - 基础移动速度
        5           // defense - 防御力
    );

    if (_player)
    {
        this->addChild(_player, 10);
    }
    else
    {
        problemLoading("'player.png' - 使用默认矩形代替");
        // 如果图片加载失败，使用占位符（创建一个简单的有色方块）
        auto placeholder = DrawNode::create();
        placeholder->drawSolidRect(
            Vec2(-15, -15), Vec2(15, 15),
            Color4F::BLUE);
        this->addChild(placeholder, 10);

        // 仍然创建Player用于逻辑，但不设置精灵
        _player = Player::create(
            "Player",
            "",  // 空路径
            Vec2(visibleSize.width / 2, visibleSize.height / 2),
            100, 150.0f, 5);
        if (_player)
        {
            this->addChild(_player, 10);
        }
    }

    // === 创建波次管理器 ===
    _waveManager = WaveManager::create(_player, this);
    if (_waveManager)
    {
        // 波次清理回调
        _waveManager->setWaveClearedCallback([this](int wave) {
            CCLOG("Wave %d cleared callback triggered!", wave);
        });

        // Boss波回调
        _waveManager->setBossWaveCallback([this](int wave) {
            CCLOG("Boss wave %d announcement!", wave);
        });

        // 所有波次完成（胜利）回调
        _waveManager->setAllWavesClearedCallback([this]() {
            victory();
        });
    }

    // === 初始化游戏状态 ===
    _keyLeft = false;
    _keyRight = false;
    _keyUp = false;
    _keyDown = false;
    _keyShootUp = false;
    _keyShootDown = false;
    _keyShootLeft = false;
    _keyShootRight = false;

    _fireCooldown = 0.0f;
    _fireCooldownMax = 0.25f;  // 每秒4发

    _gameOver = false;
    _gameWin = false;

    // === 开始第一波 ===
    if (_waveManager)
    {
        // 延迟1秒后开始第一波
        auto delay = DelayTime::create(1.0f);
        auto startWave = CallFunc::create([this]() {
            _waveManager->startWave(1);
        });
        this->runAction(Sequence::create(delay, startWave, nullptr));
    }
}

void HelloWorld::initInput()
{
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void HelloWorld::initUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // HP标签（左上角）
    _hpLabel = Label::createWithSystemFont("HP: 100/100", "Arial", 18);
    _hpLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 30));
    _hpLabel->setColor(Color3B::RED);
    this->addChild(_hpLabel, 100);

    // 波次标签（顶部中央）
    _waveLabel = Label::createWithSystemFont("Wave: 1", "Arial", 24);
    _waveLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 30));
    _waveLabel->setColor(Color3B::YELLOW);
    this->addChild(_waveLabel, 100);

    // 击杀数标签（右上角）
    _killLabel = Label::createWithSystemFont("Kills: 0", "Arial", 18);
    _killLabel->setPosition(Vec2(origin.x + visibleSize.width - 80,
        origin.y + visibleSize.height - 30));
    _killLabel->setColor(Color3B::GREEN);
    this->addChild(_killLabel, 100);

    // 经验值标签
    _expLabel = Label::createWithSystemFont("EXP: 0/100", "Arial", 16);
    _expLabel->setPosition(Vec2(origin.x + 70, origin.y + visibleSize.height - 55));
    _expLabel->setColor(Color3B::WHITE);
    this->addChild(_expLabel, 100);

    // 等级标签
    _levelLabel = Label::createWithSystemFont("LV: 1", "Arial", 16);
    _levelLabel->setPosition(Vec2(origin.x + 35, origin.y + visibleSize.height - 55));
    _levelLabel->setColor(Color3B::YELLOW);
    this->addChild(_levelLabel, 100);

    // 操作提示（底部）
    auto hintLabel = Label::createWithSystemFont(
        "WASD: Move | Arrow Keys: Shoot | ESC: Quit",
        "Arial", 14);
    hintLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
    hintLabel->setColor(Color3B::GRAY);
    this->addChild(hintLabel, 100);
}

void HelloWorld::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (_gameOver || _gameWin) return;

    switch (keyCode)
    {
        // WASD 移动
    case EventKeyboard::KeyCode::KEY_W:
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
        _keyUp = true;
        break;
    case EventKeyboard::KeyCode::KEY_S:
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        _keyDown = true;
        break;
    case EventKeyboard::KeyCode::KEY_A:
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        _keyLeft = true;
        break;
    case EventKeyboard::KeyCode::KEY_D:
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        _keyRight = true;
        break;

        // IJKL 射击方向（独立于移动）
    case EventKeyboard::KeyCode::KEY_I:
        _keyShootUp = true;
        break;
    case EventKeyboard::KeyCode::KEY_K:
        _keyShootDown = true;
        break;
    case EventKeyboard::KeyCode::KEY_J:
        _keyShootLeft = true;
        break;
    case EventKeyboard::KeyCode::KEY_L:
        _keyShootRight = true;
        break;

    case EventKeyboard::KeyCode::KEY_ESCAPE:
        menuCloseCallback(nullptr);
        break;

    default:
        break;
    }
}

void HelloWorld::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    switch (keyCode)
    {
    case EventKeyboard::KeyCode::KEY_W:
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
        _keyUp = false;
        break;
    case EventKeyboard::KeyCode::KEY_S:
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        _keyDown = false;
        break;
    case EventKeyboard::KeyCode::KEY_A:
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        _keyLeft = false;
        break;
    case EventKeyboard::KeyCode::KEY_D:
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        _keyRight = false;
        break;

    case EventKeyboard::KeyCode::KEY_I:
        _keyShootUp = false;
        break;
    case EventKeyboard::KeyCode::KEY_K:
        _keyShootDown = false;
        break;
    case EventKeyboard::KeyCode::KEY_J:
        _keyShootLeft = false;
        break;
    case EventKeyboard::KeyCode::KEY_L:
        _keyShootRight = false;
        break;

    default:
        break;
    }
}

void HelloWorld::fireBullet(const Vec2& direction)
{
    if (_gameOver || _gameWin) return;
    if (_player == nullptr || !_player->isRoleAlive()) return;

    // 创建子弹
    // 参数：名称, 图片路径, 起始位置, 方向, 速度, 伤害, 存活时间, 是否穿透
    auto bullet = Bullet::createBullet(
        "Bullet",
        "bullet.png",                          // 子弹图片
        _player->getPosition(),                // 从玩家位置发射
        direction,                             // 飞行方向
        500.0f,                                // 速度
        15,                                    // 伤害
        1.2f,                                  // 存活时间（秒）= 射程/速度
        false                                  // 非穿透
    );

    if (bullet)
    {
        this->addChild(bullet, 5);
        _bullets.push_back(bullet);
    }
}

void HelloWorld::update(float dt)
{
    if (_gameOver || _gameWin)
    {
        return;
    }

    // === 1. 处理玩家移动输入 ===
    if (_player && _player->isRoleAlive())
    {
        Vec2 inputDir = Vec2::ZERO;

        if (_keyUp)    inputDir.y += 1;
        if (_keyDown)  inputDir.y -= 1;
        if (_keyLeft)  inputDir.x -= 1;
        if (_keyRight) inputDir.x += 1;

        _player->setInputDirection(inputDir);
        _player->updatePlayer(dt);

        // 限制玩家在屏幕内
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto playerPos = _player->getPosition();
        float halfW = 15;  // 半个玩家宽度
        float halfH = 15;

        if (playerPos.x < halfW) playerPos.x = halfW;
        if (playerPos.x > visibleSize.width - halfW) playerPos.x = visibleSize.width - halfW;
        if (playerPos.y < halfH) playerPos.y = halfH;
        if (playerPos.y > visibleSize.height - halfH) playerPos.y = visibleSize.height - halfH;

        _player->setPosition(playerPos);
    }

    // === 2. 处理射击 ===
    _fireCooldown -= dt;
    if (_fireCooldown <= 0.0f)
    {
        Vec2 shootDir = Vec2::ZERO;

        if (_keyShootUp)    shootDir.y += 1;
        if (_keyShootDown)  shootDir.y -= 1;
        if (_keyShootLeft)  shootDir.x -= 1;
        if (_keyShootRight) shootDir.x += 1;

        // 如果没有按射击键但有移动方向，自动向移动方向射击
        if (shootDir == Vec2::ZERO)
        {
            Vec2 autoDir = _player->getDirection();
            if (autoDir != Vec2::ZERO)
            {
                shootDir = autoDir;
            }
        }

        if (shootDir != Vec2::ZERO)
        {
            shootDir.normalize();
            fireBullet(shootDir);
            _fireCooldown = _fireCooldownMax;
        }
    }

    // === 3. 更新波次管理器 ===
    if (_waveManager)
    {
        _waveManager->update(dt);
    }

    // === 4. 更新所有子弹 ===
    for (auto it = _bullets.begin(); it != _bullets.end(); )
    {
        Bullet* bullet = *it;

        if (bullet == nullptr)
        {
            it = _bullets.erase(it);
            continue;
        }

        if (bullet->isExpired() || !bullet->isObjectActive())
        {
            bullet->removeFromParent();
            it = _bullets.erase(it);
            continue;
        }

        bullet->updateObject(dt);
        ++it;
    }

    // === 5. 碰撞检测：子弹 vs 敌人 ===
    if (_waveManager)
    {
        auto& enemies = _waveManager->getAliveEnemies();
        CollisionManager::checkBulletEnemyCollision(_bullets, enemies);
    }

    // === 6. 清理失效子弹 ===
    CollisionManager::clearInactiveBullets(_bullets);

    // === 7. 检查玩家死亡 ===
    if (_player && !_player->isRoleAlive())
    {
        gameOver();
    }

    // === 8. 更新UI ===
    if (_player)
    {
        int currentHp = _player->getHp();
        int maxHp = _player->getMaxHp();
        _hpLabel->setString("HP: " + std::to_string(currentHp) + "/" + std::to_string(maxHp));

        _expLabel->setString("EXP: " + std::to_string(_player->getExp()) + "/" +
            std::to_string(_player->getLevel() * 100));

        _levelLabel->setString("LV: " + std::to_string(_player->getLevel()));
    }

    if (_waveManager)
    {
        int currentWave = _waveManager->getCurrentWave();
        int totalWaves = _waveManager->getTotalWaves();
        std::string waveStr = "Wave: " + std::to_string(currentWave) + "/" + std::to_string(totalWaves);

        if (_waveManager->isBossWave())
        {
            waveStr += " [BOSS]";
        }

        _waveLabel->setString(waveStr);

        _killLabel->setString("Kills: " + std::to_string(_waveManager->getKillCount()));
    }
}

void HelloWorld::gameOver()
{
    if (_gameOver) return;
    _gameOver = true;

    CCLOG("=== GAME OVER ===");

    if (_waveManager)
    {
        _waveManager->stopSpawn();
    }

    // 显示Game Over文字
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto gameOverLabel = Label::createWithSystemFont("GAME OVER", "Arial", 60);
    gameOverLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    gameOverLabel->setColor(Color3B::RED);
    this->addChild(gameOverLabel, 200);

    // 显示提示
    auto hintLabel = Label::createWithSystemFont("Press ESC to quit", "Arial", 20);
    hintLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 50));
    hintLabel->setColor(Color3B::WHITE);
    this->addChild(hintLabel, 200);
}

void HelloWorld::victory()
{
    if (_gameWin || _gameOver) return;
    _gameWin = true;

    CCLOG("=== VICTORY! ALL WAVES CLEARED! ===");

    // 显示Victory文字
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto victoryLabel = Label::createWithSystemFont("VICTORY!", "Arial", 60);
    victoryLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    victoryLabel->setColor(Color3B::GREEN);
    this->addChild(victoryLabel, 200);

    // 显示击杀统计
    if (_waveManager)
    {
        auto statsLabel = Label::createWithSystemFont(
            "Total Kills: " + std::to_string(_waveManager->getKillCount()),
            "Arial", 24);
        statsLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 50));
        statsLabel->setColor(Color3B::WHITE);
        this->addChild(statsLabel, 200);
    }

    auto hintLabel = Label::createWithSystemFont("Press ESC to quit", "Arial", 20);
    hintLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 90));
    hintLabel->setColor(Color3B::WHITE);
    this->addChild(hintLabel, 200);
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application,
    do not use Director::getInstance()->end() as given above,instead trigger a custom event
    created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}
