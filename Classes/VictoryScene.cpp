#include "VictoryScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/SaveManager.h"
#include "Managers/AudioManager.h"
#include "Core/AssetPaths.h"
#include <algorithm>
#include <cstring>
#include <new>

USING_NS_CC;

namespace
{
    Rect addFinalImage(Node* parent, const std::string& imagePath)
    {
        auto director = Director::getInstance();
        const Size visibleSize = director->getVisibleSize();
        const Vec2 origin = director->getVisibleOrigin();

        auto backdrop = LayerColor::create(Color4B::BLACK, visibleSize.width, visibleSize.height);
        backdrop->setPosition(origin);
        backdrop->setOpacity(0);
        parent->addChild(backdrop, -2);
        backdrop->runAction(FadeTo::create(0.45f, 255));

        auto bg = Sprite::create(AssetPaths::resolve(imagePath));
        if (!bg)
        {
            return Rect(origin.x, origin.y, visibleSize.width, visibleSize.height);
        }

        const Size imageSize = bg->getContentSize();
        const float scale = std::min(visibleSize.width / imageSize.width,
            visibleSize.height / imageSize.height);
        const Size displayedSize(imageSize.width * scale, imageSize.height * scale);

        bg->setScale(scale);
        bg->setPosition(origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
        bg->setOpacity(0);
        parent->addChild(bg, -1);
        bg->runAction(FadeIn::create(0.85f));

        return Rect(bg->getPositionX() - displayedSize.width * 0.5f,
            bg->getPositionY() - displayedSize.height * 0.5f,
            displayedSize.width,
            displayedSize.height);
    }

    MenuItemSprite* createImageHotspot(const Rect& imageRect,
        const Rect& normalizedRect,
        const ccMenuCallback& callback)
    {
        const Size size(imageRect.size.width * normalizedRect.size.width,
            imageRect.size.height * normalizedRect.size.height);
        auto normal = Node::create();
        normal->setContentSize(size);
        auto selected = Node::create();
        selected->setContentSize(size);

        auto item = MenuItemSprite::create(normal, selected, callback);
        item->setPosition(Vec2(
            imageRect.origin.x + imageRect.size.width * (normalizedRect.origin.x + normalizedRect.size.width * 0.5f),
            imageRect.origin.y + imageRect.size.height * (normalizedRect.origin.y + normalizedRect.size.height * 0.5f)));
        return item;
    }
}

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

    PlayerRecord previousRecord;
    const bool hadPreviousRecord = SaveManager::getInstance()->loadPlayerRecord(0, previousRecord);
    const int previousHighScore = hadPreviousRecord ? previousRecord.highScore : 0;
    const bool isNewHighScore = _score > previousHighScore;

    GameResultData data;
    data.playerId = 0;
    std::strncpy(data.playerName, "Player", 31);
    data.playerName[31] = '\0';
    data.score = _score;
    data.survivalTime = static_cast<int>(_survivalTime);
    data.progress = _progress;
    data.kills = _kills;
    data.result = static_cast<int>(GameResult::Win);
    SaveManager::getInstance()->updatePlayerAfterGame(data);

    const std::string imagePath = isNewHighScore
        ? "art/ui/final_success_at_high_score.png"
        : "art/ui/final_success.png";
    const Rect imageRect = addFinalImage(this, imagePath);

    auto leftConfirm = createImageHotspot(imageRect, Rect(0.10f, 0.06f, 0.38f, 0.22f),
        CC_CALLBACK_1(VictoryScene::onTitleClicked, this));
    auto rightConfirm = createImageHotspot(imageRect, Rect(0.52f, 0.06f, 0.38f, 0.22f),
        CC_CALLBACK_1(VictoryScene::onTitleClicked, this));

    auto menu = Menu::create(leftConfirm, rightConfirm, nullptr);
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 2);

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
