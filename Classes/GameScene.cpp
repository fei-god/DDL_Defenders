#include "GameScene.h"
#include "Weapons/KeyboardWave.h"
#include "Managers/CollisionManager.h"
#include "platform/CCImage.h"
#include "renderer/CCTextureCache.h"
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

    // --- Background ---
    auto bg = LayerColor::create(Color4B(22, 28, 40, 255), visibleSize.width, visibleSize.height);
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
        auto nameTag = Label::createWithSystemFont("You", "Arial", 14);
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
    float hpBarWidth  = 220.0f;
    float hpBarHeight = 22.0f;
    float marginX = 20.0f;
    float marginY = 20.0f;

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

    auto hpLabel = Label::createWithSystemFont("HP", "Arial", 18);
    hpLabel->setColor(Color3B(200, 200, 210));
    hpLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
    hpLabel->setPosition(Vec2(hpBarLeft - 8, hpBarTop - hpBarHeight / 2));
    this->addChild(hpLabel, 10);

    // --- Mood label ---
    _moodLabel = Label::createWithSystemFont("Mood: Normal", "Arial", 24);
    _moodLabel->setColor(Color3B(210, 210, 220));
    _moodLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _moodLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 28));
    this->addChild(_moodLabel, 10);

    // --- Survival time ---
    m_survivalTime = 0.0f;
    _survivalTimeLabel = Label::createWithSystemFont("Time: 0.0s", "Arial", 36);
    _survivalTimeLabel->setColor(Color3B(230, 230, 240));
    _survivalTimeLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 30.0f
    ));
    this->addChild(_survivalTimeLabel, 10);

    // --- Input ---
    _keyW = _keyA = _keyS = _keyD = false;
    _moveDirection = Vec2::ZERO;
    initInputListeners();

    this->scheduleUpdate();
    return true;
}

// ---------------------------------------------------------------------------
// Main game loop
// ---------------------------------------------------------------------------
void GameScene::update(float dt)
{
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
    // ---- Keyboard (WASD) ----
    auto* kbListener = EventListenerKeyboard::create();
    kbListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*)
    {
        switch (code)
        {
        case EventKeyboard::KeyCode::KEY_W: _keyW = true; break;
        case EventKeyboard::KeyCode::KEY_A: _keyA = true; break;
        case EventKeyboard::KeyCode::KEY_S: _keyS = true; break;
        case EventKeyboard::KeyCode::KEY_D: _keyD = true; break;
        default: break;
        }
        updateMoveDirection();
    };
    kbListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*)
    {
        switch (code)
        {
        case EventKeyboard::KeyCode::KEY_W: _keyW = false; break;
        case EventKeyboard::KeyCode::KEY_A: _keyA = false; break;
        case EventKeyboard::KeyCode::KEY_S: _keyS = false; break;
        case EventKeyboard::KeyCode::KEY_D: _keyD = false; break;
        default: break;
        }
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

    // Spawn from triangle head (20 px ahead)
    float headDist = 20.0f;
    Vec2 spawnPos = playerPos + dir * headDist;

    auto* bullet = Bullet::createBullet(
        "ManualBullet", "",
        spawnPos, dir,
        650.0f,   // speed
        10,       // damage
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
