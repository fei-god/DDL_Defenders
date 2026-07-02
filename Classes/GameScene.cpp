#include "GameScene.h"
#include "GameOverScene.h"
#include "MainMenuScene.h"
#include "SettingsScene.h"
#include "StoryModeScene.h"
#include "VictoryScene.h"
#include "Weapons/CoffeeGun.h"
#include "Weapons/CoffeeLaser.h"
#include "Weapons/KeyboardWave.h"
#include "Weapons/KeyboardWeap.h"
#include "Weapons/DeskLampLaser.h"
#include "Weapons/CoffeeBlast.h"
#include "Managers/AudioManager.h"
#include "Core/AssetPaths.h"
#include "Managers/CollisionManager.h"
#include "Managers/LanguageManager.h"
#include "platform/CCImage.h"
#include "renderer/CCTextureCache.h"
#include "base/CCUserDefault.h"
#include <new>
#include <cmath>
#include <algorithm>
#include <string>

USING_NS_CC;

namespace
{
    std::string localizedButtonImagePath(const std::string& imagePath)
    {
        if (LanguageManager::getInstance()->getLanguage() !=
            LanguageManager::Language::ENGLISH)
        {
            return imagePath;
        }

        const std::string::size_type dot = imagePath.find_last_of('.');
        const std::string englishPath = dot == std::string::npos
            ? imagePath + "_eng"
            : imagePath.substr(0, dot) + "_eng" + imagePath.substr(dot);

        return AssetPaths::exists(englishPath) ? englishPath : imagePath;
    }

    Node* createUiImageOrLabel(const std::string& imagePath,
        const std::string& fallbackText,
        const Size& targetSize,
        float fontSize,
        const Color3B& color)
    {
        auto root = Node::create();
        root->setContentSize(targetSize);

        std::string resolved = AssetPaths::resolve(imagePath);
        if (!resolved.empty())
        {
            auto sprite = Sprite::create(resolved);
            if (sprite)
            {
                Size imageSize = sprite->getContentSize();
                if (imageSize.width > 0.0f && imageSize.height > 0.0f)
                {
                    if (imagePath.find("pause_save") != std::string::npos)
                    {
                        // Normalize the newly added save artwork to the visible
                        // size of the other pause-menu image buttons.
                        float visualAspect = 3.0f;
                        std::string referencePath = AssetPaths::resolve("art/ui/pause_resume.png");
                        auto reference = referencePath.empty() ? nullptr : Sprite::create(referencePath);
                        if (reference && reference->getContentSize().height > 0.0f)
                        {
                            visualAspect = reference->getContentSize().width /
                                reference->getContentSize().height;
                        }
                        const float visualWidth = std::min(targetSize.width,
                            targetSize.height * visualAspect);
                        sprite->setScaleX(visualWidth / imageSize.width);
                        sprite->setScaleY(targetSize.height * 1.30f / imageSize.height);
                    }
                    else
                    {
                        sprite->setScale(std::min(targetSize.width / imageSize.width,
                            targetSize.height / imageSize.height));
                    }
                }
                sprite->setPosition(Vec2(targetSize.width * 0.5f, targetSize.height * 0.5f));
                root->addChild(sprite);
                return root;
            }
        }

        auto label = Label::createWithSystemFont(fallbackText, "Arial", fontSize);
        if (label)
        {
            label->setColor(color);
            label->setPosition(Vec2(targetSize.width * 0.5f, targetSize.height * 0.5f));
            root->addChild(label);
        }
        return root;
    }

    MenuItemSprite* createUiImageButton(const std::string& imagePath,
        const std::string& fallbackText,
        const Size& targetSize,
        float fontSize,
        const Color3B& color,
        const ccMenuCallback& callback)
    {
        const std::string localizedPath = localizedButtonImagePath(imagePath);
        auto normal = createUiImageOrLabel(localizedPath, fallbackText, targetSize, fontSize, color);
        auto selected = createUiImageOrLabel(localizedPath, fallbackText, targetSize, fontSize, color);
        selected->setScale(0.96f);
        selected->setOpacity(220);
        return MenuItemSprite::create(normal, selected, callback);
    }

    bool isChineseUi()
    {
        return LanguageManager::getInstance()->getLanguage() ==
            LanguageManager::Language::SIMPLIFIED_CHINESE;
    }

    std::string textByLanguage(const std::string& english, const std::string& chinese)
    {
        return isChineseUi() ? chinese : english;
    }

    std::string moodNameForUi(MoodType mood)
    {
        if (!isChineseUi())
        {
            switch (mood)
            {
            case MoodType::Focus: return "Focus";
            case MoodType::Irritable: return "Irritable";
            case MoodType::Exhausted: return "Exhausted";
            case MoodType::Excited: return "Excited";
            case MoodType::Fear: return "Fear";
            case MoodType::Calm: return "Calm";
            case MoodType::Panic: return "Panic";
            case MoodType::Normal:
            default: return "Normal";
            }
        }

        switch (mood)
        {
        case MoodType::Focus: return u8"专注";
        case MoodType::Irritable: return u8"烦躁";
        case MoodType::Exhausted: return u8"疲惫";
        case MoodType::Excited: return u8"兴奋";
        case MoodType::Fear: return u8"恐惧";
        case MoodType::Calm: return u8"冷静";
        case MoodType::Panic: return u8"慌乱";
        case MoodType::Normal:
        default: return u8"普通";
        }
    }

    std::string weaponNameForUi(const std::string& weaponName)
    {
        if (!isChineseUi())
        {
            return weaponName;
        }

        if (weaponName == "CoffeeGun") return u8"咖啡枪";
        if (weaponName == "CoffeeLaser") return u8"咖啡激光";
        if (weaponName == "KeyboardWave") return u8"键盘冲击波";
        if (weaponName == "KeyboardWeap") return u8"键盘武器";
        if (weaponName == "DeskLampLaser") return u8"台灯激光";
        if (weaponName == "CoffeeBlast") return u8"咖啡爆破";
        return weaponName;
    }

    std::string defaultControlHint()
    {
        return textByLanguage(
            "WASD move  O attack  1/2/3/4 switch weapons  Equipment button can change loadout",
            u8"WASD移动  O攻击  1/2/3/4切换武器  点击装备按钮更换武器");
    }
}

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

Vec2 GameScene::screenToWorldCoords(const Vec2& screenPos) const
{
    if (!_worldLayer || _worldScale <= 0.001f)
    {
        return screenPos;
    }

    return (screenPos - _worldLayer->getPosition()) / _worldScale;
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
    createAndSavePNG("enemy_phone.png",  220,  80, 220, 32);
    createAndSavePNG("enemy_boss.png",   180,  60,  60, 48);
    fileUtils->addSearchPath(writablePath);
}

void GameScene::applySpriteFit(Sprite* sprite, float maxWidth, float maxHeight)
{
    if (!sprite) return;

    Size size = sprite->getContentSize();
    if (size.width <= 0.0f || size.height <= 0.0f)
    {
        sprite->setContentSize(Size(maxWidth, maxHeight));
        return;
    }

    float scale = std::min(maxWidth / size.width, maxHeight / size.height);
    if (scale > 0.0f)
    {
        sprite->setScale(scale);
    }
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------
bool GameScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->playGameBGM();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor — keeps in-game UI proportional across resolutions
    float s = winSize.height / 640.0f;

    _worldScale = 1.16f;
    _worldSize = Size(visibleSize.width * 1.45f, visibleSize.height * 1.45f);
    _worldLayer = Node::create();
    _worldLayer->setContentSize(_worldSize);
    _worldLayer->setScale(_worldScale);
    _cameraInitialized = false;
    this->addChild(_worldLayer, 0);

    // --- Background ---
    const std::string bgPath = AssetPaths::resolve("art/backgrounds/dorm_room.png");
    if (!bgPath.empty())
    {
        auto bgSprite = Sprite::create(bgPath);
        if (bgSprite)
        {
            bgSprite->setPosition(Vec2(_worldSize.width * 0.5f,
                _worldSize.height * 0.5f));
            Size bgSize = bgSprite->getContentSize();
            if (bgSize.width > 0.0f && bgSize.height > 0.0f)
            {
                bgSprite->setScale(std::max(_worldSize.width / bgSize.width,
                    _worldSize.height / bgSize.height));
            }
            _worldLayer->addChild(bgSprite, -20);
        }
    }
    else
    {
        auto bg = LayerColor::create(Color4B(22, 28, 40, 255), _worldSize.width, _worldSize.height);
        bg->setPosition(Vec2::ZERO);
        _worldLayer->addChild(bg, -20);
    }

    // --- Placeholder textures ---
    createPlaceholderTextures();

    // --- Player ---
    Vec2 playerStart(_worldSize.width / 2, _worldSize.height / 2);
    std::string playerImagePath = AssetPaths::resolve("art/characters/player.png");
    m_player = Player::create("Hero", playerImagePath,
        playerStart, 100, 240.0f, 5);

    if (m_player)
    {
        _worldLayer->addChild(m_player, 100);
        m_player->setLocalZOrder(100);
        applySpriteFit(m_player, 153.0f, 153.0f);

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
        m_player->addChild(_playerVisual, playerImagePath.empty() ? 1 : 2);

        _playerDir = Vec2(1.0f, 0.0f); // default: facing right

        // Name tag
        auto nameTag = Label::createWithSystemFont("You", "Arial", 14.0f * s);
        nameTag->setColor(Color3B(200, 230, 255));
        nameTag->setPosition(Vec2(0, 48));
        m_player->addChild(nameTag);
    }

    // Initialize mouse position to player start (so triangle has a default direction)
    _mousePos = playerStart + Vec2(100, 0);

    // --- Bullet layer ---
    _bulletLayer = Node::create();
    _bulletLayer->setContentSize(_worldSize);
    _worldLayer->addChild(_bulletLayer, 115);
    _bulletPool.init(_bulletLayer, 32);

    // --- Weapon ---
    _currentWeaponIndex = 0;
    _currentWeapon = nullptr;
    _nextEquipmentSlot = 0;
    _equipmentLayer = nullptr;
    _upgradeLayer = nullptr;
    _endlessStatsLabel = nullptr;
    _endlessScore = 0;
    _lastHandledPlayerLevel = 1;
    _lifeOnKill = 0;
    _weaponDamageBonus = 0;
    _projectileBonus = 0;
    _energyRecoveryBonusPercent = 0.0f;
    _masteredWeaponIds.clear();
    auto* ud = UserDefault::getInstance();
    for (int i = 0; i < 4; ++i)
    {
        _weaponLoadoutIds.push_back(ud->getIntegerForKey(("weapon_slot_" + std::to_string(i)).c_str(), i));
    }
    rebuildWeaponLoadout();

    _levelIntroLayer = nullptr;
    _levelIntroActive = false;
    _levelIntroTimer = 0.0f;
    initLevelTask();
    if (_isEndlessMode && _levelNumber > 1 && m_player)
    {
        m_player->setLevel(_levelNumber);
        _lastHandledPlayerLevel = m_player->getLevel();
    }

    // --- WaveManager ---
    _waveManager = WaveManager::create(m_player, _worldLayer);
    if (_waveManager)
    {
        _waveManager->retain();
        if (_isEndlessMode)
        {
            _waveManager->setTotalWaves(1000000);
        }
    }

    if (_currentWeapon && _waveManager)
    {
        for (auto* weapon : _weapons)
        {
            if (weapon)
            {
                weapon->bindBattleData(
                    &_waveManager->getAliveEnemies(),
                    &_bullets,
                    _bulletLayer
                );
                weapon->bindBulletPool(&_bulletPool);
            }
        }
    }

    if (_waveManager)
    {
        _waveManager->setEnemyKilledCallback([this](Enemy* enemy) {
            handleEndlessEnemyKilled(enemy);
            spawnRewardForEnemy(enemy);
        });
        int startingWave = _isEndlessMode ? 1 : ((_levelNumber + 1) / 2);
        if (startingWave < 1) startingWave = 1;
        _waveManager->startWave(startingWave);
    }

    // --- HP bar ---
    float hpBarWidth  = 220.0f * s;
    float hpBarHeight = 18.0f * s;
    float marginX = 18.0f * s;
    float marginY = 44.0f * s;

    float hpBarLeft = origin.x + marginX;
    float hpBarTop  = origin.y + visibleSize.height - marginY;

    _hudPanelBg = LayerColor::create(Color4B(11, 14, 22, 178), 352.0f * s, 184.0f * s);
    _hudPanelBg->setPosition(Vec2(hpBarLeft - 10.0f * s, hpBarTop - 176.0f * s));
    this->addChild(_hudPanelBg, 8);

    auto hudAccent = LayerColor::create(Color4B(88, 196, 255, 185), 4.0f * s, 184.0f * s);
    hudAccent->setPosition(_hudPanelBg->getPosition());
    this->addChild(hudAccent, 9);

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

    auto hpLabel = Label::createWithSystemFont(textByLanguage("HP", u8"生命"), "Arial", 18.0f * s);
    hpLabel->setColor(Color3B(200, 200, 210));
    hpLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
    hpLabel->setPosition(Vec2(hpBarLeft - 8.0f * s, hpBarTop - hpBarHeight / 2));
    this->addChild(hpLabel, 10);

    // --- Mood label ---
    _moodLabel = Label::createWithSystemFont(
        textByLanguage("Mood: Normal", u8"情绪: 普通"), "Arial", 18.0f * s);
    _moodLabel->setColor(Color3B(212, 224, 238));
    _moodLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _moodLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 24.0f * s));
    this->addChild(_moodLabel, 10);

    _weaponIcon = Sprite::create();
    _weaponIcon->setPosition(Vec2(hpBarLeft + 28.0f * s, hpBarTop - hpBarHeight - 55.0f * s));
    this->addChild(_weaponIcon, 10);

    _weaponLabel = Label::createWithSystemFont(
        textByLanguage("Weapon: CoffeeGun", u8"武器: 咖啡枪"), "Arial", 18.0f * s);
    _weaponLabel->setColor(Color3B(232, 238, 244));
    _weaponLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _weaponLabel->setPosition(Vec2(hpBarLeft + 64.0f * s, hpBarTop - hpBarHeight - 55.0f * s));
    this->addChild(_weaponLabel, 10);

    float weaponEnergyWidth = 112.0f * s;
    float weaponEnergyHeight = 8.0f * s;
    _weaponEnergyBg = LayerColor::create(Color4B(45, 45, 58, 255), weaponEnergyWidth, weaponEnergyHeight);
    _weaponEnergyBg->setPosition(Vec2(hpBarLeft + 220.0f * s,
        hpBarTop - hpBarHeight - 59.0f * s));
    this->addChild(_weaponEnergyBg, 9);

    _weaponEnergyFill = LayerColor::create(Color4B(90, 190, 255, 255), weaponEnergyWidth, weaponEnergyHeight);
    _weaponEnergyFill->setPosition(_weaponEnergyBg->getPosition());
    this->addChild(_weaponEnergyFill, 10);
    _weaponEnergyBarMaxWidth = weaponEnergyWidth;

    _progressLabel = Label::createWithSystemFont(
        textByLanguage("Assignment: 0%", u8"作业进度: 0%"), "Arial", 18.0f * s);
    _progressLabel->setColor(Color3B(246, 228, 137));
    _progressLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _progressLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 82.0f * s));
    this->addChild(_progressLabel, 10);

    _taskLabel = Label::createWithSystemFont(
        textByLanguage("Desk: 0.00 / 0.00s", u8"书桌: 0.00 / 0.00秒"), "Arial", 16.0f * s);
    _taskLabel->setColor(Color3B(172, 231, 255));
    _taskLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _taskLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 106.0f * s));
    this->addChild(_taskLabel, 10);

    _taskBarMaxWidth = 222.0f * s;
    float taskBarHeight = 8.0f * s;
    _taskBarBg = LayerColor::create(Color4B(45, 45, 58, 255), _taskBarMaxWidth, taskBarHeight);
    _taskBarBg->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 131.0f * s));
    this->addChild(_taskBarBg, 9);

    _taskBarFill = LayerColor::create(Color4B(80, 205, 235, 255), _taskBarMaxWidth, taskBarHeight);
    _taskBarFill->setPosition(_taskBarBg->getPosition());
    this->addChild(_taskBarFill, 10);

    _environmentLabel = Label::createWithSystemFont(
        textByLanguage("Environment: None", u8"环境: 无"), "Arial", 16.0f * s);
    _environmentLabel->setColor(Color3B(177, 221, 208));
    _environmentLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _environmentLabel->setPosition(Vec2(hpBarLeft, hpBarTop - hpBarHeight - 154.0f * s));
    this->addChild(_environmentLabel, 10);

    _weaponSlotNodes.clear();
    _lastWeaponSlotIds.clear();
    _lastWeaponSlotIndex = -1;
    Size slotSize(76.0f * s, 76.0f * s);
    float slotGap = 12.0f * s;
    float slotStartX = origin.x + visibleSize.width * 0.5f - (slotSize.width * 4.0f + slotGap * 3.0f) * 0.5f;
    float slotY = origin.y + 24.0f * s;
    for (int i = 0; i < 4; ++i)
    {
        auto slot = Node::create();
        slot->setContentSize(slotSize);
        slot->setPosition(Vec2(slotStartX + i * (slotSize.width + slotGap), slotY));
        this->addChild(slot, 22);
        _weaponSlotNodes.push_back(slot);
    }

    // --- Survival time ---
    m_survivalTime = 0.0f;
    _survivalTimeLabel = Label::createWithSystemFont(
        textByLanguage("Time: 0.0s", u8"时间: 0.0秒"), "Arial", 36.0f * s);
    _survivalTimeLabel->setColor(Color3B(230, 230, 240));
    _survivalTimeLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 30.0f * s
    ));
    this->addChild(_survivalTimeLabel, 10);

    _endlessStatsLabel = Label::createWithSystemFont("", "Arial", 18.0f * s);
    _endlessStatsLabel->setColor(Color3B(160, 232, 255));
    _endlessStatsLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 62.0f * s
    ));
    _endlessStatsLabel->setVisible(_isEndlessMode);
    this->addChild(_endlessStatsLabel, 10);

    _topHintLabel = Label::createWithSystemFont(
        defaultControlHint(),
        "Arial",
        16.0f * s);
    _topHintLabel->setColor(Color3B(255, 245, 180));
    _topHintLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + 86.0f * s));
    this->addChild(_topHintLabel, 20);

    auto equipLabel = Label::createWithSystemFont(
        textByLanguage("Equipment", u8"装备"), "Arial", 20.0f * s);
    equipLabel->setColor(Color3B(120, 225, 255));
    auto equipItem = MenuItemLabel::create(equipLabel, [this](Ref*) {
        AudioManager::getInstance()->playButtonClick();
        showEquipmentMenu();
    });
    auto equipMenu = Menu::create(equipItem, nullptr);
    equipMenu->setPosition(Vec2(origin.x + visibleSize.width - 92.0f * s,
        origin.y + visibleSize.height - 64.0f * s));
    this->addChild(equipMenu, 30);

    // --- Input ---
    _keyW = _keyA = _keyS = _keyD = false;
    _moveDirection = Vec2::ZERO;
    _isGameOver = false;
    _isVictory = false;
    _assignmentProgress = 0.0f;
    _nearDesk = false;
    _nearPowerSocket = false;
    _lastKillCount = 0;
    _enemyContactDamageCooldown = 0.0f;
    _lowHpMoodTimer = 0.0f;
    _freezeTimer = 0.0f;
    _rewardSpawnTimer = 9.0f;
    _currentPlayerMoodImage.clear();
    _lastUiLanguageIndex = LanguageManager::getInstance()->getLanguageIndex();
    _isPaused = false;
    _pauseLayer = nullptr;
    loadKeyBindings();
    initEnvironmentZones();
    initInputListeners();
    updateCamera();
    showLevelIntro();

    this->scheduleUpdate();
    return true;
}

// ---------------------------------------------------------------------------
// Main game loop
// ---------------------------------------------------------------------------
void GameScene::update(float dt)
{
    int currentLanguageIndex = LanguageManager::getInstance()->getLanguageIndex();
    if (currentLanguageIndex != _lastUiLanguageIndex)
    {
        _lastUiLanguageIndex = currentLanguageIndex;
        if (_topHintLabel)
        {
            _topHintLabel->setString(defaultControlHint());
        }

        // Settings is pushed over the pause screen. Rebuild the existing
        // pause menu when returning so localized button artwork changes
        // immediately with the selected language.
        if (_pauseLayer)
        {
            _pauseLayer->removeFromParentAndCleanup(true);
            _pauseLayer = nullptr;
            _isPaused = false;
            showPauseMenu();
        }
    }

    // --- Death check ---
    if (!_isGameOver && m_player && !m_player->isRoleAlive())
    {
        goToGameOver();
        return;
    }

    // Don't process any game logic after death (waiting for transition)
    if (_isGameOver) return;

    // Don't process game logic while paused
    if (_isPaused) return;

    if (_levelIntroActive)
    {
        _levelIntroTimer -= dt;
        if (_levelIntroTimer <= 0.0f)
        {
            hideLevelIntro();
        }
        updateUI(m_player);
        return;
    }

    // --- Player ---
    if (m_player)
    {
        m_player->setLocalZOrder(100);

        // Face the last movement direction. Attacks use this same direction.
        if (_playerVisual)
        {
            float angle = std::atan2(_playerDir.y, _playerDir.x);
            _playerVisual->setRotation(CC_RADIANS_TO_DEGREES(angle));
        }

        Vec2 beforeMove = m_player->getPosition();
        m_player->updatePlayer(dt);
        if (_moveDirection != Vec2::ZERO &&
            beforeMove.distanceSquared(m_player->getPosition()) < 0.01f)
        {
            updatePlayerMovement(dt);
        }

        Vec2 clampedPos = m_player->getPosition();
        float margin = 35.0f;
        clampedPos.x = std::max(margin, std::min(_worldSize.width - margin, clampedPos.x));
        clampedPos.y = std::max(margin, std::min(_worldSize.height - margin, clampedPos.y));
        m_player->setPosition(clampedPos);
        if (_lowHpMoodTimer > 0.0f)
        {
            _lowHpMoodTimer -= dt;
        }
        if (_lowHpMoodTimer <= 0.0f &&
            m_player->getMoodSystem() &&
            m_player->getMaxHp() > 0 &&
            m_player->getHp() <= static_cast<int>(m_player->getMaxHp() * 0.35f))
        {
            m_player->getMoodSystem()->onLowHp();
            _lowHpMoodTimer = 2.0f;
        }
        updateCamera();
    }

    for (auto* weapon : _weapons)
    {
        if (weapon)
        {
            weapon->updateCooldown(dt);
            weapon->setAimDirection(_playerDir);
        }
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
        updateFreezeEffect(dt);
        _waveManager->update(dt);
    }

    updateEnemyPlayerContact(dt);

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
    _bulletPool.reclaimInactive();
    updateRewards(dt);

    if (_waveManager && m_player && _waveManager->getKillCount() > _lastKillCount)
    {
        int killed = _waveManager->getKillCount() - _lastKillCount;
        _lastKillCount = _waveManager->getKillCount();
        if (m_player->getMoodSystem())
        {
            for (int i = 0; i < killed; ++i)
            {
                m_player->getMoodSystem()->onEnemyKilled();
            }
        }
    }

    // --- Enemy HP bars ---
    if (_waveManager)
    {
        static const int HP_BAR_BG_TAG = 998;
        static const int HP_BAR_TAG = 999;
        for (auto* enemy : _waveManager->getAliveEnemies())
        {
            if (!enemy || !enemy->isRoleAlive() || !enemy->isObjectActive())
                continue;

            float enemyScale = enemy->getScale();
            float invScale = enemyScale > 0.001f ? 1.0f / enemyScale : 1.0f;
            float fullBarWidth = 36.0f * invScale;
            float barHeight = 5.0f * invScale;
            float barY = 24.0f * invScale;

            // Find or create HP bar background + fill
            Node* barNode = enemy->getChildByTag(HP_BAR_TAG);
            LayerColor* hpFill = dynamic_cast<LayerColor*>(barNode);
            if (!hpFill)
            {
                // Background (dark)
                auto bg = LayerColor::create(Color4B(40, 40, 40, 255), fullBarWidth, barHeight);
                bg->setTag(HP_BAR_BG_TAG);
                enemy->addChild(bg, 1);

                // Fill (green)
                hpFill = LayerColor::create(Color4B(50, 200, 50, 255), fullBarWidth, barHeight);
                hpFill->setTag(HP_BAR_TAG);
                enemy->addChild(hpFill, 2);
            }

            auto hpBg = dynamic_cast<LayerColor*>(enemy->getChildByTag(HP_BAR_BG_TAG));
            if (hpBg)
            {
                hpBg->setContentSize(Size(fullBarWidth, barHeight));
                hpBg->setPosition(Vec2(-fullBarWidth * 0.5f, barY));
            }
            hpFill->setPosition(Vec2(-fullBarWidth * 0.5f, barY));

            // Update fill width from current HP ratio
            float hpRatio = enemy->getMaxHp() > 0
                ? static_cast<float>(enemy->getHp()) / static_cast<float>(enemy->getMaxHp())
                : 0.0f;
            if (hpRatio < 0.0f) hpRatio = 0.0f;
            if (hpRatio > 1.0f) hpRatio = 1.0f;

            float barWidth = fullBarWidth * hpRatio;
            hpFill->setContentSize(Size(barWidth, barHeight));

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
        updatePlayerMoodVisual();
        updateUI(m_player);
    }
    updateEnvironmentEffects(dt);
    updateAssignmentProgress(dt);
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
        _moodLabel->setString(textByLanguage("Mood: ", u8"情绪: ") +
            moodNameForUi(player->getCurrentMood()));
    }

    if (_weaponLabel && _currentWeapon)
    {
        _weaponLabel->setString(textByLanguage("Weapon: ", u8"武器: ") +
            weaponNameForUi(_currentWeapon->getWeaponName()));
    }
    if (_weaponIcon && _currentWeapon)
    {
        auto texture = Director::getInstance()->getTextureCache()->addImage(_currentWeapon->getImagePath());
        if (texture)
        {
            _weaponIcon->setTexture(texture);
            _weaponIcon->setTextureRect(Rect(0, 0,
                texture->getContentSize().width,
                texture->getContentSize().height));
            applySpriteFit(_weaponIcon, 56.0f, 56.0f);
            _weaponIcon->setVisible(true);
        }
        else
        {
            _weaponIcon->setVisible(false);
        }
    }
    updateWeaponEnergyUI();

    if (_progressLabel)
    {
        char buf[64];
        if (_isEndlessMode)
        {
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "无尽  DDL: %d  倒计时: %.2f秒",
                    _completedDdlCount, _ddlTimeRemaining);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Endless  DDL: %d  Timer: %.2fs",
                    _completedDdlCount, _ddlTimeRemaining);
            }
        }
        else
        {
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "第%d关  DDL: %.2f秒", _levelNumber, _ddlTimeRemaining);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Level %d  DDL: %.2fs", _levelNumber, _ddlTimeRemaining);
            }
        }
        _progressLabel->setString(buf);
    }

    if (_endlessStatsLabel)
    {
        _endlessStatsLabel->setVisible(_isEndlessMode);
        if (_isEndlessMode)
        {
            char buf[128];
            int exp = player->getExp();
            int expToNext = player->getExpToNextLevel();
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "等级 %d  经验 %d/%d  分数 %d",
                    player->getLevel(), exp, expToNext, _endlessScore);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Lv %d  EXP %d/%d  Score %d",
                    player->getLevel(), exp, expToNext, _endlessScore);
            }
            _endlessStatsLabel->setString(buf);
        }
    }

    if (_taskLabel)
    {
        char buf[80];
        if (_isEndlessMode)
        {
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "生存: %.2f秒  书桌: %.2f秒",
                    m_survivalTime, _deskStayProgress);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Survival: %.2fs  Desk: %.2fs",
                    m_survivalTime, _deskStayProgress);
            }
        }
        else
        {
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "书桌: %.2f / %.2f秒", _deskStayProgress, _deskStayRequired);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Desk: %.2f / %.2fs", _deskStayProgress, _deskStayRequired);
            }
        }
        _taskLabel->setString(buf);
    }

    if (_taskBarFill)
    {
        float ratio = _isEndlessMode
            ? std::min(1.0f, _deskStayProgress / 30.0f)
            : (_deskStayRequired > 0.0f ? _deskStayProgress / _deskStayRequired : 0.0f);
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        _taskBarFill->setContentSize(Size(_taskBarMaxWidth * ratio,
            _taskBarFill->getContentSize().height));
    }

    refreshWeaponSlotUI();
}

void GameScene::updateWeaponEnergyUI()
{
    if (!_weaponEnergyFill || !_currentWeapon)
    {
        return;
    }

    float ratio = _currentWeapon->getEnergyRatio();
    _weaponEnergyFill->setContentSize(Size(_weaponEnergyBarMaxWidth * ratio,
        _weaponEnergyFill->getContentSize().height));

    if (ratio > 0.55f)
    {
        _weaponEnergyFill->setColor(Color3B(90, 190, 255));
    }
    else if (ratio > 0.25f)
    {
        _weaponEnergyFill->setColor(Color3B(245, 205, 70));
    }
    else
    {
        _weaponEnergyFill->setColor(Color3B(235, 70, 70));
    }
}

void GameScene::refreshWeaponSlotUI()
{
    if (_weaponSlotNodes.empty())
    {
        return;
    }

    bool slotDataChanged = _lastWeaponSlotIndex != _currentWeaponIndex ||
        _lastWeaponSlotIds.size() != _weaponLoadoutIds.size();
    if (!slotDataChanged)
    {
        for (int i = 0; i < static_cast<int>(_weaponLoadoutIds.size()); ++i)
        {
            if (_lastWeaponSlotIds[i] != _weaponLoadoutIds[i])
            {
                slotDataChanged = true;
                break;
            }
        }
    }
    if (!slotDataChanged)
    {
        return;
    }

    auto options = getWeaponOptions();
    float s = Director::getInstance()->getWinSize().height / 640.0f;

    for (int i = 0; i < static_cast<int>(_weaponSlotNodes.size()); ++i)
    {
        auto slot = _weaponSlotNodes[i];
        if (!slot)
        {
            continue;
        }

        slot->removeAllChildren();

        Size slotSize = slot->getContentSize();
        bool selected = i == _currentWeaponIndex;
        auto bg = LayerColor::create(
            selected ? Color4B(64, 96, 118, 232) : Color4B(18, 22, 30, 220),
            slotSize.width,
            slotSize.height);
        bg->setPosition(Vec2::ZERO);
        slot->addChild(bg, 0);

        auto border = DrawNode::create();
        Vec2 verts[4] = {
            Vec2(0, 0),
            Vec2(slotSize.width, 0),
            Vec2(slotSize.width, slotSize.height),
            Vec2(0, slotSize.height)
        };
        border->drawPolygon(
            verts,
            4,
            Color4F(0, 0, 0, 0),
            selected ? 2.5f : 1.2f,
            selected ? Color4F(0.42f, 0.92f, 1.0f, 1.0f) : Color4F(0.52f, 0.58f, 0.67f, 0.8f));
        slot->addChild(border, 3);

        int weaponId = (i < static_cast<int>(_weaponLoadoutIds.size())) ? _weaponLoadoutIds[i] : i;
        std::string imagePath;
        for (const auto& option : options)
        {
            if (option.id == weaponId)
            {
                imagePath = option.imagePath;
                break;
            }
        }

        if (!imagePath.empty())
        {
            auto icon = Sprite::create(imagePath);
            if (icon)
            {
                Size imageSize = icon->getContentSize();
                if (imageSize.width > 0.0f && imageSize.height > 0.0f)
                {
                    icon->setScale(std::min((slotSize.width - 6.0f * s) / imageSize.width,
                        (slotSize.height - 6.0f * s) / imageSize.height));
                }
                icon->setPosition(Vec2(slotSize.width * 0.5f, slotSize.height * 0.54f));
                slot->addChild(icon, 1);
            }
        }

        auto numBg = LayerColor::create(Color4B(0, 0, 0, 150), 16.0f * s, 16.0f * s);
        numBg->setPosition(Vec2(2.0f * s, slotSize.height - 18.0f * s));
        slot->addChild(numBg, 4);

        auto num = Label::createWithSystemFont(std::to_string(i + 1), "Arial", 12.0f * s);
        num->setColor(selected ? Color3B(135, 235, 255) : Color3B(222, 226, 232));
        num->setPosition(numBg->getPosition() + Vec2(8.0f * s, 8.0f * s));
        slot->addChild(num, 5);
    }

    _lastWeaponSlotIndex = _currentWeaponIndex;
    _lastWeaponSlotIds = _weaponLoadoutIds;
}

void GameScene::initLevelTask()
{
    auto ud = UserDefault::getInstance();
    _isEndlessMode = ud->getIntegerForKey("selected_game_mode", 0) == 1;
    _levelNumber = ud->getIntegerForKey("selected_level", 1);
    if (_levelNumber < 1) _levelNumber = 1;
    if (_levelNumber > 99) _levelNumber = 99;

    float randomPart = CCRANDOM_0_1() * (0.8f + _levelNumber * 0.25f);
    _deskStayRequired = 4.0f + _levelNumber * 1.25f + randomPart;
    _ddlTimeLimit = 45.0f + _levelNumber * 8.0f;
    if (_isEndlessMode)
    {
        _deskStayRequired = 0.0f;
        _ddlTimeLimit = 28.0f;
    }
    _ddlTimeRemaining = _ddlTimeLimit;
    _completedDdlCount = 0;
    _deskStayProgress = 0.0f;
    _assignmentProgress = 0.0f;
}

void GameScene::showLevelIntro()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;

    _levelIntroLayer = LayerColor::create(Color4B(0, 0, 0, 150), winSize.width, winSize.height);
    if (!_levelIntroLayer)
    {
        return;
    }

    _levelIntroLayer->setPosition(Vec2::ZERO);
    this->addChild(_levelIntroLayer, 120);

    float boxW = 560.0f * s;
    float boxH = 250.0f * s;
    Vec2 boxPos(origin.x + visibleSize.width * 0.5f - boxW * 0.5f,
        origin.y + visibleSize.height * 0.5f - boxH * 0.5f);

    auto box = LayerColor::create(Color4B(28, 34, 48, 245), boxW, boxH);
    box->setPosition(boxPos);
    _levelIntroLayer->addChild(box, 1);

    auto border = DrawNode::create();
    Vec2 verts[4] = {
        boxPos,
        boxPos + Vec2(boxW, 0),
        boxPos + Vec2(boxW, boxH),
        boxPos + Vec2(0, boxH)
    };
    border->drawPolygon(verts, 4, Color4F(0, 0, 0, 0), 2.0f, Color4F(0.7f, 0.85f, 1.0f, 1.0f));
    _levelIntroLayer->addChild(border, 2);

    auto title = Label::createWithSystemFont(
        _isEndlessMode
            ? textByLanguage("Endless Mission", u8"无尽任务")
            : textByLanguage("Level Mission", u8"关卡任务"),
        "Arial", 32.0f * s);
    title->setColor(Color3B(255, 240, 150));
    title->setPosition(boxPos + Vec2(boxW * 0.5f, boxH - 45.0f * s));
    _levelIntroLayer->addChild(title, 3);

    char mission[256];
    if (_isEndlessMode)
    {
        if (isChineseUi())
        {
            snprintf(mission, sizeof(mission),
                "无尽模式\n每个DDL都有自己的倒计时。\n存活到倒计时结束即可完成一个DDL。\n最终成绩为完成的DDL数量。");
        }
        else
        {
            snprintf(mission, sizeof(mission),
                "Endless Mode\nEach DDL has its own countdown.\nSurvive until countdown ends to complete one DDL.\nFinal result is completed DDL count.");
        }
    }
    else
    {
        if (isChineseUi())
        {
            snprintf(mission, sizeof(mission),
                "第%d关\n在书桌附近停留 %.2f 秒。\nDDL倒计时: %.2f 秒。\n保持生命值大于0。",
                _levelNumber, _deskStayRequired, _ddlTimeLimit);
        }
        else
        {
            snprintf(mission, sizeof(mission),
                "Level %d\nStay near the Desk for %.2f seconds.\nDDL timer: %.2f seconds.\nKeep HP above 0.",
                _levelNumber, _deskStayRequired, _ddlTimeLimit);
        }
    }
    auto detail = Label::createWithSystemFont(mission, "Arial", 22.0f * s);
    detail->setColor(Color3B(225, 235, 245));
    detail->setAlignment(TextHAlignment::CENTER);
    detail->setPosition(boxPos + Vec2(boxW * 0.5f, boxH * 0.46f));
    _levelIntroLayer->addChild(detail, 3);

    auto hint = Label::createWithSystemFont(
        textByLanguage("Starting soon...", u8"即将开始..."), "Arial", 18.0f * s);
    hint->setColor(Color3B(160, 210, 255));
    hint->setPosition(boxPos + Vec2(boxW * 0.5f, 28.0f * s));
    _levelIntroLayer->addChild(hint, 3);

    _levelIntroActive = true;
    _levelIntroTimer = 3.0f;
}

void GameScene::hideLevelIntro()
{
    _levelIntroActive = false;
    if (_levelIntroLayer)
    {
        _levelIntroLayer->removeFromParentAndCleanup(true);
        _levelIntroLayer = nullptr;
    }
}

void GameScene::updateSurvivalTime(float dt)
{
    m_survivalTime += dt;
    if (_survivalTimeLabel)
    {
        char buf[32];
        if (isChineseUi())
        {
            snprintf(buf, sizeof(buf), "时间: %.1f秒", m_survivalTime);
        }
        else
        {
            snprintf(buf, sizeof(buf), "Time: %.1fs", m_survivalTime);
        }
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
            if (_equipmentLayer)
            {
                hideEquipmentMenu();
                return;
            }
            if (!_isGameOver)
            {
                if (_isPaused) hidePauseMenu();
                else           showPauseMenu();
            }
            return;
        }

        if (code == EventKeyboard::KeyCode::KEY_1) { switchWeapon(0); return; }
        if (code == EventKeyboard::KeyCode::KEY_2) { switchWeapon(1); return; }
        if (code == EventKeyboard::KeyCode::KEY_3) { switchWeapon(2); return; }
        if (code == EventKeyboard::KeyCode::KEY_4) { switchWeapon(3); return; }

        // Ignore movement keys while paused
        if (_isPaused) return;

        if (code == EventKeyboard::KeyCode::KEY_O ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_O)
        {
            if (!_isGameOver && m_player && m_player->isRoleAlive())
            {
                fireBullet();
            }
            return;
        }

        if (code == _keyMoveUp || code == EventKeyboard::KeyCode::KEY_W ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_W ||
            code == EventKeyboard::KeyCode::KEY_UP_ARROW ||
            code == EventKeyboard::KeyCode::KEY_R) _keyW = true;
        if (code == _keyMoveDown || code == EventKeyboard::KeyCode::KEY_S ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_S ||
            code == EventKeyboard::KeyCode::KEY_DOWN_ARROW ||
            code == EventKeyboard::KeyCode::KEY_F) _keyS = true;
        if (code == _keyMoveLeft || code == EventKeyboard::KeyCode::KEY_A ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_A ||
            code == EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            code == EventKeyboard::KeyCode::KEY_G) _keyA = true;
        if (code == _keyMoveRight || code == EventKeyboard::KeyCode::KEY_D ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_D ||
            code == EventKeyboard::KeyCode::KEY_RIGHT_ARROW ||
            code == EventKeyboard::KeyCode::KEY_H) _keyD = true;
        updateMoveDirection();
    };
    kbListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*)
    {
        if (_isPaused) return;

        if (code == _keyMoveUp || code == EventKeyboard::KeyCode::KEY_W ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_W ||
            code == EventKeyboard::KeyCode::KEY_UP_ARROW ||
            code == EventKeyboard::KeyCode::KEY_R) _keyW = false;
        if (code == _keyMoveDown || code == EventKeyboard::KeyCode::KEY_S ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_S ||
            code == EventKeyboard::KeyCode::KEY_DOWN_ARROW ||
            code == EventKeyboard::KeyCode::KEY_F) _keyS = false;
        if (code == _keyMoveLeft || code == EventKeyboard::KeyCode::KEY_A ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_A ||
            code == EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            code == EventKeyboard::KeyCode::KEY_G) _keyA = false;
        if (code == _keyMoveRight || code == EventKeyboard::KeyCode::KEY_D ||
            code == EventKeyboard::KeyCode::KEY_CAPITAL_D ||
            code == EventKeyboard::KeyCode::KEY_RIGHT_ARROW ||
            code == EventKeyboard::KeyCode::KEY_H) _keyD = false;
        updateMoveDirection();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kbListener, this);

    // ---- Mouse ----
    auto* mouseListener = EventListenerMouse::create();

    // Track cursor every frame so triangle can follow
    mouseListener->onMouseMove = [this](EventMouse* mouseEv)
    {
        _mousePos = screenToWorldCoords(viewToGameCoords(mouseEv->getLocationInView()));
    };

    // Press O to attack in the current movement-facing direction.
    mouseListener->onMouseDown = [this](EventMouse* mouseEv)
    {
        _mousePos = screenToWorldCoords(viewToGameCoords(mouseEv->getLocationInView()));
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

    if (_moveDirection != Vec2::ZERO)
    {
        _playerDir = _moveDirection.getNormalized();
        if (m_player && _moveDirection.x != 0.0f)
        {
            // Character artwork faces right by default. Mirror only while the
            // latest horizontal movement is toward the left.
            m_player->setFlippedX(_moveDirection.x < 0.0f);
        }
        if (_playerVisual)
        {
            float angle = std::atan2(_playerDir.y, _playerDir.x);
            _playerVisual->setRotation(CC_RADIANS_TO_DEGREES(angle));
        }
    }

    if (m_player)
        m_player->setInputDirection(_moveDirection);

    if (_environmentLabel && _moveDirection != Vec2::ZERO)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Move: %.0f, %.0f", _moveDirection.x, _moveDirection.y);
        _environmentLabel->setString(buf);
    }
}

void GameScene::updatePlayerMovement(float dt)
{
    if (!m_player || _moveDirection == Vec2::ZERO)
    {
        return;
    }

    Vec2 dir = _moveDirection.getNormalized();
    Vec2 nextPos = m_player->getPosition() + dir * m_player->getCurrentSpeed() * dt;

    float margin = 35.0f;
    nextPos.x = std::max(margin, std::min(_worldSize.width - margin, nextPos.x));
    nextPos.y = std::max(margin, std::min(_worldSize.height - margin, nextPos.y));
    m_player->setPosition(nextPos);
    updateCamera();
    if (_environmentLabel)
    {
        char buf[80];
        snprintf(buf, sizeof(buf), "Player: %.0f, %.0f", nextPos.x, nextPos.y);
        _environmentLabel->setString(buf);
    }
}

void GameScene::updateCamera()
{
    if (!_worldLayer || !m_player)
    {
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 playerPos = m_player->getPosition();

    Vec2 target(origin.x + visibleSize.width * 0.5f - playerPos.x * _worldScale,
        origin.y + visibleSize.height * 0.5f - playerPos.y * _worldScale);

    float minX = origin.x + visibleSize.width - _worldSize.width * _worldScale;
    float maxX = origin.x;
    float minY = origin.y + visibleSize.height - _worldSize.height * _worldScale;
    float maxY = origin.y;

    if (minX > maxX) target.x = origin.x + (visibleSize.width - _worldSize.width * _worldScale) * 0.5f;
    else target.x = std::max(minX, std::min(maxX, target.x));

    if (minY > maxY) target.y = origin.y + (visibleSize.height - _worldSize.height * _worldScale) * 0.5f;
    else target.y = std::max(minY, std::min(maxY, target.y));

    if (!_cameraInitialized)
    {
        _worldLayer->setPosition(target);
        _cameraInitialized = true;
        return;
    }

    Vec2 current = _worldLayer->getPosition();
    _worldLayer->setPosition(current + (target - current) * 0.18f);
}

void GameScene::updateEnemyPlayerContact(float dt)
{
    if (!m_player || !_waveManager || !m_player->isRoleAlive())
    {
        return;
    }

    if (_enemyContactDamageCooldown > 0.0f)
    {
        _enemyContactDamageCooldown -= dt;
    }

    Rect playerBox = m_player->getBoundingBox();
    playerBox.origin.x += playerBox.size.width * 0.12f;
    playerBox.origin.y += playerBox.size.height * 0.12f;
    playerBox.size.width *= 0.76f;
    playerBox.size.height *= 0.76f;

    // Check if player is near world boundary (cornered)
    float margin = 55.0f;
    Vec2 playerPos = m_player->getPosition();
    bool nearWall = (playerPos.x < margin || playerPos.x > _worldSize.width - margin ||
                     playerPos.y < margin || playerPos.y > _worldSize.height - margin);

    int contactCount = 0;
    for (auto* enemy : _waveManager->getAliveEnemies())
    {
        if (!enemy || !enemy->isRoleAlive() || !enemy->isObjectActive())
        {
            continue;
        }

        Rect enemyBox = enemy->getBoundingBox();
        enemyBox.origin.x += enemyBox.size.width * 0.08f;
        enemyBox.origin.y += enemyBox.size.height * 0.08f;
        enemyBox.size.width *= 0.84f;
        enemyBox.size.height *= 0.84f;

        if (!playerBox.intersectsRect(enemyBox))
        {
            continue;
        }

        contactCount++;

        if (_enemyContactDamageCooldown <= 0.0f)
        {
            int damage = enemy->getAttackDamage();
            if (damage < 1) damage = 1;

            int hpBefore = m_player->getHp();
            m_player->takeDamage(damage);
            int hpLost = hpBefore - m_player->getHp();

            if (hpLost > 0)
            {
                if (m_player->getMoodSystem())
                {
                    m_player->getMoodSystem()->onPlayerDamaged();
                }

                AudioManager::getInstance()->playPlayerHurt();
                AudioManager::getInstance()->playPlayerMonsterCollision();

                auto label = Label::createWithSystemFont("-" + std::to_string(hpLost), "Arial", 22);
                if (label)
                {
                    label->setColor(Color3B(255, 70, 70));
                    label->setPosition(m_player->getPosition() + Vec2(0, 52));
                    (_worldLayer ? _worldLayer : this)->addChild(label, 40);
                    label->runAction(Sequence::create(
                        Spawn::create(MoveBy::create(0.45f, Vec2(0, 34)),
                            FadeOut::create(0.45f), nullptr),
                        RemoveSelf::create(),
                        nullptr));
                }

                if (_environmentLabel)
                {
                    _environmentLabel->setString(textByLanguage("Monster hit: -", u8"怪物碰撞: -") +
                        std::to_string(hpLost) + textByLanguage(" HP", u8" 生命"));
                }
            }

            _enemyContactDamageCooldown = 0.45f;
        }

        // Push ALL colliding enemies away (not just one), stronger push when cornered
        Vec2 pushDir = m_player->getPosition() - enemy->getPosition();
        if (pushDir.lengthSquared() > 0.001f)
        {
            pushDir.normalize();
            float playerPush = nearWall ? 12.0f : 8.0f;
            float enemyPush = nearWall ? 22.0f : 12.0f;
            m_player->setPosition(m_player->getPosition() + pushDir * playerPush);
            enemy->setPosition(enemy->getPosition() - pushDir * enemyPush);
        }
    }

    // When surrounded, give player a brief escape dash
    if (contactCount >= 3 && nearWall && m_player->isRoleAlive())
    {
        // Push player away from all contacting enemies
        Vec2 escapeDir;
        for (auto* enemy : _waveManager->getAliveEnemies())
        {
            if (!enemy || !enemy->isRoleAlive() || !enemy->isObjectActive())
                continue;
            if (playerBox.intersectsRect(enemy->getBoundingBox()))
            {
                escapeDir += (playerPos - enemy->getPosition());
            }
        }
        if (escapeDir.lengthSquared() > 0.001f)
        {
            escapeDir.normalize();
            m_player->setPosition(m_player->getPosition() + escapeDir * 25.0f);
        }
    }
}

void GameScene::fireBullet()
{
    if (!_currentWeapon) return;

    if (!_currentWeapon->hasEnoughEnergy())
    {
        if (_topHintLabel)
        {
            _topHintLabel->setString(textByLanguage(
                "Weapon power empty: wait for recovery",
                u8"武器能量为空: 等待恢复"));
        }
        return;
    }

    if (!_currentWeapon->isReadyToFire())
    {
        return;
    }

    _currentWeapon->setAimDirection(_playerDir);
    _currentWeapon->fire();
    applyAimWeaponDamage();

    if (_topHintLabel)
    {
        _topHintLabel->setString(textByLanguage(
            "Weapon power is recovering while you move",
            u8"武器能量会随时间恢复"));
    }

    if (_currentWeapon->getWeaponName() == "CoffeeLaser" ||
        _currentWeapon->getWeaponName() == "KeyboardWeap")
    {
        AudioManager::getInstance()->playLaserAttack();
    }
    else if (_currentWeapon->getWeaponName() == "CoffeeGun")
    {
        AudioManager::getInstance()->playCoffeeAttack();
    }
    else
    {
        AudioManager::getInstance()->playKeyboardAttack();
    }
}

void GameScene::applyAimWeaponDamage()
{
    if (!_currentWeapon || !_waveManager || !m_player)
    {
        return;
    }

    Vec2 origin = m_player->getObjectPosition();
    Vec2 dir = _playerDir.lengthSquared() > 0.0001f ? _playerDir.getNormalized() : Vec2(1, 0);
    std::string weaponName = _currentWeapon->getWeaponName();

    float range = 520.0f;
    float width = 42.0f;
    bool piercing = false;
    int damage = _currentWeapon->getModifiedAttackPower();

    if (weaponName == "CoffeeGun")
    {
        range = 560.0f;
        width = 34.0f;
    }
    else if (weaponName == "CoffeeLaser")
    {
        range = _nearPowerSocket ? 920.0f : 760.0f;
        width = _nearPowerSocket ? 54.0f : 42.0f;
        damage = static_cast<int>(damage * (_nearPowerSocket ? 1.35f : 1.0f));
        piercing = true;
    }
    else if (weaponName == "DeskLampLaser")
    {
        range = _nearPowerSocket ? 900.0f : 720.0f;
        width = _nearPowerSocket ? 48.0f : 38.0f;
        piercing = true;
    }
    else if (weaponName == "CoffeeBlast")
    {
        range = 185.0f;
        width = 185.0f;
        piercing = true;
        dir = Vec2::ZERO;
    }
    else if (weaponName == "KeyboardWave")
    {
        range = 520.0f;
        width = 68.0f;
    }
    else if (weaponName == "KeyboardWeap")
    {
        range = 640.0f;
        width = 54.0f;
        piercing = true;
    }

    Enemy* bestEnemy = nullptr;
    float bestProjection = range + 1.0f;

    for (auto* enemy : _waveManager->getAliveEnemies())
    {
        if (!enemy || !enemy->isObjectActive() || !enemy->isRoleAlive())
        {
            continue;
        }

        Vec2 toEnemy = enemy->getObjectPosition() - origin;
        float projection = dir == Vec2::ZERO ? toEnemy.length() : toEnemy.dot(dir);
        if (projection < 0.0f || projection > range)
        {
            continue;
        }

        Vec2 closest = dir == Vec2::ZERO ? origin : origin + dir * projection;
        float perpendicular = dir == Vec2::ZERO ? toEnemy.length() : enemy->getObjectPosition().distance(closest);
        float enemyRadius = std::max(enemy->getContentSize().width, enemy->getContentSize().height) *
            enemy->getScale() * 0.5f;

        if (perpendicular <= width + enemyRadius)
        {
            if (piercing)
            {
                bool wasAlive = enemy->isRoleAlive();
                enemy->takeDamage(damage);
                if (wasAlive)
                {
                    if (enemy->isDead()) AudioManager::getInstance()->playEnemyDie();
                    else AudioManager::getInstance()->playEnemyHit();
                }
                enemy->flashWhenHit();
                auto label = Label::createWithSystemFont("-" + std::to_string(damage), "Arial", 18);
                label->setColor(Color3B(255, 80, 60));
                label->setPosition(enemy->getPosition() + Vec2(0, 38));
                (_worldLayer ? _worldLayer : this)->addChild(label, 50);
                label->runAction(Sequence::create(MoveBy::create(0.35f, Vec2(0, 26)),
                    FadeOut::create(0.25f), RemoveSelf::create(), nullptr));
                if (enemy->isDead())
                {
                    enemy->setActive(false);
                }
            }
            else if (projection < bestProjection)
            {
                bestProjection = projection;
                bestEnemy = enemy;
            }
        }
    }

    if (!piercing && bestEnemy)
    {
        bool wasAlive = bestEnemy->isRoleAlive();
        bestEnemy->takeDamage(damage);
        if (wasAlive)
        {
            if (bestEnemy->isDead()) AudioManager::getInstance()->playEnemyDie();
            else AudioManager::getInstance()->playEnemyHit();
        }
        bestEnemy->flashWhenHit();
        auto label = Label::createWithSystemFont("-" + std::to_string(damage), "Arial", 18);
        label->setColor(Color3B(255, 80, 60));
        label->setPosition(bestEnemy->getPosition() + Vec2(0, 38));
        (_worldLayer ? _worldLayer : this)->addChild(label, 50);
        label->runAction(Sequence::create(MoveBy::create(0.35f, Vec2(0, 26)),
            FadeOut::create(0.25f), RemoveSelf::create(), nullptr));
        if (bestEnemy->isDead())
        {
            bestEnemy->setActive(false);
        }
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

void GameScene::switchWeapon(int index)
{
    if (index < 0 || index >= static_cast<int>(_weapons.size()))
    {
        return;
    }

    AudioManager::getInstance()->playButtonClick();

    _currentWeaponIndex = index;
    _currentWeapon = _weapons[index];
    for (int i = 0; i < static_cast<int>(_weapons.size()); ++i)
    {
        if (_weapons[i])
        {
            _weapons[i]->setVisible(i == _currentWeaponIndex);
        }
    }
    if (_currentWeapon)
    {
        _currentWeapon->readyNow();
        if (_environmentLabel)
        {
            _environmentLabel->setString(textByLanguage("Switched: ", u8"已切换: ") +
                weaponNameForUi(_currentWeapon->getWeaponName()));
        }
        if (_topHintLabel)
        {
            _topHintLabel->setString(textByLanguage(
                "1 CoffeeGun  2 CoffeeLaser  3 KeyboardWave  4 KeyboardWeap",
                u8"1 咖啡枪  2 咖啡激光  3 键盘冲击波  4 键盘武器"));
        }
    }
    updateUI(m_player);
}

Weapon* GameScene::createWeaponById(int weaponId)
{
    switch (weaponId)
    {
    case 0:
        return CoffeeGun::create(m_player);
    case 1:
        return CoffeeLaser::create(m_player);
    case 2:
        return KeyboardWave::create(m_player);
    case 3:
        return KeyboardWeap::create(m_player);
    case 4:
        return DeskLampLaser::create(m_player);
    case 5:
        return CoffeeBlast::create(m_player);
    case 6:
    default:
        return KeyboardWave::create(m_player);
    }
}

std::vector<GameScene::WeaponOption> GameScene::getWeaponOptions() const
{
    std::vector<WeaponOption> options = {
        { 0, textByLanguage("Coffee Gun", u8"咖啡枪"), "weapon/coffee_gun_sprite.png" },
        { 1, textByLanguage("Coffee Laser", u8"咖啡激光"), "weapon/coffee_bullet_sprite.png" },
        { 2, textByLanguage("Keyboard Wave", u8"键盘冲击波"), "weapon/keyboard_wave_sprite.png" },
        { 3, textByLanguage("Keyboard Weap", u8"键盘武器"), "weapon/keyboard_weapon_sprite.png" },
        { 4, textByLanguage("Desk Lamp", u8"台灯激光"), "weapon/desk_lamp_weapon_sprite.png" },
        { 5, textByLanguage("Coffee Blast", u8"咖啡爆破"), "weapon/coffee_blast_sprite.png" },
        { 6, textByLanguage("Focus Orb", u8"专注宝珠"), "weapon/focus_orb_sprite.png" }
    };

    for (auto& option : options)
    {
        std::string resolved = AssetPaths::resolve(option.imagePath);
        if (resolved.empty() && option.id == 3)
        {
            resolved = AssetPaths::resolve("weapon/keyboard_weapon_sprite.png");
        }
        option.imagePath = resolved;
    }

    return options;
}

void GameScene::rebuildWeaponLoadout()
{
    for (auto* weapon : _weapons)
    {
        if (weapon)
        {
            weapon->removeFromParentAndCleanup(true);
        }
    }
    _weapons.clear();

    if (_weaponLoadoutIds.empty())
    {
        _weaponLoadoutIds = { 0, 1, 2, 3 };
    }
    while (_weaponLoadoutIds.size() < 4)
    {
        _weaponLoadoutIds.push_back(static_cast<int>(_weaponLoadoutIds.size()));
    }

    for (int i = 0; i < 4; ++i)
    {
        Weapon* weapon = createWeaponById(_weaponLoadoutIds[i]);
        if (weapon)
        {
            applyEndlessGrowthToWeapon(weapon, _weaponLoadoutIds[i]);
            if (m_player)
            {
                float parentScale = m_player->getScale();
                if (parentScale > 0.001f)
                {
                    weapon->setScale(weapon->getScale() / parentScale);
                }
                m_player->addChild(weapon, 4);
            }
            else
            {
                (_worldLayer ? _worldLayer : this)->addChild(weapon, 120);
            }
            if (_waveManager)
            {
                weapon->bindBattleData(&_waveManager->getAliveEnemies(), &_bullets, _bulletLayer);
                weapon->bindBulletPool(&_bulletPool);
            }
            weapon->setVisible(i == _currentWeaponIndex);
            _weapons.push_back(weapon);
        }
    }

    if (_currentWeaponIndex < 0 || _currentWeaponIndex >= static_cast<int>(_weapons.size()))
    {
        _currentWeaponIndex = 0;
    }
    _currentWeapon = _weapons.empty() ? nullptr : _weapons[_currentWeaponIndex];
    for (int i = 0; i < static_cast<int>(_weapons.size()); ++i)
    {
        if (_weapons[i])
        {
            _weapons[i]->setVisible(i == _currentWeaponIndex);
        }
    }
    updateUI(m_player);
}

Node* GameScene::createEquipmentIcon(const WeaponOption& option, const Size& boxSize, bool selected)
{
    auto root = Node::create();
    root->setContentSize(boxSize);

    auto bg = LayerColor::create(selected ? Color4B(80, 120, 135, 230) : Color4B(38, 45, 58, 230),
        boxSize.width, boxSize.height);
    bg->setIgnoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setPosition(Vec2(boxSize.width * 0.5f, boxSize.height * 0.5f));
    root->addChild(bg);

    auto border = DrawNode::create();
    Vec2 verts[4] = {
        Vec2(0, 0), Vec2(boxSize.width, 0),
        Vec2(boxSize.width, boxSize.height), Vec2(0, boxSize.height)
    };
    border->drawPolygon(verts, 4, Color4F(0, 0, 0, 0),
        selected ? 3.0f : 1.5f,
        selected ? Color4F(0.45f, 0.95f, 1.0f, 1.0f) : Color4F(0.65f, 0.70f, 0.78f, 0.8f));
    root->addChild(border, 2);

    if (!option.imagePath.empty())
    {
        auto sprite = Sprite::create(option.imagePath);
        if (sprite)
        {
            Size spriteSize = sprite->getContentSize();
            if (spriteSize.width > 0.0f && spriteSize.height > 0.0f)
            {
                float padding = option.id >= 4 ? 34.0f : 22.0f;
                sprite->setScale(std::min((boxSize.width - padding) / spriteSize.width,
                    (boxSize.height - padding) / spriteSize.height));
            }
            sprite->setPosition(Vec2(boxSize.width * 0.5f, boxSize.height * 0.52f));
            root->addChild(sprite, 1);
        }
    }
    else
    {
        auto missing = Label::createWithSystemFont(option.name, "Arial", 12);
        missing->setColor(Color3B(220, 225, 235));
        missing->setDimensions(boxSize.width - 10.0f, boxSize.height - 20.0f);
        missing->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
        missing->setPosition(Vec2(boxSize.width * 0.5f, boxSize.height * 0.52f));
        root->addChild(missing, 1);
    }

    return root;
}

void GameScene::showEquipmentMenu()
{
    if (_equipmentLayer)
    {
        return;
    }

    AudioManager::getInstance()->playMenuBGM();

    _keyW = _keyA = _keyS = _keyD = false;
    updateMoveDirection();
    _isPaused = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    float s = Director::getInstance()->getWinSize().height / 640.0f;

    _equipmentLayer = Node::create();
    this->addChild(_equipmentLayer, 300);

    auto shade = LayerColor::create(Color4B(5, 8, 14, 190), visibleSize.width, visibleSize.height);
    shade->setPosition(origin);
    _equipmentLayer->addChild(shade, -2);

    Size panelSize(visibleSize.width * 0.82f, visibleSize.height * 0.66f);
    Vec2 panelCenter(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.50f);
    std::string panelImage = AssetPaths::resolve("art/ui/equipment_panel.png");
    if (!panelImage.empty())
    {
        auto panel = Sprite::create(panelImage);
        if (panel)
        {
            Size imageSize = panel->getContentSize();
            if (imageSize.width > 0.0f && imageSize.height > 0.0f)
            {
                panel->setScale(std::min(panelSize.width / imageSize.width,
                    panelSize.height / imageSize.height));
            }
            panel->setPosition(panelCenter);
            _equipmentLayer->addChild(panel, -1);
        }
    }
    else
    {
        auto panel = LayerColor::create(Color4B(25, 31, 43, 245), panelSize.width, panelSize.height);
        panel->setIgnoreAnchorPointForPosition(false);
        panel->setAnchorPoint(Vec2(0.5f, 0.5f));
        panel->setPosition(panelCenter);
        _equipmentLayer->addChild(panel, -1);
    }

    auto title = Label::createWithSystemFont(
        textByLanguage("Available Equipment", u8"可选装备"), "Arial", 28.0f * s);
    title->setColor(Color3B(235, 242, 255));
    title->setPosition(panelCenter + Vec2(0, panelSize.height * 0.29f));
    _equipmentLayer->addChild(title, 2);

    auto hint = Label::createWithSystemFont(
        textByLanguage("Click an item above to place it into slots 1-4",
            u8"点击上方装备，将其放入1-4号槽位"),
        "Arial", 14.0f * s);
    hint->setColor(Color3B(170, 220, 230));
    hint->setPosition(panelCenter + Vec2(0, panelSize.height * 0.22f));
    _equipmentLayer->addChild(hint, 2);

    auto options = getWeaponOptions();
    Size iconSize(70.0f * s, 70.0f * s);
    float gap = 11.0f * s;
    float totalWidth = options.size() * iconSize.width + (options.size() - 1) * gap;
    float startX = panelCenter.x - totalWidth * 0.5f + iconSize.width * 0.5f;
    float topY = panelCenter.y + panelSize.height * 0.055f;
    Vector<MenuItem*> items;

    for (int i = 0; i < static_cast<int>(options.size()); ++i)
    {
        bool selected = std::find(_weaponLoadoutIds.begin(), _weaponLoadoutIds.end(), options[i].id) != _weaponLoadoutIds.end();
        auto normal = createEquipmentIcon(options[i], iconSize, selected);
        auto selectedNode = createEquipmentIcon(options[i], iconSize, true);
        int weaponId = options[i].id;
        auto item = MenuItemSprite::create(normal, selectedNode, [this, weaponId](Ref*) {
            AudioManager::getInstance()->playButtonClick();
            assignWeaponToSlot(weaponId);
        });
        item->setPosition(Vec2(startX + i * (iconSize.width + gap), topY));
        items.pushBack(item);
    }

    auto chooseMenu = Menu::createWithArray(items);
    chooseMenu->setPosition(Vec2::ZERO);
    _equipmentLayer->addChild(chooseMenu, 3);

    float slotY = panelCenter.y - panelSize.height * 0.205f;
    auto slotOptions = getWeaponOptions();
    for (int i = 0; i < 4; ++i)
    {
        auto number = Label::createWithSystemFont(std::to_string(i + 1), "Arial", 24.0f * s);
        number->setColor(Color3B(255, 230, 130));
        float slotCenterX = panelCenter.x - 198.0f * s + i * 132.0f * s;
        number->setPosition(Vec2(slotCenterX, slotY - 50.0f * s));
        _equipmentLayer->addChild(number, 2);

        WeaponOption option = slotOptions[std::max(0, std::min(6, _weaponLoadoutIds[i]))];
        Size slotIconSize(78.0f * s, 78.0f * s);
        auto slotIcon = createEquipmentIcon(option, slotIconSize, i == _nextEquipmentSlot);
        slotIcon->setPosition(Vec2(slotCenterX - slotIconSize.width * 0.5f, slotY - slotIconSize.height * 0.36f));
        _equipmentLayer->addChild(slotIcon, 2);
    }

    auto closeLabel = Label::createWithSystemFont(
        textByLanguage("Done", u8"完成"), "Arial", 24.0f * s);
    closeLabel->setColor(Color3B(140, 230, 170));
    auto closeItem = MenuItemLabel::create(closeLabel, [this](Ref*) {
        AudioManager::getInstance()->playButtonClick();
        hideEquipmentMenu();
    });
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(panelCenter + Vec2(0, -panelSize.height * 0.35f));
    _equipmentLayer->addChild(closeMenu, 4);
}

void GameScene::hideEquipmentMenu()
{
    if (_equipmentLayer)
    {
        _equipmentLayer->removeFromParentAndCleanup(true);
        _equipmentLayer = nullptr;
    }
    AudioManager::getInstance()->playGameBGM();
    _isPaused = false;
}

void GameScene::assignWeaponToSlot(int weaponId)
{
    if (_weaponLoadoutIds.size() < 4)
    {
        _weaponLoadoutIds = { 0, 1, 2, 3 };
    }

    int slot = std::max(0, std::min(3, _nextEquipmentSlot));
    _weaponLoadoutIds[slot] = weaponId;
    UserDefault::getInstance()->setIntegerForKey(("weapon_slot_" + std::to_string(slot)).c_str(), weaponId);
    UserDefault::getInstance()->flush();
    _nextEquipmentSlot = (_nextEquipmentSlot + 1) % 4;
    rebuildWeaponLoadout();

    if (_equipmentLayer)
    {
        _equipmentLayer->removeFromParentAndCleanup(true);
        _equipmentLayer = nullptr;
        showEquipmentMenu();
    }
}

void GameScene::initEnvironmentZones()
{
    struct ZoneDef
    {
        EnvironmentZoneType type;
        Rect rect;
    };

    ZoneDef defs[] = {
        { EnvironmentZoneType::Bed, Rect(70, 80, 190, 120) },
        { EnvironmentZoneType::Desk, Rect(_worldSize.width * 0.75f, _worldSize.height * 0.75f,
            _worldSize.width * 0.25f, _worldSize.height * 0.25f) },
        { EnvironmentZoneType::PowerSocket, Rect(_worldSize.width - 170, _worldSize.height - 210, 90, 90) },
        { EnvironmentZoneType::CoffeeArea, Rect(80, _worldSize.height - 205, 150, 100) }
    };

    for (const auto& def : defs)
    {
        auto* zone = EnvironmentZone::createZone(def.type, def.rect);
        if (zone)
        {
            (_worldLayer ? _worldLayer : this)->addChild(zone, -1);
            _environmentZones.push_back(zone);
        }
    }
}

void GameScene::updateEnvironmentEffects(float dt)
{
    if (!m_player) return;

    _nearDesk = false;
    _nearPowerSocket = false;
    std::string effectText = textByLanguage("Environment: None", u8"环境: 无");
    Rect playerBox = m_player->getBoundingBox();

    for (auto* zone : _environmentZones)
    {
        if (!zone || !playerBox.intersectsRect(zone->getCollisionBox()))
        {
            continue;
        }

        effectText = zone->getEffectText();
        switch (zone->getZoneType())
        {
        case EnvironmentZoneType::Bed:
            if (m_player->getMoodSystem())
            {
                m_player->getMoodSystem()->reduceMoodValue(10.0f * dt);
                m_player->getMoodSystem()->changeMood(MoodType::Exhausted, 2.0f, 1.25f);
            }
            effectText = textByLanguage("Near Bed: Exhausted - speed/cooldown",
                u8"靠近床: 疲惫 - 移速/冷却变差");
            AudioManager::getInstance()->playBedEffect();
            break;
        case EnvironmentZoneType::Desk:
            _nearDesk = true;
            if (m_player->getMoodSystem())
            {
                m_player->getMoodSystem()->addMoodValue(8.0f * dt);
                m_player->getMoodSystem()->changeMood(MoodType::Focus, 1.8f, 1.15f);
            }
            effectText = textByLanguage("Near Desk: Focus + progress",
                u8"靠近书桌: 专注 + 任务进度");
            AudioManager::getInstance()->playDeskEffect();
            break;
        case EnvironmentZoneType::PowerSocket:
            _nearPowerSocket = true;
            if (m_player->getMoodSystem())
            {
                m_player->getMoodSystem()->addMoodValue(4.0f * dt);
                m_player->getMoodSystem()->changeMood(MoodType::Focus, 1.2f, 1.05f);
            }
            effectText = textByLanguage("Near Socket: Laser range/damage +",
                u8"靠近插座: 激光范围/伤害提升");
            AudioManager::getInstance()->playSocketEffect();
            break;
        case EnvironmentZoneType::CoffeeArea:
            if (m_player->getMoodSystem())
            {
                m_player->getMoodSystem()->addMoodValue(6.0f * dt);
                m_player->getMoodSystem()->changeMood(MoodType::Excited, 2.5f, 1.2f);
            }
            effectText = textByLanguage("Near Coffee: Excited + speed/damage",
                u8"靠近咖啡: 兴奋 + 速度/伤害");
            break;
        }
    }

    if (_environmentLabel)
    {
        _environmentLabel->setString(effectText);
    }
}

void GameScene::updateAssignmentProgress(float dt)
{
    if (_isVictory || _isGameOver)
    {
        return;
    }

    if (_isEndlessMode)
    {
        if (_nearDesk)
        {
            _deskStayProgress += dt;
        }
        if (_freezeTimer <= 0.0f)
        {
            _ddlTimeRemaining -= dt;
        }

        if (_ddlTimeRemaining <= 0.0f)
        {
            ++_completedDdlCount;
            _ddlTimeLimit = 28.0f + std::min(18.0f, _completedDdlCount * 2.0f);
            _ddlTimeRemaining = _ddlTimeLimit;
            if (_topHintLabel)
            {
                _topHintLabel->setString(textByLanguage("DDL completed: ", u8"DDL已完成: ") +
                    std::to_string(_completedDdlCount));
            }
        }

        _assignmentProgress = static_cast<float>(_completedDdlCount);
        return;
    }

    if (_freezeTimer <= 0.0f)
    {
        _ddlTimeRemaining -= dt;
        if (_ddlTimeRemaining < 0.0f)
        {
            _ddlTimeRemaining = 0.0f;
        }
    }

    if (_ddlTimeRemaining <= 0.0f && _deskStayProgress < _deskStayRequired)
    {
        if (_topHintLabel)
        {
            _topHintLabel->setString(textByLanguage(
                "DDL time is over. Mission failed.",
                u8"DDL时间结束，任务失败。"));
        }
        goToGameOver();
        return;
    }

    if (_nearDesk)
    {
        _deskStayProgress += dt;
        if (_deskStayProgress > _deskStayRequired)
        {
            _deskStayProgress = _deskStayRequired;
        }
    }

    _assignmentProgress = _deskStayRequired > 0.0f
        ? (_deskStayProgress / _deskStayRequired) * 100.0f
        : 0.0f;
    if (_assignmentProgress > 100.0f)
    {
        _assignmentProgress = 100.0f;
    }

    if (_deskStayProgress >= _deskStayRequired)
    {
        _assignmentProgress = 100.0f;
        goToVictory();
    }
}

void GameScene::handleEndlessEnemyKilled(Enemy* enemy)
{
    if (!_isEndlessMode || !enemy)
    {
        return;
    }

    int scoreReward = getScoreRewardForEnemy(enemy);
    _endlessScore += scoreReward;

    if (_lifeOnKill > 0 && m_player && m_player->isRoleAlive())
    {
        m_player->heal(_lifeOnKill);
    }

    if (_topHintLabel)
    {
        std::string hint = textByLanguage("Kill reward: +", u8"击杀奖励: +") +
            std::to_string(enemy->getExpReward()) +
            textByLanguage(" EXP  +", u8"经验  +") +
            std::to_string(scoreReward) +
            textByLanguage(" Score", u8"分");
        _topHintLabel->setString(hint);
    }

    checkEndlessLevelUps();
}

int GameScene::getScoreRewardForEnemy(Enemy* enemy) const
{
    if (!enemy)
    {
        return 0;
    }

    const std::string name = enemy->getObjectName();
    if (name.find("Thesis") != std::string::npos ||
        name.find("Boss") != std::string::npos)
    {
        return 520;
    }
    if (name.find("DDL") != std::string::npos)
    {
        return 170;
    }
    if (name.find("Phone") != std::string::npos)
    {
        return 120;
    }
    if (name.find("Sleepy") != std::string::npos)
    {
        return 80;
    }

    return std::max(50, enemy->getExpReward() * 2);
}

void GameScene::checkEndlessLevelUps()
{
    if (!_isEndlessMode || !m_player || _upgradeLayer)
    {
        return;
    }

    if (m_player->getLevel() <= _lastHandledPlayerLevel)
    {
        return;
    }

    int nextLevel = _lastHandledPlayerLevel + 1;
    showUpgradeMenu(nextLevel % 5 == 0);
}

std::vector<GameScene::UpgradeChoice> GameScene::rollUpgradeChoices(bool major) const
{
    std::vector<UpgradeChoice> pool;
    if (major)
    {
        pool = {
            { UpgradeType::LifeOnKill,
                textByLanguage("Sustain Notes", u8"续航笔记"),
                textByLanguage("Every kill restores +2 HP.", u8"每次击杀恢复2点生命。"),
                true },
            { UpgradeType::WeaponMastery,
                textByLanguage("Weapon Mastery", u8"武器专精"),
                textByLanguage("Current weapon gains a special stronger bonus.", u8"当前武器获得专属强力提升。"),
                true }
        };
        return pool;
    }

    pool = {
        { UpgradeType::BulletDamage,
            textByLanguage("Sharper Bullets", u8"子弹强化"),
            textByLanguage("All weapons gain +4 bullet damage.", u8"所有武器子弹伤害+4。"),
            false },
        { UpgradeType::EnergyRecovery,
            textByLanguage("Fast Recharge", u8"快速回能"),
            textByLanguage("Weapon energy recovers 18% faster.", u8"攻击能量条恢复速度+18%。"),
            false },
        { UpgradeType::ProjectileCount,
            textByLanguage("Extra Shot", u8"弹幕增加"),
            textByLanguage("All projectile weapons fire one extra bullet.", u8"所有弹道武器额外发射1颗子弹。"),
            false },
        { UpgradeType::MaxHp,
            textByLanguage("Late-night Endurance", u8"熬夜耐力"),
            textByLanguage("Maximum HP +12 and heal 12 HP.", u8"生命值上限+12，并恢复12点生命。"),
            false },
        { UpgradeType::MoveSpeed,
            textByLanguage("Quick Steps", u8"灵活走位"),
            textByLanguage("Movement speed +18.", u8"移动速度+18。"),
            false }
    };

    std::vector<UpgradeChoice> choices;
    while (!pool.empty() && choices.size() < 2)
    {
        int index = static_cast<int>(CCRANDOM_0_1() * pool.size());
        if (index < 0) index = 0;
        if (index >= static_cast<int>(pool.size())) index = static_cast<int>(pool.size()) - 1;
        choices.push_back(pool[index]);
        pool.erase(pool.begin() + index);
    }
    return choices;
}

void GameScene::showUpgradeMenu(bool major)
{
    if (_upgradeLayer || !m_player)
    {
        return;
    }

    _isPaused = true;
    _keyW = _keyA = _keyS = _keyD = false;
    updateMoveDirection();
    _currentUpgradeChoices = rollUpgradeChoices(major);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    float s = Director::getInstance()->getWinSize().height / 640.0f;
    Vec2 center(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f);

    _upgradeLayer = Node::create();
    this->addChild(_upgradeLayer, 360);

    auto shade = LayerColor::create(Color4B(5, 8, 16, 205), visibleSize.width, visibleSize.height);
    shade->setPosition(origin);
    _upgradeLayer->addChild(shade, -1);

    auto title = Label::createWithSystemFont(
        major ? textByLanguage("Major Upgrade", u8"强力升级") : textByLanguage("Level Up", u8"升级"),
        "Arial", major ? 42.0f * s : 38.0f * s);
    title->setColor(major ? Color3B(255, 220, 110) : Color3B(160, 232, 255));
    title->setPosition(center + Vec2(0, 130.0f * s));
    _upgradeLayer->addChild(title, 2);

    char levelBuf[64];
    if (isChineseUi())
    {
        snprintf(levelBuf, sizeof(levelBuf), "等级 %d，选择一项强化", _lastHandledPlayerLevel + 1);
    }
    else
    {
        snprintf(levelBuf, sizeof(levelBuf), "Level %d: choose one upgrade", _lastHandledPlayerLevel + 1);
    }
    auto subtitle = Label::createWithSystemFont(levelBuf, "Arial", 18.0f * s);
    subtitle->setColor(Color3B(230, 235, 245));
    subtitle->setPosition(center + Vec2(0, 92.0f * s));
    _upgradeLayer->addChild(subtitle, 2);

    Vector<MenuItem*> items;
    for (int i = 0; i < static_cast<int>(_currentUpgradeChoices.size()); ++i)
    {
        const auto& choice = _currentUpgradeChoices[i];
        auto createChoiceCard = [choice, s]() {
            auto root = Node::create();
            Size boxSize(310.0f * s, 126.0f * s);
            root->setContentSize(boxSize);

            auto bg = LayerColor::create(choice.major ? Color4B(88, 67, 24, 235) : Color4B(30, 45, 64, 235),
                boxSize.width, boxSize.height);
            bg->setIgnoreAnchorPointForPosition(false);
            bg->setAnchorPoint(Vec2(0.5f, 0.5f));
            bg->setPosition(Vec2(boxSize.width * 0.5f, boxSize.height * 0.5f));
            root->addChild(bg);

            auto name = Label::createWithSystemFont(choice.title, "Arial", 22.0f * s);
            name->setColor(choice.major ? Color3B(255, 227, 130) : Color3B(165, 235, 255));
            name->setPosition(Vec2(boxSize.width * 0.5f, boxSize.height - 34.0f * s));
            root->addChild(name, 2);

            auto desc = Label::createWithSystemFont(choice.description, "Arial", 15.0f * s);
            desc->setColor(Color3B(232, 236, 242));
            desc->setDimensions(boxSize.width - 34.0f * s, 56.0f * s);
            desc->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
            desc->setPosition(Vec2(boxSize.width * 0.5f, 45.0f * s));
            root->addChild(desc, 2);
            return root;
        };

        auto root = createChoiceCard();
        auto selected = createChoiceCard();
        selected->setScale(0.97f);
        selected->setOpacity(230);
        auto item = MenuItemSprite::create(root, selected, [this, i](Ref*) {
            if (i >= 0 && i < static_cast<int>(_currentUpgradeChoices.size()))
            {
                applyUpgradeChoice(_currentUpgradeChoices[i]);
            }
        });
        items.pushBack(item);
    }

    auto menu = Menu::createWithArray(items);
    menu->setPosition(center + Vec2(0, -25.0f * s));
    menu->alignItemsHorizontallyWithPadding(34.0f * s);
    _upgradeLayer->addChild(menu, 4);
}

void GameScene::applyUpgradeChoice(const UpgradeChoice& choice)
{
    if (!m_player)
    {
        hideUpgradeMenu();
        return;
    }

    AudioManager::getInstance()->playUpgradeSelected();

    switch (choice.type)
    {
    case UpgradeType::BulletDamage:
        _weaponDamageBonus += 4;
        for (auto* weapon : _weapons)
        {
            if (weapon) weapon->addAttackPower(4);
        }
        break;
    case UpgradeType::EnergyRecovery:
        _energyRecoveryBonusPercent += 0.18f;
        for (auto* weapon : _weapons)
        {
            if (weapon) weapon->addEnergyRecoverPercent(0.18f);
        }
        break;
    case UpgradeType::ProjectileCount:
        ++_projectileBonus;
        for (auto* weapon : _weapons)
        {
            if (weapon) weapon->addProjectileCountBonus(1);
        }
        break;
    case UpgradeType::MaxHp:
        m_player->addMaxHp(12);
        m_player->heal(12);
        break;
    case UpgradeType::MoveSpeed:
        m_player->setBaseSpeed(m_player->getBaseSpeed() + 18.0f);
        break;
    case UpgradeType::LifeOnKill:
        _lifeOnKill += 2;
        break;
    case UpgradeType::WeaponMastery:
        applyWeaponMastery(_currentWeapon);
        break;
    }

    m_player->spendUpgradePoint();
    ++_lastHandledPlayerLevel;
    hideUpgradeMenu();
    checkEndlessLevelUps();
}

void GameScene::hideUpgradeMenu()
{
    if (_upgradeLayer)
    {
        _upgradeLayer->removeFromParentAndCleanup(true);
        _upgradeLayer = nullptr;
    }
    _currentUpgradeChoices.clear();
    _isPaused = false;
}

void GameScene::applyWeaponMastery(Weapon* weapon)
{
    if (!weapon)
    {
        return;
    }

    if (_currentWeaponIndex >= 0 && _currentWeaponIndex < static_cast<int>(_weaponLoadoutIds.size()))
    {
        int weaponId = _weaponLoadoutIds[_currentWeaponIndex];
        _masteredWeaponIds.push_back(weaponId);
    }

    applyWeaponMasteryEffects(weapon);

    const std::string name = weapon->getWeaponName();
    if (_topHintLabel)
    {
        _topHintLabel->setString(textByLanguage("Weapon mastery activated: ", u8"武器专精已激活: ") +
            weaponNameForUi(name));
    }
}

void GameScene::applyWeaponMasteryEffects(Weapon* weapon)
{
    if (!weapon)
    {
        return;
    }

    const std::string name = weapon->getWeaponName();
    if (name == "CoffeeGun")
    {
        weapon->addProjectileCountBonus(2);
        weapon->addEnergyRecoverPercent(0.25f);
    }
    else if (name == "CoffeeLaser")
    {
        weapon->addAttackPower(12);
        weapon->addMaxEnergy(25.0f);
    }
    else if (name == "KeyboardWave")
    {
        weapon->addProjectileCountBonus(2);
        weapon->addAttackPower(5);
    }
    else if (name == "KeyboardWeap")
    {
        weapon->addAttackPower(14);
        weapon->addEnergyRecoverPercent(0.18f);
    }
    else if (name == "DeskLampLaser")
    {
        weapon->addProjectileCountBonus(1);
        weapon->addAttackPower(10);
    }
    else
    {
        weapon->addAttackPower(10);
        weapon->addMaxEnergy(20.0f);
    }
}

void GameScene::applyEndlessGrowthToWeapon(Weapon* weapon, int weaponId)
{
    if (!_isEndlessMode || !weapon)
    {
        return;
    }

    if (_weaponDamageBonus > 0)
    {
        weapon->addAttackPower(_weaponDamageBonus);
    }
    if (_energyRecoveryBonusPercent > 0.0f)
    {
        weapon->addEnergyRecoverPercent(_energyRecoveryBonusPercent);
    }
    if (_projectileBonus > 0)
    {
        weapon->addProjectileCountBonus(_projectileBonus);
    }
    for (int masteredId : _masteredWeaponIds)
    {
        if (masteredId == weaponId)
        {
            applyWeaponMasteryEffects(weapon);
        }
    }
}

void GameScene::spawnRewardForEnemy(Enemy* enemy)
{
    if (!enemy)
    {
        return;
    }

    const std::string name = enemy->getObjectName();
    const Vec2 pos = enemy->getPosition();
    bool isBoss = name.find("Thesis") != std::string::npos ||
        name.find("Boss") != std::string::npos;

    if (!isBoss && CCRANDOM_0_1() > 0.42f)
    {
        return;
    }

    if (name.find("DDL") != std::string::npos ||
        name.find("Thesis") != std::string::npos ||
        name.find("Boss") != std::string::npos)
    {
        spawnReward(RewardType::Pen, pos);
    }
    else if (name.find("Phone") != std::string::npos)
    {
        spawnReward(RewardType::Power, pos);
    }
    else if (name.find("Sleepy") != std::string::npos)
    {
        spawnReward(RewardType::Coffee, pos);
    }

    if (CCRANDOM_0_1() < (isBoss ? 0.24f : 0.06f))
    {
        spawnReward(RewardType::FreezeDevice, pos + Vec2(28.0f, 12.0f));
    }
}

void GameScene::spawnReward(RewardType type, const Vec2& position)
{
    Node* node = createRewardNode(type);
    if (!node)
    {
        return;
    }

    node->setPosition(position);
    node->setScale(0.6f);
    node->runAction(ScaleTo::create(0.15f, 3.0f));
    (_worldLayer ? _worldLayer : this)->addChild(node, 8);
    _rewardPickups.push_back({ node, type });
}

Node* GameScene::createRewardNode(RewardType type)
{
    std::string path;
    Color4F color;
    std::string text;

    switch (type)
    {
    case RewardType::Coffee:
        path = "art/rewards/coffee.png";
        color = Color4F(0.65f, 0.36f, 0.18f, 1.0f);
        text = "C";
        break;
    case RewardType::Power:
        path = "art/rewards/power.png";
        color = Color4F(0.95f, 0.85f, 0.18f, 1.0f);
        text = "E";
        break;
    case RewardType::Pen:
        path = "art/rewards/pen.png";
        color = Color4F(0.25f, 0.62f, 0.95f, 1.0f);
        text = "P";
        break;
    case RewardType::FreezeDevice:
    default:
        path = "art/rewards/freeze_device.png";
        color = Color4F(0.45f, 0.90f, 1.0f, 1.0f);
        text = "F";
        break;
    }

    path = AssetPaths::resolve(path);
    if (!path.empty())
    {
        auto sprite = Sprite::create(path);
        if (sprite)
        {
            auto node = Node::create();
            applySpriteFit(sprite, 30.0f, 30.0f);
            node->addChild(sprite);
            node->setContentSize(Size(30.0f, 30.0f));
            return node;
        }
    }

    auto node = Node::create();
    auto draw = DrawNode::create();
    draw->drawSolidCircle(Vec2::ZERO, 15.0f, 0.0f, 32, color);
    draw->drawCircle(Vec2::ZERO, 15.0f, 0.0f, 32, false, Color4F(1, 1, 1, 0.85f));
    node->addChild(draw);

    auto label = Label::createWithSystemFont(text, "Arial", 14);
    label->setColor(Color3B::WHITE);
    node->addChild(label);
    node->setContentSize(Size(30.0f, 30.0f));
    return node;
}

void GameScene::updateRewards(float dt)
{
    if (!m_player)
    {
        return;
    }

    _rewardSpawnTimer -= dt;
    if (_rewardSpawnTimer <= 0.0f)
    {
        if (_rewardPickups.size() >= 8)
        {
            _rewardSpawnTimer = 6.0f;
            return;
        }

        RewardType type = RewardType::Coffee;
        int roll = static_cast<int>(CCRANDOM_0_1() * 4.0f);
        if (roll == 1) type = RewardType::Power;
        else if (roll == 2) type = RewardType::Pen;
        else if (roll == 3) type = RewardType::FreezeDevice;

        Vec2 pos(70.0f + CCRANDOM_0_1() * (_worldSize.width - 140.0f),
            70.0f + CCRANDOM_0_1() * (_worldSize.height - 140.0f));
        spawnReward(type, pos);
        _rewardSpawnTimer = 11.0f + CCRANDOM_0_1() * 7.0f;
    }

    Rect playerBox = m_player->getBoundingBox();
    for (auto it = _rewardPickups.begin(); it != _rewardPickups.end(); )
    {
        Node* node = it->node;
        if (!node || !node->getParent())
        {
            it = _rewardPickups.erase(it);
            continue;
        }

        Rect rewardBox(node->getPositionX() - 18.0f, node->getPositionY() - 18.0f, 36.0f, 36.0f);
        if (playerBox.intersectsRect(rewardBox))
        {
            applyReward(it->type);
            node->stopAllActions();
            node->removeFromParentAndCleanup(true);
            it = _rewardPickups.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void GameScene::applyReward(RewardType type)
{
    if (!m_player)
    {
        return;
    }

    AudioManager::getInstance()->playRewardPickup();

    std::string hint;
    if (type == RewardType::FreezeDevice)
    {
        _freezeTimer = 4.0f;
        hint = textByLanguage("Freeze device collected: monsters stop moving",
            u8"获得冻结仪: 怪物停止行动");
    }
    else
    {
        int heal = 10;
        if (type == RewardType::Power) heal = 12;
        if (type == RewardType::Pen) heal = 15;

        int nextHp = std::min(m_player->getMaxHp(), m_player->getHp() + heal);
        m_player->setHp(nextHp);

        if (m_player->getMoodSystem())
        {
            m_player->getMoodSystem()->addMoodValue(5.0f);
        }

        if (type == RewardType::Coffee)
        {
            hint = textByLanguage("Coffee collected: HP +10", u8"获得咖啡: 生命 +10");
        }
        else if (type == RewardType::Power)
        {
            hint = textByLanguage("Power collected: HP +12", u8"获得电量: 生命 +12");
        }
        else
        {
            hint = textByLanguage("Pen collected: HP +15", u8"获得笔: 生命 +15");
        }
    }

    if (_topHintLabel)
    {
        _topHintLabel->setString(hint);
    }
}

void GameScene::updateFreezeEffect(float dt)
{
    if (!_waveManager)
    {
        return;
    }

    if (_freezeTimer > 0.0f)
    {
        _freezeTimer -= dt;
        _waveManager->setFrozen(true);
        if (_topHintLabel)
        {
            char buf[96];
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "冻结生效中: %.1f秒  现在可以安全攻击怪物", _freezeTimer);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Freeze active: %.1fs  You can attack safely", _freezeTimer);
            }
            _topHintLabel->setString(buf);
        }
    }
    else if (_waveManager->isFrozen())
    {
        _freezeTimer = 0.0f;
        _waveManager->setFrozen(false);
        if (_topHintLabel)
        {
            _topHintLabel->setString(defaultControlHint());
        }
    }
}

std::string GameScene::getMoodPlayerImagePath(MoodType mood) const
{
    switch (mood)
    {
    case MoodType::Focus:
        return AssetPaths::resolve("art/characters/player_mood_focus.png");
    case MoodType::Irritable:
        return AssetPaths::resolve("art/characters/player_mood_irritable.png");
    case MoodType::Exhausted:
        return AssetPaths::resolve("art/characters/player_mood_exhausted.png");
    case MoodType::Excited:
        return AssetPaths::resolve("art/characters/player_mood_excited.png");
    case MoodType::Fear:
        return AssetPaths::resolve("art/characters/player_mood_fear.png");
    case MoodType::Calm:
        return AssetPaths::resolve("art/characters/player_mood_calm.png");
    case MoodType::Panic:
        return AssetPaths::resolve("art/characters/player_mood_panic.png");
    case MoodType::Normal:
    default:
        return AssetPaths::resolve("art/characters/player_mood_normal.png");
    }
}

void GameScene::updatePlayerMoodVisual()
{
    if (!m_player)
    {
        return;
    }

    MoodType mood = m_player->getCurrentMood();
    std::string path = getMoodPlayerImagePath(mood);
    if (path.empty())
    {
        path = AssetPaths::resolve("art/characters/player.png");
    }
    if (path.empty() || path == _currentPlayerMoodImage)
    {
        return;
    }

    auto texture = Director::getInstance()->getTextureCache()->addImage(path);
    if (texture)
    {
        m_player->setTexture(texture);
        applySpriteFit(m_player, 153.0f, 153.0f);
        _currentPlayerMoodImage = path;
    }
}

int GameScene::calculateScore() const
{
    int kills = _waveManager ? _waveManager->getKillCount() : 0;
    if (_isEndlessMode)
    {
        return static_cast<int>(m_survivalTime) * 8
            + _completedDdlCount * 800
            + _endlessScore;
    }

    return static_cast<int>(m_survivalTime) * 10
        + static_cast<int>(_assignmentProgress) * 50
        + kills * 100;
}

void GameScene::goToGameOver()
{
    if (_isGameOver) return;
    _isGameOver = true;
    if (_waveManager) _waveManager->stopSpawn();

    int kills = _waveManager ? _waveManager->getKillCount() : 0;
    int progress = _isEndlessMode ? _completedDdlCount : static_cast<int>(_assignmentProgress);
    int score = calculateScore();

    auto delay = DelayTime::create(0.5f);
    auto call = CallFunc::create([this, kills, progress, score]() {
        auto gameOverScene = GameOverScene::createScene(m_survivalTime, kills, progress, score);
        Director::getInstance()->replaceScene(gameOverScene);
    });
    this->runAction(Sequence::create(delay, call, nullptr));
}

void GameScene::goToVictory()
{
    if (_isVictory || _isGameOver) return;
    _isVictory = true;
    if (_waveManager) _waveManager->stopSpawn();

    auto ud = UserDefault::getInstance();
    int unlocked = ud->getIntegerForKey("unlocked_level", 1);
    if (_levelNumber >= unlocked)
    {
        ud->setIntegerForKey("unlocked_level", _levelNumber + 1);
        ud->flush();
    }

    // --- Story mode auto-save: advance to next level ---
    if (!_isEndlessMode)
    {
        StoryModeScene::addAutoSave(_levelNumber + 1);
    }

    int kills = _waveManager ? _waveManager->getKillCount() : 0;
    int score = calculateScore();
    AudioManager::getInstance()->playProgressComplete();
    Director::getInstance()->replaceScene(
        VictoryScene::createScene(m_survivalTime, kills, 100, score));
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

    Size titleSize(360.0f * s, 72.0f * s);
    Size buttonSize(330.0f * s, 58.0f * s);
    auto title = createUiImageOrLabel("art/ui/pause_title.png", lm->getString("pause_title"),
        titleSize, 42.0f * s, Color3B(220, 220, 240));
    title->setPosition(Vec2(cx - titleSize.width * 0.5f, cy + 135.0f * s));
    _pauseLayer->addChild(title);

    auto resumeItem = createUiImageButton("art/ui/pause_resume.png", lm->getString("resume"),
        buttonSize, 30.0f * s, Color3B(100, 220, 100),
        CC_CALLBACK_1(GameScene::onPauseResumeClicked, this));

    auto restartItem = createUiImageButton("art/ui/pause_restart.png", lm->getString("restart"),
        buttonSize, 30.0f * s, Color3B(220, 200, 100),
        CC_CALLBACK_1(GameScene::onPauseRestartClicked, this));

    auto titleItem = createUiImageButton("art/ui/pause_back_to_menu.png", lm->getString("back_to_title"),
        buttonSize, 30.0f * s, Color3B(200, 140, 120),
        CC_CALLBACK_1(GameScene::onPauseTitleClicked, this));

    auto settingsItem = createUiImageButton("art/ui/pause_settings.png", lm->getString("settings"),
        buttonSize, 30.0f * s, Color3B(160, 180, 220),
        CC_CALLBACK_1(GameScene::onPauseSettingsClicked, this));

    if (resumeItem && restartItem && settingsItem && titleItem)
    {
        Vector<MenuItem*> pauseItems;
        pauseItems.pushBack(resumeItem);
        pauseItems.pushBack(restartItem);

        auto saveItem = createUiImageButton("art/ui/pause_save.png",
            lm->getString("save_game"),
            buttonSize, 30.0f * s, Color3B(100, 180, 240),
            CC_CALLBACK_1(GameScene::onPauseSaveClicked, this));
        if (saveItem)
            pauseItems.pushBack(saveItem);

        pauseItems.pushBack(titleItem);
        pauseItems.pushBack(settingsItem);

        auto menu = Menu::createWithArray(pauseItems);
        menu->setPosition(Vec2(cx, cy - 45.0f * s));
        menu->alignItemsVerticallyWithPadding(12.0f * s);
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

void GameScene::onPauseSaveClicked(Ref*)
{
    const int saveLevel = (_isEndlessMode && m_player) ? m_player->getLevel() : _levelNumber;
    if (!StoryModeScene::addManualSave(saveLevel, _isEndlessMode))
    {
        // Save slots full — show a brief hint
        auto* lm = LanguageManager::getInstance();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();
        auto hint = Label::createWithSystemFont(lm->getString("save_full"), "Arial", 22.0f);
        hint->setColor(Color3B(255, 150, 100));
        hint->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height * 0.25f));
        hint->setName("save_full_hint");
        _pauseLayer->addChild(hint, 10);

        // Auto-remove after 2 seconds
        auto delay = DelayTime::create(2.0f);
        auto remove = CallFunc::create([hint]() {
            hint->removeFromParent();
        });
        hint->runAction(Sequence::create(delay, remove, nullptr));
        return;
    }

    hidePauseMenu();
}
