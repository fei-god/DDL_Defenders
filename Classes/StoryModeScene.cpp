#include "StoryModeScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"
#include "Managers/AudioManager.h"
#include "base/CCUserDefault.h"
#include <algorithm>
#include <cstdio>
#include <ctime>

USING_NS_CC;

// ============================================================================
// Static save helpers
// ============================================================================

static std::string saveKey(int idx, const char* field)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "story_save_%d_%s", idx, field);
    return buf;
}

std::vector<StoryModeScene::SaveEntry> StoryModeScene::loadAllSaves()
{
    std::vector<SaveEntry> result;
    auto ud = UserDefault::getInstance();
    int count = ud->getIntegerForKey("story_save_count", 0);

    // Read all saves, then sort newest-first (highest index = newest)
    for (int i = 0; i < count; ++i)
    {
        SaveEntry e;
        e.index = i;  // storage index
        e.type = ud->getIntegerForKey(saveKey(i, "type").c_str(), 0);
        e.level = ud->getIntegerForKey(saveKey(i, "level").c_str(), 1);
        e.timestamp = ud->getStringForKey(saveKey(i, "ts").c_str(), "");
        if (!e.timestamp.empty())
            result.push_back(e);
    }

    // Sort newest first (higher storage index = newer)
    std::reverse(result.begin(), result.end());

    // Reassign display index after sorting
    for (size_t i = 0; i < result.size(); ++i)
        result[i].index = static_cast<int>(i);

    return result;
}

void StoryModeScene::addAutoSave(int level)
{
    auto ud = UserDefault::getInstance();
    int count = ud->getIntegerForKey("story_save_count", 0);

    // Count existing auto-saves
    int autoCount = 0;
    for (int i = 0; i < count; ++i)
    {
        if (ud->getIntegerForKey(saveKey(i, "type").c_str(), 0) == 0)
            ++autoCount;
    }

    // If we have 5 auto-saves, remove the oldest one
    if (autoCount >= 5)
    {
        // Find and remove the oldest auto-save (lowest index)
        for (int i = 0; i < count; ++i)
        {
            if (ud->getIntegerForKey(saveKey(i, "type").c_str(), 0) == 0)
            {
                // Shift all saves after this one down
                for (int j = i; j < count - 1; ++j)
                {
                    ud->setIntegerForKey(saveKey(j, "type").c_str(),
                        ud->getIntegerForKey(saveKey(j + 1, "type").c_str(), 0));
                    ud->setIntegerForKey(saveKey(j, "level").c_str(),
                        ud->getIntegerForKey(saveKey(j + 1, "level").c_str(), 1));
                    ud->setStringForKey(saveKey(j, "ts").c_str(),
                        ud->getStringForKey(saveKey(j + 1, "ts").c_str(), ""));
                }
                --count;
                break;
            }
        }
    }

    // Timestamp
    time_t now = time(nullptr);
    char timeBuf[32];
    struct tm localTm;
#ifdef _WIN32
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &localTm);

    // Append new auto-save at the end
    int idx = count;
    ud->setIntegerForKey(saveKey(idx, "type").c_str(), 0);  // auto
    ud->setIntegerForKey(saveKey(idx, "level").c_str(), level);
    ud->setStringForKey(saveKey(idx, "ts").c_str(), timeBuf);
    ud->setIntegerForKey("story_save_count", count + 1);
    ud->flush();
}

bool StoryModeScene::addManualSave(int level)
{
    auto ud = UserDefault::getInstance();
    int count = ud->getIntegerForKey("story_save_count", 0);

    // Count existing manual saves
    int manualCount = 0;
    for (int i = 0; i < count; ++i)
    {
        if (ud->getIntegerForKey(saveKey(i, "type").c_str(), 0) == 1)
            ++manualCount;
    }

    if (manualCount >= 10)
        return false;  // full

    // Timestamp
    time_t now = time(nullptr);
    char timeBuf[32];
    struct tm localTm;
#ifdef _WIN32
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &localTm);

    // Append new manual save at the end
    int idx = count;
    ud->setIntegerForKey(saveKey(idx, "type").c_str(), 1);  // manual
    ud->setIntegerForKey(saveKey(idx, "level").c_str(), level);
    ud->setStringForKey(saveKey(idx, "ts").c_str(), timeBuf);
    ud->setIntegerForKey("story_save_count", count + 1);
    ud->flush();
    return true;
}

void StoryModeScene::deleteSaveByIndex(int displayIndex)
{
    // displayIndex is the position in the newest-first sorted list.
    // We need to map it back to the storage index.
    auto saves = loadAllSaves();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(saves.size()))
        return;

    // The SaveEntry.index field from loadAllSaves is the display index.
    // We need to find the storage index.
    // Since loadAllSaves sorts newest-first, display index 0 = storage index (count-1),
    // display index 1 = storage index (count-2), etc.
    auto ud = UserDefault::getInstance();
    int count = ud->getIntegerForKey("story_save_count", 0);
    int storageIndex = count - 1 - displayIndex;

    // Shift all saves after storageIndex down by one
    for (int j = storageIndex; j < count - 1; ++j)
    {
        ud->setIntegerForKey(saveKey(j, "type").c_str(),
            ud->getIntegerForKey(saveKey(j + 1, "type").c_str(), 0));
        ud->setIntegerForKey(saveKey(j, "level").c_str(),
            ud->getIntegerForKey(saveKey(j + 1, "level").c_str(), 1));
        ud->setStringForKey(saveKey(j, "ts").c_str(),
            ud->getStringForKey(saveKey(j + 1, "ts").c_str(), ""));
    }
    ud->setIntegerForKey("story_save_count", count - 1);
    ud->flush();
}

// ============================================================================
// Scene
// ============================================================================

Scene* StoryModeScene::createScene()
{
    return StoryModeScene::create();
}

bool StoryModeScene::init()
{
    if (!Scene::init()) return false;

    AudioManager::getInstance()->playMenuBGM();

    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;

    // Background
    auto bg = LayerColor::create(Color4B(25, 30, 45, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    showMainView(s);
    return true;
}

void StoryModeScene::clearCurrentView()
{
    if (_currentView)
    {
        _currentView->removeFromParent();
        _currentView = nullptr;
    }
}

// ============================================================================
// Main View: New Game + Load Game + Back
// ============================================================================
void StoryModeScene::showMainView(float s)
{
    clearCurrentView();
    hideDeleteConfirm();

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    float cx = origin.x + visibleSize.width / 2;
    float cy = origin.y + visibleSize.height / 2;

    _currentView = Node::create();
    this->addChild(_currentView, 1);

    // Title
    auto title = Label::createWithSystemFont(lm->getString("story_mode"), "Arial", 48.0f * s);
    title->setColor(Color3B(220, 220, 100));
    title->setPosition(Vec2(cx, cy + 110.0f * s));
    _currentView->addChild(title);

    // New Game button (large, prominent)
    Size bigBtnSize(420.0f * s, 64.0f * s);
    auto newGameLabel = Label::createWithSystemFont(lm->getString("new_game"), "Arial", 30.0f * s);
    newGameLabel->setColor(Color3B(130, 230, 130));
    auto newGameItem = MenuItemLabel::create(newGameLabel,
        CC_CALLBACK_1(StoryModeScene::onNewGameClicked, this));

    // Load Game button
    auto loadGameLabel = Label::createWithSystemFont(lm->getString("load_game"), "Arial", 30.0f * s);
    loadGameLabel->setColor(Color3B(220, 200, 100));
    auto loadGameItem = MenuItemLabel::create(loadGameLabel,
        CC_CALLBACK_1(StoryModeScene::onLoadGameClicked, this));

    // Check if any saves exist
    auto saves = loadAllSaves();
    if (saves.empty())
    {
        loadGameItem->setEnabled(false);
        loadGameLabel->setColor(Color3B(95, 95, 105));
    }

    // Back button
    auto backLabel = Label::createWithSystemFont(lm->getString("back_to_title"), "Arial", 24.0f * s);
    backLabel->setColor(Color3B(180, 180, 200));
    auto backItem = MenuItemLabel::create(backLabel,
        CC_CALLBACK_1(StoryModeScene::onBackClicked, this));

    auto menu = Menu::create(newGameItem, loadGameItem, backItem, nullptr);
    menu->setPosition(Vec2(cx, cy - 10.0f * s));
    menu->alignItemsVerticallyWithPadding(30.0f * s);
    _currentView->addChild(menu);
}

// ============================================================================
// Load View: save list
// ============================================================================
void StoryModeScene::showLoadView(float s)
{
    clearCurrentView();
    hideDeleteConfirm();

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float cx = origin.x + visibleSize.width / 2;

    _currentView = Node::create();
    this->addChild(_currentView, 1);

    // Title
    auto title = Label::createWithSystemFont(lm->getString("load_game"), "Arial", 40.0f * s);
    title->setColor(Color3B(220, 200, 100));
    title->setPosition(Vec2(cx, origin.y + visibleSize.height * 0.88f));
    _currentView->addChild(title);

    auto saves = loadAllSaves();

    if (saves.empty())
    {
        auto noData = Label::createWithSystemFont(lm->getString("no_saves"), "Arial", 24.0f * s);
        noData->setColor(Color3B(140, 140, 160));
        noData->setPosition(Vec2(cx, origin.y + visibleSize.height * 0.5f));
        _currentView->addChild(noData);
    }
    else
    {
        float cardW = 560.0f * s;
        float cardH = 70.0f * s;
        float gap = 12.0f * s;
        float startY = origin.y + visibleSize.height * 0.78f;
        int maxVisible = 7;
        int totalToShow = std::min(static_cast<int>(saves.size()), maxVisible);

        for (int i = 0; i < totalToShow; ++i)
        {
            const auto& save = saves[i];
            float cardY = startY - i * (cardH + gap);

            // Card background
            auto cardBg = LayerColor::create(Color4B(35, 40, 60, 230), cardW, cardH);
            cardBg->setPosition(Vec2(cx - cardW * 0.5f, cardY - cardH * 0.5f));
            _currentView->addChild(cardBg);

            // Type label
            const char* typeKey = (save.type == 0) ? "auto_save" : "manual_save";
            Color3B typeColor = (save.type == 0) ? Color3B(100, 180, 220) : Color3B(130, 220, 130);
            auto typeLabel = Label::createWithSystemFont(lm->getString(typeKey), "Arial", 18.0f * s);
            typeLabel->setColor(typeColor);
            typeLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
            typeLabel->setPosition(Vec2(cx - cardW * 0.5f + 20.0f * s, cardY + 16.0f * s));
            _currentView->addChild(typeLabel);

            // Level + time info
            char levelBuf[64];
            snprintf(levelBuf, sizeof(levelBuf),
                lm->getString("level_info_fmt").c_str(), save.level);
            char infoBuf[128];
            snprintf(infoBuf, sizeof(infoBuf), "%s  |  %s", levelBuf, save.timestamp.c_str());
            auto infoLabel = Label::createWithSystemFont(infoBuf, "Arial", 16.0f * s);
            infoLabel->setColor(Color3B(160, 160, 180));
            infoLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
            infoLabel->setPosition(Vec2(cx - cardW * 0.5f + 20.0f * s, cardY - 16.0f * s));
            _currentView->addChild(infoLabel);

            // Load button (right side)
            auto loadBtnLabel = Label::createWithSystemFont(lm->getString("load"), "Arial", 20.0f * s);
            loadBtnLabel->setColor(Color3B(100, 220, 100));
            auto loadBtn = MenuItemLabel::create(loadBtnLabel,
                CC_CALLBACK_1(StoryModeScene::onSaveLoadClicked, this));
            loadBtn->setTag(i);  // display index

            // Delete button (only for manual saves)
            Vector<MenuItem*> btnItems;
            btnItems.pushBack(loadBtn);

            if (save.type == 1)  // manual save can be deleted
            {
                auto delBtnLabel = Label::createWithSystemFont(
                    "X", "Arial", 22.0f * s);
                delBtnLabel->setColor(Color3B(220, 120, 120));
                auto delBtn = MenuItemLabel::create(delBtnLabel,
                    CC_CALLBACK_1(StoryModeScene::onSaveDeleteClicked, this));
                delBtn->setTag(i);
                btnItems.pushBack(delBtn);
            }

            auto btnMenu = Menu::createWithArray(btnItems);
            btnMenu->setPosition(Vec2(cx + cardW * 0.5f - 50.0f * s, cardY));
            btnMenu->alignItemsHorizontallyWithPadding(16.0f * s);
            _currentView->addChild(btnMenu);
        }
    }

    // Back button
    auto backLabel = Label::createWithSystemFont(lm->getString("back"), "Arial", 24.0f * s);
    backLabel->setColor(Color3B(180, 180, 200));
    auto backItem = MenuItemLabel::create(backLabel,
        CC_CALLBACK_1(StoryModeScene::onLoadBackClicked, this));
    auto backMenu = Menu::create(backItem, nullptr);
    backMenu->setPosition(Vec2(cx, origin.y + 35.0f * s));
    _currentView->addChild(backMenu);
}

// ============================================================================
// Main view callbacks
// ============================================================================
void StoryModeScene::onNewGameClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    startNewGame();
}

void StoryModeScene::onLoadGameClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;
    showLoadView(s);
}

void StoryModeScene::onBackClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    Director::getInstance()->replaceScene(MainMenuScene::createScene());
}

// ============================================================================
// Load view callbacks
// ============================================================================
void StoryModeScene::onSaveLoadClicked(Ref* sender)
{
    auto item = dynamic_cast<MenuItem*>(sender);
    int displayIndex = item ? item->getTag() : 0;

    AudioManager::getInstance()->playButtonClick();
    loadSaveAndStart(displayIndex);
}

void StoryModeScene::onSaveDeleteClicked(Ref* sender)
{
    auto item = dynamic_cast<MenuItem*>(sender);
    int displayIndex = item ? item->getTag() : 0;
    showDeleteConfirm(displayIndex);
}

void StoryModeScene::onLoadBackClicked(Ref*)
{
    AudioManager::getInstance()->playButtonClick();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;
    showMainView(s);
}

// ============================================================================
// Start game
// ============================================================================
void StoryModeScene::startNewGame()
{
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey("selected_level", 1);
    ud->setIntegerForKey("selected_game_mode", 0);
    ud->flush();

    AudioManager::getInstance()->playGameStart();
    Director::getInstance()->replaceScene(GameScene::createScene());
}

void StoryModeScene::loadSaveAndStart(int displayIndex)
{
    auto saves = loadAllSaves();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(saves.size()))
        return;

    int level = saves[displayIndex].level;
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey("selected_level", level);
    ud->setIntegerForKey("selected_game_mode", 0);
    ud->flush();

    AudioManager::getInstance()->playGameStart();
    Director::getInstance()->replaceScene(GameScene::createScene());
}

// ============================================================================
// Delete confirmation
// ============================================================================
void StoryModeScene::showDeleteConfirm(int displayIndex)
{
    _pendingDeleteIndex = displayIndex;
    _confirmLayer = Node::create();
    this->addChild(_confirmLayer, 10);

    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;
    float cx = origin.x + visibleSize.width / 2;
    float cy = origin.y + visibleSize.height / 2;

    auto overlay = LayerColor::create(Color4B(0, 0, 0, 180), winSize.width, winSize.height);
    overlay->setPosition(Vec2::ZERO);
    _confirmLayer->addChild(overlay);

    Size dialogSize(400.0f * s, 140.0f * s);
    auto dialogBg = LayerColor::create(Color4B(45, 50, 70, 255),
        dialogSize.width, dialogSize.height);
    dialogBg->setPosition(Vec2(cx - dialogSize.width * 0.5f, cy - dialogSize.height * 0.5f));
    _confirmLayer->addChild(dialogBg);

    auto confirmLabel = Label::createWithSystemFont(
        lm->getString("delete_save_confirm"), "Arial", 22.0f * s);
    confirmLabel->setColor(Color3B(220, 220, 200));
    confirmLabel->setPosition(Vec2(cx, cy + 25.0f * s));
    _confirmLayer->addChild(confirmLabel);

    auto yesLabel = Label::createWithSystemFont(lm->getString("confirm"), "Arial", 24.0f * s);
    yesLabel->setColor(Color3B(220, 100, 100));
    auto yesItem = MenuItemLabel::create(yesLabel,
        CC_CALLBACK_1(StoryModeScene::onDeleteConfirmYes, this));

    auto noLabel = Label::createWithSystemFont(lm->getString("back"), "Arial", 24.0f * s);
    noLabel->setColor(Color3B(180, 180, 200));
    auto noItem = MenuItemLabel::create(noLabel,
        CC_CALLBACK_1(StoryModeScene::onDeleteConfirmNo, this));

    auto menu = Menu::create(yesItem, noItem, nullptr);
    menu->setPosition(Vec2(cx, cy - 30.0f * s));
    menu->alignItemsHorizontallyWithPadding(60.0f * s);
    _confirmLayer->addChild(menu);
}

void StoryModeScene::hideDeleteConfirm()
{
    if (_confirmLayer)
    {
        _confirmLayer->removeFromParent();
        _confirmLayer = nullptr;
    }
    _pendingDeleteIndex = -1;
}

void StoryModeScene::onDeleteConfirmYes(Ref*)
{
    if (_pendingDeleteIndex >= 0)
    {
        deleteSaveByIndex(_pendingDeleteIndex);
    }
    hideDeleteConfirm();

    auto winSize = Director::getInstance()->getWinSize();
    float s = winSize.height / 640.0f;
    showLoadView(s);  // refresh
}

void StoryModeScene::onDeleteConfirmNo(Ref*)
{
    hideDeleteConfirm();
}
