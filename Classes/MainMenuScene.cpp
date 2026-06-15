#include "MainMenuScene.h"
#include "GameScene.h"
#include "SettingsScene.h"
#include "LeaderboardScene.h"
#include "Managers/AudioManager.h"
#include "Managers/LanguageManager.h"
#include "Core/AssetPaths.h"
#include "base/CCUserDefault.h"
#include <algorithm>
#include <string>

USING_NS_CC;

namespace
{
    Node* createMenuImageOrLabel(const std::string& imagePath,
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
                    sprite->setScale(std::min(targetSize.width / imageSize.width,
                        targetSize.height / imageSize.height));
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

    MenuItemSprite* createMenuImageButton(const std::string& imagePath,
        const std::string& fallbackText,
        const Size& targetSize,
        float fontSize,
        const Color3B& color,
        const ccMenuCallback& callback)
    {
        auto normal = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, color);
        auto selected = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, color);
        auto disabled = createMenuImageOrLabel(imagePath, fallbackText, targetSize, fontSize, Color3B(95, 95, 105));
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
    int unlockedLevel = ud->getIntegerForKey("unlocked_level", 1);
    if (unlockedLevel < 1) unlockedLevel = 1;

    // --- Level select + menu buttons ---
    Vector<MenuItem*> levelItems;
    Size buttonSize(300.0f * s, 48.0f * s);
    for (int level = 1; level <= 5; ++level)
    {
        std::string labelText = "Level " + std::to_string(level);
        if (level > unlockedLevel)
        {
            labelText += "  Locked";
        }

        auto levelItem = createMenuImageButton(
            "art/ui/menu_level" + std::to_string(level) + ".png",
            labelText,
            buttonSize,
            24.0f * s,
            Color3B(220, 220, 100),
            CC_CALLBACK_1(MainMenuScene::onLevelClicked, this));
        levelItem->setTag(level);
        if (level > unlockedLevel)
        {
            levelItem->setEnabled(false);
        }
        levelItems.pushBack(levelItem);
    }

    auto endlessItem = createMenuImageButton("art/ui/menu_endless.png", "Endless Mode",
        buttonSize, 24.0f * s, Color3B(245, 175, 95),
        CC_CALLBACK_1(MainMenuScene::onEndlessClicked, this));

    auto leaderboardItem = createMenuImageButton("art/ui/menu_leaderboard.png", "Leaderboard",
        buttonSize, 25.0f * s, Color3B(140, 220, 190),
        CC_CALLBACK_1(MainMenuScene::onLeaderboardClicked, this));

    auto settingsItem = createMenuImageButton("art/ui/menu_settings.png", lm->getString("settings"),
        buttonSize, 25.0f * s, Color3B(160, 180, 220),
        CC_CALLBACK_1(MainMenuScene::onSettingsClicked, this));

    auto exitItem = createMenuImageButton("art/ui/menu_exit.png", lm->getString("exit_game"),
        buttonSize, 25.0f * s, Color3B(200, 130, 130),
        CC_CALLBACK_1(MainMenuScene::onExitGameClicked, this));

    if (leaderboardItem && settingsItem && exitItem)
    {
        Vector<MenuItem*> otherItems;
        otherItems.pushBack(endlessItem);
        otherItems.pushBack(leaderboardItem);
        otherItems.pushBack(settingsItem);
        otherItems.pushBack(exitItem);

        float centerY = origin.y + visibleSize.height * 0.47f;
        float columnOffset = 170.0f * s;
        auto levelMenu = Menu::createWithArray(levelItems);
        levelMenu->setPosition(Vec2(origin.x + visibleSize.width * 0.5f - columnOffset, centerY));
        levelMenu->alignItemsVerticallyWithPadding(10.0f * s);
        this->addChild(levelMenu, 2);

        auto otherMenu = Menu::createWithArray(otherItems);
        otherMenu->setPosition(Vec2(origin.x + visibleSize.width * 0.5f + columnOffset, centerY));
        otherMenu->alignItemsVerticallyWithPadding(18.0f * s);
        this->addChild(otherMenu, 2);
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

void MainMenuScene::onStartGameClicked(Ref* sender)
{
    AudioManager::getInstance()->playButtonClick();
    AudioManager::getInstance()->playGameStart();
    CCLOG("Start Game Clicked!");
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(gameScene);
}

void MainMenuScene::onLevelClicked(Ref* sender)
{
    auto item = dynamic_cast<MenuItem*>(sender);
    int level = item ? item->getTag() : 1;
    if (level < 1) level = 1;

    auto ud = UserDefault::getInstance();
    int unlockedLevel = ud->getIntegerForKey("unlocked_level", 1);
    if (level > unlockedLevel)
    {
        return;
    }

    ud->setIntegerForKey("selected_level", level);
    ud->setIntegerForKey("selected_game_mode", 0);
    ud->flush();

    AudioManager::getInstance()->playButtonClick();
    AudioManager::getInstance()->playGameStart();
    Director::getInstance()->replaceScene(GameScene::createScene());
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
