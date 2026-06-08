#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

// 主菜单场景，继承自 cocos2d::Scene
class MainMenuScene : public cocos2d::Scene
{
public:
    // 静态方法，用于创建并返回带有当前层的场景实例
    static cocos2d::Scene* createScene();

    // 真正的初始化逻辑在这个回调中实现
    virtual bool init() override;

    // 回调函数声明
    // 点击开始游戏按钮回调
    void onStartGameClicked(cocos2d::Ref* sender);
    
    // 点击退出游戏按钮回调
    void onExitGameClicked(cocos2d::Ref* sender);

    // 宏定义，利用它可以快速生成 create() 方法并实现内存管理机制
    CREATE_FUNC(MainMenuScene);
};

#endif // __MAIN_MENU_SCENE_H__
