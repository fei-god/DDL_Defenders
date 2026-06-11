/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Player.h"
#include "WaveManager.h"
#include "Bullet.h"
#include <vector>

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // 每帧更新
    virtual void update(float dt) override;

    // 菜单回调
    void menuCloseCallback(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

private:
    // 初始化游戏
    void initGame();
    // 初始化输入监听
    void initInput();
    // 初始化UI
    void initUI();

    // 键盘事件回调
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    // 射击
    void fireBullet(const cocos2d::Vec2& direction);

    // 游戏结束
    void gameOver();
    // 胜利
    void victory();

    // 核心游戏对象
    Player* _player;
    WaveManager* _waveManager;

    // 子弹列表
    std::vector<Bullet*> _bullets;

    // 输入状态
    bool _keyLeft;
    bool _keyRight;
    bool _keyUp;
    bool _keyDown;
    bool _keyShootUp;
    bool _keyShootDown;
    bool _keyShootLeft;
    bool _keyShootRight;

    // UI元素
    cocos2d::Label* _hpLabel;
    cocos2d::Label* _waveLabel;
    cocos2d::Label* _killLabel;
    cocos2d::Label* _expLabel;
    cocos2d::Label* _levelLabel;

    // 射击冷却
    float _fireCooldown;
    float _fireCooldownMax;

    // 游戏状态
    bool _gameOver;
    bool _gameWin;
};

#endif // __HELLOWORLD_SCENE_H__
