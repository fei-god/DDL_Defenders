#include "DDLTestScene.h"

USING_NS_CC;

Scene* DDLTestScene::createScene()
{
    return DDLTestScene::create();
}

bool DDLTestScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // close button (��ʹ�� lambda�������׼��������)
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(DDLTestScene::menuCloseCallback, this)
    );

    if (closeItem)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + visibleSize.height - closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));

        auto menu = Menu::create(closeItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        addChild(menu, 10);
    }

    // ���� Player������ HelloWorld.png ռλͼ����֤��Դ�ܼ��أ�
    _player = Player::create(
        "Player",
        "HelloWorld.png",
        Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y),
        100,      // maxHp
        200.0f,   // baseSpeed
        0         // defense
    );

    if (!_player)
        return false;

    addChild(_player, 1);

    // HUD
    _moodLabel = Label::createWithTTF("Mood: Normal", "fonts/Marker Felt.ttf", 20);
    if (_moodLabel)
    {
        _moodLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 30));
        addChild(_moodLabel, 2);
    }

    setupKeyboard();
    scheduleUpdate();

    return true;
}

void DDLTestScene::menuCloseCallback(Ref* sender)
{
    (void)sender;
    Director::getInstance()->end();
}

void DDLTestScene::setupKeyboard()
{
    auto dispatcher = Director::getInstance()->getEventDispatcher();

    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(DDLTestScene::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(DDLTestScene::onKeyReleased, this);

    dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void DDLTestScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event)
{
    (void)event;

    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
    case EventKeyboard::KeyCode::KEY_A: _keyLeft = true; break;

    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
    case EventKeyboard::KeyCode::KEY_D: _keyRight = true; break;

    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_W: _keyUp = true; break;

    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_S: _keyDown = true; break;

    case EventKeyboard::KeyCode::KEY_SPACE:
        if (_player)
        {
            _player->takeDamage(15);
            _player->changeMood(MoodType::Irritable, 1.5f);
        }
        break;

    default:
        break;
    }
}

void DDLTestScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event)
{
    (void)event;

    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
    case EventKeyboard::KeyCode::KEY_A: _keyLeft = false; break;

    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
    case EventKeyboard::KeyCode::KEY_D: _keyRight = false; break;

    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_W: _keyUp = false; break;

    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_S: _keyDown = false; break;

    default:
        break;
    }
}

void DDLTestScene::update(float dt)
{
    // ����δʹ�ò��������÷��������� CC_UNUSED_PARAM��
    (void)dt;

    updateDirectionFromKeys();

    if (_player && (_keyLeft || _keyRight || _keyUp || _keyDown))
    {
        _player->changeMood(MoodType::Focus, 0.35f);
    }

    if (_player)
        _player->updatePlayer(dt);

    updateHUD();
}

void DDLTestScene::updateDirectionFromKeys()
{
    if (!_player) return;

    int x = 0;
    int y = 0;

    if (_keyLeft) x -= 1;
    if (_keyRight) x += 1;
    if (_keyUp) y += 1;
    if (_keyDown) y -= 1;

    Vec2 dir((float)x, (float)y);
    _player->setInputDirection(dir);
}

void DDLTestScene::updateHUD()
{
    if (!_player || !_moodLabel) return;
    _moodLabel->setString("Mood: " + _player->getCurrentMoodName());
}