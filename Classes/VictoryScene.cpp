#include "VictoryScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/SaveManager.h"
#include "Managers/AudioManager.h"
#include <cstring>
#include <new>

USING_NS_CC;

Scene* VictoryScene::createScene(float survivalTime, int kills, int progress, int score)
{
    auto scene = new (std::nothrow) VictoryScene();
    if (scene)
    {
        scene->setResultData(survivalTime, kills, progress, score);
        if (scene->init())
        {
            scene->autorelease();
            return scene;
        }
        CC_SAFE_DELETE(scene);
    }
    return nullptr;
}

void VictoryScene::setResultData(float survivalTime, int kills, int progress, int score)
{
    _survivalTime = survivalTime;
    _kills = kills;
    _progress = progress;
    _score = score;
}

bool VictoryScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->playVictory();

    GameResultData data;
    data.playerId = 0;
    std::strncpy(data.playerName, "Player", 31);
    data.score = _score;
    data.survivalTime = static_cast<int>(_survivalTime);
    data.progress = _progress;
    data.kills = _kills;
    data.result = static_cast<int>(GameResult::Win);
    SaveManager::getInstance()->updatePlayerAfterGame(data);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto bg = LayerColor::create(Color4B(22, 42, 34, 255), visibleSize.width, visibleSize.height);
    bg->setPosition(Vec2::ZERO);
    addChild(bg, -1);

    auto title = Label::createWithSystemFont("DDL Completed!", "Arial", 54);
    title->setColor(Color3B(120, 230, 150));
    title->setPosition(origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.72f));
    addChild(title);

    char buffer[160];
    snprintf(buffer, sizeof(buffer), "Time: %.1fs  Kills: %d  Progress: %d%%  Score: %d",
        _survivalTime, _kills, _progress, _score);
    auto detail = Label::createWithSystemFont(buffer, "Arial", 26);
    detail->setColor(Color3B(220, 230, 220));
    detail->setPosition(origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.56f));
    addChild(detail);

    auto restartLabel = Label::createWithSystemFont("Restart", "Arial", 32);
    restartLabel->setColor(Color3B(120, 220, 120));
    auto restart = MenuItemLabel::create(restartLabel, CC_CALLBACK_1(VictoryScene::onRestartClicked, this));

    auto titleLabel = Label::createWithSystemFont("Back to Menu", "Arial", 32);
    titleLabel->setColor(Color3B(220, 200, 130));
    auto back = MenuItemLabel::create(titleLabel, CC_CALLBACK_1(VictoryScene::onTitleClicked, this));

    auto menu = Menu::create(restart, back, nullptr);
    menu->setPosition(origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.35f));
    menu->alignItemsVerticallyWithPadding(28);
    addChild(menu);

    return true;
}

void VictoryScene::onRestartClicked(Ref*)
{
    Director::getInstance()->replaceScene(GameScene::createScene());
}

void VictoryScene::onTitleClicked(Ref*)
{
    Director::getInstance()->replaceScene(MainMenuScene::createScene());
}
