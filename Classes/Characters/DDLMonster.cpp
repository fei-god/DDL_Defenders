#include "DDLMonster.h"

USING_NS_CC;

// Create a burst of particles (same helper as Enemy.cpp)
static void createBurstEffect(Node* parent, const Vec2& pos,
    const Color4F& innerColor, const Color4F& outerColor,
    int particleCount, float radius, float duration)
{
    auto burstNode = DrawNode::create();
    burstNode->setPosition(pos);
    burstNode->drawSolidCircle(Vec2::ZERO, radius * 0.3f, 0, 16, innerColor);
    burstNode->drawCircle(Vec2::ZERO, radius * 0.5f, 0, 20, false,
        Color4F(innerColor.r, innerColor.g, innerColor.b, 0.6f));
    for (int i = 0; i < particleCount; i++)
    {
        float angle = (i * 2.0f * M_PI) / particleCount;
        float r = radius * 0.4f + (rand() % 100) / 100.0f * radius * 0.6f;
        Vec2 pt = Vec2(cosf(angle), sinf(angle)) * r;
        float size = 2.0f + (rand() % 100) / 100.0f * 4.0f;
        float alpha = 0.5f + (rand() % 100) / 100.0f * 0.5f;
        burstNode->drawDot(pt, size,
            Color4F(outerColor.r, outerColor.g, outerColor.b, alpha));
    }
    parent->addChild(burstNode, 100);
    auto scaleUp = ScaleTo::create(duration * 0.5f, 2.0f);
    auto fadeOut = FadeOut::create(duration);
    burstNode->runAction(Sequence::create(
        Spawn::create(scaleUp, fadeOut, nullptr),
        RemoveSelf::create(), nullptr));
}

DDLMonster* DDLMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    DDLMonster* monster = new DDLMonster();
    if (monster && monster->initDDLMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool DDLMonster::initDDLMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    int scaledHp = 28 + waveLevel * 5;
    float scaledSpeed = 115.0f + waveLevel * 3.0f;
    int scaledAtk = 10 + waveLevel * 1;
    int scaledExp = 50 + waveLevel * 8;
    bool ok = initEnemy("DDLMonster",
        imagePath,
        startPosition,
        scaledHp,
        scaledSpeed,
        0,
        scaledAtk,
        64.0f,
        scaledExp);
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

    if (_chargeCooldown > 0.0f)
        _chargeCooldown -= dt;

    if (_isCharging)
    {
        float chargeSpeed = getSpeed() * 2.5f;
        Vec2 newPos = getPosition() + _chargeDirection * chargeSpeed * dt;
        setPosition(newPos);

        // Charge trail - fiery orange particles
        if (CCRANDOM_0_1() < 0.5f)
        {
            auto trail = DrawNode::create();
            trail->setPosition(Vec2::ZERO);
            float size = 5.0f + (rand() % 8);
            trail->drawSolidCircle(Vec2::ZERO, size, 0, 8, Color4F(1.0f, 0.4f, 0.05f, 0.7f));
            trail->drawSolidCircle(Vec2::ZERO, size * 0.5f, 0, 6, Color4F(1.0f, 0.7f, 0.2f, 0.9f));
            this->addChild(trail, -1);
            auto fade = FadeOut::create(0.35f);
            auto scaleDown = ScaleTo::create(0.35f, 0.3f);
            trail->runAction(Sequence::create(
                Spawn::create(fade, scaleDown, nullptr),
                RemoveSelf::create(), nullptr));
        }

        if (_chargeCooldown <= 0.0f)
        {
            _isCharging = false;
            _chargeCooldown = 1.5f;
        }
        return;
    }
    else
    {
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());

        // Shoot red spike projectile at player when at medium-long range
        if (_projectileCooldown > 0.0f)
            _projectileCooldown -= dt;
        if (_projectileCooldown <= 0.0f && distToPlayer > 100.0f && distToPlayer < 350.0f)
        {
            fireProjectileAtPlayer(280.0f,
                Color4F(1.0f, 0.2f, 0.05f, 0.75f),   // glow: deep red
                Color4F(1.0f, 0.5f, 0.1f, 0.5f),      // trail: orange
                _attackDamage / 2, 8.0f, 2.5f);        // half damage, small radius
            _projectileCooldown = _projectileCooldownMax;
        }

        if (_chargeCooldown <= 0.0f && distToPlayer < 150.0f)
        {
            _isCharging = true;
            _chargeDirection = dir;
            _chargeCooldown = 0.4f;

            // Charge startup burst
            auto chargeNode = DrawNode::create();
            chargeNode->setPosition(Vec2::ZERO);
            chargeNode->drawSolidCircle(Vec2::ZERO, 20.0f, 0, 12, Color4F(1.0f, 0.5f, 0.0f, 0.6f));
            for (int i = 0; i < 8; i++)
            {
                float a = i * M_PI / 4;
                Vec2 pt = Vec2(cosf(a), sinf(a)) * 18.0f;
                chargeNode->drawDot(pt, 4, Color4F(1.0f, 0.8f, 0.3f, 0.8f));
            }
            this->addChild(chargeNode, 100);
            auto fade = FadeOut::create(0.3f);
            auto scale = ScaleTo::create(0.3f, 2.5f);
            chargeNode->runAction(Sequence::create(
                Spawn::create(fade, scale, nullptr),
                RemoveSelf::create(), nullptr));
        }
    }
}

void DDLMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    _targetPlayer->takeDamage(_attackDamage);
}

void DDLMonster::playAttackEffect()
{
    // DDL charge slash: fiery red/orange burst with directional slash
    createBurstEffect(this, Vec2::ZERO,
        Color4F(1.0f, 0.25f, 0.05f, 0.85f),  // inner: deep red-orange
        Color4F(1.0f, 0.55f, 0.1f, 0.7f),    // outer: bright orange sparks
        16, 40.0f, 0.45f);

    // Directional slash arc
    auto slashNode = DrawNode::create();
    slashNode->setPosition(Vec2::ZERO);
    float baseAngle = atan2f(getDirection().y, getDirection().x);
    for (int i = 0; i < 7; i++)
    {
        float a = baseAngle - 0.5f + i * 0.166f;
        Vec2 pt = Vec2(cosf(a), sinf(a)) * 38.0f;
        slashNode->drawSolidCircle(pt, 5.0f + i, 0, 6, Color4F(1.0f, 0.4f + i * 0.08f, 0.05f, 0.85f));
    }
    this->addChild(slashNode, 100);
    auto fade = FadeOut::create(0.5f);
    auto expand = ScaleTo::create(0.5f, 2.0f);
    slashNode->runAction(Sequence::create(
        Spawn::create(fade, expand, nullptr),
        RemoveSelf::create(), nullptr));

    // Screen shake via position jitter
    auto jitter = Sequence::create(
        MoveBy::create(0.02f, Vec2(3, -3)),
        MoveBy::create(0.02f, Vec2(-6, 6)),
        MoveBy::create(0.02f, Vec2(5, -2)),
        MoveBy::create(0.02f, Vec2(-2, -1)),
        nullptr);
    this->runAction(jitter);

    float curScale = getScale();
    auto pop = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.06f, curScale * 1.3f)),
        EaseBackIn::create(ScaleTo::create(0.18f, curScale)),
        nullptr);
    this->runAction(pop);
}
