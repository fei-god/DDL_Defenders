#include "KeyboardWave.h"
#include <cmath>

USING_NS_CC;

//武器特点：范围宽，单发伤害低，穿透力低，适合清理小怪和近距离战斗。

KeyboardWave* KeyboardWave::create(Player* owner)
{
    KeyboardWave* weapon = new (std::nothrow) KeyboardWave();

    if (weapon && weapon->initKeyboardWave(owner))
    {
        weapon->autorelease();
        return weapon;
    }

    delete weapon;
    return nullptr;
}

bool KeyboardWave::initKeyboardWave(Player* owner)
{
    if (!initWeapon(
        "KeyboardWave",    // 武器名字
        "",                // 武器本体
        owner,             // 武器所属玩家
        18,                // 攻击力
        0.55f              // 冷却时间
    ))
    {
        return false;
    }

    _bulletSpeed = 520.0f;

    // 你需要把图片放到 Resources/weapon/keyboard_wave.png
    // 如果图片不存在，Bullet 里面有兜底逻辑，但建议还是放图片。
    _bulletImagePath = "weapon/keyboard_wave.png";

    return true;
}

void KeyboardWave::fire()
{
    Enemy* target = findNearestEnemy();

    if (target == nullptr)
    {
        return;
    }

    Vec2 baseDir = getDirectionToEnemy(target);
    Vec2 startPos = _owner->getObjectPosition();

    // 三发子弹分别偏转 -12°、0°、12°，形成扇形攻击。
    float angles[3] = { -12.0f, 0.0f, 12.0f };

    for (int i = 0; i < 3; i++)
    {
        Vec2 dir = rotateDirection(baseDir, angles[i]);

        Bullet* bullet = Bullet::createBullet(
            "KeyboardWaveBullet",
            _bulletImagePath,
            startPos,
            dir,
            _bulletSpeed,
            _attackPower,
            1.4f,
            false
        );

        if (bullet != nullptr)
        {
            _bulletLayer->addChild(bullet);
            _bulletList->push_back(bullet);
        }
    }

    // 发射后进入冷却，防止每一帧都疯狂发射。
    resetCooldown();
}

Vec2 KeyboardWave::rotateDirection(const Vec2& dir, float degree)
{
    float rad = CC_DEGREES_TO_RADIANS(degree);

    float cosValue = std::cos(rad);
    float sinValue = std::sin(rad);

    // 二维向量旋转公式：
    // x' = x cosθ - y sinθ
    // y' = x sinθ + y cosθ
    return Vec2(
        dir.x * cosValue - dir.y * sinValue,
        dir.x * sinValue + dir.y * cosValue
    ).getNormalized();
}