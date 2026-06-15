#include "LeaderboardScene.h"
#include "MainMenuScene.h"
#include "Core/AssetPaths.h"
#include "Managers/AudioManager.h"
#include "Managers/SaveManager.h"
#include <algorithm>
#include <sstream>

USING_NS_CC;

Scene* LeaderboardScene::createScene()
{
    return LeaderboardScene::create();
}

bool LeaderboardScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->playMenuBGM();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;

    std::string bgPath = AssetPaths::resolve("art/ui/leaderboard_background.png");
    if (!bgPath.empty())
    {
        auto bg = Sprite::create(bgPath);
        if (bg)
        {
            bg->setPosition(origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
            Size bgSize = bg->getContentSize();
            if (bgSize.width > 0.0f && bgSize.height > 0.0f)
            {
                bg->setScale(std::max(visibleSize.width / bgSize.width, visibleSize.height / bgSize.height));
            }
            addChild(bg, -3);
        }
    }
    else
    {
        auto bg = LayerColor::create(Color4B(16, 22, 33, 255), visibleSize.width, visibleSize.height);
        bg->setPosition(origin);
        addChild(bg, -3);
    }

    auto shade = LayerColor::create(Color4B(0, 0, 0, 90), visibleSize.width, visibleSize.height);
    shade->setPosition(origin);
    addChild(shade, -2);

    Size panelSize(visibleSize.width * 0.82f, visibleSize.height * 0.72f);
    Vec2 panelCenter = origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.52f);
    std::string panelPath = AssetPaths::resolve("art/ui/leaderboard_panel.png");
    if (!panelPath.empty())
    {
        auto panel = Sprite::create(panelPath);
        if (panel)
        {
            Size panelImageSize = panel->getContentSize();
            if (panelImageSize.width > 0.0f && panelImageSize.height > 0.0f)
            {
                panel->setScale(std::min(panelSize.width / panelImageSize.width,
                    panelSize.height / panelImageSize.height));
            }
            panel->setPosition(panelCenter);
            addChild(panel, -1);
        }
    }
    else
    {
        auto panel = LayerColor::create(Color4B(25, 31, 44, 235), panelSize.width, panelSize.height);
        panel->setIgnoreAnchorPointForPosition(false);
        panel->setAnchorPoint(Vec2(0.5f, 0.5f));
        panel->setPosition(panelCenter);
        addChild(panel, -1);
    }

    auto title = Label::createWithSystemFont("Leaderboard", "Arial", 34.0f * s);
    title->setColor(Color3B(245, 239, 210));
    title->setPosition(panelCenter + Vec2(0, panelSize.height * 0.34f));
    addChild(title);

    auto subtitle = Label::createWithSystemFont("Best DDL Defender Records", "Arial", 18.0f * s);
    subtitle->setColor(Color3B(170, 225, 235));
    subtitle->setPosition(panelCenter + Vec2(0, panelSize.height * 0.27f));
    addChild(subtitle);

    auto records = SaveManager::getInstance()->loadLeaderboardByHighScore(10);
    float y = panelCenter.y + panelSize.height * 0.17f;
    float columns[] = {
        panelCenter.x - panelSize.width * 0.30f,
        panelCenter.x - panelSize.width * 0.20f,
        panelCenter.x - panelSize.width * 0.06f,
        panelCenter.x + panelSize.width * 0.06f,
        panelCenter.x + panelSize.width * 0.19f,
        panelCenter.x + panelSize.width * 0.30f,
        panelCenter.x + panelSize.width * 0.39f
    };
    const char* headers[] = { "Rank", "Player", "Score", "Time", "Progress", "Kills", "Games" };
    for (int i = 0; i < 7; ++i)
    {
        auto header = Label::createWithSystemFont(headers[i], "Arial", 15.0f * s);
        header->setAnchorPoint(i == 1 ? Vec2(0.0f, 0.5f) : Vec2(0.5f, 0.5f));
        header->setColor(Color3B(150, 205, 220));
        header->setPosition(Vec2(columns[i], y));
        addChild(header, 2);
    }
    y -= 32.0f * s;

    int rank = 1;
    for (const auto& record : records)
    {
        auto rowBg = LayerColor::create(
            rank <= 3 ? Color4B(74, 60, 34, 160) : Color4B(38, 46, 60, 145),
            panelSize.width * 0.70f,
            25.0f * s);
        rowBg->setPosition(Vec2(panelCenter.x - panelSize.width * 0.35f, y - 12.5f * s));
        addChild(rowBg, 0);

        std::string values[] = {
            std::to_string(rank),
            record.playerName,
            std::to_string(record.highScore),
            std::to_string(record.bestSurvivalTime) + "s",
            std::to_string(record.bestProgress) + "%",
            std::to_string(record.bestKills),
            std::to_string(record.totalGames)
        };
        for (int i = 0; i < 7; ++i)
        {
            auto cell = Label::createWithSystemFont(values[i], "Arial", 15.0f * s);
            cell->setAnchorPoint(i == 1 ? Vec2(0.0f, 0.5f) : Vec2(0.5f, 0.5f));
            cell->setColor(rank <= 3 ? Color3B(255, 226, 145) : Color3B(220, 225, 235));
            cell->setPosition(Vec2(columns[i], y));
            addChild(cell, 2);
        }
        y -= 28.0f * s;
        ++rank;
    }

    if (records.empty())
    {
        auto empty = Label::createWithSystemFont("No records yet. Finish a game to create one.", "Arial", 26.0f * s);
        empty->setColor(Color3B(180, 185, 200));
        empty->setPosition(panelCenter);
        addChild(empty);
    }

    auto backLabel = Label::createWithSystemFont("Back", "Arial", 30.0f * s);
    backLabel->setColor(Color3B(220, 200, 130));
    auto back = MenuItemLabel::create(backLabel, CC_CALLBACK_1(LeaderboardScene::onBackClicked, this));
    auto menu = Menu::create(back, nullptr);
    menu->setPosition(panelCenter + Vec2(0, -panelSize.height * 0.36f));
    addChild(menu);

    return true;
}

void LeaderboardScene::onBackClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->replaceScene(MainMenuScene::createScene());
}
