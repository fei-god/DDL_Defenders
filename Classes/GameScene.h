#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include <string>
#include <vector>
#include "Characters/Player.h"
#include "Managers/WaveManager.h"
#include "Weapons/Weapon.h"
#include "Weapons/Bullet.h"
#include "Weapons/BulletPool.h"
#include "Environment/EnvironmentZone.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;
    virtual ~GameScene();
    virtual void update(float dt) override;

    void updateUI(Player* player);
    void updateSurvivalTime(float dt);

    CREATE_FUNC(GameScene);

private:
    enum class RewardType
    {
        Coffee,
        Power,
        Pen,
        FreezeDevice
    };

    struct RewardPickup
    {
        cocos2d::Node* node;
        RewardType type;
    };

    struct WeaponOption
    {
        int id;
        std::string name;
        std::string imagePath;
    };

    // --- Placeholder textures ---
    void createPlaceholderTextures();
    void applySpriteFit(cocos2d::Sprite* sprite, float maxWidth, float maxHeight);

    // --- Coordinate conversion ---
    cocos2d::Vec2 viewToGameCoords(const cocos2d::Vec2& viewPos) const;
    cocos2d::Vec2 screenToWorldCoords(const cocos2d::Vec2& screenPos) const;

    // --- Input ---
    void initInputListeners();
    void updateMoveDirection();
    void updatePlayerMovement(float dt);
    void updateCamera();
    void updateEnemyPlayerContact(float dt);
    Weapon* createWeaponById(int weaponId);
    std::vector<WeaponOption> getWeaponOptions() const;
    void rebuildWeaponLoadout();
    cocos2d::Node* createEquipmentIcon(const WeaponOption& option, const cocos2d::Size& boxSize, bool selected);
    void updateWeaponEnergyUI();
    void refreshWeaponSlotUI();
    void applyWeaponLoadout();
    void initSceneConfig();
    void showLevelIntro();
    void hideLevelIntro();
    void initEnvironmentZones();
    void updateEnvironmentEffects(float dt);
    void updateAssignmentProgress(float dt);
    void spawnRewardForEnemy(Enemy* enemy);
    void handleEndlessEnemyKilled(Enemy* enemy);
    int getScoreRewardForEnemy(Enemy* enemy) const;
    void goToAfterBattle(int wave);
    void applyWeaponMastery(Weapon* weapon);
    void applyWeaponMasteryEffects(Weapon* weapon);
    void applyEndlessGrowthToWeapon(Weapon* weapon, int weaponId);

    // ==================== Hub System ====================
    void updateHubInteraction();
    void showDeskPanel();
    void hideDeskPanel();
    void showBedPanel();
    void hideBedPanel();
    bool isInRect(const cocos2d::Vec2& pos, const cocos2d::Rect& rect) const;
    void spawnReward(RewardType type, const cocos2d::Vec2& position);
    cocos2d::Node* createRewardNode(RewardType type);
    void updateRewards(float dt);
    void applyReward(RewardType type);
    void updateFreezeEffect(float dt);
    void updatePlayerMoodVisual();
    void updateExpBarUI();
    void updateWaveTimerUI();
    std::string getMoodPlayerImagePath(MoodType mood) const;
    void goToGameOver();
    void goToVictory();
    int calculateScore() const;

    // ==================== Pause V2 ====================
    void buildPauseStatsPanel(cocos2d::Node* parent, const cocos2d::Vec2& pos, float s);
    void buildPauseWeaponPanel(cocos2d::Node* parent, const cocos2d::Vec2& pos, float s);
    void onPauseEquipWeapon(cocos2d::Ref* sender);
    void onPauseUnequipWeapon(cocos2d::Ref* sender);

    // ==================== UI ====================
    cocos2d::LayerColor* _hpBarBg;
    cocos2d::LayerColor* _hpBarFill;
    float _hpBarMaxWidth;
    cocos2d::LayerColor* _hudPanelBg;
    cocos2d::Label* _moodLabel;
    cocos2d::Sprite* _weaponIcon;
    cocos2d::Label* _weaponLabel;
    cocos2d::LayerColor* _weaponEnergyBg;
    cocos2d::LayerColor* _weaponEnergyFill;
    float _weaponEnergyBarMaxWidth;
    cocos2d::Label* _progressLabel;
    cocos2d::Label* _taskLabel;
    cocos2d::LayerColor* _taskBarBg;
    cocos2d::LayerColor* _taskBarFill;
    float _taskBarMaxWidth;
    cocos2d::Label* _environmentLabel;
    cocos2d::Label* _survivalTimeLabel;
    cocos2d::Label* _topHintLabel;
    cocos2d::Label* _endlessStatsLabel;
    // EXP bar
    cocos2d::LayerColor* _expBarBg;
    cocos2d::LayerColor* _expBarFill;
    float _expBarMaxWidth;
    cocos2d::Label* _expLevelLabel;
    cocos2d::Label* _expFractionLabel;
    // Wave timer
    cocos2d::Label* _waveTimerLabel;
    std::vector<cocos2d::Node*> _weaponSlotNodes;
    std::vector<int> _lastWeaponSlotIds;
    int _lastWeaponSlotIndex;
    float m_survivalTime;

    // ==================== World / Camera ====================
    cocos2d::Node* _worldLayer;
    cocos2d::Size _worldSize;
    float _worldScale;
    bool _cameraInitialized;

    // ==================== Player ====================
    Player* m_player;

    // Triangle visual that rotates toward mouse
    cocos2d::DrawNode* _playerVisual;

    // Normalized direction the player is facing (single source of truth)
    cocos2d::Vec2 _playerDir;

    // Current mouse position in game coordinates
    cocos2d::Vec2 _mousePos;

    // ==================== Combat ====================
    std::vector<Weapon*> _weapons;
    std::vector<int> _weaponLoadoutIds;   // 2 equipped slots
    std::vector<int> _backpackWeaponIds;  // 4 reserve backpack slots
    cocos2d::Node* _equipmentLayer;
    std::vector<Bullet*> _bullets;
    cocos2d::Node* _bulletLayer;
    BulletPool _bulletPool;

    // ==================== Wave / Spawning ====================
    WaveManager* _waveManager;

    // ==================== Input state ====================
    cocos2d::Vec2 _moveDirection;
    bool _keyW, _keyA, _keyS, _keyD;

    // Key bindings (loaded from UserDefault, can be rebound in Settings)
    cocos2d::EventKeyboard::KeyCode _keyMoveUp;
    cocos2d::EventKeyboard::KeyCode _keyMoveDown;
    cocos2d::EventKeyboard::KeyCode _keyMoveLeft;
    cocos2d::EventKeyboard::KeyCode _keyMoveRight;

    void loadKeyBindings();

    // ==================== Hub panels ====================
    cocos2d::Node* _deskPanel = nullptr;
    cocos2d::Node* _bedPanel = nullptr;
    cocos2d::Node* _hubOverlay = nullptr;  // 50% dark overlay
    cocos2d::Label* _hubHintLabel = nullptr;  // "[E] Next Level"
    bool _deskPanelOpen = false;
    bool _bedPanelOpen = false;

    // ==================== Pause ====================
    bool _isPaused;
    cocos2d::Node* _pauseLayer;

    void showPauseMenu();
    void hidePauseMenu();
    void onPauseResumeClicked(cocos2d::Ref* sender);
    void onPauseRestartClicked(cocos2d::Ref* sender);
    void onPauseSaveClicked(cocos2d::Ref* sender);
    void onPauseSettingsClicked(cocos2d::Ref* sender);
    void onPauseTitleClicked(cocos2d::Ref* sender);

    // ==================== Game state ====================
    bool _isGameOver;
    bool _isVictory;
    bool _isEndlessMode;
    bool _pendingAfterBattle = false;
    int _pendingWave = -1;
    int _sceneId = 0;            // 0=hub, 1=library, 2=classroom, 3=office
    bool _isHubScene = false;
    bool _isBossScene = false;
    int _levelNumber;
    float _ddlTimeLimit;
    float _ddlTimeRemaining;
    int _completedDdlCount;
    bool _levelIntroActive;
    float _levelIntroTimer;
    cocos2d::Node* _levelIntroLayer;
    float _assignmentProgress;
    bool _nearDesk;
    bool _nearPowerSocket;
    // Thesis Boss
    float _thesisProgress = 0.0f;
    // DDL Pressure (endless mode)
    float _ddlPressure = 0.0f;
    cocos2d::LayerColor* _vignetteLayer = nullptr;
    int _lastKillCount;
    float _enemyContactDamageCooldown;
    float _lowHpMoodTimer;
    float _freezeTimer;
    float _rewardSpawnTimer;
    int _endlessScore;
    int _lastHandledPlayerLevel;
    int _lifeOnKill;
    int _weaponDamageBonus;
    int _projectileBonus;
    float _energyRecoveryBonusPercent;
    std::vector<int> _masteredWeaponIds;
    std::string _currentPlayerMoodImage;
    std::vector<EnvironmentZone*> _environmentZones;
    int _lastUiLanguageIndex;
    std::vector<RewardPickup> _rewardPickups;
};

#endif // __GAME_SCENE_H__
