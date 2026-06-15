#include "GameOverScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"
#include "Managers/SaveManager.h"
#include "Managers/AudioManager.h"
#include "base/CCUserDefault.h"
#include <cstring>
#include <new>
#include <sstream>

USING_NS_CC;

Scene* GameOverScene::createScene(float survivalTime)
{
    return createScene(survivalTime, 0, 0, static_cast<int>(survivalTime));
}

Scene* GameOverScene::createScene(float survivalTime, int kills, int progress, int score)
{
    auto scene = new (std::nothrow) GameOverScene();
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

void GameOverScene::setSurvivalTime(float time)
{
    setResultData(time, 0, 0, static_cast<int>(time));
}

void GameOverScene::setResultData(float survivalTime, int kills, int progress, int score)
{
    _survivalTime = survivalTime;
    _kills = kills;
    _progress = progress;
    _score = score;
}

bool GameOverScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->playGameOver();

    GameResultData data;
    data.playerId = 0;
    std::strncpy(data.playerName, "Player", 31);
    data.playerName[31] = '\0';
    data.score = _score;
    data.survivalTime = static_cast<int>(_survivalTime);
    data.progress = _progress;
    data.kills = _kills;
    data.result = static_cast<int>(GameResult::Lose);
    SaveManager::getInstance()->updatePlayerAfterGame(data);

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;

    auto bg = LayerColor::create(Color4B(30, 20, 25, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    auto title = Label::createWithSystemFont(lm->getString("gameover_title"), "Arial", 56.0f * s);
    title->setColor(Color3B(220, 60, 60));
    title->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.72f));
    this->addChild(title, 1);

    std::string timeStr = lm->getStringF("survival_time_fmt", _survivalTime);
    auto timeLabel = Label::createWithSystemFont(timeStr, "Arial", 28.0f * s);
    timeLabel->setColor(Color3B(200, 200, 210));
    timeLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.58f));
    this->addChild(timeLabel, 1);

    bool endlessMode = UserDefault::getInstance()->getIntegerForKey("selected_game_mode", 0) == 1;
    char resultBuf[180];
    if (endlessMode)
    {
        snprintf(resultBuf, sizeof(resultBuf), "Kills: %d  Completed DDL: %d  Score: %d",
            _kills, _progress, _score);
    }
    else
    {
        snprintf(resultBuf, sizeof(resultBuf), "Kills: %d  Progress: %d%%  Score: %d",
            _kills, _progress, _score);
    }
    auto resultLabel = Label::createWithSystemFont(resultBuf, "Arial", 24.0f * s);
    resultLabel->setColor(Color3B(210, 210, 220));
    resultLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.51f));
    this->addChild(resultLabel, 1);

    auto records = SaveManager::getInstance()->loadLeaderboardByHighScore(5);
    float rowY = origin.y + visibleSize.height * 0.44f;
    int rank = 1;
    for (const auto& record : records)
    {
        std::ostringstream oss;
        oss << rank << ". " << record.playerName << " Score " << record.highScore
            << " Progress " << record.bestProgress << "%";
        auto row = Label::createWithSystemFont(oss.str(), "Arial", 18.0f * s);
        row->setColor(Color3B(190, 195, 205));
        row->setPosition(Vec2(origin.x + visibleSize.width / 2, rowY));
        this->addChild(row, 1);
        rowY -= 24.0f * s;
        ++rank;
    }

    auto restartLabel = Label::createWithSystemFont(lm->getString("restart"), "Arial", 34.0f * s);
    restartLabel->setColor(Color3B(100, 220, 100));
    auto restartItem = MenuItemLabel::create(restartLabel,
        CC_CALLBACK_1(GameOverScene::onRestartClicked, this));

    auto titleLabel = Label::createWithSystemFont(lm->getString("back_to_title"), "Arial", 34.0f * s);
    titleLabel->setColor(Color3B(200, 180, 120));
    auto titleItem = MenuItemLabel::create(titleLabel,
        CC_CALLBACK_1(GameOverScene::onTitleClicked, this));

    if (restartItem && titleItem)
    {
        auto menu = Menu::create(restartItem, titleItem, nullptr);
        menu->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.20f));
        menu->alignItemsVerticallyWithPadding(28.0f * s);
        this->addChild(menu, 2);
    }

    return true;
}

void GameOverScene::onRestartClicked(Ref*)
{
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(gameScene);
}

void GameOverScene::onTitleClicked(Ref*)
{
    auto titleScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(titleScene);
}
