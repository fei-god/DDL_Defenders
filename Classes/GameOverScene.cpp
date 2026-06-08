#include "GameOverScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"

USING_NS_CC;

Scene* GameOverScene::createScene(float survivalTime)
{
    auto scene = GameOverScene::create();
    if (scene)
    {
        scene->setSurvivalTime(survivalTime);
    }
    return scene;
}

void GameOverScene::setSurvivalTime(float time)
{
    _survivalTime = time;
}

bool GameOverScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // --- Background ---
    auto bg = LayerColor::create(Color4B(30, 20, 25, 255), visibleSize.width, visibleSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    // --- Game Over title ---
    auto title = Label::createWithSystemFont("Game Over", "Arial", 56);
    title->setColor(Color3B(220, 60, 60));
    title->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.7f
    ));
    this->addChild(title, 1);

    // --- Survival time ---
    char buf[32];
    snprintf(buf, sizeof(buf), "Survival Time: %.1fs", _survivalTime);
    auto timeLabel = Label::createWithSystemFont(buf, "Arial", 28);
    timeLabel->setColor(Color3B(200, 200, 210));
    timeLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.55f
    ));
    this->addChild(timeLabel, 1);

    // --- Restart button ---
    auto restartLabel = Label::createWithSystemFont("Restart", "Arial", 34);
    restartLabel->setColor(Color3B(100, 220, 100));
    auto restartItem = MenuItemLabel::create(restartLabel,
        CC_CALLBACK_1(GameOverScene::onRestartClicked, this));

    // --- Title button ---
    auto titleLabel = Label::createWithSystemFont("Back to Title", "Arial", 34);
    titleLabel->setColor(Color3B(200, 180, 120));
    auto titleItem = MenuItemLabel::create(titleLabel,
        CC_CALLBACK_1(GameOverScene::onTitleClicked, this));

    if (restartItem && titleItem)
    {
        auto menu = Menu::create(restartItem, titleItem, NULL);
        menu->setPosition(Vec2(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height * 0.35f
        ));
        menu->alignItemsVerticallyWithPadding(30.0f);
        this->addChild(menu, 2);
    }

    return true;
}

void GameOverScene::onRestartClicked(Ref* sender)
{
    CCLOG("Restart Game");
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(gameScene);
}

void GameOverScene::onTitleClicked(Ref* sender)
{
    CCLOG("Back to Title");
    auto titleScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(titleScene);
}
