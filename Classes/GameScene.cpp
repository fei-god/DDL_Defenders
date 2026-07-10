#include "GameScene.h"
#include "GameOverScene.h"
#include "MainMenuScene.h"
#include "SettingsScene.h"
#include "StoryModeScene.h"
#include "DeskUpgradeLayer.h"

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
#include "base/CCEventListenerTouch.h"
#include "ui/UIScrollView.h"
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
                        float scaleX = visualWidth / imageSize.width;
                        float scaleY = targetSize.height * 1.30f / imageSize.height;
                        sprite->setScale(std::min(scaleX, scaleY));
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

    Sprite* createFloatingInteractionArrow(float targetHeight)
    {
        std::string arrowPath = AssetPaths::resolve("highlight arrow.png");
        if (arrowPath.empty())
        {
            return nullptr;
        }

        auto arrow = Sprite::create(arrowPath);
        if (!arrow)
        {
            return nullptr;
        }

        Size arrowSize = arrow->getContentSize();
        if (arrowSize.height > 0.0f)
        {
            arrow->setScale(targetHeight / arrowSize.height);
        }
        arrow->setOpacity(215);
        arrow->runAction(RepeatForever::create(Sequence::create(
            EaseSineInOut::create(MoveBy::create(0.55f, Vec2(0.0f, 10.0f))),
            EaseSineInOut::create(MoveBy::create(0.55f, Vec2(0.0f, -10.0f))),
            nullptr)));
        return arrow;
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

    Sprite* createFittedSprite(const std::string& imagePath, const Size& bounds)
    {
        std::string resolved = AssetPaths::resolve(imagePath);
        auto sprite = resolved.empty() ? nullptr : Sprite::create(resolved);
        if (!sprite)
            return nullptr;

        Size imageSize = sprite->getContentSize();
        if (imageSize.width > 0.0f && imageSize.height > 0.0f)
        {
            sprite->setScale(std::min(bounds.width / imageSize.width,
                bounds.height / imageSize.height));
        }
        sprite->setPosition(Vec2(bounds.width * 0.5f, bounds.height * 0.5f));
        return sprite;
    }

    LayerColor* createPausePanel(const Size& size)
    {
        auto panel = LayerColor::create(Color4B(5, 14, 28, 178), size.width, size.height);
        auto topLine = LayerColor::create(Color4B(178, 139, 72, 150), size.width, 2.0f);
        topLine->setPosition(Vec2(0.0f, size.height - 2.0f));
        panel->addChild(topLine, 1);
        auto bottomLine = LayerColor::create(Color4B(48, 69, 94, 110), size.width, 1.0f);
        panel->addChild(bottomLine, 1);
        return panel;
    }

    Node* createPauseSectionTitle(const std::string& text, const Size& size, float fontSize)
    {
        auto root = Node::create();
        root->setContentSize(size);
        auto bg = LayerColor::create(Color4B(18, 33, 54, 218), size.width, size.height);
        root->addChild(bg, 0);
        auto line = LayerColor::create(Color4B(203, 158, 78, 180), size.width, 2.0f);
        line->setPosition(Vec2(0.0f, size.height - 2.0f));
        root->addChild(line, 1);
        auto label = Label::createWithSystemFont(text, "Arial", fontSize);
        label->setColor(Color3B(236, 219, 169));
        label->enableOutline(Color4B(18, 20, 28, 180), 1);
        label->setPosition(Vec2(size.width * 0.5f, size.height * 0.52f));
        root->addChild(label, 2);
        return root;
    }

    Node* createPauseInfoRow(const std::string& name, const std::string& value,
        const Size& size, float fontSize)
    {
        auto root = Node::create();
        root->setContentSize(size);
        auto bg = LayerColor::create(Color4B(15, 28, 46, 188), size.width, size.height);
        root->addChild(bg, 0);
        auto accent = LayerColor::create(Color4B(177, 138, 74, 92), 3.0f, size.height);
        root->addChild(accent, 1);

        auto nameLabel = Label::createWithSystemFont(name, "Arial", fontSize);
        nameLabel->setColor(Color3B(205, 216, 226));
        nameLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
        nameLabel->setPosition(Vec2(14.0f, size.height * 0.52f));
        root->addChild(nameLabel, 2);

        auto valueLabel = Label::createWithSystemFont(value, "Arial", fontSize);
        valueLabel->setColor(Color3B(246, 218, 142));
        valueLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
        valueLabel->setPosition(Vec2(size.width - 14.0f, size.height * 0.52f));
        root->addChild(valueLabel, 2);
        return root;
    }

    Node* createPauseItemRow(const std::string& slotText, const std::string& name,
        const std::string& imagePath, const Size& size, float iconSize, float fontSize)
    {
        auto root = Node::create();
        root->setContentSize(size);
        auto bg = LayerColor::create(Color4B(16, 30, 48, 190), size.width, size.height);
        root->addChild(bg, 0);

        auto iconBg = LayerColor::create(Color4B(5, 12, 22, 170), iconSize, iconSize);
        iconBg->setPosition(Vec2(10.0f, (size.height - iconSize) * 0.5f));
        root->addChild(iconBg, 1);

        auto icon = createFittedSprite(imagePath, Size(iconSize - 8.0f, iconSize - 8.0f));
        if (icon)
        {
            icon->setPosition(Vec2(10.0f + iconSize * 0.5f, size.height * 0.5f));
            root->addChild(icon, 2);
        }

        auto slot = Label::createWithSystemFont(slotText, "Arial", fontSize);
        slot->setColor(Color3B(237, 202, 123));
        slot->setAnchorPoint(Vec2(0.0f, 0.5f));
        slot->setPosition(Vec2(20.0f + iconSize, size.height * 0.52f));
        root->addChild(slot, 2);

        auto nameLabel = Label::createWithSystemFont(name, "Arial", fontSize);
        nameLabel->setColor(Color3B(218, 226, 236));
        nameLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
        nameLabel->setPosition(Vec2(72.0f + iconSize, size.height * 0.52f));
        root->addChild(nameLabel, 2);
        return root;
    }

    Node* createPauseActionButton(const std::string& imagePath, const std::string& text,
        const Size& iconBounds, float fontSize, const ccMenuCallback& callback)
    {
        auto root = Node::create();
        root->setContentSize(Size(iconBounds.width, iconBounds.height + fontSize + 18.0f));

        auto normal = createUiImageOrLabel(imagePath, text, iconBounds, fontSize, Color3B(235, 220, 178));
        auto selected = createUiImageOrLabel(imagePath, text, iconBounds, fontSize, Color3B(255, 238, 190));
        selected->setScale(0.95f);
        auto item = MenuItemSprite::create(normal, selected, callback);
        item->setPosition(Vec2(iconBounds.width * 0.5f,
            root->getContentSize().height - iconBounds.height * 0.5f));
        auto menu = Menu::create(item, nullptr);
        menu->setPosition(Vec2::ZERO);
        root->addChild(menu, 2);

        auto label = Label::createWithSystemFont(text, "Arial", fontSize);
        label->setColor(Color3B(226, 218, 198));
        label->enableOutline(Color4B(10, 13, 20, 180), 1);
        label->setPosition(Vec2(iconBounds.width * 0.5f, fontSize * 0.52f));
        root->addChild(label, 3);
        return root;
    }

    bool isChineseUi()
    {
        return LanguageManager::getInstance()->getLanguage() ==
            LanguageManager::Language::SIMPLIFIED_CHINESE;
    }

    bool hasWeaponId(const std::vector<int>& ids, int weaponId)
    {
        return std::find(ids.begin(), ids.end(), weaponId) != ids.end();
    }

    void pushUniqueWeapon(std::vector<int>& ids, int weaponId, int maxCount = -1)
    {
        if (weaponId < 0 || weaponId > 6 || hasWeaponId(ids, weaponId))
        {
            return;
        }
        if (maxCount >= 0 && static_cast<int>(ids.size()) >= maxCount)
        {
            return;
        }
        ids.push_back(weaponId);
    }

    void normalizeWeaponLists(std::vector<int>& equipped, std::vector<int>& backpack)
    {
        std::vector<int> oldEquipped = equipped;
        std::vector<int> oldBackpack = backpack;
        equipped.clear();

        for (int weaponId : oldEquipped)
        {
            pushUniqueWeapon(equipped, weaponId, 2);
        }
        for (int weaponId : oldBackpack)
        {
            pushUniqueWeapon(equipped, weaponId, 2);
        }
        if (equipped.empty() && oldBackpack.empty())
        {
            equipped = { 0, 1 };
        }
        while (static_cast<int>(equipped.size()) < 2)
        {
            for (int weaponId = 0; weaponId <= 6; ++weaponId)
            {
                if (!hasWeaponId(equipped, weaponId))
                {
                    equipped.push_back(weaponId);
                    break;
                }
            }
        }

        backpack.clear();
        for (int weaponId : oldBackpack)
        {
            if (!hasWeaponId(equipped, weaponId))
            {
                pushUniqueWeapon(backpack, weaponId, 4);
            }
        }
        for (int weaponId : oldEquipped)
        {
            if (!hasWeaponId(equipped, weaponId))
            {
                pushUniqueWeapon(backpack, weaponId, 4);
            }
        }
        for (int weaponId = 0; weaponId <= 6; ++weaponId)
        {
            if (!hasWeaponId(equipped, weaponId))
            {
                pushUniqueWeapon(backpack, weaponId, 4);
            }
        }
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
            "WASD move  Auto-attack  Esc pause",
            u8"WASD移动  自动攻击  Esc暂停");
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

    // --- Read scene config early (needed for background & hub routing) ---
    auto* ud = UserDefault::getInstance();
    _sceneId = ud->getIntegerForKey("selected_scene", 0);
    _isHubScene = (_sceneId == 0);
    _isEndlessMode = ud->getIntegerForKey("selected_game_mode", 0) == 1;
    _isBossScene = (_sceneId == 3);
    _levelNumber = ud->getIntegerForKey("selected_level", 1);
    if (_levelNumber < 1) _levelNumber = 1;
    _ddlTimeLimit = 999.0f;
    _ddlTimeRemaining = _ddlTimeLimit;
    _completedDdlCount = 0;
    _assignmentProgress = 0.0f;
    _thesisProgress = 0.0f;
    _ddlPressure = 0.0f;

    // --- Background (scene-based routing) ---
    const char* bgNames[] = {"art/backgrounds/dorm_room.png", "art/backgrounds/library.png",
        "art/backgrounds/class_room.png", "art/backgrounds/office_room.png"};
    std::string bgPath = AssetPaths::resolve(bgNames[_sceneId]);
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
        playerStart, 160, 250.0f, 8);

    if (m_player)
    {
        _worldLayer->addChild(m_player, 100);
        m_player->setLocalZOrder(100);
        applySpriteFit(m_player, 306.0f, 306.0f);
        {
            const float sourceSize = 1254.0f;
            const Rect playerAlphaUnion(357.0f, 18.0f, 691.0f, 1019.0f);
            Size playerContent = m_player->getContentSize();
            float fitScale = std::min(playerContent.width / sourceSize,
                playerContent.height / sourceSize);
            float fittedWidth = sourceSize * fitScale;
            float fittedHeight = sourceSize * fitScale;
            float offsetX = (playerContent.width - fittedWidth) * 0.5f;
            float offsetY = (playerContent.height - fittedHeight) * 0.5f;
            m_player->setCollisionLocalBox(Rect(
                offsetX + playerAlphaUnion.origin.x * fitScale,
                offsetY + playerAlphaUnion.origin.y * fitScale,
                playerAlphaUnion.size.width * fitScale,
                playerAlphaUnion.size.height * fitScale));
        }

        // Character animation sprites (replaces the triangle)
        _characterAnimTimer = 0.0f;
        _characterWasMoving = false;

        auto charTex1 = Director::getInstance()->getTextureCache()
            ->addImage(AssetPaths::resolve("art/characters/character1.png"));
        auto charTex2 = Director::getInstance()->getTextureCache()
            ->addImage(AssetPaths::resolve("art/characters/character2.png"));

        _characterSprite1 = Sprite::createWithTexture(charTex1);
        _characterSprite2 = Sprite::createWithTexture(charTex2);

        applySpriteFit(_characterSprite1, 306.0f, 306.0f);
        applySpriteFit(_characterSprite2, 306.0f, 306.0f);

        _characterSprite1->setPosition(Vec2::ZERO);
        _characterSprite2->setPosition(Vec2::ZERO);
        _characterSprite2->setVisible(false);  // Start showing frame 1

        m_player->addChild(_characterSprite1, 2);
        m_player->addChild(_characterSprite2, 2);

        // Walk animation sprites
        auto walkTex1 = Director::getInstance()->getTextureCache()
            ->addImage(AssetPaths::resolve("art/characters/characterwalk1.png"));
        auto walkTex2 = Director::getInstance()->getTextureCache()
            ->addImage(AssetPaths::resolve("art/characters/characterwalk2.png"));

        _characterWalkSprite1 = Sprite::createWithTexture(walkTex1);
        _characterWalkSprite2 = Sprite::createWithTexture(walkTex2);

        applySpriteFit(_characterWalkSprite1, 306.0f, 306.0f);
        applySpriteFit(_characterWalkSprite2, 306.0f, 306.0f);

        _characterWalkSprite1->setPosition(Vec2::ZERO);
        _characterWalkSprite2->setPosition(Vec2::ZERO);
        _characterWalkSprite1->setVisible(false);
        _characterWalkSprite2->setVisible(false);

        m_player->addChild(_characterWalkSprite1, 2);
        m_player->addChild(_characterWalkSprite2, 2);

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
    _equipmentLayer = nullptr;
    _endlessStatsLabel = nullptr;
    _endlessScore = 0;
    _lifeOnKill = 0;
    _weaponDamageBonus = 0;
    _projectileBonus = 0;
    _energyRecoveryBonusPercent = 0.0f;
    _masteredWeaponIds.clear();
    for (int i = 0; i < 4; ++i)
    {
        int bid = ud->getIntegerForKey(("weapon_backpack_" + std::to_string(i)).c_str(), -1);
        if (bid >= 0) _backpackWeaponIds.push_back(bid);
    }
    for (int i = 0; i < 2; ++i)
    {
        int wid = ud->getIntegerForKey(("weapon_equipped_" + std::to_string(i)).c_str(), -1);
        if (wid >= 0) _weaponLoadoutIds.push_back(wid);
    }
    // Default on first launch only — respect user's empty choice
    if (_weaponLoadoutIds.empty() && _backpackWeaponIds.empty())
        _weaponLoadoutIds = {0, 1};
    normalizeWeaponLists(_weaponLoadoutIds, _backpackWeaponIds);

    _levelIntroLayer = nullptr;
    _levelIntroActive = false;
    _levelIntroTimer = 0.0f;
    initSceneConfig();

    // Restore player state carried over from previous scene (Hub ↔ Combat)
    if (m_player)
    {
        int savedLevel = ud->getIntegerForKey("player_level", 1);
        if (savedLevel > 1) // Has progression carried over
        {
            m_player->setLevel(savedLevel);
            int savedMaxHp = ud->getIntegerForKey("player_max_hp", 100);
            if (savedMaxHp > 100) m_player->setMaxHp(savedMaxHp);
            m_player->setHp(m_player->getMaxHp()); // full heal on scene entry
            float savedSpeed = ud->getFloatForKey("player_speed", 240.0f);
            if (savedSpeed > 240.0f) m_player->setBaseSpeed(savedSpeed);
            int savedPts = ud->getIntegerForKey("player_upgrade_points", 0);
            for (int i = 0; i < savedPts; ++i) m_player->addUpgradePoint(1);

            _endlessScore = ud->getIntegerForKey("continue_score", 0);
            _weaponDamageBonus = ud->getIntegerForKey("weapon_damage_bonus", 0);
            _energyRecoveryBonusPercent = ud->getFloatForKey("energy_recovery_bonus", 0.0f);
            _projectileBonus = ud->getIntegerForKey("projectile_bonus", 0);
            _lifeOnKill = ud->getIntegerForKey("life_on_kill", 0);
        }
    }
    if (_isEndlessMode && _levelNumber > 1 && m_player && !_isHubScene)
    {
        m_player->setLevel(_levelNumber);
    }

    // Build weapons AFTER restoring bonuses so they get applied
    rebuildWeaponLoadout();

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

    if (_waveManager)
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

        // --- Scene-specific wave config ---
        if (!_isHubScene)
        {
            if (!_isEndlessMode)
            {
                // ============ Story Mode ============
                if (_sceneId == 1) // Library: continuous DDL spawn, survive 90s + kill 10
                {
                    _waveManager->setAllowedTypes({1}); // DDLMonster only
                    _waveManager->setTotalWaves(9999);  // effectively infinite
                    _waveManager->setWaveTimerExpiredCallback([this](int wave) {
                        // Advance wave number so difficulty keeps increasing
                        _waveManager->startWave(wave + 1);
                    });
                }
                else if (_sceneId == 2) // Classroom: 4 waves
                {
                    _waveManager->setAllowedTypes({0, 1}); // Sleepy + DDL
                    _waveManager->setTotalWaves(4);
                    _waveManager->setWaveDuration(25.0f);
                    _waveManager->setWaveTimerExpiredCallback([this](int wave) {
                        if (wave < 4) {
                            _waveManager->startWave(wave + 1);
                        } else {
                            _waveManager->stopSpawn();
                        }
                    });
                    _waveManager->setAllWavesClearedCallback([this]() {
                        goToVictory();
                    });
                }
                else if (_sceneId == 3) // Office Boss
                {
                    _waveManager->setAllowedTypes({2, 3}); // ThesisBoss + PhoneMonster
                    _waveManager->setTotalWaves(9999);
                    _waveManager->setWaveTimerExpiredCallback([this](int wave) {
                        // Restart: boss fight is continuous
                        _waveManager->startWave(wave);
                    });
                }
            }
            else
            {
                // Endless: continuous wave cycling
                _waveManager->setTotalWaves(9999);
                _waveManager->setWaveTimerExpiredCallback([this](int wave) {
                    // Advance wave: spawn difficulty scales with survival time + wave number
                    _waveManager->startWave(wave + 1);
                });
            }
            int startingWave = _isEndlessMode ? 1 : ((_levelNumber + 1) / 2);
            if (startingWave < 1) startingWave = 1;
            _waveManager->startWave(startingWave);
        }
    }

    _hpBarBg = nullptr;
    _hpBarFill = nullptr;
    _hudPanelBg = nullptr;
    _moodLabel = nullptr;
    _weaponIcon = nullptr;
    _weaponLabel = nullptr;
    _weaponEnergyBg = nullptr;
    _weaponEnergyFill = nullptr;
    _progressLabel = nullptr;
    _taskLabel = nullptr;
    _taskBarBg = nullptr;
    _taskBarFill = nullptr;
    _environmentLabel = nullptr;
    _survivalTimeLabel = nullptr;
    _endlessStatsLabel = nullptr;
    _expBarBg = nullptr;
    _expBarFill = nullptr;
    _expLevelLabel = nullptr;
    _expFractionLabel = nullptr;
    _waveTimerLabel = nullptr;
    _hpBarMaxWidth = 0.0f;
    _weaponEnergyBarMaxWidth = 0.0f;
    _taskBarMaxWidth = 0.0f;
    _expBarMaxWidth = 0.0f;

    if (!_isHubScene)
    {
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
    }

    _weaponSlotNodes.clear();
    _lastWeaponSlotIds.clear();
    Size slotSize(76.0f * s, 76.0f * s);
    float slotGap = 12.0f * s;
    float slotStartX = origin.x + visibleSize.width * 0.5f - (slotSize.width * 2.0f + slotGap * 1.0f) * 0.5f;
    float slotY = origin.y + 24.0f * s;
    for (int i = 0; i < 2; ++i)
    {
        auto slot = Node::create();
        slot->setContentSize(slotSize);
        slot->setPosition(Vec2(slotStartX + i * (slotSize.width + slotGap), slotY));
        this->addChild(slot, 22);
        _weaponSlotNodes.push_back(slot);
    }

    m_survivalTime = 0.0f;
    if (!_isHubScene)
    {
        // --- Survival time ---
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

        // --- EXP bar ---
        _expBarMaxWidth = 200.0f * s;
        float expBarY = origin.y + visibleSize.height - 58.0f * s;
        _expBarBg = LayerColor::create(Color4B(40, 40, 55, 200), _expBarMaxWidth, 14.0f * s);
        _expBarBg->setPosition(Vec2(origin.x + visibleSize.width / 2 - _expBarMaxWidth / 2, expBarY));
        this->addChild(_expBarBg, 10);

        _expBarFill = LayerColor::create(Color4B(80, 200, 120, 255), _expBarMaxWidth * 0.5f, 14.0f * s);
        _expBarFill->setPosition(Vec2(0, 0));
        _expBarBg->addChild(_expBarFill);

        _expLevelLabel = Label::createWithSystemFont("Lv.1", "Arial", 16.0f * s);
        _expLevelLabel->setColor(Color3B(200, 220, 100));
        _expLevelLabel->setAnchorPoint(Vec2(0, 0.5f));
        _expLevelLabel->setPosition(Vec2(origin.x + visibleSize.width / 2 + _expBarMaxWidth / 2 + 8.0f * s, expBarY + 7.0f * s));
        this->addChild(_expLevelLabel, 10);

        _expFractionLabel = Label::createWithSystemFont("0/100 EXP", "Arial", 11.0f * s);
        _expFractionLabel->setColor(Color3B(180, 180, 200));
        _expFractionLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, expBarY - 18.0f * s));
        this->addChild(_expFractionLabel, 10);

        // --- Wave timer ---
        _waveTimerLabel = Label::createWithSystemFont("", "Arial", 20.0f * s);
        _waveTimerLabel->setColor(Color3B(255, 200, 100));
        _waveTimerLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, expBarY - 38.0f * s));
        _waveTimerLabel->setVisible(false);
        this->addChild(_waveTimerLabel, 10);
    }

    _topHintLabel = Label::createWithSystemFont(
        defaultControlHint(),
        "Arial",
        16.0f * s);
    _topHintLabel->setColor(Color3B(255, 245, 180));
    _topHintLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + 86.0f * s));
    this->addChild(_topHintLabel, 20);

    // --- Input ---
    _keyW = _keyA = _keyS = _keyD = false;
    _moveDirection = Vec2::ZERO;
    _isGameOver = false;
    _isVictory = false;
    _assignmentProgress = 0.0f;
    _nearDesk = false;
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

    // --- Hub interaction hint (screen-space, visible near door) ---
    if (_isHubScene)
    {
        float hcx = _worldSize.width * 0.5f;
        float hcy = _worldSize.height * 0.5f;
        const Vec2 hubArrowPositions[] = {
            Vec2(hcx - 540.0f, hcy + 280.0f),
            Vec2(hcx + 410.0f, hcy + 250.0f),
            Vec2(hcx - 40.0f, hcy - 340.0f)
        };

        for (const auto& position : hubArrowPositions)
        {
            auto arrow = createFloatingInteractionArrow(172.0f * s);
            if (arrow)
            {
                arrow->setPosition(position);
                (_worldLayer ? _worldLayer : this)->addChild(arrow, 8);
            }
        }

        _hubHintLabel = Label::createWithSystemFont("", "Arial", 22.0f);
        _hubHintLabel->setColor(Color3B(255, 240, 140));
        _hubHintLabel->setVisible(false);
        auto vs2 = Director::getInstance()->getVisibleSize();
        auto o2 = Director::getInstance()->getVisibleOrigin();
        _hubHintLabel->setPosition(Vec2(o2.x + vs2.width / 2, o2.y + 80.0f));
        this->addChild(_hubHintLabel, 30);
    }

    initInputListeners();
    updateCamera();
    if (!_isHubScene)
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
        _isGameOver = true;
        if (_waveManager) _waveManager->stopSpawn();
        return;
    }

    // Process scene transition after death on the next frame
    if (_isGameOver)
    {
        int kills = _waveManager ? _waveManager->getKillCount() : 0;
        int progress = _isEndlessMode ? _completedDdlCount : static_cast<int>(_assignmentProgress);
        int score = calculateScore();
        auto gameOverScene = GameOverScene::createScene(m_survivalTime, kills, progress, score);
        if (gameOverScene)
            Director::getInstance()->replaceScene(gameOverScene);
        else
            Director::getInstance()->replaceScene(MainMenuScene::createScene());
        return;
    }

    // Process wave/level completion → return to Hub or Main Menu
    if (_pendingAfterBattle)
    {
        _pendingAfterBattle = false;
        if (_goToMainMenu)
        {
            // Story mode all 3 levels cleared → main menu
            _goToMainMenu = false;
            Director::getInstance()->replaceScene(
                TransitionFade::create(0.8f, MainMenuScene::createScene(), Color3B::BLACK));
        }
        else
        {
            auto ud = UserDefault::getInstance();
            ud->setIntegerForKey("selected_scene", 0); // return to hub
            ud->flush();
            auto scene = GameScene::createScene();
            if (scene)
                Director::getInstance()->replaceScene(
                    TransitionFade::create(0.3f, scene, Color3B::BLACK));
            else
                Director::getInstance()->replaceScene(MainMenuScene::createScene());
        }
        return;
    }

    // Don't process game logic while paused
    if (_isPaused) return;

    // --- Victory delay: freeze gameplay, count down to scene transition ---
    if (_victoryDelayActive)
    {
        _victoryDelayTimer -= dt;
        // Update countdown hint
        if (_topHintLabel && !_goToMainMenu)
        {
            int secs = static_cast<int>(std::ceil(_victoryDelayTimer));
            _topHintLabel->setString(textByLanguage(
                "Victory! Returning to Hub in " + std::to_string(secs) + "...",
                u8"胜利！" + std::to_string(secs) + u8"秒后返回宿舍…"));
        }
        if (_victoryDelayTimer <= 0.0f)
        {
            _victoryDelayActive = false;
            _pendingAfterBattle = true;
            _pendingWave = 0;
        }
        return;
    }

    // --- Level Intro (blocks gameplay during countdown; never active in Hub) ---
    if (_levelIntroActive)
    {
        _levelIntroTimer -= dt;
        if (_levelIntroTimer <= 0.0f)
        {
            hideLevelIntro();
        }
        updateUI(m_player);
        updateSurvivalTime(dt); // keep timer running during intro
        return;
    }

    // ===================================================================
    // Player movement + camera (always runs: Hub & combat scenes)
    // ===================================================================
    if (m_player)
    {
        m_player->setLocalZOrder(100);

        // Character animation: idle (0.25s), walk speed-dependent
        bool isMoving = (_moveDirection != Vec2::ZERO);
        float animInterval;
        if (isMoving)
        {
            float v = m_player->getCurrentSpeed();
            float v0 = m_player->getBaseSpeed();
            const float f0 = 5.0f;  // base frequency (Hz), gives 0.2s at base speed
            animInterval = (v > 0.0f) ? (v0 / (f0 * v)) : 0.2f;
        }
        else
        {
            animInterval = 0.25f;
        }

        // On transition, immediately switch to the correct sprite set
        if (isMoving != _characterWasMoving)
        {
            _characterAnimTimer = 0.0f;
            _characterWasMoving = isMoving;
            if (isMoving)
            {
                _characterSprite1->setVisible(false);
                _characterSprite2->setVisible(false);
                _characterWalkSprite1->setVisible(true);
                _characterWalkSprite2->setVisible(false);
            }
            else
            {
                _characterWalkSprite1->setVisible(false);
                _characterWalkSprite2->setVisible(false);
                _characterSprite1->setVisible(true);
                _characterSprite2->setVisible(false);
            }
        }

        _characterAnimTimer += dt;
        if (_characterAnimTimer >= animInterval)
        {
            _characterAnimTimer -= animInterval;
            if (isMoving)
            {
                bool showOne = _characterWalkSprite1->isVisible();
                _characterWalkSprite1->setVisible(!showOne);
                _characterWalkSprite2->setVisible(showOne);
            }
            else
            {
                bool showOne = _characterSprite1->isVisible();
                _characterSprite1->setVisible(!showOne);
                _characterSprite2->setVisible(showOne);
            }
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

    // ===================================================================
    // Hub mode: interaction + UI only — no combat / spawn / enemies
    // ===================================================================
    if (_isHubScene)
    {
        updateHubInteraction();
        if (m_player)
        {
            updatePlayerMoodVisual();
            updateUI(m_player);
        }
        updateSurvivalTime(dt);
        return;
    }

    // ===================================================================
    // Combat mode (non-Hub) — weapons auto-aim, bullets, wave manager, collision
    // ===================================================================
    // Visible screen radius (world coords): only target enemies on screen
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float visibleRadius = std::min(visibleSize.width, visibleSize.height) * 0.5f / _worldScale;

    for (auto* weapon : _weapons)
    {
        if (weapon)
        {
            Vec2 aimDir = _playerDir;
            bool enemyInRange = false;

            Enemy* nearest = weapon->findNearestEnemy();
            if (nearest && m_player)
            {
                Vec2 enemyPos = nearest->getObjectPosition();
                Vec2 playerPos = m_player->getObjectPosition();
                Vec2 playerToEnemy = enemyPos - playerPos;
                float dist = playerToEnemy.length();
                // Only attack enemies visible on screen AND within weapon range
                if (dist <= weapon->getAttackRange() && dist <= visibleRadius)
                {
                    if (playerToEnemy.lengthSquared() > 0.0001f)
                    {
                        // Step 1: set rough aim (player→enemy)
                        aimDir = playerToEnemy.getNormalized();
                        weapon->setAimDirection(aimDir);

                        // Step 2: position weapon + tick cooldown
                        weapon->updateCooldown(dt);

                        // Step 3: get REAL muzzle position (handles all
                        // parent scales: player scale, worldLayer scale)
                        Vec2 muzzlePos = weapon->getMuzzlePosition(aimDir);

                        // Step 4: precise aim from muzzle to enemy
                        Vec2 muzzleToEnemy = enemyPos - muzzlePos;
                        if (muzzleToEnemy.lengthSquared() > 0.0001f)
                        {
                            aimDir = muzzleToEnemy.getNormalized();
                        }
                    }
                    else
                    {
                        aimDir = Vec2(1.0f, 0.0f);
                        weapon->setAimDirection(aimDir);
                        weapon->updateCooldown(dt);
                    }
                    weapon->setAimDirection(aimDir);
                    enemyInRange = true;
                }
            }

            if (!enemyInRange)
            {
                weapon->setAimDirection(aimDir);
                weapon->updateCooldown(dt);
            }

            if (enemyInRange && weapon->isReadyToFire())
            {
                weapon->fire();
            }
        }
    }

    // --- Bullets ---
    for (auto* bullet : _bullets)
    {
        if (bullet)
            bullet->updateObject(dt);
    }

    // --- WaveManager ---
    if (_waveManager)
    {
        _waveManager->setElapsedTime(m_survivalTime);
        updateFreezeEffect(dt);
        _waveManager->update(dt);

        // --- Scene-specific victory conditions ---
        if (!_isEndlessMode && !_isVictory && !_isGameOver)
        {
            int kills = _waveManager->getKillCount();
            if (_sceneId == 1 && m_survivalTime >= 90.0f && kills >= 10)
                goToVictory(); // Library: survive 90s + kill 10
            else if (_sceneId == 2 && _waveManager->getCurrentWave() >= 4
                     && !_waveManager->isWaveActive())
                goToVictory(); // Classroom: 4 waves done + all enemies cleared
            else if (_sceneId == 3 && _thesisProgress >= 100.0f)
                goToVictory(); // Office: boss defeated
        }

        // Endless: time-based spawn table
        if (_isEndlessMode && _waveManager)
        {
            float t = m_survivalTime;
            if (t < 120.0f)
                _waveManager->setAllowedTypes({1}); // DDL only
            else if (t < 240.0f)
                _waveManager->setAllowedTypes({0, 1}); // +Sleepy
            else if (t < 360.0f)
                _waveManager->setAllowedTypes({0, 1, 3}); // +Phone
            else
                _waveManager->setAllowedTypes({0, 1, 2, 3}); // +ThesisBoss

            // DDL Pressure
            _ddlPressure += dt * (0.08f + t * 0.001f);
            if (_ddlPressure >= 100.0f)
            {
                _ddlPressure = 100.0f;
                goToGameOver();
            }
        }
    }

    updateEnemyPlayerContact(dt);

    // --- Collision ---
    if (_waveManager)
    {
        CollisionManager::checkBulletEnemyCollision(
            _bullets,
            _waveManager->getAliveEnemies()
        );

        // Boss scene: track thesisProgress during combat
        if (_isBossScene && !_isVictory)
        {
            bool bossAlive = false;
            for (auto* enemy : _waveManager->getAliveEnemies())
            {
                if (enemy && enemy->getObjectName() == "ThesisBoss")
                {
                    bossAlive = true;
                    break;
                }
            }
            if (bossAlive)
                _thesisProgress += dt * 3.0f; // passive progress while boss is alive
        }
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
            float fullBarWidth = 20.0f * invScale;
            float barHeight = 3.0f * invScale;
            float barY = 14.0f * invScale;

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

    if (_weaponLabel && !_weapons.empty() && _weapons[0])
    {
        _weaponLabel->setString(textByLanguage("Weapon: ", u8"武器: ") +
            weaponNameForUi(_weapons[0]->getWeaponName()));
    }
    if (_weaponIcon && !_weapons.empty() && _weapons[0])
    {
        auto texture = Director::getInstance()->getTextureCache()->addImage(_weapons[0]->getImagePath());
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
            if (isChineseUi())
            {
                snprintf(buf, sizeof(buf), "分数 %d", _endlessScore);
            }
            else
            {
                snprintf(buf, sizeof(buf), "Score %d", _endlessScore);
            }
            _endlessStatsLabel->setString(buf);
        }
    }

    updateExpBarUI();
    updateWaveTimerUI();

    // DDL Pressure (endless)
    if (_isEndlessMode && _endlessStatsLabel)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "DDL Pressure: %.0f%%", _ddlPressure);
        _endlessStatsLabel->setString(buf);
        _endlessStatsLabel->setVisible(true);
        _endlessStatsLabel->setColor(_ddlPressure > 70 ? Color3B(255, 80, 60) : Color3B(255, 200, 100));
    }

    // Thesis progress (boss scene)
    if (_isBossScene && _progressLabel)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Thesis Progress: %.0f%%", _thesisProgress);
        _progressLabel->setString(buf);
    }

    if (_taskLabel)
    {
        char buf[80];
        if (_isHubScene)
        {
            buf[0] = '\0';
        }
        else if (_isEndlessMode)
        {
            if (isChineseUi())
                snprintf(buf, sizeof(buf), "生存: %.0f秒  DDL: %d", m_survivalTime, _completedDdlCount);
            else
                snprintf(buf, sizeof(buf), "Survival: %.0fs  DDL: %d", m_survivalTime, _completedDdlCount);
        }
        else if (_sceneId == 1) // Library
        {
            int kills = _waveManager ? _waveManager->getKillCount() : 0;
            if (isChineseUi())
                snprintf(buf, sizeof(buf), "击杀: %d / 10  时间: %.0f / 90秒", kills, m_survivalTime);
            else
                snprintf(buf, sizeof(buf), "Kills: %d / 10  Time: %.0f / 90s", kills, m_survivalTime);
        }
        else if (_sceneId == 2) // Classroom
        {
            int wave = _waveManager ? _waveManager->getCurrentWave() : 0;
            if (isChineseUi())
                snprintf(buf, sizeof(buf), "波次: %d / 6", wave);
            else
                snprintf(buf, sizeof(buf), "Wave: %d / 6", wave);
        }
        else if (_sceneId == 3) // Office Boss
        {
            if (isChineseUi())
                snprintf(buf, sizeof(buf), "论文进度: %.0f%%", _thesisProgress);
            else
                snprintf(buf, sizeof(buf), "Thesis: %.0f%%", _thesisProgress);
        }
        else
        {
            buf[0] = '\0';
        }
        _taskLabel->setString(buf);
    }

    if (_taskBarFill)
    {
        float ratio = 0.0f;
        if (!_isHubScene)
        {
            if (_isEndlessMode)
                ratio = std::min(1.0f, _ddlPressure / 100.0f);
            else if (_sceneId == 1) // Library
                ratio = std::min(1.0f, m_survivalTime / 90.0f);
            else if (_sceneId == 2) // Classroom
                ratio = _waveManager ? std::min(1.0f, _waveManager->getCurrentWave() / 6.0f) : 0.0f;
            else if (_sceneId == 3) // Office
                ratio = std::min(1.0f, _thesisProgress / 100.0f);
        }
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        _taskBarFill->setContentSize(Size(_taskBarMaxWidth * ratio,
            _taskBarFill->getContentSize().height));
    }

    refreshWeaponSlotUI();
}

void GameScene::updateExpBarUI()
{
    if (!_expBarFill || !_expBarBg || !m_player) return;

    int exp = m_player->getExp();
    int need = m_player->getExpToNextLevel();
    float pct = (need > 0) ? std::min(1.0f, (float)exp / (float)need) : 1.0f;
    _expBarFill->setContentSize(Size(_expBarMaxWidth * pct, _expBarFill->getContentSize().height));

    if (pct > 0.6f)
        _expBarFill->setColor(Color3B(80, 200, 120));
    else if (pct > 0.3f)
        _expBarFill->setColor(Color3B(220, 180, 40));
    else
        _expBarFill->setColor(Color3B(220, 70, 50));

    if (_expLevelLabel)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Lv.%d", m_player->getLevel());
        _expLevelLabel->setString(buf);
    }
    if (_expFractionLabel)
    {
        char buf[64];
        auto* lm = LanguageManager::getInstance();
        snprintf(buf, sizeof(buf), "%d/%d %s", exp, need, lm->getString("exp_label").c_str());
        _expFractionLabel->setString(buf);
    }
}

void GameScene::updateWaveTimerUI()
{
    if (!_waveTimerLabel) return;
    if (!_isEndlessMode || !_waveManager)
    {
        _waveTimerLabel->setVisible(false);
        return;
    }

    _waveTimerLabel->setVisible(true);
    float remaining = _waveManager->getWaveTimerRemaining();
    int wave = _waveManager->getCurrentWave();

    char buf[64];
    auto* lm = LanguageManager::getInstance();
    snprintf(buf, sizeof(buf), lm->getString("wave_countdown_fmt").c_str(), wave, remaining);
    _waveTimerLabel->setString(buf);

    _waveTimerLabel->setColor(remaining < 5.0f ? Color3B(255, 80, 60) : Color3B(255, 200, 100));
}

void GameScene::updateWeaponEnergyUI()
{
    if (!_weaponEnergyFill || _weapons.empty() || !_weapons[0])
    {
        return;
    }

    float ratio = _weapons[0]->getEnergyRatio();
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

    bool slotDataChanged = _lastWeaponSlotIds.size() != _weaponLoadoutIds.size();
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
        auto bg = LayerColor::create(
            Color4B(18, 22, 30, 220),
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
            1.2f,
            Color4F(0.52f, 0.58f, 0.67f, 0.8f));
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

    }

    _lastWeaponSlotIds = _weaponLoadoutIds;
}

void GameScene::initSceneConfig()
{
    auto ud = UserDefault::getInstance();
    _isEndlessMode = ud->getIntegerForKey("selected_game_mode", 0) == 1;
    _sceneId = ud->getIntegerForKey("selected_scene", 0);
    _isHubScene = (_sceneId == 0);
    _isBossScene = (_sceneId == 3);
    _levelNumber = ud->getIntegerForKey("selected_level", 1);
    if (_levelNumber < 1) _levelNumber = 1;

    _ddlTimeLimit = 999.0f;
    _ddlTimeRemaining = _ddlTimeLimit;
    _completedDdlCount = 0;
    _assignmentProgress = 0.0f;
    _thesisProgress = 0.0f;
    _ddlPressure = 0.0f;
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
            snprintf(mission, sizeof(mission), "无尽模式\n生存尽可能久！\n击杀敌人获取经验升级。\nDDL压力会不断增长。");
        else
            snprintf(mission, sizeof(mission), "Endless Mode\nSurvive as long as possible!\nKill enemies to gain EXP and level up.\nDDL Pressure will keep rising.");
    }
    else if (_sceneId == 1)
    {
        if (isChineseUi())
            snprintf(mission, sizeof(mission), "图书馆\n存活90秒并击杀10只DDL怪物。");
        else
            snprintf(mission, sizeof(mission), "Library\nSurvive 90 seconds and kill 10 DDL monsters.");
    }
    else if (_sceneId == 2)
    {
        if (isChineseUi())
            snprintf(mission, sizeof(mission), "教室\n撑过6波敌人的进攻！");
        else
            snprintf(mission, sizeof(mission), "Classroom\nSurvive 6 waves of enemies!");
    }
    else if (_sceneId == 3)
    {
        if (isChineseUi())
            snprintf(mission, sizeof(mission), "办公室\n击败论文Boss！\n攻击Boss积累进度，注意躲避 Phone 怪物。");
        else
            snprintf(mission, sizeof(mission), "Office\nDefeat the Thesis Boss!\nAttack the boss to build progress.\nWatch out for Phone monsters.");
    }
    else
    {
        if (isChineseUi())
            snprintf(mission, sizeof(mission), "第%d关\n准备战斗！", _levelNumber);
        else
            snprintf(mission, sizeof(mission), "Level %d\nPrepare for battle!", _levelNumber);
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
            if (!_isGameOver)
            {
                if (_isPaused) hidePauseMenu();
                else           showPauseMenu();
            }
            return;
        }

        // E key: door interaction in hub
        if ((code == EventKeyboard::KeyCode::KEY_E ||
             code == EventKeyboard::KeyCode::KEY_CAPITAL_E) && _isHubScene && m_player)
        {
            Vec2 pos = m_player->getPosition();
            float hcx = _worldSize.width * 0.5f;
            float hcy = _worldSize.height * 0.5f;
            Rect doorRect(hcx - 150.0f, hcy - 450.0f, 300.0f, 180.0f);
            if (isInRect(pos, doorRect))
            {
                AudioManager::getInstance()->playButtonClick();
                // Save player state before leaving Hub
                auto ud = UserDefault::getInstance();
                ud->setIntegerForKey("player_upgrade_points", m_player->getUpgradePoints());
                ud->setIntegerForKey("player_level", m_player->getLevel());
                ud->setIntegerForKey("player_max_hp", static_cast<int>(m_player->getMaxHp()));
                ud->setFloatForKey("player_speed", m_player->getBaseSpeed());
                ud->setIntegerForKey("continue_score", _endlessScore);
                ud->setIntegerForKey("weapon_damage_bonus", _weaponDamageBonus);
                ud->setFloatForKey("energy_recovery_bonus", _energyRecoveryBonusPercent);
                ud->setIntegerForKey("projectile_bonus", _projectileBonus);
                ud->setIntegerForKey("life_on_kill", _lifeOnKill);
                // Determine next scene
                if (!_isEndlessMode)
                {
                    // Story mode: _levelNumber maps to scene
                    // Level 1→Library(1), 2→Classroom(2), 3→Office(3), 4+→Office(3)
                    int nextScene = _levelNumber;
                    if (nextScene < 1) nextScene = 1;
                    if (nextScene > 3) nextScene = 3;
                    ud->setIntegerForKey("selected_scene", nextScene);
                    ud->setIntegerForKey("selected_level", _levelNumber + 1);
                    StoryModeScene::addAutoSave(_levelNumber + 1);
                }
                else
                {
                    // Endless: random scene
                    int scenes[] = {1, 2, 3};
                    int nextScene = scenes[rand() % 3];
                    ud->setIntegerForKey("selected_scene", nextScene);
                }
                ud->setIntegerForKey("continue_score", _endlessScore);
                ud->flush();
                auto scene = GameScene::createScene();
                Director::getInstance()->replaceScene(
                    TransitionFade::create(0.3f, scene, Color3B::BLACK));
            }
            return;
        }

        // Ignore movement keys while paused
        if (_isPaused) return;

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
        if (_moveDirection.x != 0.0f)
        {
            // Character artwork faces right by default. Mirror only while the
            // latest horizontal movement is toward the left.
            bool flipX = (_moveDirection.x < 0.0f);
            if (_characterSprite1) _characterSprite1->setFlippedX(flipX);
            if (_characterSprite2) _characterSprite2->setFlippedX(flipX);
            if (_characterWalkSprite1) _characterWalkSprite1->setFlippedX(flipX);
            if (_characterWalkSprite2) _characterWalkSprite2->setFlippedX(flipX);
        }
    }

    if (m_player)
        m_player->setInputDirection(_moveDirection);
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
    _worldLayer->setPosition(current + (target - current) * 0.45f);
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

    Rect playerBox = m_player->getCollisionBox();

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

        Rect enemyBox = enemy->getCollisionBox();

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

            _enemyContactDamageCooldown = 0.7f;
        }

        // Push ALL colliding enemies away — stronger push to prevent surround
        Vec2 pushDir = m_player->getPosition() - enemy->getPosition();
        if (pushDir.lengthSquared() > 0.001f)
        {
            pushDir.normalize();
            float playerPush = nearWall ? 18.0f : 12.0f;
            float enemyPush = nearWall ? 30.0f : 18.0f;
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
            if (playerBox.intersectsRect(enemy->getCollisionBox()))
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

    for (int i = 0; i < (int)_weaponLoadoutIds.size(); ++i)
    {
        Weapon* weapon = createWeaponById(_weaponLoadoutIds[i]);
        if (weapon)
        {
            weapon->setHandSlot(i);
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
            weapon->setVisible(true);
            _weapons.push_back(weapon);
        }
    }

    // Both weapons always visible — auto-attack
    for (int i = 0; i < static_cast<int>(_weapons.size()); ++i)
    {
        if (_weapons[i])
        {
            _weapons[i]->setVisible(true);
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
    std::string effectText = textByLanguage("Environment: None", u8"环境: 无");
    Rect playerBox = m_player->getCollisionBox();

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
    if (_isVictory || _isGameOver || _isHubScene)
    {
        return;
    }

    if (_isEndlessMode)
    {
        // Near desk gives a small progress bonus in endless mode
        if (_nearDesk)
        {
            _assignmentProgress += dt * 5.0f;
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

    // Story mode: no desk-stay or DDL-countdown mechanics.
    // Victory conditions per scene are checked in update().
    // Track kill count as progress for scoring.
    if (_waveManager)
    {
        _assignmentProgress = static_cast<float>(_waveManager->getKillCount());
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

    // Award EXP in endless mode so player can earn upgrade points
    if (enemy)
    {
        m_player->addExp(enemy->getExpReward());
    }
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

    Rect playerBox = m_player->getCollisionBox();
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
        applySpriteFit(m_player, 306.0f, 306.0f);
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

// ==================== Hub Interaction ====================
bool GameScene::isInRect(const Vec2& pos, const Rect& rect) const
{
    return pos.x >= rect.getMinX() && pos.x <= rect.getMaxX()
        && pos.y >= rect.getMinY() && pos.y <= rect.getMaxY();
}

void GameScene::updateHubInteraction()
{
    if (!m_player || !_isHubScene) return;
    Vec2 pos = m_player->getPosition();

    // Hub collision rects (world coordinates, relative to world center)
    float hcx = _worldSize.width * 0.5f;
    float hcy = _worldSize.height * 0.5f;
    Rect bedRect(hcx - 650.0f, hcy + 200.0f, 220.0f, 160.0f);
    Rect deskRect(hcx + 700.0f, hcy + 250.0f, 260.0f, 200.0f);
    Rect doorRect(hcx - 150.0f, hcy - 450.0f, 300.0f, 180.0f);

    bool nearBed = isInRect(pos, bedRect);
    bool nearDesk = isInRect(pos, deskRect);
    bool nearDoor = isInRect(pos, doorRect);

    // Bed: auto popup when entering
    if (nearBed && !_bedPanelOpen)
        showBedPanel();
    else if (!nearBed && _bedPanelOpen)
        hideBedPanel();

    // Desk: auto popup when entering
    if (nearDesk && !_deskPanelOpen && DeskUpgradeLayer::canShowPanel())
        showDeskPanel();
    else if (!nearDesk)
    {
        DeskUpgradeLayer::resetCanShow();  // always reset on exit
        if (_deskPanelOpen)
            hideDeskPanel();
    }

    // Door: E-key hint
    if (_hubHintLabel)
    {
        _hubHintLabel->setVisible(nearDoor);
        if (nearDoor)
        {
            if (_isEndlessMode)
            {
                _hubHintLabel->setString("[E] Random Scene / 随机关卡");
            }
            else
            {
                // Story mode: show which scene is next
                int nextScene = _levelNumber;
                if (nextScene < 1) nextScene = 1;
                if (nextScene > 3) nextScene = 3;
                const char* sceneNames[] = {"", "Library / 图书馆", "Classroom / 教室", "Office / 办公室"};
                _hubHintLabel->setString(std::string("[E] ") + sceneNames[nextScene]);
            }
        }
    }
}

void GameScene::showBedPanel()
{
    if (_bedPanelOpen || _bedPanel) return;
    _bedPanelOpen = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // Dark overlay
    _hubOverlay = LayerColor::create(Color4B(0, 0, 0, 128), winSize.width, winSize.height);
    _hubOverlay->setPosition(Vec2::ZERO);
    _hubOverlay->setOpacity(0);
    this->addChild(_hubOverlay, 50);
    _hubOverlay->runAction(FadeTo::create(0.25f, 128));

    // Bed panel
    _bedPanel = Node::create();
    _bedPanel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.6f));
    this->addChild(_bedPanel, 55);

    auto* lm = LanguageManager::getInstance();
    float s = Director::getInstance()->getWinSize().height / 640.0f;

    auto bg = LayerColor::create(Color4B(15, 25, 15, 230), 300.0f * s, 120.0f * s);
    bg->setPosition(Vec2(-150.0f * s, -60.0f * s));
    _bedPanel->addChild(bg);

    auto label = Label::createWithSystemFont("Rest / 休息", "Arial", 28.0f * s);
    label->setColor(Color3B(120, 255, 120));
    label->setPosition(Vec2(0, 20.0f * s));
    _bedPanel->addChild(label);

    auto sub = Label::createWithSystemFont("HP restored!", "Arial", 18.0f * s);
    sub->setColor(Color3B(180, 220, 180));
    sub->setPosition(Vec2(0, -20.0f * s));
    _bedPanel->addChild(sub);

    // Pop-in animation
    _bedPanel->setScale(0.0f);
    _bedPanel->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));

    // Heal
    if (m_player)
    {
        m_player->heal(9999);
        _ddlPressure = 0.0f;
    }
}

void GameScene::hideBedPanel()
{
    if (!_bedPanelOpen) return;
    _bedPanelOpen = false;

    if (_hubOverlay)
    {
        _hubOverlay->runAction(Sequence::create(
            FadeTo::create(0.2f, 0),
            RemoveSelf::create(),
            nullptr));
        _hubOverlay = nullptr;
    }
    if (_bedPanel)
    {
        _bedPanel->runAction(Sequence::create(
            EaseBackIn::create(ScaleTo::create(0.2f, 0.0f)),
            RemoveSelf::create(),
            nullptr));
        _bedPanel = nullptr;
    }
}

void GameScene::showDeskPanel()
{
    if (_deskPanelOpen || _deskPanel) return;
    _deskPanelOpen = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // --- Dark overlay ---
    _hubOverlay = LayerColor::create(Color4B(0, 0, 0, 160), winSize.width, winSize.height);
    _hubOverlay->setPosition(Vec2::ZERO);
    _hubOverlay->setOpacity(0);
    this->addChild(_hubOverlay, 50);
    _hubOverlay->runAction(FadeTo::create(0.25f, 160));

    // --- DeskUpgradeLayer ---
    auto deskLayer = DeskUpgradeLayer::create();
    _deskPanel = deskLayer;
    _deskPanel->setOpacity(0);
    this->addChild(_deskPanel, 55);

    // Wire close button to full cleanup
    deskLayer->setOnClose([this]() {
        hideDeskPanel();
    });

    // Sync current equipped weapons
    int w1 = _weaponLoadoutIds.size() > 0 ? _weaponLoadoutIds[0] : -1;
    int w2 = _weaponLoadoutIds.size() > 1 ? _weaponLoadoutIds[1] : -1;
    deskLayer->setEquippedWeapons(w1, w2);

    // Sync upgrade points
    deskLayer->setUpgradePoints(m_player ? m_player->getUpgradePoints() : 0);

    // Weapon equip/unequip → update real loadout & player visuals
    deskLayer->setOnWeaponChanged([this, deskLayer](int s1, int s2) {
        // Remove old weapons (use _weapons — guaranteed to track all weapons)
        for (auto* w : _weapons) {
            if (w) w->removeFromParentAndCleanup(true);
        }
        _weapons.clear();

        // Update loadout — allow empty
        _weaponLoadoutIds.clear();
        _backpackWeaponIds.clear();
        if (s1 >= 0) _weaponLoadoutIds.push_back(s1);
        if (s2 >= 0 && s2 != s1) _weaponLoadoutIds.push_back(s2);
        for (int i = 0; i <= 6; i++) {
            if (i != s1 && i != s2) _backpackWeaponIds.push_back(i);
        }
        normalizeWeaponLists(_weaponLoadoutIds, _backpackWeaponIds);

        // Create weapon nodes for each equipped slot
        deskLayer->setEquippedWeapons(
            _weaponLoadoutIds.size() > 0 ? _weaponLoadoutIds[0] : -1,
            _weaponLoadoutIds.size() > 1 ? _weaponLoadoutIds[1] : -1);
        for (int wid : _weaponLoadoutIds) {
            Weapon* weapon = createWeaponById(wid);
            if (weapon && m_player) {
                weapon->setHandSlot(static_cast<int>(_weapons.size()));
                float parentScale = m_player->getScale();
                if (parentScale > 0.001f)
                    weapon->setScale(weapon->getScale() / parentScale);
                m_player->addChild(weapon, 4);
                if (_waveManager) {
                    weapon->bindBattleData(&_waveManager->getAliveEnemies(), &_bullets, _bulletLayer);
                    weapon->bindBulletPool(&_bulletPool);
                }
                weapon->setVisible(true);
                _weapons.push_back(weapon);
            }
        }

        // Save
        auto ud = UserDefault::getInstance();
        for (int j = 0; j < 2; ++j)
            ud->setIntegerForKey(("weapon_equipped_"+std::to_string(j)).c_str(),
                j < (int)_weaponLoadoutIds.size() ? _weaponLoadoutIds[j] : -1);
        for (int j = 0; j < 4; ++j)
            ud->setIntegerForKey(("weapon_backpack_"+std::to_string(j)).c_str(),
                j < (int)_backpackWeaponIds.size() ? _backpackWeaponIds[j] : -1);
        ud->flush();
    });

    // Attribute upgrade → apply real stat
    deskLayer->setOnUpgrade([this](int attrIdx) {
        if (!m_player || m_player->getUpgradePoints() <= 0) return;
        m_player->spendUpgradePoint(1);
        auto ud = UserDefault::getInstance();
        switch (attrIdx) {
        case 0: _weaponDamageBonus += 2;
            ud->setIntegerForKey("weapon_damage_bonus", _weaponDamageBonus); break;
        case 1: m_player->addMaxHp(10); m_player->heal(10); break;
        case 2: m_player->setBaseSpeed(m_player->getBaseSpeed() + 10.0f); break;
        case 3: _energyRecoveryBonusPercent += 0.1f;
            ud->setFloatForKey("energy_recovery_bonus", _energyRecoveryBonusPercent); break;
        case 4: ++_projectileBonus;
            ud->setIntegerForKey("projectile_bonus", _projectileBonus); break;
        }
        ud->setIntegerForKey("player_upgrade_points", m_player->getUpgradePoints());
        ud->flush();
        // Update panel display
        auto* dl = dynamic_cast<DeskUpgradeLayer*>(_deskPanel);
        if (dl) dl->setUpgradePoints(m_player->getUpgradePoints());
    });

    // Entrance animation
    _deskPanel->runAction(FadeIn::create(0.25f));
}

void GameScene::hideDeskPanel()
{
    if (!_deskPanelOpen) return;
    _deskPanelOpen = false;

    if (_hubOverlay)
    {
        _hubOverlay->runAction(Sequence::create(
            FadeTo::create(0.2f, 0), RemoveSelf::create(), nullptr));
        _hubOverlay = nullptr;
    }
    if (_deskPanel)
    {
        _deskPanel->runAction(Sequence::create(
            FadeOut::create(0.2f),
            RemoveSelf::create(), nullptr));
        _deskPanel = nullptr;
    }
}

void GameScene::goToGameOver()
{
    // Just set the flag; the actual scene transition happens in update()
    // on the next frame to avoid calling replaceScene mid-update.
    if (_isGameOver) return;
    _isGameOver = true;
    if (_waveManager) _waveManager->stopSpawn();
}

void GameScene::goToVictory()
{
    if (_isVictory || _isGameOver) return;
    _isVictory = true;
    if (_waveManager) _waveManager->stopSpawn();

    // Clear remaining enemies for time-based / boss levels
    if (_waveManager && (_sceneId == 1 || _sceneId == 3))
    {
        auto& enemies = _waveManager->getAliveEnemies();
        for (auto* enemy : enemies)
        {
            if (enemy && enemy->isRoleAlive())
            {
                enemy->die();
                enemy->removeFromParent();
            }
        }
        enemies.clear();
    }

    auto ud = UserDefault::getInstance();
    _goToMainMenu = false;

    if (!_isEndlessMode)
    {
        if (_levelNumber >= 3)
        {
            // Story mode complete — go to main menu after delay
            _goToMainMenu = true;
            // Keep save at level 3 (don't unlock level 4)
            int nextLevel = 3;
            ud->setIntegerForKey("unlocked_level", nextLevel);
            StoryModeScene::addAutoSave(nextLevel);
        }
        else
        {
            int nextLevel = _levelNumber + 1;
            ud->setIntegerForKey("unlocked_level", nextLevel);
            StoryModeScene::addAutoSave(nextLevel);
        }
    }

    // Persist player state for hub return
    ud->setIntegerForKey("continue_score", _endlessScore);
    ud->setIntegerForKey("weapon_damage_bonus", _weaponDamageBonus);
    ud->setFloatForKey("energy_recovery_bonus", _energyRecoveryBonusPercent);
    ud->setIntegerForKey("projectile_bonus", _projectileBonus);
    ud->setIntegerForKey("life_on_kill", _lifeOnKill);
    if (m_player)
    {
        ud->setIntegerForKey("player_upgrade_points", m_player->getUpgradePoints());
        ud->setIntegerForKey("player_level", m_player->getLevel());
        ud->setIntegerForKey("player_exp", m_player->getExp());
        ud->setIntegerForKey("player_exp_to_next", m_player->getExpToNextLevel());
        ud->setIntegerForKey("player_hp", static_cast<int>(m_player->getHp()));
        ud->setIntegerForKey("player_max_hp", static_cast<int>(m_player->getMaxHp()));
        ud->setFloatForKey("player_speed", m_player->getBaseSpeed());
    }
    ud->flush();

    // Start 5-second victory delay before scene transition
    _victoryDelayTimer = 5.0f;
    _victoryDelayActive = true;

    // Show victory / completion message
    if (_topHintLabel)
    {
        if (_goToMainMenu)
        {
            _topHintLabel->setString("DDL FINISHED! Thanks for playing!");
        }
        else
        {
            _topHintLabel->setString(textByLanguage("Victory! Returning to Hub...",
                u8"胜利！即将返回宿舍…"));
        }
    }
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
    const float cx = origin.x + visibleSize.width * 0.5f;
    const float cy = origin.y + visibleSize.height * 0.5f;
    float s = std::min(visibleSize.width / 1920.0f, visibleSize.height / 1080.0f);
    s = std::max(0.70f, std::min(s, 2.0f));

    _pauseLayer = LayerColor::create(Color4B(0, 5, 15, 150),
        visibleSize.width, visibleSize.height);
    _pauseLayer->setPosition(origin);
    this->addChild(_pauseLayer, 100);

    auto swallow = EventListenerTouchOneByOne::create();
    swallow->setSwallowTouches(true);
    swallow->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(swallow, _pauseLayer);

    const float panelW = visibleSize.width * 0.84f;
    const float panelH = visibleSize.height * 0.72f;
    const float gap = panelW * 0.04f;
    const float leftW = panelW * 0.35f;
    const float rightW = panelW - leftW - gap;
    const float contentBottom = origin.y + visibleSize.height * 0.18f;
    const float contentLeft = cx - panelW * 0.5f;

    auto content = Node::create();
    content->setContentSize(Size(panelW, panelH));
    content->setPosition(Vec2(contentLeft, contentBottom));
    _pauseLayer->addChild(content, 20);

    buildPauseStatsPanel(content, Vec2(0.0f, 0.0f), s);
    buildPauseWeaponPanel(content, Vec2(leftW + gap, 0.0f), s);

    const Size titleBounds(460.0f * s, 118.0f * s);
    auto titleSprite = createFittedSprite("art/ui/pause_title.png", titleBounds);
    if (titleSprite)
    {
        auto titleNode = Node::create();
        titleNode->setContentSize(titleBounds);
        titleNode->addChild(titleSprite);
        titleNode->setPosition(Vec2(cx - titleBounds.width * 0.5f,
            contentBottom + panelH - titleBounds.height * 0.46f));
        _pauseLayer->addChild(titleNode, 50);
    }
    else
    {
        auto titleLabel = Label::createWithSystemFont(
            lm->getString("game_paused"), "Arial", 46.0f * s);
        titleLabel->setColor(Color3B(238, 230, 210));
        titleLabel->setPosition(Vec2(cx, contentBottom + panelH + 16.0f * s));
        _pauseLayer->addChild(titleLabel, 50);
    }

    const Size iconBounds(112.0f * s, 78.0f * s);
    const float buttonY = origin.y + visibleSize.height * 0.055f;
    const float buttonGap = 34.0f * s;
    const std::vector<Node*> buttons = {
        createPauseActionButton("art/ui/pause_resume.png", lm->getString("resume"),
            iconBounds, 18.0f * s, CC_CALLBACK_1(GameScene::onPauseResumeClicked, this)),
        createPauseActionButton("art/ui/pause_restart.png", lm->getString("restart"),
            iconBounds, 18.0f * s, CC_CALLBACK_1(GameScene::onPauseRestartClicked, this)),
        createPauseActionButton("art/ui/pause_save.png", lm->getString("save_game"),
            iconBounds, 18.0f * s, CC_CALLBACK_1(GameScene::onPauseSaveClicked, this)),
        createPauseActionButton("art/ui/pause_settings.png", lm->getString("settings"),
            iconBounds, 18.0f * s, CC_CALLBACK_1(GameScene::onPauseSettingsClicked, this)),
        createPauseActionButton("art/ui/pause_back_to_menu.png", lm->getString("back_to_title"),
            iconBounds, 18.0f * s, CC_CALLBACK_1(GameScene::onPauseTitleClicked, this))
    };

    const float totalButtonsW = buttons.size() * iconBounds.width +
        (buttons.size() - 1) * buttonGap;
    float x = cx - totalButtonsW * 0.5f;
    for (auto* button : buttons)
    {
        button->setPosition(Vec2(x, buttonY));
        _pauseLayer->addChild(button, 60);
        x += iconBounds.width + buttonGap;
    }
}

void GameScene::buildPauseStatsPanel(Node* parent, const Vec2& pos, float s)
{
    auto* lm = LanguageManager::getInstance();
    const Size contentSize = parent->getContentSize();
    const float panelW = contentSize.width * 0.35f;
    const float panelH = contentSize.height;
    const float pad = 22.0f * s;

    auto panel = createPausePanel(Size(panelW, panelH));
    panel->setPosition(pos);
    parent->addChild(panel, 20);

    const Size portraitBounds(panelW - pad * 2.0f, panelH * 0.34f);
    auto portraitBox = LayerColor::create(Color4B(8, 18, 33, 190),
        portraitBounds.width, portraitBounds.height);
    portraitBox->setPosition(Vec2(pos.x + pad, pos.y + panelH - portraitBounds.height - pad));
    parent->addChild(portraitBox, 21);

    std::string portraitPath = m_player ? getMoodPlayerImagePath(m_player->getCurrentMood()) : "";
    if (portraitPath.empty())
        portraitPath = AssetPaths::resolve("art/characters/character1.png");
    auto portrait = createFittedSprite(portraitPath, Size(portraitBounds.width * 0.92f,
        portraitBounds.height * 0.92f));
    if (portrait)
    {
        portrait->setPosition(Vec2(pos.x + panelW * 0.5f,
            portraitBox->getPositionY() + portraitBounds.height * 0.5f));
        parent->addChild(portrait, 30);
    }

    const Size titleSize(panelW - pad * 2.0f, 46.0f * s);
    auto title = createPauseSectionTitle(textByLanguage("Character Stats", u8"角色属性"),
        titleSize, 22.0f * s);
    title->setPosition(Vec2(pos.x + pad,
        portraitBox->getPositionY() - 18.0f * s - titleSize.height));
    parent->addChild(title, 30);

    std::vector<std::pair<std::string, std::string>> rows;
    if (m_player)
    {
        rows.push_back({ lm->getString("stat_level"), "Lv." + std::to_string(m_player->getLevel()) });
        rows.push_back({ lm->getString("stat_hp"), std::to_string((int)m_player->getHp()) + " / " +
            std::to_string((int)m_player->getMaxHp()) });
        rows.push_back({ lm->getString("stat_atk"), std::to_string((!_weapons.empty() && _weapons[0])
            ? _weapons[0]->getModifiedAttackPower() : 0) });
        rows.push_back({ lm->getString("stat_spd"), std::to_string((int)m_player->getMoveSpeed()) });
        rows.push_back({ textByLanguage("Upgrade Points", u8"升级点数"),
            std::to_string(m_player->getUpgradePoints()) });
        rows.push_back({ textByLanguage("Mood", u8"情绪"), moodNameForUi(m_player->getCurrentMood()) });
        rows.push_back({ textByLanguage("Environment", u8"环境"),
            textByLanguage("None", u8"无") });
    }

    const float rowH = 38.0f * s;
    float y = title->getPositionY() - 16.0f * s - rowH;
    for (const auto& row : rows)
    {
        auto rowNode = createPauseInfoRow(row.first, row.second,
            Size(panelW - pad * 2.0f, rowH), 16.0f * s);
        rowNode->setPosition(Vec2(pos.x + pad, y));
        parent->addChild(rowNode, 30);
        y -= rowH + 8.0f * s;
    }
}

void GameScene::buildPauseWeaponPanel(Node* parent, const Vec2& pos, float s)
{
    auto* lm = LanguageManager::getInstance();
    auto options = getWeaponOptions();
    const Size contentSize = parent->getContentSize();
    const float leftW = contentSize.width * 0.35f;
    const float gap = contentSize.width * 0.04f;
    const float panelW = contentSize.width - leftW - gap;
    const float panelH = contentSize.height;
    const float pad = 22.0f * s;
    const float rowH = 50.0f * s;
    const float iconSz = 34.0f * s;

    auto panel = createPausePanel(Size(panelW, panelH));
    panel->setPosition(pos);
    parent->addChild(panel, 20);

    auto itemInfo = [&](int weaponId, std::string& name, std::string& imgPath) {
        name = textByLanguage("Unequipped", u8"未装备");
        imgPath.clear();
        for (const auto& opt : options)
        {
            if (opt.id == weaponId)
            {
                name = opt.name;
                imgPath = opt.imagePath;
                return;
            }
        }
    };

    const Size titleSize(panelW - pad * 2.0f, 44.0f * s);
    float yTop = pos.y + panelH - pad;
    auto eqTitle = createPauseSectionTitle(lm->getString("equipped"),
        titleSize, 22.0f * s);
    eqTitle->setPosition(Vec2(pos.x + pad, yTop - titleSize.height));
    parent->addChild(eqTitle, 30);

    for (int i = 0; i < 2; ++i)
    {
        int weaponId = (i < (int)_weaponLoadoutIds.size()) ? _weaponLoadoutIds[i] : -1;
        std::string name;
        std::string imgPath;
        itemInfo(weaponId, name, imgPath);
        auto row = createPauseItemRow("[" + std::to_string(i + 1) + "]", name, imgPath,
            Size(panelW - pad * 2.0f, rowH), iconSz, 16.0f * s);
        row->setPosition(Vec2(pos.x + pad,
            eqTitle->getPositionY() - 14.0f * s - (i + 1) * rowH - i * 8.0f * s));
        parent->addChild(row, 30);
    }

    const float backpackTitleY = pos.y + panelH * 0.54f;
    auto bpTitle = createPauseSectionTitle(lm->getString("backpack"),
        titleSize, 22.0f * s);
    bpTitle->setPosition(Vec2(pos.x + pad, backpackTitleY));
    parent->addChild(bpTitle, 30);

    const float scrollH = backpackTitleY - pos.y - pad - 14.0f * s;
    auto scroll = ui::ScrollView::create();
    scroll->setDirection(ui::ScrollView::Direction::VERTICAL);
    scroll->setBounceEnabled(true);
    scroll->setContentSize(Size(panelW - pad * 2.0f, scrollH));
    scroll->setInnerContainerSize(Size(panelW - pad * 2.0f,
        std::max(scrollH, (rowH + 8.0f * s) * 4.0f)));
    scroll->setPosition(Vec2(pos.x + pad, pos.y + pad));
    parent->addChild(scroll, 30);

    const Size innerSize = scroll->getInnerContainerSize();
    for (int i = 0; i < std::max(4, (int)_backpackWeaponIds.size()); ++i)
    {
        int weaponId = (i < (int)_backpackWeaponIds.size()) ? _backpackWeaponIds[i] : -1;
        std::string name;
        std::string imgPath;
        itemInfo(weaponId, name, imgPath);
        auto row = createPauseItemRow("", name, imgPath,
            Size(panelW - pad * 2.0f, rowH), iconSz, 16.0f * s);
        row->setPosition(Vec2(0.0f, innerSize.height - (i + 1) * rowH - i * 8.0f * s));
        scroll->addChild(row);
    }
}

void GameScene::applyWeaponLoadout()
{
    normalizeWeaponLists(_weaponLoadoutIds, _backpackWeaponIds);
    auto ud = UserDefault::getInstance();
    for (int i = 0; i < 2; ++i)
    {
        int id = (i < (int)_weaponLoadoutIds.size()) ? _weaponLoadoutIds[i] : i;
        ud->setIntegerForKey(("weapon_equipped_" + std::to_string(i)).c_str(), id);
    }
    for (int i = 0; i < 4; ++i)
    {
        int id = (i < (int)_backpackWeaponIds.size()) ? _backpackWeaponIds[i] : (i + 2);
        ud->setIntegerForKey(("weapon_backpack_" + std::to_string(i)).c_str(), id);
    }
    ud->flush();
    rebuildWeaponLoadout();
    refreshWeaponSlotUI();
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
