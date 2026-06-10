#include "GameScene.h"
#include "GameOverScene.h"
#include "MainMenuScene.h"
#include "SettingsScene.h"
#include "Weapons/KeyboardWave.h"
#include "Managers/CollisionManager.h"
#include "Managers/LanguageManager.h"
#include "platform/CCImage.h"
#include "renderer/CCTextureCache.h"
#include "base/CCUserDefault.h"
#include <new>
#include <cmath>

USING_NS_CC;

// ---------------------------------------------------------------------------
// Factory / Destructor
// ---------------------------------------------------------------------------
Scene* GameScene::createScene()
{
    return GameScene::create();
}

GameScene::~GameScene()
{
    CC_SAFE_RELEASE(_waveManager);
}

// ---------------------------------------------------------------------------
// Coordinate conversion: raw window-pixel coords → game-world coords
// View:  (0,0) top-left,  Y increases downward
// Game:  (0,0) bottom-left, Y increases upward
// ---------------------------------------------------------------------------
Vec2 GameScene::viewToGameCoords(const Vec2& viewPos) const
{
    auto fsize = Director::getInstance()->getOpenGLView()->getFrameSize();

    // Flip Y: gameY = frameHeight - viewY  (no scaling, 1:1 with frame)
    return Vec2(viewPos.x, fsize.height - viewPos.y);
}

// ---------------------------------------------------------------------------
// Placeholder textures — write real PNG files to disk so FileUtils can find them
// ---------------------------------------------------------------------------
void GameScene::createPlaceholderTextures()
{
    auto fileUtils = FileUtils::getInstance();
    std::string writablePath = fileUtils->getWritablePath();

    auto createAndSavePNG = [writablePath](
        const std::string& filename,
        unsigned char r, unsigned char g, unsigned char b,
        int size
    ) {
        int dataLen = size * size * 4;
        auto* pixels = new (std::nothrow) unsigned char[dataLen];
        if (!pixels) return;

        for (int i = 0; i < size * size; ++i)
        {
            pixels[i * 4 + 0] = r;
            pixels[i * 4 + 1] = g;
            pixels[i * 4 + 2] = b;
            pixels[i * 4 + 3] = 255;
        }

        auto* image = new (std::nothrow) Image();
        if (image && image->initWithRawData(pixels, dataLen, size, size, 8))
        {
            std::string savePath = writablePath + filename;
            image->saveToFile(savePath, false);
        }
        CC_SAFE_RELEASE(image);
        delete[] pixels;
    };

    createAndSavePNG("enemy_ddl.png",    255,  40,  40, 32);
    createAndSavePNG("enemy_sleepy.png",  50, 200,  50, 32);
    fileUtils->addSearchPath(writablePath);
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------
bool GameScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor — keeps in-game UI proportional across resolutions
    float s = winSize.height / 640.0f;

    // --- Background (covers full design resolution to avoid gaps) ---
    auto bg = LayerColor::create(Color4B(22, 28, 40, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -10);

    // --- Placeholder textures ---
    createPlaceholderTextures();

    // --- Player ---
    Vec2 playerStart(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    m_player = Player::create("Hero", "",
        playerStart, 100, 150.0f, 5);

    if (m_player)
    {
        this->addChild(m_player, 5);

        // Triangle visual — points right (+X) in local space, rotates toward mouse
        _playerVisual = DrawNode::create();
        Vec2 triVerts[3] = {
            Vec2( 18,   0),  // head  (right)
            Vec2(-10, -10),  // bottom
            Vec2(-10,  10)   // top
        };
        Color4F triFill(0.31f, 0.73f, 0.96f, 1.0f);   // bright sky-blue
        Color4F triBorder(1.0f, 1.0f, 1.0f, 0.6f);     // white outline
        _playerVisual->drawPolygon(triVerts, 3, triFill, 1.2f, triBorder);
        _playerVisual->setPosition(Vec2::ZERO);
        m_player->addChild(_playerVisual, -1);

        _playerDir = Vec2(1.0f, 0.0f); // default: facing right

        // Name tag
        auto nameTag = Label::createWithSystemFont("You", "Arial", 14.0f * s);
        nameTag->setColor(Color3B(200, 230, 255));
        nameTag->setPosition(Vec2(0, 22));
        m_player->addChild(nameTag);
    }

    // Initialize mouse position to player start (so triangle has a default direction)
    _mousePos = playerStart + Vec2(100, 0);

    // --- Bullet layer ---
    _bulletLayer = Node::create();
    this->addChild(_bulletLayer, 6);

    // --- Weapon ---
    _currentWeapon = KeyboardWave::create(m_player);
    if (_currentWeapon)
        this->addChild(_currentWeapon, 7);

    // --- WaveManager ---
    _waveManager = WaveManager::create(m_player, this);
    if (_waveManager)
        _waveManager->retain();

    if (_currentWeapon && _waveManager)
    {
        _currentWeapon->bindBattleData(
            &_waveManager->getAliveEnemies(),
            &_bullets,
            _bulletLayer
        );
    }

    if (_waveManager)
        _waveManager->startWave(1);

    // --- HP bar ---
    float hpBarWidth  = 220.0f * s;
    float hpBarHeight = 22.0f * s;
    float marginX = 20.0f * s;
    float marginY = 20.0f * s;

    float hpBarLeft = origin.x + marginX;
    float hpBarTop  = origin.y + visibleSize.height - marginY;

    auto hpFrame = LayerColor::create(Color4B(100, 100, 110, 255), hpBarWidth + 4, hpBarHeight + 4);
    hpFrame->setPosition(Vec2(hpBarLeft - 2, hpBarTop - hpBarHeight - 2));
    this->addChild(hpFrame, 9);

    _hpBarBg = LayerColor::create(Color4B(40, 40, 50, 255), hpBarWidth, hpBarHeight);
    _hpBarBg->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight));
    this->addChild(_hpBarBg, 9);

    _hpBarFill = LayerColor::create(Color4B(50, 210, 50, 255), hpBarWidth, hpBarHeight);
    _hpBarFill->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight));
    this->addChild(_hpBarFill, 10);
    _hpBarMaxWidth = hpBarWidth;

    auto hpLabel = Label::createWithSystemFont("HP", "Arial", 18.0f * s);
    hpLabel->setColor(Color3B(200, 200, 210));
    hpLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
    hpLabel->setPosition(Vec2(hpBarLeft - 8.0f * s, hpBarTop - hpBarHeight / 2));
    this->addChild(hpLabel, 10);

    // --- Mood label ---
    _moodLabel = Label::createWithSystemFont("Mood: Normal", "Arial", 24.0f * s);
    _moodLabel->setColor(Color3B(210, 210, 220));
    _moodLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _moodLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 28.0f * s));
    this->addChild(_moodLabel, 10);

    // --- Survival time ---
    m_survivalTime = 0.0f;
    _survivalTimeLabel = Label::createWithSystemFont("Time: 0.0s", "Arial", 36.0f * s);
    _survivalTimeLabel->setColor(Color3B(230, 230, 240));
    _survivalTimeLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 30.0f * s
    ));
    this->addChild(_survivalTimeLabel, 10);

    // --- Input ---
    _keyW = _keyA = _keyS = _keyD = false;
    _moveDirection = Vec2::ZERO;
    _isGameOver = false;
    _isPaused = false;
    _pauseLayer = nullptr;
    loadKeyBindings();
    initInputListeners();

    this->scheduleUpdate();
    return true;
}

// ---------------------------------------------------------------------------
// Main game loop
// ---------------------------------------------------------------------------
void GameScene::update(float dt)
{
    // --- Death check ---
    if (!_isGameOver && m_player && !m_player->isRoleAlive())
    {
        _isGameOver = true;
        // Use a delay then create the GameOverScene lazily *inside* the callback,
        // so there is no autoreleased-scene-dangling-pointer issue.
        auto delay = DelayTime::create(0.5f);
        auto call = CallFunc::create([this]() {
            auto gameOverScene = GameOverScene::createScene(m_survivalTime);
            Director::getInstance()->replaceScene(gameOverScene);
        });
        this->runAction(Sequence::create(delay, call, nullptr));
        return;
    }

    // Don't process any game logic after death (waiting for transition)
    if (_isGameOver) return;

    // Don't process game logic while paused
    if (_isPaused) return;

    // --- Player ---
    if (m_player)
    {
        // Rotate triangle toward mouse
        if (_playerVisual)
        {
            Vec2 playerPos = m_player->getObjectPosition();
            Vec2 dir = _mousePos - playerPos;
            if (dir.lengthSquared() > 0.5f)
            {
                _playerDir = dir.getNormalized();
                float angle = std::atan2(_playerDir.y, _playerDir.x);
                _playerVisual->setRotation(CC_RADIANS_TO_DEGREES(angle));
            }
        }

        m_player->updatePlayer(dt);
    }

    // --- Weapon (kept as reference for future weapon-switching, but NO auto-fire) ---
    // Auto-fire disabled: player uses mouse clicks instead.
    // If you re-enable, call: _currentWeapon->updateObject(dt);

    // --- Bullets ---
    for (auto* bullet : _bullets)
    {
        if (bullet)
            bullet->updateObject(dt);
    }

    // --- WaveManager ---
    if (_waveManager)
    {
        _waveManager->update(dt);
    }

    // --- Collision ---
    if (_waveManager)
    {
        CollisionManager::checkBulletEnemyCollision(
            _bullets,
            _waveManager->getAliveEnemies()
        );
    }

    // --- Cleanup ---
    CollisionManager::clearInactiveBullets(_bullets);

    // --- Enemy HP bars ---
    if (_waveManager)
    {
        static const int HP_BAR_TAG = 999;
        for (auto* enemy : _waveManager->getAliveEnemies())
        {
            if (!enemy || !enemy->isRoleAlive() || !enemy->isObjectActive())
                continue;

            // Find or create HP bar background + fill
            Node* barNode = enemy->getChildByTag(HP_BAR_TAG);
            LayerColor* hpFill = dynamic_cast<LayerColor*>(barNode);
            if (!hpFill)
            {
                // Background (dark)
                auto bg = LayerColor::create(Color4B(40, 40, 40, 255), 32, 4);
                bg->setPosition(Vec2(-16, 18));
                enemy->addChild(bg, 1);

                // Fill (green)
                hpFill = LayerColor::create(Color4B(50, 200, 50, 255), 32, 4);
                hpFill->setPosition(Vec2(-16, 18));
                hpFill->setTag(HP_BAR_TAG);
                enemy->addChild(hpFill, 2);
            }

            // Update fill width from current HP ratio
            float hpRatio = enemy->getMaxHp() > 0
                ? static_cast<float>(enemy->getHp()) / static_cast<float>(enemy->getMaxHp())
                : 0.0f;
            if (hpRatio < 0.0f) hpRatio = 0.0f;
            if (hpRatio > 1.0f) hpRatio = 1.0f;

            float barWidth = 32.0f * hpRatio;
            hpFill->setContentSize(Size(barWidth, 4));

            Color3B barColor;
            if      (hpRatio > 0.5f)  barColor = Color3B(50, 200, 50);
            else if (hpRatio > 0.25f) barColor = Color3B(220, 220, 30);
            else                       barColor = Color3B(220, 30, 30);
            hpFill->setColor(barColor);
        }
    }

    // --- UI ---
    if (m_player)
    {
        updateUI(m_player);
    }
    updateSurvivalTime(dt);
}

// ---------------------------------------------------------------------------
// UI helpers
// ---------------------------------------------------------------------------
void GameScene::updateUI(Player* player)
{
    if (!player) return;

    if (_hpBarFill)
    {
        float hpRatio = player->getMaxHp() > 0
            ? static_cast<float>(player->getHp()) / static_cast<float>(player->getMaxHp())
            : 0.0f;
        if (hpRatio < 0.0f) hpRatio = 0.0f;
        if (hpRatio > 1.0f) hpRatio = 1.0f;

        float newWidth = _hpBarMaxWidth * hpRatio;
        _hpBarFill->setContentSize(Size(newWidth, _hpBarFill->getContentSize().height));

        Color3B barColor;
        if      (hpRatio > 0.5f)  barColor = Color3B(50, 210, 50);
        else if (hpRatio > 0.25f) barColor = Color3B(230, 210, 30);
        else                       barColor = Color3B(230, 40, 40);

        _hpBarFill->setColor(barColor);
    }

    if (_moodLabel)
    {
        _moodLabel->setString("Mood: " + player->getCurrentMoodName());
    }
}

void GameScene::updateSurvivalTime(float dt)
{
    m_survivalTime += dt;
    if (_survivalTimeLabel)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Time: %.1fs", m_survivalTime);
        _survivalTimeLabel->setString(buf);
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void GameScene::initInputListeners()
{
    // ---- Keyboard (movement + pause) ----
    auto* kbListener = EventListenerKeyboard::create();
    kbListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*)
    {
        // ESC toggles pause (only if game is still active)
        if (code == EventKeyboard::KeyCode::KEY_ESCAPE)
        {
            if (!_isGameOver)
            {
                if (_isPaused) hidePauseMenu();
                else           showPauseMenu();
            }
            return;
        }

        // Ignore movement keys while paused
        if (_isPaused) return;

        if (code == _keyMoveUp)    _keyW = true;
        if (code == _keyMoveDown)  _keyS = true;
        if (code == _keyMoveLeft)  _keyA = true;
        if (code == _keyMoveRight) _keyD = true;
        updateMoveDirection();
    };
    kbListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*)
    {
        if (_isPaused) return;

        if (code == _keyMoveUp)    _keyW = false;
        if (code == _keyMoveDown)  _keyS = false;
        if (code == _keyMoveLeft)  _keyA = false;
        if (code == _keyMoveRight) _keyD = false;
        updateMoveDirection();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, this);

    // ---- Mouse ----
    auto* mouseListener = EventListenerMouse::create();

    // Track cursor every frame so triangle can follow
    mouseListener->onMouseMove = [this](EventMouse* mouseEv)
    {
        _mousePos = viewToGameCoords(mouseEv->getLocationInView());
    };

    // Left-click = fire in the direction the triangle is facing
    mouseListener->onMouseDown = [this](EventMouse* mouseEv)
    {
        if (mouseEv->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
            return;
        if (_isPaused || _isGameOver)
            return;
        if (!m_player || !m_player->isRoleAlive())
            return;

        fireBullet();
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void GameScene::updateMoveDirection()
{
    _moveDirection = Vec2::ZERO;
    if (_keyW) _moveDirection.y += 1.0f;
    if (_keyS) _moveDirection.y -= 1.0f;
    if (_keyD) _moveDirection.x += 1.0f;
    if (_keyA) _moveDirection.x -= 1.0f;

    if (m_player)
        m_player->setInputDirection(_moveDirection);
}

void GameScene::fireBullet()
{
    if (!m_player) return;

    Vec2 playerPos = m_player->getObjectPosition();

    // Direction = same vector the triangle head is pointing
    Vec2 dir = _playerDir;
    // Negate Y to compensate for screen-to-world coordinate flip
    dir.y = -dir.y;

    // Spawn from triangle head (20 px ahead)
    float headDist = 20.0f;
    Vec2 spawnPos = playerPos + dir * headDist;

    auto* bullet = Bullet::createBullet(
        "ManualBullet", "",
        spawnPos, dir,
        650.0f,   // speed
        35,       // damage  <- increased from 10 to 35
        1.0f,     // lifetime
        false     // not piercing
    );

    if (bullet)
    {
        // Give it a collision box (textureless Sprite has (0,0) content size)
        bullet->setContentSize(Size(8, 8));

        // Make it visible: bright yellow 8×8 square
        auto dot = LayerColor::create(Color4B(255, 230, 60, 255), 8, 8);
        dot->setPosition(Vec2(-4, -4));
        bullet->addChild(dot);

        _bulletLayer->addChild(bullet);
        _bullets.push_back(bullet);
    }
}

// ---------------------------------------------------------------------------
// Key bindings (loaded from UserDefault — see SettingsScene for rebinding)
// ---------------------------------------------------------------------------
void GameScene::loadKeyBindings()
{
    auto ud = UserDefault::getInstance();
    _keyMoveUp    = static_cast<EventKeyboard::KeyCode>(
        ud->getIntegerForKey("key_move_up",    static_cast<int>(EventKeyboard::KeyCode::KEY_W)));
    _keyMoveDown  = static_cast<EventKeyboard::KeyCode>(
        ud->getIntegerForKey("key_move_down",  static_cast<int>(EventKeyboard::KeyCode::KEY_S)));
    _keyMoveLeft  = static_cast<EventKeyboard::KeyCode>(
        ud->getIntegerForKey("key_move_left",  static_cast<int>(EventKeyboard::KeyCode::KEY_A)));
    _keyMoveRight = static_cast<EventKeyboard::KeyCode>(
        ud->getIntegerForKey("key_move_right", static_cast<int>(EventKeyboard::KeyCode::KEY_D)));
}

// ---------------------------------------------------------------------------
// Pause menu
// ---------------------------------------------------------------------------
void GameScene::showPauseMenu()
{
    if (_isPaused || _pauseLayer) return;
    _isPaused = true;

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;
    float cx = origin.x + visibleSize.width / 2;
    float cy = origin.y + visibleSize.height / 2;

    // Semi-transparent overlay (covers full design resolution to avoid gaps)
    _pauseLayer = LayerColor::create(Color4B(0, 0, 0, 160), winSize.width, winSize.height);
    _pauseLayer->setPosition(Vec2::ZERO);
    this->addChild(_pauseLayer, 100);

    // Pause title
    auto title = Label::createWithSystemFont(lm->getString("pause_title"), "Arial", 48.0f * s);
    title->setColor(Color3B(220, 220, 240));
    title->setPosition(Vec2(cx, cy + 120.0f * s));
    _pauseLayer->addChild(title);

    // Resume button
    auto resumeLabel = Label::createWithSystemFont(lm->getString("resume"), "Arial", 32.0f * s);
    resumeLabel->setColor(Color3B(100, 220, 100));
    auto resumeItem = MenuItemLabel::create(resumeLabel,
        CC_CALLBACK_1(GameScene::onPauseResumeClicked, this));

    // Restart button
    auto restartLabel = Label::createWithSystemFont(lm->getString("restart"), "Arial", 32.0f * s);
    restartLabel->setColor(Color3B(220, 200, 100));
    auto restartItem = MenuItemLabel::create(restartLabel,
        CC_CALLBACK_1(GameScene::onPauseRestartClicked, this));

    // Settings button
    auto settingsLabel = Label::createWithSystemFont(lm->getString("settings"), "Arial", 32.0f * s);
    settingsLabel->setColor(Color3B(160, 180, 220));
    auto settingsItem = MenuItemLabel::create(settingsLabel,
        CC_CALLBACK_1(GameScene::onPauseSettingsClicked, this));

    // Back to title button
    auto titleLabel = Label::createWithSystemFont(lm->getString("back_to_title"), "Arial", 32.0f * s);
    titleLabel->setColor(Color3B(200, 140, 120));
    auto titleItem = MenuItemLabel::create(titleLabel,
        CC_CALLBACK_1(GameScene::onPauseTitleClicked, this));

    if (resumeItem && restartItem && settingsItem && titleItem)
    {
        auto menu = Menu::create(resumeItem, restartItem, settingsItem, titleItem, nullptr);
        menu->setPosition(Vec2(cx, cy));
        menu->alignItemsVerticallyWithPadding(28.0f * s);
        _pauseLayer->addChild(menu);
    }
}

void GameScene::hidePauseMenu()
{
    if (!_isPaused) return;
    _isPaused = false;

    if (_pauseLayer)
    {
        _pauseLayer->removeFromParent();
        _pauseLayer = nullptr;
    }
}

void GameScene::onPauseResumeClicked(Ref*)
{
    hidePauseMenu();
}

void GameScene::onPauseRestartClicked(Ref*)
{
    Director::getInstance()->replaceScene(GameScene::createScene());
}

void GameScene::onPauseSettingsClicked(Ref*)
{
    auto settingsScene = SettingsScene::createScene(SettingsScene::Entry::PAUSE_MENU);
    Director::getInstance()->pushScene(settingsScene);
}

void GameScene::onPauseTitleClicked(Ref*)
{
    Director::getInstance()->replaceScene(MainMenuScene::createScene());
}
