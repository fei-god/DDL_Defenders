#include "MainMenuScene.h"
#include "GameScene.h"
#include "SettingsScene.h"
#include "Managers/LanguageManager.h"
#include "base/CCUserDefault.h"

USING_NS_CC;

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

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor — keeps layout & font sizes proportional across resolutions.
    // Base: 960×640 → s=1.0.  1920×1080 → s≈1.69.
    float s = winSize.height / 640.0f;

    // --- Background (covers full design resolution to avoid gaps) ---
    auto bg = LayerColor::create(Color4B(25, 30, 45, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    // --- Title ---
    auto titleLabel = Label::createWithSystemFont("DDL Defenders", "Arial", 48.0f * s);
    if (titleLabel)
    {
        titleLabel->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - 80.0f * s
        ));
        this->addChild(titleLabel, 1);
    }

    // --- Menu buttons ---
    auto startLabel = Label::createWithSystemFont(lm->getString("start_game"), "Arial", 32.0f * s);
    auto startItem = MenuItemLabel::create(startLabel,
        CC_CALLBACK_1(MainMenuScene::onStartGameClicked, this));

    auto settingsLabel = Label::createWithSystemFont(lm->getString("settings"), "Arial", 32.0f * s);
    auto settingsItem = MenuItemLabel::create(settingsLabel,
        CC_CALLBACK_1(MainMenuScene::onSettingsClicked, this));

    auto exitLabel = Label::createWithSystemFont(lm->getString("exit_game"), "Arial", 32.0f * s);
    auto exitItem = MenuItemLabel::create(exitLabel,
        CC_CALLBACK_1(MainMenuScene::onExitGameClicked, this));

    if (startItem && settingsItem && exitItem)
    {
        startLabel->setColor(Color3B(220, 220, 100));    // warm yellow
        settingsLabel->setColor(Color3B(160, 180, 220)); // soft blue
        exitLabel->setColor(Color3B(200, 130, 130));      // muted red

        auto menu = Menu::create(startItem, settingsItem, exitItem, NULL);
        menu->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2
        ));
        menu->alignItemsVerticallyWithPadding(26.0f * s);
        this->addChild(menu, 2);
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
    CCLOG("Start Game Clicked!");
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(gameScene);
}

void MainMenuScene::onSettingsClicked(Ref* sender)
{
    CCLOG("Settings Clicked!");
    auto settingsScene = SettingsScene::createScene(SettingsScene::Entry::MAIN_MENU);
    Director::getInstance()->replaceScene(settingsScene);
}

void MainMenuScene::onExitGameClicked(Ref* sender)
{
    CCLOG("Exit Game Clicked!");
    Director::getInstance()->end();
}
