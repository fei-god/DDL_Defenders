#include "GameOverScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"

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

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor
    float s = winSize.height / 640.0f;

    // --- Background (covers full design resolution to avoid gaps) ---
    auto bg = LayerColor::create(Color4B(30, 20, 25, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    // --- Game Over title ---
    auto title = Label::createWithSystemFont(lm->getString("gameover_title"), "Arial", 56.0f * s);
    title->setColor(Color3B(220, 60, 60));
    title->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.7f
    ));
    this->addChild(title, 1);

    // --- Survival time ---
    std::string timeStr = lm->getStringF("survival_time_fmt", _survivalTime);
    auto timeLabel = Label::createWithSystemFont(timeStr, "Arial", 28.0f * s);
    timeLabel->setColor(Color3B(200, 200, 210));
    timeLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.55f
    ));
    this->addChild(timeLabel, 1);

    // --- Restart button ---
    auto restartLabel = Label::createWithSystemFont(lm->getString("restart"), "Arial", 34.0f * s);
    restartLabel->setColor(Color3B(100, 220, 100));
    auto restartItem = MenuItemLabel::create(restartLabel,
        CC_CALLBACK_1(GameOverScene::onRestartClicked, this));

    // --- Title button ---
    auto titleLabel = Label::createWithSystemFont(lm->getString("back_to_title"), "Arial", 34.0f * s);
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
        menu->alignItemsVerticallyWithPadding(30.0f * s);
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
