#pragma once
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "cocos2d.h"
#include <string>

// 游戏对象类型，方便后面区分 Player、Enemy、Weapon、Item 等
enum class GameObjectType
{
    Unknown,
    Player,
    Enemy,
    Weapon,
    Item,
    Bullet,
    Environment
};

class GameObject : public cocos2d::Sprite
{
public:
    CREATE_FUNC(GameObject);

    GameObject();
    virtual ~GameObject();

    // Cocos2d-x 基础初始化
    virtual bool init() override;

    // 自定义初始化游戏对象
    virtual bool initObject(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition
    );

    // 创建对象
    static GameObject* create(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition
    );

    // 每帧更新接口，子类可以重写
    virtual void updateObject(float dt);

    // 设置 / 获取对象名字
    void setObjectName(const std::string& name);
    std::string getObjectName() const;

    // 设置 / 获取对象类型
    void setObjectType(GameObjectType type);
    GameObjectType getObjectType() const;

    // 设置 / 获取图片路径
    virtual bool setObjectImage(const std::string& imagePath);
    std::string getImagePath() const;

    // 设置 / 获取位置
    virtual void setObjectPosition(const cocos2d::Vec2& position);
    virtual cocos2d::Vec2 getObjectPosition() const;

    // 获取碰撞框
    virtual cocos2d::Rect getCollisionBox() const;

    // 设置 / 获取是否激活
    virtual void setActive(bool active);
    virtual bool isObjectActive() const;

protected:
    std::string objectName;        // 对象名称
    GameObjectType objectType;     // 对象类型
    std::string imagePath;         // 图片路径
    bool isActive;                 // 是否激活
};

#endif
