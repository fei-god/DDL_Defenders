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

    // �����������ӷ� Irritable -> Irritable
    if (nowIrr && !_prevWasIrritable)
    {
        _irritableTriggers++;

        // ��Ҫ��ġ�ÿ�δ��� Irritable ����ս������
        _player->onIrritableTriggered();

        tryAssist();
    }

    _prevWasIrritable = nowIrr;
}

void RoommateAssistManager::tryAssist()
{
    if (_hasAssisted) return;

    // �ﵽ 25 �δ���
    if (_irritableTriggers >= 25)
    {
        _hasAssisted = true;

        // ��ս���ݣ�����Ƭ�Э������/���е��ˡ���Ҫ����ϵͳ֧�֣�Ŀǰû��ɣ�
        // ������ʵ����Ƭ��ȷ���ķ���/��Ѫ���֣���������δ������/�ӵ�ϵͳ���롣
        //
        // ��������ʱ�޵�/���˺�
        _player->setInvincible(true);

        // ��Ѫ
        _player->heal(30);

        // ����������ټ�һ����ʾ UI/��Ч/��Ч����ѡ��
        CCLOG("[RoommateAssist] Triggered after 25 Irritable hits!");
    }
}