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
    Node* createBeveledButtonNode(const std::string& text,
        const Size& size,
        float fontSize,
        const Color3B& textColor,
        bool selected,
        bool disabled)
    {
        auto root = Node::create();
        root->setContentSize(size);

        float w = size.width;
        float h = size.height;
        float cut = std::min(h * 0.18f, 14.0f);

        auto drawChamfer = [](DrawNode* draw, float x, float y, float w, float h,
            float cut, const Color4F& color)
        {
            Vec2 points[8] = {
                Vec2(x + cut, y),
                Vec2(x + w - cut, y),
                Vec2(x + w, y + cut),
                Vec2(x + w, y + h - cut),
                Vec2(x + w - cut, y + h),
                Vec2(x + cut, y + h),
                Vec2(x, y + h - cut),
                Vec2(x, y + cut)
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
            label->setScaleX(1.08f);
            label->setPosition(Vec2(w * 0.5f, h * 0.54f));
            root->addChild(label);
        }

        return root;
    }

    Node* createMenuImageOrLabel(const std::string& imagePath,
        const std::string& fallbackText,
        const Size& targetSize,
        float fontSize,
        const Color3B& color,
        bool selected = false,
        bool disabled = false)
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

        Size visualSize = targetSize;
        std::string referencePath = AssetPaths::resolve("art/ui/menu_endless.png");
        if (!referencePath.empty())
        {
            auto reference = Sprite::create(referencePath);
            if (reference)
            {
                Size refSize = reference->getContentSize();
                if (refSize.width > 0.0f && refSize.height > 0.0f)
                {
                    visualSize.width = targetSize.height * (refSize.width / refSize.height);
                    visualSize.height = targetSize.height;
                }
            }
        }

        auto fallback = createBeveledButtonNode(fallbackText, visualSize, fontSize, color, selected, disabled);
        fallback->setPosition(Vec2((targetSize.width - visualSize.width) * 0.5f,
            (targetSize.height - visualSize.height) * 0.5f));
        root->addChild(fallback);
        return root;
    }

    MenuItemSprite* createMenuImageButton(const std::string& imagePath,
        const std::string& fallbackText,
        const Size& targetSize,
        float fontSize,
        const Color3B& color,
        const ccMenuCallback& callback)
    {
        auto normal = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, color);
        auto selected = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, color, true, false);
        auto disabled = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, Color3B(95, 95, 105), false, true);
        selected->setScale(0.96f);
        selected->setOpacity(220);
        disabled->setOpacity(125);
        return MenuItemSprite::create(normal, selected, disabled, callback);
    }
}

Scene* MainMenuScene::createScene()
{
    // Restore saved language on startup
    auto ud = UserDefault::getInstance();
    int langIdx = ud->getIntegerForKey("language_index", 0);
    LanguageManager::getInstance()->setLanguage(
        LanguageManager::intToLanguage(langIdx));

    return MainMenuScene::create();
}

bool MainMenuScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->preloadAll();
    AudioManager::getInstance()->playMenuBGM();

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor 鈥?keeps layout & font sizes proportional across resolutions.
    // Base: 960脳640 鈫?s=1.0.  1920脳1080 鈫?s鈮?.69.
    float s = winSize.height / 640.0f;

    // --- Background (image if provided, color fallback) ---
    std::string menuBgPath = AssetPaths::resolve("art/ui/main_menu_background.png");
    if (!menuBgPath.empty())
    {
        auto bgSprite = Sprite::create(menuBgPath);
        if (bgSprite)
        {
            bgSprite->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
                origin.y + visibleSize.height * 0.5f));
            Size bgSize = bgSprite->getContentSize();
            if (bgSize.width > 0.0f && bgSize.height > 0.0f)
            {
                bgSprite->setScale(std::max(visibleSize.width / bgSize.width,
                    visibleSize.height / bgSize.height));
            }
            this->addChild(bgSprite, -2);
        }
    }

    if (menuBgPath.empty())
    {
        auto bg = LayerColor::create(Color4B(25, 30, 45, 255), winSize.width, winSize.height);
        bg->setPosition(Vec2::ZERO);
        this->addChild(bg, -1);
    }

    auto ud = UserDefault::getInstance();

    // --- Story Mode + Endless (left column) ---
    Vector<MenuItem*> leftItems;
    Size buttonSize(300.0f * s, 48.0f * s);

    auto storyItem = createMenuImageButton("art/ui/menu_story.png",
        lm->getString("story_mode"),
        Size(400.0f * s, 56.0f * s), 38.0f * s,
        Color3B(245, 248, 255),
        CC_CALLBACK_1(MainMenuScene::onStoryModeClicked, this));

    auto endlessItem = createMenuImageButton("art/ui/menu_endless.png", "Endless Mode",
        Size(400.0f * s, 56.0f * s), 24.0f * s, Color3B(245, 175, 95),
        CC_CALLBACK_1(MainMenuScene::onEndlessClicked, this));

    leftItems.pushBack(storyItem);
    leftItems.pushBack(endlessItem);

    // --- Leaderboard + Settings + Exit (right column) ---
    Vector<MenuItem*> rightItems;

    auto leaderboardItem = createMenuImageButton("art/ui/menu_leaderboard.png", "Leaderboard",
        buttonSize, 25.0f * s, Color3B(140, 220, 190),
        CC_CALLBACK_1(MainMenuScene::onLeaderboardClicked, this));

    auto settingsItem = createMenuImageButton("art/ui/menu_settings.png", lm->getString("settings"),
        buttonSize, 25.0f * s, Color3B(160, 180, 220),
        CC_CALLBACK_1(MainMenuScene::onSettingsClicked, this));

    auto exitItem = createMenuImageButton("art/ui/menu_exit.png", lm->getString("exit_game"),
        buttonSize, 25.0f * s, Color3B(200, 130, 130),
        CC_CALLBACK_1(MainMenuScene::onExitGameClicked, this));

    rightItems.pushBack(leaderboardItem);
    rightItems.pushBack(settingsItem);
    rightItems.pushBack(exitItem);

    if (storyItem && endlessItem && leaderboardItem && settingsItem && exitItem)
    {
        float centerY = origin.y + visibleSize.height * 0.47f;
        float columnOffset = 170.0f * s;

        auto leftMenu = Menu::createWithArray(leftItems);
        leftMenu->setPosition(Vec2(origin.x + visibleSize.width * 0.5f - columnOffset, centerY));
        leftMenu->alignItemsVerticallyWithPadding(16.0f * s);
        this->addChild(leftMenu, 2);

        auto rightMenu = Menu::createWithArray(rightItems);
        rightMenu->setPosition(Vec2(origin.x + visibleSize.width * 0.5f + columnOffset, centerY));
        rightMenu->alignItemsVerticallyWithPadding(18.0f * s);
        this->addChild(rightMenu, 2);
    }

    // --- Subtitle hint ---
    auto hint = Label::createWithSystemFont(lm->getString("mainmenu_hint"), "Arial", 18.0f * s);
    if (hint)
    {
        hint->setColor(Color3B(150, 150, 170));
        hint->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + 60.0f * s
        ));
        this->addChild(hint, 1);
    }

    return true;
}

void MainMenuScene::onStoryModeClicked(Ref* sender)
{
    AudioManager::getInstance()->playButtonClick();
    CCLOG("Story Mode Clicked!");
    auto storyScene = StoryModeScene::createScene();
    Director::getInstance()->replaceScene(storyScene);
}

void MainMenuScene::onEndlessClicked(Ref* sender)
{
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey("selected_game_mode", 1);
    ud->setIntegerForKey("selected_level", 1);
    ud->flush();

    AudioManager::getInstance()->playButtonClick();
    AudioManager::getInstance()->playGameStart();
    Director::getInstance()->replaceScene(GameScene::createScene());
}

void MainMenuScene::onSettingsClicked(Ref* sender)
{
    AudioManager::getInstance()->playButtonClick();
    CCLOG("Settings Clicked!");
    auto settingsScene = SettingsScene::createScene(SettingsScene::Entry::MAIN_MENU);
    Director::getInstance()->replaceScene(settingsScene);
}

void MainMenuScene::onExitGameClicked(Ref* sender)
{
    AudioManager::getInstance()->playButtonClick();
    CCLOG("Exit Game Clicked!");
    Director::getInstance()->end();
}

void MainMenuScene::onLeaderboardClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->replaceScene(LeaderboardScene::createScene());
}
