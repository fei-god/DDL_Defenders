#include "MoodEventDemoScene.h"

USING_NS_CC;

Scene* MoodEventDemoScene::createScene()
{
    return MoodEventDemoScene::create();
}

bool MoodEventDemoScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // ��������Դռλ��������н�ɫͼ���滻
    _player = Player::create(
        "Player",
        "HelloWorld.png",
        Vec2(visibleSize.width * 0.5f + origin.x,
            visibleSize.height * 0.5f + origin.y),
        100,       // maxHp
        200.0f,    // baseSpeed
        0          // defense
    );

    if (!_player) return false;

    addChild(_player, 1);

    _moodLabel = Label::createWithTTF("Mood: Normal", "fonts/Marker Felt.ttf", 22);
    _moodLabel->setPosition(Vec2(origin.x + 90, origin.y + visibleSize.height - 30));
    addChild(_moodLabel, 2);

    _hpLabel = Label::createWithTTF("HP: 100/100", "fonts/Marker Felt.ttf", 20);
    _hpLabel->setPosition(Vec2(origin.x + 90, origin.y + visibleSize.height - 55));
    addChild(_hpLabel, 2);

    // ������ player->updatePlayer(dt)������ͼ���ƶ�
    _t = 0.0f;
    _phase = 0;

    scheduleUpdate();
    refreshHUD();
    return true;
}

void MoodEventDemoScene::triggerMoveStart()
{
    if (!_player) return;
    // �ƶ��¼���ֻ�� setInputDirection ���� Focus�����������λ�ã�
    _player->setInputDirection(Vec2(1, 0));
}

void MoodEventDemoScene::triggerMoveStop()
{
    if (!_player) return;
    _player->setInputDirection(Vec2::ZERO);
}

void MoodEventDemoScene::triggerHit(int damage)
{
    if (!_player) return;
    _player->takeDamage(damage);
}

void MoodEventDemoScene::update(float dt)
{
    // �� MoodSystem �ĵ���ʱ��Ч
    if (_player)
        _player->getMoodSystem()->updateMood(dt);

    _t += dt;

    // �ű��¼���ȷ�����м�� > 1.0s�������޵���ֹ�ڶ��� takeDamage��
    // ���裺0.8s Focus��2.0s Irritable��>50%����3.4s Exhausted��<=50%��
    switch (_phase)
    {
    case 0:
        if (_t >= 0.8f) { triggerMoveStart(); _phase = 1; }
        break;
    case 1:
        if (_t >= 1.0f) { triggerMoveStop(); _phase = 2; } // �̵ܶġ��ƶ���ʼ/ֹͣ��
        break;
    case 2:
        if (_t >= 2.0f) { triggerHit(30); _phase = 3; } // 100-30=70 -> Irritable
        break;
    case 3:
        if (_t >= 3.4f) { triggerHit(30); _phase = 4; } // 70-30=40 -> Exhausted
        break;
    default:
        break;
    }

    refreshHUD();
}

void MoodEventDemoScene::refreshHUD()
{
    if (!_player) return;

    if (_moodLabel)
        _moodLabel->setString("Mood: " + _player->getCurrentMoodName());

    if (_hpLabel)
        _hpLabel->setString(StringUtils::format("HP: %d/%d", _player->getHp(), _player->getMaxHp()));
}