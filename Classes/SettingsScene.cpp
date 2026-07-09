#include "SettingsScene.h"
#include "MainMenuScene.h"
#include "Managers/LanguageManager.h"
#include "Core/AssetPaths.h"
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
#include "2d/CCDrawNode.h"
#include "ui/UIScale9Sprite.h"
#include "glfw3.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cmath>
#include <functional>

USING_NS_CC;

namespace
{
    const char* SETTINGS_BG_PATH = "art/ui/settings/background.png";
    const char* PANEL_PATH = "art/ui/settings/_backup_before_trim/ui_panel_main.png";
    const char* BUTTON_PATH = "art/ui/settings/_backup_before_trim/block_title.png";
    const char* TEXT_BG_PATH = "art/ui/settings/block_detail.png";
    const char* LEFT_ARROW_PATH = "art/ui/settings/left_botton.png";
    const char* RIGHT_ARROW_PATH = "art/ui/settings/right_botton.png";

    const Color3B COLOR_TITLE(232, 226, 204);
    const Color3B COLOR_SECTION(208, 218, 224);
    const Color3B COLOR_TEXT(216, 222, 226);
    const Color3B COLOR_VALUE(246, 218, 142);
    const Color3B COLOR_BUTTON_TEXT(242, 228, 184);
    const Color3B COLOR_DISABLED_TEXT(126, 126, 132);
    const float FONT_SCALE = 1.10f;

    const Rect TEXT_BG_CAP_INSETS(360.0f, 90.0f, 1680.0f, 210.0f);
    const Rect BUTTON_BG_CAP_INSETS(260.0f, 150.0f, 1260.0f, 300.0f);

    Sprite* createFittedSprite(const std::string& path, const Size& bounds,
        const Color3B& color = Color3B::WHITE, GLubyte opacity = 255)
    {
        std::string resolved = AssetPaths::resolve(path);
        auto sprite = resolved.empty() ? nullptr : Sprite::create(resolved);
        if (!sprite)
            return nullptr;

        Size imageSize = sprite->getContentSize();
        float scale = std::min(bounds.width / imageSize.width, bounds.height / imageSize.height);
        sprite->setScale(scale);
        sprite->setColor(color);
        sprite->setOpacity(opacity);
        sprite->setPosition(Vec2(bounds.width * 0.5f, bounds.height * 0.5f));
        return sprite;
    }

    void fitLabelInside(Label* label, const Size& bounds)
    {
        if (!label)
            return;
        Size labelSize = label->getContentSize();
        if (labelSize.width <= 0.0f || labelSize.height <= 0.0f)
            return;

        float scale = std::min(1.0f, std::min(bounds.width / labelSize.width,
            bounds.height / labelSize.height));
        label->setScale(scale);
    }

    ui::Scale9Sprite* createScale9(const std::string& path, const Size& size,
        const Rect& capInsets, const Color3B& color = Color3B::WHITE, GLubyte opacity = 255)
    {
        std::string resolved = AssetPaths::resolve(path);
        ui::Scale9Sprite* sprite = nullptr;
        if (!resolved.empty())
        {
            sprite = ui::Scale9Sprite::create(resolved);
            if (sprite)
            {
                if (path == BUTTON_PATH)
                    sprite->setCapInsets(BUTTON_BG_CAP_INSETS);
                else if (path == TEXT_BG_PATH)
                    sprite->setCapInsets(TEXT_BG_CAP_INSETS);
                else
                    sprite->setCapInsets(capInsets);
            }
        }
        if (!sprite)
        {
            sprite = ui::Scale9Sprite::create();
        }
        sprite->setPreferredSize(size);
        sprite->setColor(color);
        sprite->setOpacity(opacity);
        return sprite;
    }
}

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

#if 0
// ---------------------------------------------------------------------------
// Legacy single-column settings UI kept out of compilation for reference.
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

#endif

// ---------------------------------------------------------------------------
// Build the whole settings UI
// ---------------------------------------------------------------------------
void SettingsScene::buildUI()
{
    resetUiPointers();

    auto* director = Director::getInstance();
    const Size visibleSize = director->getVisibleSize();
    const Vec2 origin = director->getVisibleOrigin();

    _uiScale = std::min(visibleSize.width / 1920.0f, visibleSize.height / 1080.0f);
    _uiScale = std::max(0.66f, std::min(_uiScale, 2.0f));

    createBackground();

    const float panelW = std::min(1500.0f * _uiScale, visibleSize.width * 0.90f);
    const float panelH = std::min(930.0f * _uiScale, visibleSize.height * 0.90f);
    auto panel = createMainPanel(Size(panelW, panelH));
    panel->setPosition(origin + Vec2(visibleSize.width * 0.5f - panelW * 0.5f,
        visibleSize.height * 0.5f - panelH * 0.5f));
    this->addChild(panel, 1);

    auto menu = Menu::create();
    menu->setPosition(Vec2::ZERO);
    panel->addChild(menu, 12);

    auto* lm = LanguageManager::getInstance();
    auto title = createTextWithBackground(lm->getString("settings_title"),
        58.0f * _uiScale, Size(520.0f * _uiScale, 130.0f * _uiScale),
        COLOR_TITLE, nullptr, true);
    title->setPosition(Vec2(panelW * 0.5f - title->getContentSize().width * 0.5f,
        panelH - 78.0f * _uiScale - title->getContentSize().height * 0.5f));
    panel->addChild(title, 11);

    const float graphicsTopY = panelH - 190.0f * _uiScale;
    const float displayY = graphicsTopY - 100.0f * _uiScale;
    const float languageY = 160.0f * _uiScale;
    const float keySettingsY = (displayY + languageY) * 0.5f;

    createGraphicsSettings(panel, menu, panelW * 0.5f,
        graphicsTopY, panelW);
    createKeyBindingSettings(panel, menu, panelW * 0.5f,
        keySettingsY, panelW);
    createLanguageSettings(panel, menu, panelW * 0.5f,
        languageY, panelW);

    const Size backSize(360.0f * _uiScale, 96.0f * _uiScale);
    auto back = createImageButton(lm->getString("back"),
        backSize,
        32.0f * _uiScale,
        [this](Ref* sender) { onBackClicked(sender); });
    back->setPosition(Vec2(panelW * 0.5f, -28.0f * _uiScale));
    menu->addChild(back);
}

void SettingsScene::resetUiPointers()
{
    _resolutionLabel = nullptr;
    _displayModeLabel = nullptr;
    _languageLabel = nullptr;
    _keyLabelUp = nullptr;
    _keyLabelDown = nullptr;
    _keyLabelLeft = nullptr;
    _keyLabelRight = nullptr;
    _keyRowBackgrounds.clear();
}

void SettingsScene::createBackground()
{
    const Size visibleSize = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();
    const Vec2 center = origin + Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f);

    auto fallback = LayerColor::create(Color4B(10, 18, 28, 255),
        visibleSize.width, visibleSize.height);
    fallback->setPosition(origin);
    this->addChild(fallback, -20);

    std::string bgPath = AssetPaths::resolve(SETTINGS_BG_PATH);
    if (!bgPath.empty())
    {
        auto bg = Sprite::create(bgPath);
        if (bg)
        {
            Size imgSize = bg->getContentSize();
            float scale = std::max(visibleSize.width / imgSize.width,
                visibleSize.height / imgSize.height);
            bg->setScale(scale);
            bg->setPosition(center);
            this->addChild(bg, -18);
        }
    }

    auto shade = LayerColor::create(Color4B(4, 8, 14, 72),
        visibleSize.width, visibleSize.height);
    shade->setPosition(origin);
    this->addChild(shade, -17);
}

Node* SettingsScene::createMainPanel(const Size& size)
{
    auto root = Node::create();
    root->setContentSize(size);

    std::string panelPath = AssetPaths::resolve(PANEL_PATH);
    auto panelImage = panelPath.empty() ? nullptr : Sprite::create(panelPath);
    if (panelImage)
    {
        Size imageSize = panelImage->getContentSize();
        panelImage->setScaleX(size.width / imageSize.width);
        panelImage->setScaleY(size.height / imageSize.height);
        panelImage->setOpacity(212);
        panelImage->setPosition(Vec2(size.width * 0.5f, size.height * 0.5f));
        root->addChild(panelImage, 0);
    }
    else
    {
        auto panel = LayerColor::create(Color4B(3, 9, 18, 118), size.width, size.height);
        panel->setPosition(Vec2::ZERO);
        root->addChild(panel, 0);
    }
    return root;
}

Label* SettingsScene::createStyledLabel(const std::string& text, float fontSize,
    const Color3B& color, bool bold)
{
    fontSize *= FONT_SCALE;
    auto label = Label::createWithSystemFont(text, "Arial", fontSize);
    label->setColor(color);
    if (bold)
        label->enableBold();
    label->enableOutline(Color4B(20, 24, 32, 190),
        std::max(1, static_cast<int>(fontSize * 0.055f)));
    return label;
}

Node* SettingsScene::createTextWithBackground(const std::string& text, float fontSize,
    const Size& minSize, const Color3B& textColor, Label** outLabel, bool highlighted,
    bool useButtonFrame)
{
    auto label = createStyledLabel(text, fontSize, textColor, false);
    Size labelSize = label->getContentSize();
    Size bgSize(std::max(minSize.width, labelSize.width + 120.0f * _uiScale),
        std::max(minSize.height, labelSize.height + 56.0f * _uiScale));

    auto root = Node::create();
    root->setContentSize(bgSize);

    if (useButtonFrame)
    {
        auto bg = createFittedSprite(BUTTON_PATH, bgSize,
            highlighted ? Color3B(255, 232, 166) : Color3B(218, 216, 210),
            highlighted ? 238 : 206);
        if (bg)
            root->addChild(bg, 0);
        fitLabelInside(label, Size(bgSize.width * 0.62f, bgSize.height * 0.48f));
    }
    else
    {
        auto bg = createScale9(TEXT_BG_PATH, bgSize,
            Rect(350, 150, 1800, 320),
            highlighted ? Color3B(255, 232, 166) : Color3B(218, 216, 210),
            highlighted ? 238 : 206);
        bg->setPosition(Vec2(bgSize.width * 0.5f, bgSize.height * 0.5f));
        root->addChild(bg, 0);
    }

    label->setPosition(Vec2(bgSize.width * 0.5f, bgSize.height * 0.52f));
    root->addChild(label, 1);

    if (outLabel)
        *outLabel = label;
    return root;
}

Node* SettingsScene::createButtonState(const std::string& text,
    const Size& size, float fontSize, const Color3B& textColor,
    bool selected, bool enabled)
{
    auto root = Node::create();
    root->setContentSize(size);

    const char* iconPath = nullptr;
    if (text == "<")
        iconPath = LEFT_ARROW_PATH;
    else if (text == ">")
        iconPath = RIGHT_ARROW_PATH;

    if (iconPath)
    {
        std::string iconResolved = AssetPaths::resolve(iconPath);
        auto icon = Sprite::create(iconResolved);
        if (icon)
        {
            Size iconSize = icon->getContentSize();
            float iconScale = std::min(size.width / iconSize.width, size.height / iconSize.height);
            icon->setScale(iconScale);
            icon->setColor(enabled ? (selected ? Color3B(255, 238, 184) : Color3B(238, 226, 204))
                                   : Color3B(118, 118, 122));
            icon->setOpacity(enabled ? (selected ? 255 : 232) : 126);
            icon->setPosition(Vec2(size.width * 0.5f, size.height * 0.5f));
            root->addChild(icon, 0);
            return root;
        }
    }

    auto bg = createFittedSprite(BUTTON_PATH, size,
        enabled ? (selected ? Color3B(255, 238, 184) : Color3B(232, 224, 204))
                : Color3B(132, 132, 132),
        enabled ? (selected ? 248 : 228) : 120);
    if (bg)
        root->addChild(bg, 0);

    auto label = createStyledLabel(text, fontSize, textColor, true);
    label->enableShadow(Color4B(0, 0, 0, 190), Size(1.6f * _uiScale, -1.6f * _uiScale), 1);
    fitLabelInside(label, Size(size.width * 0.62f, size.height * 0.48f));
    label->setPosition(Vec2(size.width * 0.5f, size.height * 0.54f));
    root->addChild(label, 1);
    return root;
}

MenuItemSprite* SettingsScene::createImageButton(const std::string& text,
    const Size& size, float fontSize,
    const std::function<void(Ref*)>& callback, bool enabled)
{
    auto normal = createButtonState(text, size, fontSize,
        enabled ? COLOR_BUTTON_TEXT : COLOR_DISABLED_TEXT, false, enabled);
    auto selected = createButtonState(text, size, fontSize,
        enabled ? Color3B(255, 242, 190) : COLOR_DISABLED_TEXT, true, enabled);
    auto disabled = createButtonState(text, size, fontSize,
        COLOR_DISABLED_TEXT, false, false);
    selected->setScale(0.96f);

    auto item = MenuItemSprite::create(normal, selected, disabled,
        [callback](Ref* sender) { if (callback) callback(sender); });
    item->setContentSize(size);
    item->setEnabled(enabled);
    return item;
}

void SettingsScene::createSectionTitle(Node* parent, const std::string& text,
    const Vec2& position, float width)
{
    auto title = createTextWithBackground(text, 30.0f * _uiScale,
        Size(width, 58.0f * _uiScale), COLOR_SECTION, nullptr, true);
    title->setPosition(Vec2(position.x - title->getContentSize().width * 0.5f,
        position.y - title->getContentSize().height * 0.5f));
    parent->addChild(title, 11);
}

void SettingsScene::createGraphicsSettings(Node* parent, Menu* menu,
    float centerX, float topY, float panelW)
{
    auto* lm = LanguageManager::getInstance();
    const float y1 = topY;
    const float y2 = y1 - 100.0f * _uiScale;
    const float titleX = centerX - 500.0f * _uiScale;
    const float leftX = centerX - 280.0f * _uiScale;
    const float valueX = centerX + 50.0f * _uiScale;
    const float rightX = centerX + 380.0f * _uiScale;
    const float applyX = centerX + 580.0f * _uiScale;

    int rw = RESOLUTION_PRESETS[_currentResIndex].first;
    int rh = RESOLUTION_PRESETS[_currentResIndex].second;
    std::string resStr = std::to_string(rw) + " \xC3\x97 " + std::to_string(rh);

    auto resName = createTextWithBackground(lm->getString("resolution"),
        30.0f * _uiScale, Size(360.0f * _uiScale, 88.0f * _uiScale),
        COLOR_SECTION, nullptr, true);
    resName->setPosition(Vec2(titleX - resName->getContentSize().width * 0.5f,
        y1 - resName->getContentSize().height * 0.5f));
    parent->addChild(resName, 11);

    auto resValue = createTextWithBackground(resStr, 28.0f * _uiScale,
        Size(460.0f * _uiScale, 88.0f * _uiScale), COLOR_VALUE, &_resolutionLabel);
    resValue->setPosition(Vec2(valueX - resValue->getContentSize().width * 0.5f,
        y1 - resValue->getContentSize().height * 0.5f));
    parent->addChild(resValue, 11);

    auto resLeft = createImageButton("<", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onResolutionLeft(sender); });
    auto resRight = createImageButton(">", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onResolutionRight(sender); });
    auto apply = createImageButton(lm->getString("apply"),
        Size(130.0f * _uiScale, 44.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onApplyResolution(sender); });
    resLeft->setPosition(Vec2(leftX, y1));
    resRight->setPosition(Vec2(rightX, y1));
    apply->setPosition(Vec2(applyX, y1));
    menu->addChild(resLeft);
    menu->addChild(resRight);
    menu->addChild(apply);

    auto modeName = createTextWithBackground(lm->getString("display_mode"),
        30.0f * _uiScale, Size(360.0f * _uiScale, 88.0f * _uiScale),
        COLOR_SECTION, nullptr, true);
    modeName->setPosition(Vec2(titleX - modeName->getContentSize().width * 0.5f,
        y2 - modeName->getContentSize().height * 0.5f));
    parent->addChild(modeName, 11);

    static const char* DM_NAMES[] = { "windowed", "borderless", "fullscreen" };
    auto modeValue = createTextWithBackground(lm->getString(DM_NAMES[_currentDisplayMode]),
        28.0f * _uiScale, Size(410.0f * _uiScale, 88.0f * _uiScale),
        COLOR_VALUE, &_displayModeLabel);
    modeValue->setPosition(Vec2(valueX - modeValue->getContentSize().width * 0.5f,
        y2 - modeValue->getContentSize().height * 0.5f));
    parent->addChild(modeValue, 11);

    auto modeLeft = createImageButton("<", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onDisplayModeLeft(sender); });
    auto modeRight = createImageButton(">", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onDisplayModeRight(sender); });
    modeLeft->setPosition(Vec2(leftX, y2));
    modeRight->setPosition(Vec2(rightX, y2));
    menu->addChild(modeLeft);
    menu->addChild(modeRight);
}

void SettingsScene::createKeyBindingSettings(Node* parent, Menu* menu,
    float centerX, float topY, float panelW)
{
    auto* lm = LanguageManager::getInstance();
    auto* ud = UserDefault::getInstance();
    const float titleX = centerX - 500.0f * _uiScale;
    const float actionNameX = centerX - 115.0f * _uiScale;
    const float keyValueX = centerX + 215.0f * _uiScale;
    const float actionColumnX = centerX + 480.0f * _uiScale;

    createSectionTitle(parent, lm->getString("key_bindings"), Vec2(titleX, topY),
        360.0f * _uiScale);

    const float rowH = 76.0f * _uiScale;
    const float startY = topY + rowH * 1.5f;

    int defUp = static_cast<int>(EventKeyboard::KeyCode::KEY_W);
    int defDown = static_cast<int>(EventKeyboard::KeyCode::KEY_S);
    int defLeft = static_cast<int>(EventKeyboard::KeyCode::KEY_A);
    int defRight = static_cast<int>(EventKeyboard::KeyCode::KEY_D);

    auto addRow = [&](int row, const std::string& name, const std::string& value,
        Label** keyLabel, const std::function<void(Ref*)>& cb, bool canRebind, int actionIdx)
    {
        float y = startY - row * rowH;
        bool listening = _listeningAction == actionIdx;

        auto rowBg = LayerColor::create(listening ? Color4B(206, 153, 58, 66) : Color4B(5, 12, 22, 0),
            820.0f * _uiScale, rowH - 10.0f * _uiScale);
        rowBg->setPosition(Vec2(centerX - 410.0f * _uiScale,
            y - (rowH - 10.0f * _uiScale) * 0.5f));
        parent->addChild(rowBg, 2);
        if (actionIdx >= 0)
            _keyRowBackgrounds.push_back(rowBg);

        auto nameNode = createTextWithBackground(name, 22.0f * _uiScale,
            Size(350.0f * _uiScale, 70.0f * _uiScale), COLOR_TEXT,
            nullptr, false, true);
        nameNode->setPosition(Vec2(actionNameX - nameNode->getContentSize().width * 0.5f, y - nameNode->getContentSize().height * 0.5f));
        parent->addChild(nameNode, 11);

        auto keyNode = createTextWithBackground(listening ? lm->getString("press_key") : value,
            26.0f * _uiScale, Size(200.0f * _uiScale, 70.0f * _uiScale),
            canRebind ? COLOR_VALUE : Color3B(172, 178, 186), keyLabel, listening, true);
        keyNode->setPosition(Vec2(keyValueX - keyNode->getContentSize().width * 0.5f, y - keyNode->getContentSize().height * 0.5f));
        parent->addChild(keyNode, 11);

        auto button = createImageButton(lm->getString("rebind"),
            Size(220.0f * _uiScale, 72.0f * _uiScale), 24.0f * _uiScale,
            cb, canRebind);
        button->setPosition(Vec2(actionColumnX, y));
        menu->addChild(button);
    };

    addRow(0, lm->getString("move_up"),
        keyCodeToName(ud->getIntegerForKey(USERD_KEY_UP, defUp)),
        &_keyLabelUp, [this](Ref* sender) { onRebindMoveUp(sender); }, true, 0);
    addRow(1, lm->getString("move_down"),
        keyCodeToName(ud->getIntegerForKey(USERD_KEY_DOWN, defDown)),
        &_keyLabelDown, [this](Ref* sender) { onRebindMoveDown(sender); }, true, 1);
    addRow(2, lm->getString("move_left"),
        keyCodeToName(ud->getIntegerForKey(USERD_KEY_LEFT, defLeft)),
        &_keyLabelLeft, [this](Ref* sender) { onRebindMoveLeft(sender); }, true, 2);
    addRow(3, lm->getString("move_right"),
        keyCodeToName(ud->getIntegerForKey(USERD_KEY_RIGHT, defRight)),
        &_keyLabelRight, [this](Ref* sender) { onRebindMoveRight(sender); }, true, 3);
}

void SettingsScene::createLanguageSettings(Node* parent, Menu* menu,
    float centerX, float topY, float panelW)
{
    auto* lm = LanguageManager::getInstance();
    const float y = topY;
    const float titleX = centerX - 500.0f * _uiScale;
    const float leftX = centerX - 280.0f * _uiScale;
    const float valueX = centerX + 50.0f * _uiScale;
    const float rightX = centerX + 380.0f * _uiScale;

    auto langName = createTextWithBackground(lm->getString("language"),
        30.0f * _uiScale, Size(360.0f * _uiScale, 88.0f * _uiScale),
        COLOR_SECTION, nullptr, true);
    langName->setPosition(Vec2(titleX - langName->getContentSize().width * 0.5f,
        y - langName->getContentSize().height * 0.5f));
    parent->addChild(langName, 11);

    auto langValue = createTextWithBackground(lm->getLanguageName(),
        28.0f * _uiScale, Size(410.0f * _uiScale, 88.0f * _uiScale),
        COLOR_VALUE, &_languageLabel);
    langValue->setPosition(Vec2(valueX - langValue->getContentSize().width * 0.5f,
        y - langValue->getContentSize().height * 0.5f));
    parent->addChild(langValue, 11);

    auto left = createImageButton("<", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onLanguageLeft(sender); });
    auto right = createImageButton(">", Size(104.0f * _uiScale, 96.0f * _uiScale),
        28.0f * _uiScale, [this](Ref* sender) { onLanguageRight(sender); });
    left->setPosition(Vec2(leftX, y));
    right->setPosition(Vec2(rightX, y));
    menu->addChild(left);
    menu->addChild(right);
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
    _resolutionLabel->setString(std::to_string(rw) + " \xC3\x97 " + std::to_string(rh));
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

    for (size_t i = 0; i < _keyRowBackgrounds.size(); ++i)
    {
        auto rowBg = dynamic_cast<LayerColor*>(_keyRowBackgrounds[i]);
        if (!rowBg) continue;
        rowBg->setColor(i == static_cast<size_t>(_listeningAction)
            ? Color3B(206, 153, 58)
            : Color3B(5, 12, 22));
        rowBg->setOpacity(i == static_cast<size_t>(_listeningAction) ? 66 : 0);
    }
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
