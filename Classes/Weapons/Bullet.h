#pragma once
#ifndef BULLET_H
#define BULLET_H

#include "GameObject.h"
#include <vector>
#include <string>
using namespace std;

class Bullet : public GameObject
{
public:
	Bullet();
	virtual ~Bullet();

	static Bullet* createBullet(
		const string& name,
		const string& imagePath,
		const cocos2d::Vec2& startPosition,
		const cocos2d::Vec2& direction,
		float speed,
		int damage,
		float lifeTime,
		bool canPierce
	);

	bool initBullet(
		const string& name,
		const string& imagePath,
		const cocos2d::Vec2& startPosition,
		const cocos2d::Vec2& direction,
		float speed,
		int damage,
		float lifeTime,
		bool canPierce
	);

	virtual void updateObject(float dt) override;

	virtual cocos2d::Rect getCollisionBox() const override;

	int getDamage() const;
	bool isExpired() const;
	bool isPiercing() const;
	void markHit();
	bool hasHitObject(GameObject* object) const;
	void recordHitObject(GameObject* object);

private:
	cocos2d::Vec2 _direction;
	float _speed;
	int _damage;
	float _lifeTime;
	bool _canPierce;
	float _timer;
	bool _expired;

	vector<GameObject*> _hitObjects; // 记录已经击中的对象，避免重复伤害

};



#endif