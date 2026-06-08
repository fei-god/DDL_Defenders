#include "MainMenuScene.h"
#include "GameScene.h"

USING_NS_CC;

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

bool MainMenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // --- Background ---
    auto bg = LayerColor::create(Color4B(25, 30, 45, 255), visibleSize.width, visibleSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    // --- Title ---
    auto titleLabel = Label::createWithSystemFont("DDL Defenders", "Arial", 48);
    if (titleLabel)
    {
        titleLabel->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - 80.0f
        ));
        this->addChild(titleLabel, 1);
    }

    // --- Menu buttons using MenuItemLabel (system font = full Unicode support) ---
    auto startLabel = Label::createWithSystemFont("开始游戏", "Arial", 32);     // 开始游戏
    auto startItem = MenuItemLabel::create(startLabel,
        CC_CALLBACK_1(MainMenuScene::onStartGameClicked, this));

    auto exitLabel = Label::createWithSystemFont("退出游戏", "Arial", 32);      // 退出游戏
    auto exitItem = MenuItemLabel::create(exitLabel,
        CC_CALLBACK_1(MainMenuScene::onExitGameClicked, this));

    if (startItem && exitItem)
    {
        // Give menu items a tint color so they stand out against dark bg
        startLabel->setColor(Color3B(220, 220, 100));   // warm yellow
        exitLabel->setColor(Color3B(200, 130, 130));     // muted red

        auto menu = Menu::create(startItem, exitItem, NULL);
        menu->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2
        ));
        menu->alignItemsVerticallyWithPadding(30.0f);
        this->addChild(menu, 2);
    }

    // --- Subtitle hint ---
    auto hint = Label::createWithSystemFont("WASD 移动 | 鼠标点击 射击", "Arial", 18);
    if (hint)
    {
        hint->setColor(Color3B(150, 150, 170));
        hint->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + 60.0f
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

void MainMenuScene::onExitGameClicked(Ref* sender)
{
    CCLOG("Exit Game Clicked!");
    Director::getInstance()->end();
}
