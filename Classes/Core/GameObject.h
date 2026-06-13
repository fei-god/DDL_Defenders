#pragma once
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "cocos2d.h"
#include <string>
#include <vector>

// 游戏对象类型，方便后面区分 Player、Enemy、Weapon、Item 等
enum class GameObjectType
{
    Unknown,
    Player,
    Enemy,
    Weapon,
    Item,
    Bullet,
    Environment,
    Effect
};

// 游戏对象阵营
// 用于判断玩家、敌人、子弹、掉落物之间的关系
enum class GameObjectCamp
{
    Neutral,
    Player,
    Enemy
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

    // =========================
    // 基础身份信息
    // =========================

    int getObjectId() const;

    void setObjectName(const std::string& name);
    std::string getObjectName() const;

    void setObjectType(GameObjectType type);
    GameObjectType getObjectType() const;

    void setObjectCamp(GameObjectCamp camp);
    GameObjectCamp getObjectCamp() const;

    bool isSameCampWith(const GameObject* other) const;
    bool isEnemyCampWith(const GameObject* other) const;

    // =========================
    // 标签系统
    // 例如：Boss、Burnable、Gun、Melee、Pickup
    // =========================

    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    void clearTags();

    // =========================
    // 图片与显示
    // =========================

    virtual bool setObjectImage(const std::string& imagePath);
    std::string getImagePath() const;

    void setObjectScale(float scale);
    float getObjectScale() const;

    void setObjectSize(const cocos2d::Size& size);
    cocos2d::Size getObjectSize() const;

    // =========================
    // 位置、方向、移动
    // =========================

    virtual void setObjectPosition(const cocos2d::Vec2& position);
    virtual cocos2d::Vec2 getObjectPosition() const;

    void setVelocity(const cocos2d::Vec2& velocity);
    cocos2d::Vec2 getVelocity() const;

    void setDirection(const cocos2d::Vec2& direction);
    cocos2d::Vec2 getDirection() const;

    void setMoveSpeed(float speed);
    float getMoveSpeed() const;

    virtual void moveByVelocity(float dt);
    virtual void moveInDirection(float dt);

    void faceTo(const cocos2d::Vec2& targetPosition);
    void faceToDirection(const cocos2d::Vec2& direction);

    // =========================
    // 碰撞与范围判断
    // =========================

    virtual cocos2d::Rect getCollisionBox() const;

    void setCollisionRadius(float radius);
    float getCollisionRadius() const;

    void setUseCircleCollision(bool useCircle);
    bool isUsingCircleCollision() const;

    void setCollisionEnabled(bool enabled);
    bool isCollisionEnabled() const;

    virtual bool isCollidingWith(const GameObject* other) const;
    virtual bool isInRangeOf(const GameObject* other, float range) const;

    float distanceTo(const GameObject* other) const;
    float distanceToPoint(const cocos2d::Vec2& point) const;

    // =========================
    // 生命周期
    // 适合 Bullet、Effect、Trap 等临时对象
    // =========================

    void setLifeTime(float time);
    float getLifeTime() const;
    float getCurrentLifeTime() const;
    void resetLifeTime();

    virtual void updateLifeTime(float dt);
    virtual bool isExpired() const;

    // =========================
    // 激活、更新、交互、销毁
    // =========================

    virtual void setActive(bool active);
    virtual bool isObjectActive() const;

    void setUpdateEnabled(bool enabled);
    bool isUpdateEnabled() const;

    void setInteractable(bool interactable);
    bool isInteractable() const;

    void markForDestroy();
    bool isMarkedForDestroy() const;

    // =========================
    // 地图边界
    // =========================

    bool isOutOfBounds(const cocos2d::Rect& bounds) const;
    void clampPositionInBounds(const cocos2d::Rect& bounds);

    // =========================
    // 调试信息
    // =========================

    virtual std::string getDebugInfo() const;

protected:
    static int nextObjectId;

    int objectId;                   // 对象唯一 ID
    std::string objectName;         // 对象名称
    GameObjectType objectType;      // 对象类型
    GameObjectCamp objectCamp;      // 对象阵营
    std::string imagePath;          // 图片路径

    bool isActive;                  // 是否激活
    bool updateEnabled;             // 是否执行 updateObject
    bool collisionEnabled;          // 是否参与碰撞
    bool interactable;              // 是否可交互
    bool shouldDestroy;             // 是否等待销毁

    std::vector<std::string> tags;  // 标签列表

    cocos2d::Vec2 velocity;         // 当前速度向量
    cocos2d::Vec2 direction;        // 当前方向
    float moveSpeed;                // 移动速度

    float collisionRadius;          // 圆形碰撞半径
    bool useCircleCollision;        // 是否使用圆形碰撞

    float lifeTime;                 // 最大生命周期
    float currentLifeTime;          // 当前已存在时间
    bool hasLifeTimeLimit;          // 是否启用生命周期限制
};

#endif
