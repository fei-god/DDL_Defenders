#include "StoryModeScene.h"
#include "GameScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"
#include "Managers/AudioManager.h"
#include "Core/AssetPaths.h"
#include "base/CCUserDefault.h"
#include "2d/CCDrawNode.h"
#include <algorithm>
#include <cstdio>
#include <ctime>

USING_NS_CC;

namespace
{
    Node* createUiPanel(const Size& size, const Color4B& fill)
    {
        auto root = Node::create();
        root->setContentSize(size);

        auto bg = LayerColor::create(fill, size.width, size.height);
        root->addChild(bg);

        auto border = DrawNode::create();
        border->drawRect(Vec2::ZERO, Vec2(size.width, size.height),
            Color4F(0.20f, 0.56f, 0.95f, 0.95f));
        border->drawRect(Vec2(5.0f, 5.0f), Vec2(size.width - 5.0f, size.height - 5.0f),
            Color4F(0.62f, 0.86f, 1.0f, 0.55f));
        root->addChild(border);

        return root;
    }

    MenuItemSprite* createStoryButton(const std::string& text,
        const Size& size,
        float fontSize,
        const Color3B& textColor,
        const ccMenuCallback& callback)
    {
        auto buildState = [&](bool selected, bool disabled) {
            auto root = Node::create();
            root->setContentSize(size);

            float w = size.width;
            float h = size.height;
            float cut = std::min(h * 0.18f, 14.0f);

            auto drawChamfer = [](DrawNode* draw, float x, float y, float w, float h,
                float cut, const Color4F& color)
            {
                Vec2 points[8] = {
                    Vec2(x + cut, y),
                    Vec2(x + w - cut, y),
                    Vec2(x + w, y + cut),
                    Vec2(x + w, y + h - cut),
                    Vec2(x + w - cut, y + h),
                    Vec2(x + cut, y + h),
                    Vec2(x, y + h - cut),
                    Vec2(x, y + cut)
                };
                draw->drawSolidPoly(points, 8, color);
            };

            auto body = DrawNode::create();
            Color4F shadow = disabled ? Color4F(0.03f, 0.04f, 0.06f, 0.72f) : Color4F(0.01f, 0.04f, 0.09f, 0.88f);
            Color4F shell = disabled ? Color4F(0.20f, 0.23f, 0.30f, 0.95f) : Color4F(0.02f, 0.20f, 0.40f, 1.0f);
            Color4F rim = disabled ? Color4F(0.43f, 0.46f, 0.54f, 0.82f) : Color4F(0.15f, 0.58f, 0.98f, 1.0f);
            Color4F innerShell = disabled ? Color4F(0.08f, 0.10f, 0.14f, 0.96f) : Color4F(0.01f, 0.08f, 0.18f, 1.0f);
            Color4F fillTop = disabled ? Color4F(0.13f, 0.15f, 0.19f, 0.78f)
                : (selected ? Color4F(0.10f, 0.36f, 0.58f, 0.98f) : Color4F(0.07f, 0.27f, 0.48f, 0.96f));
            Color4F fillBottom = disabled ? Color4F(0.08f, 0.09f, 0.12f, 0.82f)
                : (selected ? Color4F(0.02f, 0.14f, 0.28f, 0.98f) : Color4F(0.01f, 0.10f, 0.22f, 0.98f));

            drawChamfer(body, 0.0f, -3.0f, w, h, cut, shadow);
            drawChamfer(body, 0.0f, 0.0f, w, h, cut, shell);
            drawChamfer(body, 3.0f, 3.0f, w - 6.0f, h - 6.0f, std::max(2.0f, cut - 3.0f), rim);
            drawChamfer(body, 6.0f, 6.0f, w - 12.0f, h - 12.0f, std::max(2.0f, cut - 6.0f), innerShell);
            drawChamfer(body, 8.0f, 8.0f, w - 16.0f, h - 16.0f, std::max(2.0f, cut - 8.0f), fillBottom);
            drawChamfer(body, 10.0f, h * 0.52f, w - 20.0f, h * 0.27f, std::max(2.0f, cut - 10.0f),
                Color4F(fillTop.r + 0.03f, fillTop.g + 0.05f, fillTop.b + 0.06f, fillTop.a));
            drawChamfer(body, 11.0f, h - 12.0f, w - 22.0f, 2.0f, 1.0f,
                disabled ? Color4F(0.55f, 0.56f, 0.60f, 0.20f) : Color4F(0.78f, 0.93f, 1.0f, 0.35f));
            root->addChild(body);

            auto label = Label::createWithSystemFont(text, "Arial", fontSize);
            label->setColor(disabled ? Color3B(95, 95, 105) : textColor);
            label->enableBold();
            label->enableItalics();
            label->enableOutline(disabled ? Color4B(20, 22, 28, 180) : Color4B(5, 18, 36, 230),
                std::max(2, static_cast<int>(fontSize * 0.11f)));
            label->enableShadow(Color4B(0, 0, 0, 220), Size(2.5f, -2.5f), 1);
            label->setSkewX(-8.0f);
            label->setScaleX(1.08f);
            label->setPosition(Vec2(size.width * 0.5f, size.height * 0.54f));
            root->addChild(label);
            return root;
        };

        auto normal = buildState(false, false);
        auto selected = buildState(true, false);
        auto disabled = buildState(false, true);
        selected->setScale(0.97f);
        return MenuItemSprite::create(normal, selected, disabled, callback);
    }
}

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

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

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

    auto shade = LayerColor::create(Color4B(6, 10, 18, 185), winSize.width, winSize.height);
    shade->setPosition(Vec2::ZERO);
    this->addChild(shade, -1);

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

    auto title = Label::createWithSystemFont(lm->getString("story_mode"), "Arial", 44.0f * s);
    title->setColor(Color3B(220, 220, 100));
    title->enableOutline(Color4B(20, 18, 8, 220), std::max(1, static_cast<int>(3.0f * s)));
    title->enableShadow(Color4B(0, 0, 0, 200), Size(3.0f * s, -3.0f * s), 1);
    title->setPosition(Vec2(cx, cy + 135.0f * s));
    _currentView->addChild(title);

    Size bigBtnSize(400.0f * s, 56.0f * s);
    auto newGameItem = createStoryButton(lm->getString("new_game"), bigBtnSize, 28.0f * s,
        Color3B(235, 245, 255), CC_CALLBACK_1(StoryModeScene::onNewGameClicked, this));

    auto loadGameItem = createStoryButton(lm->getString("load_game"), bigBtnSize, 28.0f * s,
        Color3B(235, 245, 255), CC_CALLBACK_1(StoryModeScene::onLoadGameClicked, this));

    // Check if any saves exist
    auto saves = loadAllSaves();
    if (saves.empty())
    {
        loadGameItem->setEnabled(false);
    }

    auto backItem = createStoryButton(lm->getString("back_to_title"), bigBtnSize, 26.0f * s,
        Color3B(235, 245, 255), CC_CALLBACK_1(StoryModeScene::onBackClicked, this));

    auto menu = Menu::create(newGameItem, loadGameItem, backItem, nullptr);
    menu->setPosition(Vec2(cx, cy - 28.0f * s));
    menu->alignItemsVerticallyWithPadding(24.0f * s);
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

    Size panelSize(std::max(360.0f * s, std::min(700.0f * s, visibleSize.width - 80.0f * s)),
        std::max(360.0f * s, visibleSize.height - 110.0f * s));
    auto panel = createUiPanel(panelSize, Color4B(15, 25, 42, 220));
    panel->setPosition(Vec2(cx - panelSize.width * 0.5f,
        origin.y + 70.0f * s));
    _currentView->addChild(panel);

    auto title = Label::createWithSystemFont(lm->getString("load_game"), "Arial", 38.0f * s);
    title->setColor(Color3B(220, 200, 100));
    title->setPosition(Vec2(cx, origin.y + visibleSize.height - 58.0f * s));
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
        float cardW = panelSize.width - 54.0f * s;
        float cardH = 70.0f * s;
        float gap = 12.0f * s;
        float startY = origin.y + visibleSize.height - 125.0f * s;
        int maxVisible = 7;
        int totalToShow = std::min(static_cast<int>(saves.size()), maxVisible);

        for (int i = 0; i < totalToShow; ++i)
        {
            const auto& save = saves[i];
            float cardY = startY - i * (cardH + gap);

            // Card background
            auto cardBg = LayerColor::create(Color4B(28, 42, 64, 225), cardW, cardH);
            cardBg->setPosition(Vec2(cx - cardW * 0.5f, cardY - cardH * 0.5f));
            _currentView->addChild(cardBg);

            auto cardBorder = DrawNode::create();
            cardBorder->drawRect(Vec2(cx - cardW * 0.5f, cardY - cardH * 0.5f),
                Vec2(cx + cardW * 0.5f, cardY + cardH * 0.5f),
                Color4F(0.18f, 0.50f, 0.86f, 0.65f));
            _currentView->addChild(cardBorder);

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
            auto loadBtn = createStoryButton(lm->getString("load"), Size(78.0f * s, 34.0f * s),
                18.0f * s, Color3B(220, 245, 235), CC_CALLBACK_1(StoryModeScene::onSaveLoadClicked, this));
            loadBtn->setTag(i);  // display index

            // Delete button (only for manual saves)
            Vector<MenuItem*> btnItems;
            btnItems.pushBack(loadBtn);

            if (save.type == 1)  // manual save can be deleted
            {
                auto delBtn = createStoryButton("X", Size(36.0f * s, 34.0f * s),
                    18.0f * s, Color3B(255, 160, 140), CC_CALLBACK_1(StoryModeScene::onSaveDeleteClicked, this));
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
    auto backItem = createStoryButton(lm->getString("back"), Size(220.0f * s, 42.0f * s),
        21.0f * s, Color3B(220, 230, 245), CC_CALLBACK_1(StoryModeScene::onLoadBackClicked, this));
    auto backMenu = Menu::create(backItem, nullptr);
    backMenu->setPosition(Vec2(cx, origin.y + 38.0f * s));
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
    auto dialogBg = createUiPanel(dialogSize, Color4B(18, 29, 48, 245));
    dialogBg->setPosition(Vec2(cx - dialogSize.width * 0.5f, cy - dialogSize.height * 0.5f));
    _confirmLayer->addChild(dialogBg);

    auto confirmLabel = Label::createWithSystemFont(
        lm->getString("delete_save_confirm"), "Arial", 22.0f * s);
    confirmLabel->setColor(Color3B(220, 220, 200));
    confirmLabel->setPosition(Vec2(cx, cy + 25.0f * s));
    _confirmLayer->addChild(confirmLabel);

    auto yesItem = createStoryButton(lm->getString("confirm"), Size(118.0f * s, 40.0f * s),
        21.0f * s, Color3B(255, 160, 140), CC_CALLBACK_1(StoryModeScene::onDeleteConfirmYes, this));

    auto noItem = createStoryButton(lm->getString("back"), Size(118.0f * s, 40.0f * s),
        21.0f * s, Color3B(220, 230, 245), CC_CALLBACK_1(StoryModeScene::onDeleteConfirmNo, this));

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
