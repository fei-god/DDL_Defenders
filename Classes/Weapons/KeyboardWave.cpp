#include "KeyboardWave.h"
#include <algorithm>
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
        "weapon/keyboard_weapon_sprite.png", // 武器本体
        owner,             // 武器所属玩家
        26,                // 攻击力
        0.42f              // 冷却时间
    ))
    {
        return false;
    }

    _bulletSpeed = 520.0f;
    configureEnergy(100.0f, 18.0f, 24.0f);
    setObjectScale(0.095f);

    // 你需要把图片放到 Resources/weapon/keyboard_wave.png
    // 如果图片不存在，Bullet 里面有兜底逻辑，但建议还是放图片。
    _bulletImagePath = "weapon/keyboard_wave_sprite.png";

    return true;
}

void KeyboardWave::fire()
{
    if (!canFire()) return;
    if (!consumeEnergyForShot()) return;

    Vec2 baseDir = getAimDirection();
    if (baseDir.lengthSquared() < 0.0001f)
    {
        Enemy* target = findNearestEnemy();
        baseDir = target ? getDirectionToEnemy(target) : Vec2(1, 0);
    }
    int projectileCount = 3 + getProjectileCountBonus();
    float startAngle = -12.0f - getProjectileCountBonus() * 4.0f;
    float endAngle = 12.0f + getProjectileCountBonus() * 4.0f;

    for (int i = 0; i < projectileCount; i++)
    {
        float t = projectileCount > 1
            ? static_cast<float>(i) / static_cast<float>(projectileCount - 1)
            : 0.5f;
        Vec2 dir = rotateDirection(baseDir, startAngle + (endAngle - startAngle) * t);

        Bullet* bullet = spawnBullet(
            "KeyboardWaveBullet",
            _bulletImagePath,
            getMuzzlePosition(dir),
            dir,
            _bulletSpeed,
            getModifiedAttackPower(),
            1.4f,
            false
        );

        if (bullet)
        {
            auto size = bullet->getContentSize();
            if (size.width > 0.0f && size.height > 0.0f)
            {
                bullet->setScale(std::min(28.0f / size.width, 28.0f / size.height));
            }
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
