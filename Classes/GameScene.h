#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include <string>
#include <vector>
#include "Characters/Player.h"
#include "Managers/WaveManager.h"
#include "Weapons/Weapon.h"
#include "Weapons/Bullet.h"

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
    // --- Placeholder textures ---
    void createPlaceholderTextures();

    // --- Coordinate conversion ---
    cocos2d::Vec2 viewToGameCoords(const cocos2d::Vec2& viewPos) const;

    // --- Input ---
    void initInputListeners();
    void updateMoveDirection();
    void fireBullet();

    // ==================== UI ====================
    cocos2d::LayerColor* _hpBarBg;
    cocos2d::LayerColor* _hpBarFill;
    float _hpBarMaxWidth;
    cocos2d::Label* _moodLabel;
    cocos2d::Label* _survivalTimeLabel;
    float m_survivalTime;

    // ==================== Player ====================
    Player* m_player;

    // Triangle visual that rotates toward mouse
    cocos2d::DrawNode* _playerVisual;

    // Normalized direction the player is facing (single source of truth)
    cocos2d::Vec2 _playerDir;

    // Current mouse position in game coordinates
    cocos2d::Vec2 _mousePos;

    // ==================== Combat ====================
    Weapon* _currentWeapon;
    std::vector<Bullet*> _bullets;
    cocos2d::Node* _bulletLayer;

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

    // ==================== Pause ====================
    bool _isPaused;
    cocos2d::Node* _pauseLayer;

    void showPauseMenu();
    void hidePauseMenu();
    void onPauseResumeClicked(cocos2d::Ref* sender);
    void onPauseRestartClicked(cocos2d::Ref* sender);
    void onPauseSettingsClicked(cocos2d::Ref* sender);
    void onPauseTitleClicked(cocos2d::Ref* sender);

    // ==================== Game state ====================
    bool _isGameOver;
};

#endif // __GAME_SCENE_H__
