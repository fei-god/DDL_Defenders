#include "SettingsScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"
#include "base/CCDirector.h"
#include "base/CCUserDefault.h"
#include "base/CCEventListenerKeyboard.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
#include "platform/desktop/CCGLViewImpl-desktop.h"
#else
#include "platform/mac/CCGLViewImpl-mac.h"
#endif
#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "2d/CCMenu.h"
#include "2d/CCMenuItem.h"
#include "glfw3.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cmath>

USING_NS_CC;

// ---------------------------------------------------------------------------
// Resolution presets
// ---------------------------------------------------------------------------
static const std::vector<std::pair<int, int>> RESOLUTION_PRESETS = {
    { 960,  640 },
    { 1280, 720 },
    { 1920, 1080 },
    { 2560, 1440 },
    { 3840, 2160 },
};

static const char* USERD_KEY_RES_W    = "resolution_width";
static const char* USERD_KEY_RES_H    = "resolution_height";
static const char* USERD_KEY_LANG     = "language_index";
static const char* USERD_KEY_DISPMODE = "display_mode";
static const char* USERD_KEY_UP       = "key_move_up";
static const char* USERD_KEY_DOWN     = "key_move_down";
static const char* USERD_KEY_LEFT     = "key_move_left";
static const char* USERD_KEY_RIGHT    = "key_move_right";

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
Scene* SettingsScene::createScene(Entry entry)
{
    auto scene = SettingsScene::create();
    if (scene)
        scene->setEntry(entry);
    return scene;
}

void SettingsScene::setEntry(Entry entry)
{
    _entry = entry;
}

SettingsScene::~SettingsScene()
{
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------
bool SettingsScene::init()
{
    if (!Scene::init()) return false;

    _listeningAction = -1;
    _keyListener = nullptr;

    auto ud = UserDefault::getInstance();

    // Restore saved settings or defaults
    int resW = ud->getIntegerForKey(USERD_KEY_RES_W, 960);
    int resH = ud->getIntegerForKey(USERD_KEY_RES_H, 640);
    _currentResIndex = findResolutionIndex(resW, resH);

    _currentLangIndex = ud->getIntegerForKey(USERD_KEY_LANG, 0);
    LanguageManager::getInstance()->setLanguage(
        LanguageManager::intToLanguage(_currentLangIndex));

    // Restore saved display mode, or detect from the GLFW window
    _currentDisplayMode = ud->getIntegerForKey(USERD_KEY_DISPMODE, -1);
    if (_currentDisplayMode < 0 || _currentDisplayMode > 2)
    {
        // First launch or invalid value — detect actual mode
        _currentDisplayMode = 0;
        auto* glview = dynamic_cast<GLViewImpl*>(Director::getInstance()->getOpenGLView());
        if (glview)
        {
            if (glview->isFullscreen())
                _currentDisplayMode = 2;
            else
            {
                auto* window = glview->getWindow();
                if (window && glfwGetWindowAttrib(window, GLFW_DECORATED) == GLFW_FALSE)
                    _currentDisplayMode = 1;
            }
        }
    }
    _savedWindowX = 100;
    _savedWindowY = 100;
    _savedWindowW = resW;
    _savedWindowH = resH;

    buildUI();
    initKeyListener();

    return true;
}

int SettingsScene::findResolutionIndex(int w, int h) const
{
    for (size_t i = 0; i < RESOLUTION_PRESETS.size(); ++i)
    {
        if (RESOLUTION_PRESETS[i].first == w && RESOLUTION_PRESETS[i].second == h)
            return static_cast<int>(i);
    }
    return 0; // default to first preset
}

// ---------------------------------------------------------------------------
// Key-code → display-name
// ---------------------------------------------------------------------------
std::string SettingsScene::keyCodeToName(int code) const
{
    switch (static_cast<EventKeyboard::KeyCode>(code))
    {
    case EventKeyboard::KeyCode::KEY_A:          return "A";
    case EventKeyboard::KeyCode::KEY_B:          return "B";
    case EventKeyboard::KeyCode::KEY_C:          return "C";
    case EventKeyboard::KeyCode::KEY_D:          return "D";
    case EventKeyboard::KeyCode::KEY_E:          return "E";
    case EventKeyboard::KeyCode::KEY_F:          return "F";
    case EventKeyboard::KeyCode::KEY_G:          return "G";
    case EventKeyboard::KeyCode::KEY_H:          return "H";
    case EventKeyboard::KeyCode::KEY_I:          return "I";
    case EventKeyboard::KeyCode::KEY_J:          return "J";
    case EventKeyboard::KeyCode::KEY_K:          return "K";
    case EventKeyboard::KeyCode::KEY_L:          return "L";
    case EventKeyboard::KeyCode::KEY_M:          return "M";
    case EventKeyboard::KeyCode::KEY_N:          return "N";
    case EventKeyboard::KeyCode::KEY_O:          return "O";
    case EventKeyboard::KeyCode::KEY_P:          return "P";
    case EventKeyboard::KeyCode::KEY_Q:          return "Q";
    case EventKeyboard::KeyCode::KEY_R:          return "R";
    case EventKeyboard::KeyCode::KEY_S:          return "S";
    case EventKeyboard::KeyCode::KEY_T:          return "T";
    case EventKeyboard::KeyCode::KEY_U:          return "U";
    case EventKeyboard::KeyCode::KEY_V:          return "V";
    case EventKeyboard::KeyCode::KEY_W:          return "W";
    case EventKeyboard::KeyCode::KEY_X:          return "X";
    case EventKeyboard::KeyCode::KEY_Y:          return "Y";
    case EventKeyboard::KeyCode::KEY_Z:          return "Z";
    case EventKeyboard::KeyCode::KEY_SPACE:      return "Space";
    case EventKeyboard::KeyCode::KEY_UP_ARROW:   return u8"↑";
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW: return u8"↓";
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW: return u8"←";
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:return u8"→";
    case EventKeyboard::KeyCode::KEY_TAB:        return "Tab";
    case EventKeyboard::KeyCode::KEY_SHIFT:      return "Shift";
    case EventKeyboard::KeyCode::KEY_CTRL:       return "Ctrl";
    case EventKeyboard::KeyCode::KEY_ALT:        return "Alt";
    default: return "?";
    }
}

// ---------------------------------------------------------------------------
// Keyboard listener (captures rebind key-press)
// ---------------------------------------------------------------------------
void SettingsScene::initKeyListener()
{
    _keyListener = EventListenerKeyboard::create();
    _keyListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*)
    {
        if (_listeningAction < 0) return;

        // Don't bind ESC (it cancels / goes back)
        if (code == EventKeyboard::KeyCode::KEY_ESCAPE)
        {
            _listeningAction = -1;
            refreshKeyLabels();
            return;
        }

        auto ud = UserDefault::getInstance();
        const char* udKey = nullptr;
        switch (_listeningAction)
        {
        case 0: udKey = USERD_KEY_UP;    break;
        case 1: udKey = USERD_KEY_DOWN;  break;
        case 2: udKey = USERD_KEY_LEFT;  break;
        case 3: udKey = USERD_KEY_RIGHT; break;
        default: break;
        }

        if (udKey)
        {
            ud->setIntegerForKey(udKey, static_cast<int>(code));
            ud->flush();
        }

        _listeningAction = -1;
        refreshKeyLabels();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_keyListener, this);
}

// ---------------------------------------------------------------------------
// Build the whole settings UI
// ---------------------------------------------------------------------------
void SettingsScene::buildUI()
{
    auto* lm = LanguageManager::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto winSize = Director::getInstance()->getWinSize();

    // UI scale factor — keeps layout proportional across resolutions.
    // Base: 960×640 → s=1.0.  1920×1080 → s≈1.69.  3840×2160 → s≈3.38.
    float s = std::min(winSize.height / 760.0f, winSize.width / 960.0f);
    s = std::max(0.72f, s);

    float cx = origin.x + visibleSize.width / 2;
    float topY = origin.y + visibleSize.height - 30.0f * s;

    // Background (covers full design resolution to avoid gaps)
    auto bg = LayerColor::create(Color4B(22, 28, 40, 255), winSize.width, winSize.height);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -10);

    // Title
    auto title = Label::createWithSystemFont(lm->getString("settings_title"), "Arial", 40.0f * s);
    title->setColor(Color3B(220, 220, 240));
    title->setPosition(Vec2(cx, topY));
    this->addChild(title, 1);
    topY -= 54.0f * s;

    // ==================== Resolution ====================
    auto resHeader = Label::createWithSystemFont(lm->getString("resolution"), "Arial", 26.0f * s);
    resHeader->setColor(Color3B(180, 200, 220));
    resHeader->setPosition(Vec2(cx, topY));
    this->addChild(resHeader, 1);
    topY -= 32.0f * s;

    // Resolution label
    int rw = RESOLUTION_PRESETS[_currentResIndex].first;
    int rh = RESOLUTION_PRESETS[_currentResIndex].second;
    std::string resStr = std::to_string(rw) + " x " + std::to_string(rh);
    _resolutionLabel = Label::createWithSystemFont(resStr, "Arial", 30.0f * s);
    _resolutionLabel->setColor(Color3B(240, 240, 200));
    _resolutionLabel->setPosition(Vec2(cx, topY));
    this->addChild(_resolutionLabel, 1);

    // Left / Right arrows for resolution
    auto resLeft = MenuItemLabel::create(
        Label::createWithSystemFont(u8"◀", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onResolutionLeft, this));
    auto resRight = MenuItemLabel::create(
        Label::createWithSystemFont(u8"▶", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onResolutionRight, this));
    if (resLeft && resRight)
    {
        resLeft->setColor(Color3B(160, 180, 200));
        resRight->setColor(Color3B(160, 180, 200));
        auto resMenu = Menu::create(resLeft, resRight, nullptr);
        resMenu->setPosition(Vec2(cx, topY));
        resMenu->alignItemsHorizontallyWithPadding(160.0f * s);
        this->addChild(resMenu, 2);
    }
    topY -= 36.0f * s;

    // Apply button (applies resolution immediately without restart)
    auto applyLabel = Label::createWithSystemFont(lm->getString("apply"), "Arial", 22.0f * s);
    applyLabel->setColor(Color3B(100, 210, 120));
    auto applyItem = MenuItemLabel::create(applyLabel,
        CC_CALLBACK_1(SettingsScene::onApplyResolution, this));
    if (applyItem)
    {
        auto applyMenu = Menu::create(applyItem, nullptr);
        applyMenu->setPosition(Vec2(cx, topY));
        this->addChild(applyMenu, 2);
    }
    topY -= 34.0f * s;

    // ==================== Display Mode ====================
    auto dmHeader = Label::createWithSystemFont(lm->getString("display_mode"), "Arial", 26.0f * s);
    dmHeader->setColor(Color3B(180, 200, 220));
    dmHeader->setPosition(Vec2(cx, topY));
    this->addChild(dmHeader, 1);
    topY -= 32.0f * s;

    // Display mode label
    static const char* DM_NAMES[] = { "windowed", "borderless", "fullscreen" };
    _displayModeLabel = Label::createWithSystemFont(
        lm->getString(DM_NAMES[_currentDisplayMode]), "Arial", 30.0f * s);
    _displayModeLabel->setColor(Color3B(240, 240, 200));
    _displayModeLabel->setPosition(Vec2(cx, topY));
    this->addChild(_displayModeLabel, 1);

    // Left / Right arrows for display mode
    auto dmLeft = MenuItemLabel::create(
        Label::createWithSystemFont(u8"◀", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onDisplayModeLeft, this));
    auto dmRight = MenuItemLabel::create(
        Label::createWithSystemFont(u8"▶", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onDisplayModeRight, this));
    if (dmLeft && dmRight)
    {
        dmLeft->setColor(Color3B(160, 180, 200));
        dmRight->setColor(Color3B(160, 180, 200));
        auto dmMenu = Menu::create(dmLeft, dmRight, nullptr);
        dmMenu->setPosition(Vec2(cx, topY));
        dmMenu->alignItemsHorizontallyWithPadding(200.0f * s);
        this->addChild(dmMenu, 2);
    }
    topY -= 36.0f * s;

    // ==================== Key Bindings ====================
    auto keyHeader = Label::createWithSystemFont(lm->getString("key_bindings"), "Arial", 26.0f * s);
    keyHeader->setColor(Color3B(180, 200, 220));
    keyHeader->setPosition(Vec2(cx, topY));
    this->addChild(keyHeader, 1);
    topY -= 32.0f * s;

    auto ud = UserDefault::getInstance();
    int defUp    = static_cast<int>(EventKeyboard::KeyCode::KEY_W);
    int defDown  = static_cast<int>(EventKeyboard::KeyCode::KEY_S);
    int defLeft  = static_cast<int>(EventKeyboard::KeyCode::KEY_A);
    int defRight = static_cast<int>(EventKeyboard::KeyCode::KEY_D);

    auto makeKeyRow = [&](const std::string& actionName, int defaultKey,
                          const char* udKey, const ccMenuCallback& rebindCb,
                          Label** outLabel, float& y) -> cocos2d::Label*
    {
        int keyCode = ud->getIntegerForKey(udKey, defaultKey);
        std::string keyName = keyCodeToName(keyCode);

        // Action name label (left side)
        auto nameLbl = Label::createWithSystemFont(actionName, "Arial", 22.0f * s);
        nameLbl->setColor(Color3B(200, 200, 220));
        nameLbl->setAnchorPoint(Vec2(0, 0.5f));
        float leftX = cx - 160.0f * s;
        nameLbl->setPosition(Vec2(leftX, y));
        this->addChild(nameLbl, 1);

        // Key name label (center)
        std::string displayText;
        if (_listeningAction >= 0)
        {
            int thisAction = -1;
            if (udKey == USERD_KEY_UP)    thisAction = 0;
            if (udKey == USERD_KEY_DOWN)  thisAction = 1;
            if (udKey == USERD_KEY_LEFT)  thisAction = 2;
            if (udKey == USERD_KEY_RIGHT) thisAction = 3;

            if (thisAction == _listeningAction)
                displayText = lm->getString("press_key");
            else
                displayText = keyName;
        }
        else
        {
            displayText = keyName;
        }

        auto keyLbl = Label::createWithSystemFont(displayText, "Arial", 22.0f * s);
        keyLbl->setColor(Color3B(240, 230, 100));
        keyLbl->setPosition(Vec2(cx, y));
        this->addChild(keyLbl, 1);
        if (outLabel) *outLabel = keyLbl;

        // Rebind button (right side)
        auto rebindLbl = Label::createWithSystemFont(lm->getString("rebind"), "Arial", 20.0f * s);
        rebindLbl->setColor(Color3B(160, 180, 200));
        auto rebindItem = MenuItemLabel::create(rebindLbl, rebindCb);
        if (rebindItem)
        {
            auto rowMenu = Menu::create(rebindItem, nullptr);
            rowMenu->setPosition(Vec2(cx + 150.0f * s, y));
            this->addChild(rowMenu, 2);
        }

        y -= 28.0f * s;
        return keyLbl;
    };

    _keyLabelUp    = makeKeyRow(lm->getString("move_up"),    defUp,    USERD_KEY_UP,
                                CC_CALLBACK_1(SettingsScene::onRebindMoveUp, this),
                                &_keyLabelUp, topY);
    _keyLabelDown  = makeKeyRow(lm->getString("move_down"),  defDown,  USERD_KEY_DOWN,
                                CC_CALLBACK_1(SettingsScene::onRebindMoveDown, this),
                                &_keyLabelDown, topY);
    _keyLabelLeft  = makeKeyRow(lm->getString("move_left"),  defLeft,  USERD_KEY_LEFT,
                                CC_CALLBACK_1(SettingsScene::onRebindMoveLeft, this),
                                &_keyLabelLeft, topY);
    _keyLabelRight = makeKeyRow(lm->getString("move_right"), defRight, USERD_KEY_RIGHT,
                                CC_CALLBACK_1(SettingsScene::onRebindMoveRight, this),
                                &_keyLabelRight, topY);

    // Fire & Pause (read-only, shown for info)
    {
        auto fireName = Label::createWithSystemFont(lm->getString("fire_key"), "Arial", 22.0f * s);
        fireName->setColor(Color3B(200, 200, 220));
        fireName->setAnchorPoint(Vec2(0, 0.5f));
        fireName->setPosition(Vec2(cx - 160.0f * s, topY));
        this->addChild(fireName, 1);

        auto fireVal = Label::createWithSystemFont(lm->getString("mouse_left"), "Arial", 22.0f * s);
        fireVal->setColor(Color3B(160, 160, 180));
        fireVal->setPosition(Vec2(cx, topY));
        this->addChild(fireVal, 1);
        topY -= 28.0f * s;

        auto pauseName = Label::createWithSystemFont(lm->getString("pause_key"), "Arial", 22.0f * s);
        pauseName->setColor(Color3B(200, 200, 220));
        pauseName->setAnchorPoint(Vec2(0, 0.5f));
        pauseName->setPosition(Vec2(cx - 160.0f * s, topY));
        this->addChild(pauseName, 1);

        auto pauseVal = Label::createWithSystemFont("ESC", "Arial", 22.0f * s);
        pauseVal->setColor(Color3B(160, 160, 180));
        pauseVal->setPosition(Vec2(cx, topY));
        this->addChild(pauseVal, 1);
        topY -= 34.0f * s;
    }

    // ==================== Language ====================
    auto langHeader = Label::createWithSystemFont(lm->getString("language"), "Arial", 26.0f * s);
    langHeader->setColor(Color3B(180, 200, 220));
    langHeader->setPosition(Vec2(cx, topY));
    this->addChild(langHeader, 1);
    topY -= 32.0f * s;

    _languageLabel = Label::createWithSystemFont(lm->getLanguageName(), "Arial", 30.0f * s);
    _languageLabel->setColor(Color3B(240, 240, 200));
    _languageLabel->setPosition(Vec2(cx, topY));
    this->addChild(_languageLabel, 1);

    auto langLeft = MenuItemLabel::create(
        Label::createWithSystemFont(u8"◀", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onLanguageLeft, this));
    auto langRight = MenuItemLabel::create(
        Label::createWithSystemFont(u8"▶", "Arial", 28.0f * s),
        CC_CALLBACK_1(SettingsScene::onLanguageRight, this));
    if (langLeft && langRight)
    {
        langLeft->setColor(Color3B(160, 180, 200));
        langRight->setColor(Color3B(160, 180, 200));
        auto langMenu = Menu::create(langLeft, langRight, nullptr);
        langMenu->setPosition(Vec2(cx, topY));
        langMenu->alignItemsHorizontallyWithPadding(200.0f * s);
        this->addChild(langMenu, 2);
    }
    topY -= 38.0f * s;

    // ==================== Back button ====================
    auto backLabel = Label::createWithSystemFont(lm->getString("back"), "Arial", 32.0f * s);
    backLabel->setColor(Color3B(200, 160, 120));
    auto backItem = MenuItemLabel::create(backLabel,
        CC_CALLBACK_1(SettingsScene::onBackClicked, this));
    if (backItem)
    {
        auto backMenu = Menu::create(backItem, nullptr);
        backMenu->setPosition(Vec2(cx, origin.y + 32.0f * s));
        this->addChild(backMenu, 2);
    }
}

// ---------------------------------------------------------------------------
// Resolution callbacks
// ---------------------------------------------------------------------------
void SettingsScene::onResolutionLeft(Ref*)
{
    _currentResIndex = (_currentResIndex - 1 + (int)RESOLUTION_PRESETS.size()) % (int)RESOLUTION_PRESETS.size();
    updateResolutionLabel();
}

void SettingsScene::onResolutionRight(Ref*)
{
    _currentResIndex = (_currentResIndex + 1) % (int)RESOLUTION_PRESETS.size();
    updateResolutionLabel();
}

void SettingsScene::onApplyResolution(Ref*)
{
    int rw = RESOLUTION_PRESETS[_currentResIndex].first;
    int rh = RESOLUTION_PRESETS[_currentResIndex].second;

    // Save to UserDefault
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey(USERD_KEY_RES_W, rw);
    ud->setIntegerForKey(USERD_KEY_RES_H, rh);
    ud->flush();

    auto* glview = dynamic_cast<GLViewImpl*>(Director::getInstance()->getOpenGLView());
    if (glview)
    {
        auto* window = glview->getWindow();

        // In windowed mode, resize the GLFW window to match
        if (_currentDisplayMode == 0 && window && !glview->isFullscreen())
            glfwSetWindowSize(window, rw, rh);

        // Apply the design resolution
        glview->setDesignResolutionSize(
            static_cast<float>(rw), static_cast<float>(rh),
            ResolutionPolicy::NO_BORDER);

        // Sync the OpenGL viewport
        Director::getInstance()->setViewport();
    }

    // Rebuild UI at the new resolution
    this->removeAllChildren();
    buildUI();
}

void SettingsScene::updateResolutionLabel()
{
    if (!_resolutionLabel) return;
    int rw = RESOLUTION_PRESETS[_currentResIndex].first;
    int rh = RESOLUTION_PRESETS[_currentResIndex].second;
    _resolutionLabel->setString(std::to_string(rw) + " x " + std::to_string(rh));
}

// ---------------------------------------------------------------------------
// Display mode callbacks
// ---------------------------------------------------------------------------
void SettingsScene::onDisplayModeLeft(Ref*)
{
    int oldMode = _currentDisplayMode;
    _currentDisplayMode = (_currentDisplayMode - 1 + 3) % 3;
    updateDisplayModeLabel();
    applyDisplayMode(_currentDisplayMode, oldMode);
}

void SettingsScene::onDisplayModeRight(Ref*)
{
    int oldMode = _currentDisplayMode;
    _currentDisplayMode = (_currentDisplayMode + 1) % 3;
    updateDisplayModeLabel();
    applyDisplayMode(_currentDisplayMode, oldMode);
}

void SettingsScene::updateDisplayModeLabel()
{
    if (!_displayModeLabel) return;
    auto* lm = LanguageManager::getInstance();
    static const char* DM_NAMES[] = { "windowed", "borderless", "fullscreen" };
    _displayModeLabel->setString(lm->getString(DM_NAMES[_currentDisplayMode]));
}

void SettingsScene::fitResolutionToScreen(int screenW, int screenH)
{
    if (screenW <= 0 || screenH <= 0) return;

    float screenAspect = static_cast<float>(screenW) / static_cast<float>(screenH);
    int screenArea = screenW * screenH;

    // Find the preset closest to the screen:
    // primary key = aspect-ratio match, secondary key = area proximity
    int bestIdx = 0;
    float bestDiff = 999.0f;
    int bestAreaDist = 0x7fffffff;

    for (size_t i = 0; i < RESOLUTION_PRESETS.size(); ++i)
    {
        float presetAspect = static_cast<float>(RESOLUTION_PRESETS[i].first)
                           / static_cast<float>(RESOLUTION_PRESETS[i].second);
        float diff = std::abs(presetAspect - screenAspect);
        int area = RESOLUTION_PRESETS[i].first * RESOLUTION_PRESETS[i].second;
        int areaDist = std::abs(area - screenArea);

        // Tie-breaking: same aspect-ratio diff → pick the one closer to screen size
        if (diff < bestDiff - 0.0001f ||
            (std::abs(diff - bestDiff) < 0.0001f && areaDist < bestAreaDist))
        {
            bestDiff = diff;
            bestIdx = static_cast<int>(i);
            bestAreaDist = areaDist;
        }
    }

    int rw = RESOLUTION_PRESETS[bestIdx].first;
    int rh = RESOLUTION_PRESETS[bestIdx].second;

    auto* glview = dynamic_cast<GLViewImpl*>(Director::getInstance()->getOpenGLView());
    if (glview)
    {
        glview->setDesignResolutionSize(
            static_cast<float>(rw), static_cast<float>(rh),
            ResolutionPolicy::NO_BORDER);
    }

    // Ensure the OpenGL viewport matches the new design resolution
    Director::getInstance()->setViewport();

    _currentResIndex = bestIdx;
    updateResolutionLabel();

    // Rebuild UI at the new resolution
    this->removeAllChildren();
    buildUI();
}

void SettingsScene::applyDisplayMode(int mode, int oldMode)
{
    auto* glview = dynamic_cast<GLViewImpl*>(Director::getInstance()->getOpenGLView());
    if (!glview) return;

    auto* window = glview->getWindow();
    if (!window) return;

    // Only save position when leaving genuine windowed mode
    if (oldMode == 0)
    {
        glfwGetWindowPos(window, &_savedWindowX, &_savedWindowY);
        glfwGetWindowSize(window, &_savedWindowW, &_savedWindowH);
    }

    // Remember the display mode for next launch
    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey(USERD_KEY_DISPMODE, mode);
    ud->flush();

    switch (mode)
    {
    case 0: // Windowed
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        if (glview->isFullscreen())
        {
            glview->setWindowed(_savedWindowW > 0 ? _savedWindowW : 960,
                                _savedWindowH > 0 ? _savedWindowH : 640);
        }
        else
        {
            glfwSetWindowMonitor(window, nullptr,
                _savedWindowX, _savedWindowY,
                _savedWindowW > 0 ? _savedWindowW : 960,
                _savedWindowH > 0 ? _savedWindowH : 640,
                GLFW_DONT_CARE);
        }
        glfwSetWindowPos(window, _savedWindowX, _savedWindowY);

        // Restore user's chosen windowed resolution and rebuild UI
        {
            int rw = ud->getIntegerForKey(USERD_KEY_RES_W, 960);
            int rh = ud->getIntegerForKey(USERD_KEY_RES_H, 640);
            glview->setDesignResolutionSize(
                static_cast<float>(rw), static_cast<float>(rh),
                ResolutionPolicy::NO_BORDER);
            Director::getInstance()->setViewport();
            _currentResIndex = findResolutionIndex(rw, rh);
            updateResolutionLabel();
            this->removeAllChildren();
            buildUI();
        }
        break;

    case 1: // Borderless — keep current resolution, just rebuild UI
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwMaximizeWindow(window);
        Director::getInstance()->setViewport();
        this->removeAllChildren();
        buildUI();
        break;

    case 2: // Fullscreen — keep current resolution, just rebuild UI
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glview->setFullscreen();
        Director::getInstance()->setViewport();
        this->removeAllChildren();
        buildUI();
        break;
    }
}

// ---------------------------------------------------------------------------
// Key rebind callbacks
// ---------------------------------------------------------------------------
void SettingsScene::onRebindMoveUp(Ref*)    { _listeningAction = 0; refreshKeyLabels(); }
void SettingsScene::onRebindMoveDown(Ref*)  { _listeningAction = 1; refreshKeyLabels(); }
void SettingsScene::onRebindMoveLeft(Ref*)  { _listeningAction = 2; refreshKeyLabels(); }
void SettingsScene::onRebindMoveRight(Ref*) { _listeningAction = 3; refreshKeyLabels(); }

void SettingsScene::refreshKeyLabels()
{
    auto lm = LanguageManager::getInstance();
    auto ud = UserDefault::getInstance();
    int defUp    = static_cast<int>(EventKeyboard::KeyCode::KEY_W);
    int defDown  = static_cast<int>(EventKeyboard::KeyCode::KEY_S);
    int defLeft  = static_cast<int>(EventKeyboard::KeyCode::KEY_A);
    int defRight = static_cast<int>(EventKeyboard::KeyCode::KEY_D);

    auto updateOne = [&](Label* lbl, const char* udKey, int defaultKey, int actionIdx) {
        if (!lbl) return;
        if (_listeningAction == actionIdx)
            lbl->setString(lm->getString("press_key"));
        else
        {
            int keyCode = ud->getIntegerForKey(udKey, defaultKey);
            lbl->setString(keyCodeToName(keyCode));
        }
    };

    updateOne(_keyLabelUp,    USERD_KEY_UP,    defUp,    0);
    updateOne(_keyLabelDown,  USERD_KEY_DOWN,  defDown,  1);
    updateOne(_keyLabelLeft,  USERD_KEY_LEFT,  defLeft,  2);
    updateOne(_keyLabelRight, USERD_KEY_RIGHT, defRight, 3);
}

// ---------------------------------------------------------------------------
// Language callbacks
// ---------------------------------------------------------------------------
void SettingsScene::onLanguageLeft(Ref*)
{
    int total = 2;
    _currentLangIndex = (_currentLangIndex - 1 + total) % total;

    LanguageManager::getInstance()->setLanguage(
        LanguageManager::intToLanguage(_currentLangIndex));

    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey(USERD_KEY_LANG, _currentLangIndex);
    ud->flush();

    updateLanguageLabel();

    // Rebuild UI to reflect new language
    this->removeAllChildren();
    buildUI();
}

void SettingsScene::onLanguageRight(Ref*)
{
    int total = 2;
    _currentLangIndex = (_currentLangIndex + 1) % total;

    LanguageManager::getInstance()->setLanguage(
        LanguageManager::intToLanguage(_currentLangIndex));

    auto ud = UserDefault::getInstance();
    ud->setIntegerForKey(USERD_KEY_LANG, _currentLangIndex);
    ud->flush();

    updateLanguageLabel();

    // Rebuild UI to reflect new language
    this->removeAllChildren();
    buildUI();
}

void SettingsScene::updateLanguageLabel()
{
    if (!_languageLabel) return;
    _languageLabel->setString(LanguageManager::getInstance()->getLanguageName());
}

// ---------------------------------------------------------------------------
// Back
// ---------------------------------------------------------------------------
void SettingsScene::onBackClicked(Ref*)
{
    if (_entry == Entry::PAUSE_MENU)
    {
        Director::getInstance()->popScene();
    }
    else
    {
        // MAIN_MENU or default
        auto menuScene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(menuScene);
    }
}
