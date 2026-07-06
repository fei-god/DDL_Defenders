#ifndef __STORY_MODE_SCENE_H__
#define __STORY_MODE_SCENE_H__

#include "cocos2d.h"
#include <string>
#include <vector>

class StoryModeScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    static cocos2d::Scene* createEndlessScene();

    virtual bool init() override;

    CREATE_FUNC(StoryModeScene);

    // --- Save data helpers (static so GameScene can call them) ---
    struct SaveEntry
    {
        int index;         // position in the save list (0 = newest)
        int type;          // 0 = auto, 1 = manual
        int level;         // level number
        std::string timestamp;
    };

    // Load all saves sorted newest-first
    static std::vector<SaveEntry> loadAllSaves(bool endlessMode = false);

    // Add an auto-save (max 5 auto-saves, oldest gets removed)
    static void addAutoSave(int level);

    // Add a manual save (max 10, returns false if full)
    static bool addManualSave(int level, bool endlessMode = false);

    // Delete a save by its index in the loadAllSaves() result
    static void deleteSaveByIndex(int displayIndex, bool endlessMode = false);

private:
    // --- Views ---
    void showMainView(float s);
    void showLoadView(float s);
    void clearCurrentView();

    // --- Main view callbacks ---
    void onNewGameClicked(cocos2d::Ref* sender);
    void onLoadGameClicked(cocos2d::Ref* sender);
    void onBackClicked(cocos2d::Ref* sender);

    // --- Load view callbacks ---
    void onSaveLoadClicked(cocos2d::Ref* sender);        // tag = display index
    void onSaveDeleteClicked(cocos2d::Ref* sender);       // tag = display index
    void onLoadBackClicked(cocos2d::Ref* sender);

    // --- Delete confirmation ---
    void showDeleteConfirm(int displayIndex);
    void hideDeleteConfirm();
    void onDeleteConfirmYes(cocos2d::Ref* sender);
    void onDeleteConfirmNo(cocos2d::Ref* sender);

    void startNewGame();
    void loadSaveAndStart(int displayIndex);

    cocos2d::Node* _currentView = nullptr;
    cocos2d::Node* _confirmLayer = nullptr;
    int _pendingDeleteIndex = -1;
    bool _isEndlessMode = false;
};

#endif // __STORY_MODE_SCENE_H__
