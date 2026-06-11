#include "RoommateAssistManager.h"
#include <cstdlib>

USING_NS_CC;

RoommateAssistManager* RoommateAssistManager::create(Player* player)
{
    auto mgr = new (std::nothrow) RoommateAssistManager();
    if (mgr && mgr->init(player))
    {
        mgr->autorelease();
        return mgr;
    }
    CC_SAFE_DELETE(mgr);
    return nullptr;
}

bool RoommateAssistManager::init(Player* player)
{
    _player = player;
    return true;
}

void RoommateAssistManager::update(float /*dt*/)
{
    if (!_player || !_player->isObjectActive()) return;

    bool nowIrr = (_player->getCurrentMood() == MoodType::Irritable);

    // 触发计数：从非 Irritable -> Irritable
    if (nowIrr && !_prevWasIrritable)
    {
        _irritableTriggers++;

        // 你要求的“每次触发 Irritable 增加战斗力”
        _player->onIrritableTriggered();

        tryAssist();
    }

    _prevWasIrritable = nowIrr;
}

void RoommateAssistManager::tryAssist()
{
    if (_hasAssisted) return;

    // 达到 25 次触发
    if (_irritableTriggers >= 25)
    {
        _hasAssisted = true;

        // 助战内容（你照片里“协助攻击/命中敌人”需要攻击系统支持，目前没完成）
        // 我们先实现照片里确定的防御/补血部分，攻击留给未来武器/子弹系统接入。
        //
        // 防御：短时无敌/降伤害
        _player->setInvincible(true);

        // 补血
        _player->heal(30);

        // 这里你可以再加一个提示 UI/音效/特效（可选）
        CCLOG("[RoommateAssist] Triggered after 25 Irritable hits!");
    }
}