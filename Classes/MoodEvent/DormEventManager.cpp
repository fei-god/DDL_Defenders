#include "DormEventManager.h"
#include <cstdlib>

USING_NS_CC;

DormEventManager* DormEventManager::create(Player* player, cocos2d::Node* parentLayer, float intervalSeconds)
{
    auto mgr = new (std::nothrow) DormEventManager();
    if (mgr && mgr->init(player, parentLayer, intervalSeconds))
    {
        mgr->autorelease();
        return mgr;
    }
    CC_SAFE_DELETE(mgr);
    return nullptr;
}

bool DormEventManager::init(Player* player, cocos2d::Node* parentLayer, float intervalSeconds)
{
    _player = player;
    _parentLayer = parentLayer;
    _interval = intervalSeconds;
    _timer = _interval;
    return true;
}

void DormEventManager::update(float dt)
{
    if (!_player || !_player->isObjectActive()) return;

    _timer -= dt;
    if (_timer <= 0.0f)
    {
        int type = rand() % 3; // 三种事件
        triggerEvent(type);
        _timer = _interval;
    }
}

void DormEventManager::triggerEvent(int eventType)
{
    if (!_player) return;

    // 事件类型：
    // 0: 断电 -> 降低速度（用 Exhausted）
    // 1: 舍友补课 -> Focus
    // 2: 弹窗来袭 -> Irritable
    switch (eventType)
    {
    case 0:
        _player->changeMood(MoodType::Exhausted, 5.0f);
        break;
    case 1:
        _player->heal(15);
        _player->changeMood(MoodType::Focus, 4.0f);
        break;
    case 2:
        _player->changeMood(MoodType::Irritable, 4.0f);
        break;
    default:
        break;
    }
}