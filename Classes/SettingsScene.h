#pragma once
#ifndef __SETTINGS_SCENE_H__
#define __SETTINGS_SCENE_H__

#include "cocos2d.h"

class SettingsScene : public cocos2d::Scene
{
public:
    enum class Entry
    {
        MAIN_MENU,   // Back → replaceScene(MainMenuScene)
        PAUSE_MENU   // Back → popScene()
    };

    static cocos2d::Scene* createScene(Entry entry);

    virtual bool init() override;
    virtual ~SettingsScene();

    CREATE_FUNC(SettingsScene);

private:
    void setEntry(Entry entry);

    void buildUI();

    // Resolution
    void onResolutionLeft(cocos2d::Ref* sender);
    void onResolutionRight(cocos2d::Ref* sender);
    void onApplyResolution(cocos2d::Ref* sender);
    void updateResolutionLabel();
    int  findResolutionIndex(int w, int h) const;

    // Key bindings
    void onRebindMoveUp(cocos2d::Ref* sender);
    void onRebindMoveDown(cocos2d::Ref* sender);
    void onRebindMoveLeft(cocos2d::Ref* sender);
    void onRebindMoveRight(cocos2d::Ref* sender);
    void refreshKeyLabels();
    std::string keyCodeToName(int code) const;

    // Display mode
    void onDisplayModeLeft(cocos2d::Ref* sender);
    void onDisplayModeRight(cocos2d::Ref* sender);
    void applyDisplayMode(int mode, int oldMode);
    void updateDisplayModeLabel();
    void fitResolutionToScreen(int screenW, int screenH);

    // Language
    void onLanguageLeft(cocos2d::Ref* sender);
    void onLanguageRight(cocos2d::Ref* sender);
    void updateLanguageLabel();

    // Navigation
    void onBackClicked(cocos2d::Ref* sender);

    // Keyboard listener (for key rebinding)
    void initKeyListener();
    cocos2d::EventListenerKeyboard* _keyListener;

    // Which action is currently being rebound (-1 = none)
    int _listeningAction;  // 0=up, 1=down, 2=left, 3=right

    // Current resolution index
    int _currentResIndex;

    // Current language index
    int _currentLangIndex;

    // Entry point
    Entry _entry;

    // Current display mode (0=windowed, 1=borderless, 2=fullscreen)
    int _currentDisplayMode;

    // Saved window position & size for returning to windowed mode
    int _savedWindowX, _savedWindowY, _savedWindowW, _savedWindowH;

    // UI labels that need updating
    cocos2d::Label* _resolutionLabel;
    cocos2d::Label* _displayModeLabel;
    cocos2d::Label* _languageLabel;
    cocos2d::Label* _keyLabelUp;
    cocos2d::Label* _keyLabelDown;
    cocos2d::Label* _keyLabelLeft;
    cocos2d::Label* _keyLabelRight;
};

#endif // __SETTINGS_SCENE_H__
