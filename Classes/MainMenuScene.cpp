#include "MainMenuScene.h"
#include "GameScene.h"
#include "SettingsScene.h"
#include "StoryModeScene.h"
#include "LeaderboardScene.h"
#include "Managers/AudioManager.h"
#include "Managers/LanguageManager.h"
#include "Core/AssetPaths.h"
#include "base/CCUserDefault.h"
#include "2d/CCDrawNode.h"
#include <algorithm>
#include <string>

USING_NS_CC;

namespace
{
    std::string localizedButtonImagePath(const std::string& imagePath)
    {
        if (LanguageManager::getInstance()->getLanguage() !=
            LanguageManager::Language::ENGLISH)
            return imagePath;
        const std::string::size_type dot = imagePath.find_last_of('.');
        const std::string englishPath = dot == std::string::npos
            ? imagePath + "_eng"
            : imagePath.substr(0, dot) + "_eng" + imagePath.substr(dot);
        return AssetPaths::exists(englishPath) ? englishPath : imagePath;
    }

    Node* createBeveledButtonNode(const std::string& text,
        const Size& size, float fontSize, const Color3B& textColor,
        bool selected, bool disabled)
    {
        auto root = Node::create();
        root->setContentSize(size);
        float w = size.width, h = size.height;
        float cut = std::min(h * 0.18f, 14.0f);

        auto drawChamfer = [](DrawNode* draw, float x, float y, float w, float h,
            float cut, const Color4F& color) {
            Vec2 points[8] = {
                Vec2(x + cut, y), Vec2(x + w - cut, y),
                Vec2(x + w, y + cut), Vec2(x + w, y + h - cut),
                Vec2(x + w - cut, y + h), Vec2(x + cut, y + h),
                Vec2(x, y + h - cut), Vec2(x, y + cut)
            };
            draw->drawSolidPoly(points, 8, color);
        };

        auto body = DrawNode::create();
        Color4F shadow = disabled ? Color4F(0.03f, 0.04f, 0.06f, 0.72f) : Color4F(0.01f, 0.04f, 0.09f, 0.88f);
        Color4F shell = disabled ? Color4F(0.20f, 0.23f, 0.30f, 0.95f) : Color4F(0.02f, 0.20f, 0.40f, 1.0f);
        Color4F rim = disabled ? Color4F(0.43f, 0.46f, 0.54f, 0.82f) : Color4F(0.15f, 0.58f, 0.98f, 1.0f);
        Color4F innerShell = disabled ? Color4F(0.08f, 0.10f, 0.14f, 0.96f) : Color4F(0.01f, 0.08f, 0.18f, 1.0f);
        Color4F fillTop = disabled ? Color4F(0.13f, 0.15f, 0.19f, 0.78f)
            : (selected ? Color4F(0.10f, 0.36f, 0.58f, 0.98f) : Color4F(0.07f, 0.27f, 0.48f, 0.96f));
        Color4F fillBottom = disabled ? Color4F(0.08f, 0.09f, 0.12f, 0.82f)
            : (selected ? Color4F(0.02f, 0.14f, 0.28f, 0.98f) : Color4F(0.01f, 0.10f, 0.22f, 0.98f));

        drawChamfer(body, 0.0f, -3.0f, w, h, cut, shadow);
        drawChamfer(body, 0.0f, 0.0f, w, h, cut, shell);
        drawChamfer(body, 3.0f, 3.0f, w - 6.0f, h - 6.0f, std::max(2.0f, cut - 3.0f), rim);
        drawChamfer(body, 6.0f, 6.0f, w - 12.0f, h - 12.0f, std::max(2.0f, cut - 6.0f), innerShell);
        drawChamfer(body, 8.0f, 8.0f, w - 16.0f, h - 16.0f, std::max(2.0f, cut - 8.0f), fillBottom);
        drawChamfer(body, 10.0f, h * 0.52f, w - 20.0f, h * 0.27f, std::max(2.0f, cut - 10.0f),
            Color4F(fillTop.r + 0.03f, fillTop.g + 0.05f, fillTop.b + 0.06f, fillTop.a));
        drawChamfer(body, 11.0f, h - 12.0f, w - 22.0f, 2.0f, 1.0f,
            disabled ? Color4F(0.55f, 0.56f, 0.60f, 0.20f) : Color4F(0.78f, 0.93f, 1.0f, 0.35f));
        root->addChild(body);

        auto label = Label::createWithSystemFont(text, "Arial", fontSize);
        if (label)
        {
            label->setColor(disabled ? Color3B(115, 118, 128) : textColor);
            label->enableBold();
            label->enableItalics();
            label->enableOutline(disabled ? Color4B(20, 22, 28, 180) : Color4B(5, 18, 36, 230),
                std::max(2, static_cast<int>(fontSize * 0.11f)));
            label->enableShadow(Color4B(0, 0, 0, 220), Size(2.5f, -2.5f), 1);
            label->setSkewX(-8.0f);
            label->setScale(1.08f);
            label->setPosition(Vec2(w * 0.5f, h * 0.54f));
            root->addChild(label);
        }
        return root;
    }

    Node* createMenuImageOrLabel(const std::string& imagePath,
        const std::string& fallbackText, const Size& targetSize,
        float fontSize, const Color3B& color,
        bool selected = false, bool disabled = false)
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
                    sprite->setScale(std::min(targetSize.width / imageSize.width,
                        targetSize.height / imageSize.height));
                }
                sprite->setPosition(Vec2(targetSize.width * 0.5f, targetSize.height * 0.5f));
                root->addChild(sprite);
                return root;
            }
        }

        auto fallback = createBeveledButtonNode(fallbackText, targetSize, fontSize, color, selected, disabled);
        fallback->setPosition(Vec2::ZERO);
        root->addChild(fallback);
        return root;
    }

    MenuItemSprite* createMenuImageButton(const std::string& imagePath,
        const std::string& fallbackText, const Size& targetSize,
        float fontSize, const Color3B& color, const ccMenuCallback& callback)
    {
        const std::string localizedPath = localizedButtonImagePath(imagePath);
        auto normal = createMenuImageOrLabel(localizedPath, fallbackText, targetSize, fontSize, color);
        auto selected = createMenuImageOrLabel(localizedPath, fallbackText, targetSize, fontSize, color, true, false);
        auto disabled = createMenuImageOrLabel(localizedPath, fallbackText, targetSize, fontSize, Color3B(95, 95, 105), false, true);
        selected->setScale(0.96f);
        selected->setOpacity(220);
        disabled->setOpacity(125);
        return MenuItemSprite::create(normal, selected, disabled, callback);
    }
}

// ---------------------------------------------------------------------------
Scene* MainMenuScene::createScene()
{
    auto ud = UserDefault::getInstance();
    int langIdx = ud->getIntegerForKey("language_index", 0);
    LanguageManager::getInstance()->setLanguage(
        LanguageManager::intToLanguage(langIdx));
    return MainMenuScene::create();
}

// ---------------------------------------------------------------------------
bool MainMenuScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->preloadAll();
    AudioManager::getInstance()->playMenuBGM();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float cx = origin.x + visibleSize.width / 2;
    float cy = origin.y + visibleSize.height / 2;

    // Scale factor — base: 1920×1080
    _s = std::min(winSize.height / 1080.0f, winSize.width / 1920.0f);
    _s = std::max(0.62f, _s);

    // --- Background Normal ---
    std::string normalPath = AssetPaths::resolve("art/ui/main_menu_background.png");
    if (!normalPath.empty())
    {
        _bgNormal = Sprite::create(normalPath);
        if (_bgNormal)
        {
            _bgNormal->setPosition(Vec2(cx, cy));
            Size bgSize = _bgNormal->getContentSize();
            if (bgSize.width > 0.0f && bgSize.height > 0.0f)
                _bgNormal->setScale(std::max(visibleSize.width / bgSize.width,
                    visibleSize.height / bgSize.height));
            _bgNormal->setOpacity(255);
            this->addChild(_bgNormal, -2);
        }
    }
    if (!_bgNormal)
    {
        auto fallback = LayerColor::create(Color4B(25, 30, 45, 255), winSize.width, winSize.height);
        fallback->setPosition(Vec2::ZERO);
        this->addChild(fallback, -2);
    }

    // --- Background Blur ---
    std::string blurPath = AssetPaths::resolve("art/ui/main_menu_blur.png");
    if (!blurPath.empty())
    {
        _bgBlur = Sprite::create(blurPath);
        if (_bgBlur)
        {
            _bgBlur->setPosition(Vec2(cx, cy));
            Size bgSize = _bgBlur->getContentSize();
            if (bgSize.width > 0.0f && bgSize.height > 0.0f)
            {
                float coverScale = std::max(visibleSize.width / bgSize.width,
                    visibleSize.height / bgSize.height);
                _bgBlur->setScale(coverScale);
            }
            _bgBlur->setOpacity(0);
            this->addChild(_bgBlur, -1);
        }
    }
    if (!_bgBlur)
    {
        auto fallback = LayerColor::create(Color4B(5, 8, 18, 220),
            visibleSize.width, visibleSize.height);
        fallback->setPosition(origin);
        fallback->setOpacity(0);
        this->addChild(fallback, -1);
        _bgBlur = fallback;
    }

    // --- Menu Root (initially hidden) ---
    _menuRoot = Node::create();
    _menuRoot->setVisible(false);
    this->addChild(_menuRoot, 10);

    auto* lm = LanguageManager::getInstance();

    // Button positions (3/4 of 4x = 3x original size)
    float offsetsY[4] = { 300.0f, 100.0f, -100.0f, -300.0f };
    float scales[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::string paths[4] = {
        "art/ui/menu_story.png",
        "art/ui/menu_endless.png",
        "art/ui/menu_settings.png",
        "art/ui/menu_exit.png"
    };
    std::string fallbacks[4] = {
        lm->getString("story_mode"), "Endless Mode",
        lm->getString("settings"), lm->getString("exit_game")
    };
    Color3B colors[4] = {
        Color3B(245, 248, 255), Color3B(245, 175, 95),
        Color3B(160, 180, 220), Color3B(200, 130, 130)
    };
    ccMenuCallback callbacks[4] = {
        CC_CALLBACK_1(MainMenuScene::onStoryModeClicked, this),
        CC_CALLBACK_1(MainMenuScene::onEndlessClicked, this),
        CC_CALLBACK_1(MainMenuScene::onSettingsClicked, this),
        CC_CALLBACK_1(MainMenuScene::onExitGameClicked, this)
    };
    MenuItemSprite** btnRefs[4] = { &_btnStory, &_btnEndless, &_btnSettings, &_btnExit };

    for (int i = 0; i < 4; ++i)
    {
        float y = cy + offsetsY[i] * _s;
        Vec2 targetPos(cx, y);
        Vec2 startPos(cx, y + 90.0f * _s);
        _btnTargetPos[i] = targetPos;
        _btnStartPos[i] = startPos;

        Size btnSize(1140.0f * _s, 192.0f * _s);
        auto btn = createMenuImageButton(paths[i], fallbacks[i], btnSize,
            54.0f * _s, colors[i], callbacks[i]);
        if (btn)
        {
            btn->setPosition(startPos);
            btn->setScale(scales[i]);
            btn->setOpacity(0);
            btn->setEnabled(false);
            *btnRefs[i] = btn;
        }
    }

    // Build a single Menu from all 4 buttons
    Vector<MenuItem*> allItems;
    if (_btnStory) allItems.pushBack(_btnStory);
    if (_btnEndless) allItems.pushBack(_btnEndless);
    if (_btnSettings) allItems.pushBack(_btnSettings);
    if (_btnExit) allItems.pushBack(_btnExit);

    auto menu = Menu::createWithArray(allItems);
    menu->setPosition(Vec2::ZERO);
    _menuRoot->addChild(menu);

    // --- Touch listener: click anywhere to start ---
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = [this](Touch*, Event*) -> bool {
        if (_state == MenuState::MAIN)
        {
            startTransition();
        }
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    return true;
}

// ---------------------------------------------------------------------------
void MainMenuScene::startTransition()
{
    if (_state != MenuState::MAIN) return;
    _state = MenuState::TRANSITION;

    // 1) Background animations (parallel, 0.6s)
    if (_bgNormal)
        _bgNormal->runAction(FadeOut::create(0.6f));

    if (_bgBlur)
    {
        auto fadeIn = FadeIn::create(0.6f);
        auto scaleUp = ScaleBy::create(0.6f, 1.05f);
        _bgBlur->runAction(Spawn::create(
            EaseCubicActionOut::create(fadeIn),
            EaseCubicActionOut::create(scaleUp),
            nullptr));
    }

    // 2) At 0.35s, show menu root + animate buttons
    auto showMenu = CallFunc::create([this]() {
        _menuRoot->setVisible(true);
        animateButtons();
    });
    this->runAction(Sequence::create(
        DelayTime::create(0.35f), showMenu, nullptr));
}

// ---------------------------------------------------------------------------
void MainMenuScene::animateButtons()
{
    std::vector<MenuItemSprite*> buttons = { _btnStory, _btnEndless, _btnSettings, _btnExit };
    float staggerDelays[4] = { 0.00f, 0.05f, 0.10f, 0.15f };
    float animDuration = 0.4f;

    for (int i = 0; i < 4; ++i)
    {
        auto btn = buttons[i];
        if (!btn) continue;

        // Move from startPos to targetPos + fade in
        Vec2 offset = _btnTargetPos[i] - _btnStartPos[i];
        auto move = MoveBy::create(animDuration, offset);
        auto fade = FadeIn::create(animDuration);
        auto spawn = Spawn::create(EaseBackOut::create(move), fade, nullptr);

        btn->runAction(Sequence::create(
            DelayTime::create(staggerDelays[i]),
            spawn,
            nullptr));
    }

    // After the last button finishes, enable menu interaction
    float lastFinish = staggerDelays[3] + animDuration;
    auto enableMenu = CallFunc::create([this]() {
        _canClickMenu = true;
        _state = MenuState::MENU;
        if (_btnStory) _btnStory->setEnabled(true);
        if (_btnEndless) _btnEndless->setEnabled(true);
        if (_btnSettings) _btnSettings->setEnabled(true);
        if (_btnExit) _btnExit->setEnabled(true);
    });
    this->runAction(Sequence::create(
        DelayTime::create(lastFinish), enableMenu, nullptr));
}

// ---------------------------------------------------------------------------
void MainMenuScene::onStoryModeClicked(Ref*)
{
    if (!_canClickMenu) return;
    AudioManager::getInstance()->playButtonClick();
    // Go through StoryModeScene for save/load management
    Director::getInstance()->replaceScene(StoryModeScene::createScene());
}

void MainMenuScene::onEndlessClicked(Ref*)
{
    if (!_canClickMenu) return;
    AudioManager::getInstance()->playButtonClick();
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey("selected_game_mode", 1); // endless
    ud->setIntegerForKey("selected_scene", 0);     // hub
    ud->setIntegerForKey("selected_level", 1);
    ud->flush();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.3f, GameScene::createScene(), Color3B::BLACK));
}

void MainMenuScene::onSettingsClicked(Ref*)
{
    if (!_canClickMenu) return;
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->replaceScene(
        SettingsScene::createScene(SettingsScene::Entry::MAIN_MENU));
}

void MainMenuScene::onExitGameClicked(Ref*)
{
    if (!_canClickMenu) return;
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->end();
}

void MainMenuScene::onLeaderboardClicked(Ref*)
{
    if (!_canClickMenu) return;
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->replaceScene(LeaderboardScene::createScene());
}
