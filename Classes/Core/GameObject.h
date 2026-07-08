#pragma once
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "cocos2d.h"
#include <string>
#include <vector>

// ��Ϸ�������ͣ������������ Player��Enemy��Weapon��Item ��
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

// ��Ϸ������Ӫ
// �����ж���ҡ����ˡ��ӵ���������֮��Ĺ�ϵ
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

    // Cocos2d-x ������ʼ��
    virtual bool init() override;

    // �Զ����ʼ����Ϸ����
    virtual bool initObject(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition
    );

    // ��������
    static GameObject* create(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition
    );

    // ÿ֡���½ӿڣ����������д
    virtual void updateObject(float dt);

    // =========================
    // ����������Ϣ
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
    // ��ǩϵͳ
    // ���磺Boss��Burnable��Gun��Melee��Pickup
    // =========================

    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    void clearTags();

    // =========================
    // ͼƬ����ʾ
    // =========================

    virtual bool setObjectImage(const std::string& imagePath);
    std::string getImagePath() const;

    void setObjectScale(float scale);
    float getObjectScale() const;

    // =========================
    // λ�á������ƶ�
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

    // =========================
    // ��ײ�뷶Χ�ж�
    // =========================

    virtual cocos2d::Rect getCollisionBox() const;

    void setCollisionRadius(float radius);
    float getCollisionRadius() const;

    bool isUsingCircleCollision() const;

    void setCollisionEnabled(bool enabled);
    bool isCollisionEnabled() const;

    virtual bool isCollidingWith(const GameObject* other) const;
    virtual bool isInRangeOf(const GameObject* other, float range) const;

    float distanceTo(const GameObject* other) const;
    float distanceToPoint(const cocos2d::Vec2& point) const;

    // =========================
    // ��������
    // �ʺ� Bullet��Effect��Trap ����ʱ����
    // =========================

    void setLifeTime(float time);
    float getLifeTime() const;

    virtual void updateLifeTime(float dt);
    virtual bool isExpired() const;

    // =========================
    // ������¡�����������
    // =========================

    virtual void setActive(bool active);
    virtual bool isObjectActive() const;

    void setUpdateEnabled(bool enabled);
    bool isUpdateEnabled() const;

    void markForDestroy();
    bool isMarkedForDestroy() const;

    // =========================
    // ��ͼ�߽�
    // =========================

    bool isOutOfBounds(const cocos2d::Rect& bounds) const;

    // =========================
    // ������Ϣ
    // =========================

    virtual std::string getDebugInfo() const;

protected:
    static int nextObjectId;

    int objectId;                   // ����Ψһ ID
    std::string objectName;         // ��������
    GameObjectType objectType;      // ��������
    GameObjectCamp objectCamp;      // ������Ӫ
    std::string imagePath;          // ͼƬ·��

    bool isActive;                  // �Ƿ񼤻�
    bool updateEnabled;             // �Ƿ�ִ�� updateObject
    bool collisionEnabled;          // �Ƿ������ײ
    bool shouldDestroy;             // �Ƿ�ȴ�����

    std::vector<std::string> tags;  // ��ǩ�б�

    cocos2d::Vec2 velocity;         // ��ǰ�ٶ�����
    cocos2d::Vec2 direction;        // ��ǰ����
    float moveSpeed;                // �ƶ��ٶ�

    float collisionRadius;          // Բ����ײ�뾶
    bool useCircleCollision;        // �Ƿ�ʹ��Բ����ײ

    float lifeTime;                 // �����������
    float currentLifeTime;          // ��ǰ�Ѵ���ʱ��
    bool hasLifeTimeLimit;          // �Ƿ�����������������
};

#endif
