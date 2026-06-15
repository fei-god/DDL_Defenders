#include "DDLMonster.h"

USING_NS_CC;

DDLMonster* DDLMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    DDLMonster* monster = new DDLMonster();
    if (monster && monster->initDDLMonster(imagePath, startPosition, target))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool DDLMonster::initDDLMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    // 鍐查攱鍨嬫€墿锛氫腑绛夎閲忥紝杈冨揩閫熷害锛屼腑绛夋敾鍑?
    bool ok = initEnemy("DDLMonster",
        imagePath,
        startPosition,
        24,        // maxHp
        115.0f,    // speed
        0,         // defense
        12,        // attackDamage
        64.0f,     // attackRange
        50);       // expReward    (闄嶄綆锛?0鈫?0)
    if (!ok) return false;

    setTargetPlayer(target);
    _chargeCooldown = 0.0f;
    _isCharging = false;
    _chargeDirection = Vec2::ZERO;
    return true;
}

void DDLMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    // 鍐峰嵈璁℃椂
    if (_chargeCooldown > 0.0f)
        _chargeCooldown -= dt;

    if (_isCharging)
    {
        // 鍐查攱鐘舵€侊細娌垮啿閿嬫柟鍚戝揩閫熺Щ鍔?
        float chargeSpeed = getSpeed() * 2.5f;
        Vec2 newPos = getPosition() + _chargeDirection * chargeSpeed * dt;
        setPosition(newPos);

        // 鍐查攱鎸佺画 0.4 绉?
        if (_chargeCooldown <= 0.0f)
        {
            _isCharging = false;
            _chargeCooldown = 1.5f;  // 鍐查攱缁撴潫鍚庡喎鍗?1.5 绉?
        }
        return;
    }
    else
    {
        // 鏅€氱姸鎬侊細鍚戠帺瀹剁Щ鍔?
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        // 鍐峰嵈缁撴潫涓旂帺瀹惰緝杩戞椂锛屽皾璇曞彂璧峰啿閿?
        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
        if (_chargeCooldown <= 0.0f && distToPlayer < 150.0f)
        {
            _isCharging = true;
            _chargeDirection = dir;
            _chargeCooldown = 0.4f;  // 鍐查攱鎸佺画 0.4 绉?
        }
    }
}

void DDLMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    // 鍐查攱鎬墿杩戞垬鏀诲嚮
    _targetPlayer->takeDamage(_attackDamage);
}


