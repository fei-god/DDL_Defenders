#include "BossMonster.h"
#include "cocos2d.h"

USING_NS_CC;

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

BossMonster* BossMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    BossMonster* boss = new BossMonster();
    if (boss && boss->initBossMonster(imagePath, startPosition, target, waveLevel))
    {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

bool BossMonster::initBossMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    int baseHp = 85 + waveLevel * 10;
    float baseSpeed = 40.0f + waveLevel * 3.0f;
    int baseDefense = 2 + waveLevel / 2;
    int baseAttack = 18 + waveLevel * 3;
    float baseRange = 70.0f;
    int baseExp = 200 + waveLevel * 50;

    bool ok = initEnemy("BossMonster",
        imagePath,
        startPosition,
        baseHp,
        baseSpeed,
        baseDefense,
        baseAttack,
        baseRange,
        baseExp);
    if (!ok) return false;

    setTargetPlayer(target);

    _chargeTimer = 0.0f;
    _chargeInterval = 3.0f;
    _isCharging = false;
    _chargeDir = Vec2::ZERO;
    _chargeDuration = 0.0f;

    _specialAttackTimer = 0.0f;
    _specialAttackInterval = 5.0f;

    _isEnraged = false;
    _waveLevel = waveLevel;

    _wanderTimer = 1.0f;
    _wanderTarget = Vec2::ZERO;
    _isWandering = true;

    setScale(1.3f);

    CCLOG("BossMonster created! HP:%d Speed:%.1f Atk:%d (Wave %d)",
        getMaxHp(), getSpeed(), getAttackDamage(), waveLevel);

    return true;
}

void BossMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    if (!_isEnraged && getHp() <= getMaxHp() / 2)
    {
        _isEnraged = true;
        setSpeed(getSpeed() * 1.5f);
        _chargeInterval = 2.0f;
        _specialAttackInterval = 3.0f;
        showEnrageAura();
        CCLOG("BossMonster ENRAGED! Speed:%.1f", getSpeed());
    }

    // Shoot fireball volleys periodically
    if (_projectileCooldown > 0.0f)
        _projectileCooldown -= dt;
    if (_projectileCooldown <= 0.0f)
    {
        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
        if (distToPlayer > 80.0f)
        {
            // Boss shoots a spread volley of 3-5 fireballs
            int volleyCount = _isEnraged ? 5 : 3;
            Vec2 baseDir = _targetPlayer->getPosition() - getPosition();
            baseDir.normalize();
            for (int i = 0; i < volleyCount; i++)
            {
                float spreadAngle = (i - (volleyCount - 1) * 0.5f) * 0.25f;
                Vec2 dir = Vec2(
                    baseDir.x * cosf(spreadAngle) - baseDir.y * sinf(spreadAngle),
                    baseDir.x * sinf(spreadAngle) + baseDir.y * cosf(spreadAngle)
                );
                // Stagger the shots slightly
                auto delayedFire = CallFunc::create([this, dir]() {
                    fireProjectile(dir, 260.0f,
                        Color4F(1.0f, 0.15f, 0.03f, 0.8f),   // glow: crimson
                        Color4F(1.0f, 0.4f, 0.1f, 0.55f),     // trail: fire orange
                        _attackDamage / 2, 14.0f, 3.0f);       // big projectiles
                });
                this->runAction(Sequence::create(
                    DelayTime::create(i * 0.1f),
                    delayedFire,
                    nullptr));
            }
            _projectileCooldown = _isEnraged ? 1.5f : 2.5f;
        }
    }

    if (_chargeTimer > 0.0f)
        _chargeTimer -= dt;
    if (_specialAttackTimer > 0.0f)
        _specialAttackTimer -= dt;

    if (_isCharging)
    {
        _chargeDuration -= dt;
        float chargeSpeed = getSpeed() * 3.0f;
        Vec2 newPos = getPosition() + _chargeDir * chargeSpeed * dt;
        setPosition(newPos);

        // Large charge trail
        if (CCRANDOM_0_1() < 0.6f)
        {
            auto trail = DrawNode::create();
            trail->setPosition(Vec2::ZERO);
            float r = 8.0f + (rand() % 10);
            trail->drawSolidCircle(Vec2::ZERO, r, 0, 10, Color4F(0.9f, 0.2f, 0.05f, 0.6f));
            trail->drawSolidCircle(Vec2::ZERO, r * 0.5f, 0, 6, Color4F(1.0f, 0.5f, 0.1f, 0.8f));
            this->addChild(trail, -1);
            auto fade = FadeOut::create(0.4f);
            auto shrink = ScaleTo::create(0.4f, 0.2f);
            trail->runAction(Sequence::create(
                Spawn::create(fade, shrink, nullptr),
                RemoveSelf::create(), nullptr));
        }

        if (_chargeDuration <= 0.0f)
        {
            _isCharging = false;
            _isWandering = true;
            _wanderTimer = 1.0f;
        }
        return;
    }

    float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
    if (_specialAttackTimer <= 0.0f && distToPlayer <= _attackRange * 1.5f)
    {
        showAOEIndicator();
        performSpecialAttack();
        _specialAttackTimer = _specialAttackInterval;
        return;
    }

    if (_chargeTimer <= 0.0f && distToPlayer > 60.0f && distToPlayer < 300.0f)
    {
        startCharge();
        return;
    }

    if (_isWandering)
    {
        _wanderTimer -= dt;
        if (_wanderTimer <= 0.0f || _wanderTarget == Vec2::ZERO)
        {
            float offsetX = (CCRANDOM_0_1() - 0.5f) * 200.0f;
            float offsetY = (CCRANDOM_0_1() - 0.5f) * 200.0f;
            _wanderTarget = _targetPlayer->getPosition() + Vec2(offsetX, offsetY);
            _wanderTimer = 2.0f + CCRANDOM_0_1() * 2.0f;
        }

        Vec2 dir = _wanderTarget - getPosition();
        float wanderDist = dir.length();
        if (wanderDist < 20.0f)
        {
            _isWandering = false;
        }
        else
        {
            dir.normalize();
            setDirection(dir);
            Vec2 newPos = getPosition() + dir * getSpeed() * dt;
            setPosition(newPos);
        }
    }
    else
    {
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        _wanderTimer -= dt;
        if (_wanderTimer <= 0.0f)
        {
            _isWandering = true;
            _wanderTimer = 0.0f;
        }
    }
}

void BossMonster::startCharge()
{
    _isCharging = true;
    _isWandering = false;
    _chargeDir = _targetPlayer->getPosition() - getPosition();
    _chargeDir.normalize();
    _chargeDuration = 0.6f;
    _chargeTimer = _chargeInterval;

    // Charge wind-up: expanding ring + ground crackle
    auto windUpRing = DrawNode::create();
    windUpRing->setPosition(Vec2::ZERO);
    windUpRing->drawCircle(Vec2::ZERO, 15.0f, 0, 24, false, Color4F(0.9f, 0.3f, 0.05f, 0.5f));
    this->addChild(windUpRing, 50);
    auto expand = ScaleTo::create(0.4f, 3.0f);
    auto fade = FadeOut::create(0.4f);
    windUpRing->runAction(Sequence::create(
        Spawn::create(expand, fade, nullptr),
        RemoveSelf::create(), nullptr));

    float curScale = getScale();
    auto windUp = Sequence::create(
        EaseBackIn::create(ScaleTo::create(0.15f, curScale * 0.85f)),
        EaseBackOut::create(ScaleTo::create(0.25f, curScale * 1.15f)),
        nullptr);
    this->runAction(windUp);

    CCLOG("BossMonster starts charging!");
}

void BossMonster::showAOEIndicator()
{
    // Large expanding red danger zone
    float aoeRadius = _attackRange * 1.5f;

    // Outer warning ring
    auto outerRing = DrawNode::create();
    outerRing->setPosition(Vec2::ZERO);
    outerRing->drawCircle(Vec2::ZERO, aoeRadius, 0, 32, false, Color4F(1.0f, 0.1f, 0.05f, 0.4f));
    this->addChild(outerRing, 50);

    // Inner danger circle
    auto innerCircle = DrawNode::create();
    innerCircle->setPosition(Vec2::ZERO);
    innerCircle->drawSolidCircle(Vec2::ZERO, aoeRadius * 0.4f, 0, 20, Color4F(1.0f, 0.2f, 0.1f, 0.15f));
    this->addChild(innerCircle, 49);

    // Both pulse and fade
    auto pulseOuter = Sequence::create(
        Spawn::create(ScaleTo::create(0.5f, 2.0f), FadeOut::create(0.5f), nullptr),
        RemoveSelf::create(), nullptr);
    auto pulseInner = Sequence::create(DelayTime::create(0.1f),
        Spawn::create(ScaleTo::create(0.4f, 2.5f), FadeOut::create(0.4f), nullptr),
        RemoveSelf::create(), nullptr);
    outerRing->runAction(pulseOuter);
    innerCircle->runAction(pulseInner);

    // Ground crackle particles inside AOE
    for (int i = 0; i < 15; i++)
    {
        auto crack = DrawNode::create();
        float angle = (rand() % 360) * M_PI / 180.0f;
        float dist = (rand() % 80) / 100.0f * aoeRadius;
        crack->setPosition(Vec2(cosf(angle) * dist, sinf(angle) * dist));
        crack->drawDot(Vec2::ZERO, 2.0f + (rand() % 4),
            Color4F(1.0f, 0.3f, 0.1f, 0.6f));
        this->addChild(crack, 51);
        auto fadeCrack = Sequence::create(
            DelayTime::create((rand() % 30) / 100.0f),
            FadeOut::create(0.3f),
            RemoveSelf::create(), nullptr);
        crack->runAction(fadeCrack);
    }
}

void BossMonster::showEnrageAura()
{
    // Remove old aura if exists
    this->removeChildByTag(777);

    // Pulsing dark-red aura
    auto auraContainer = DrawNode::create();
    auraContainer->setPosition(Vec2::ZERO);
    auraContainer->setTag(777);

    // Outer misty ring
    auraContainer->drawSolidCircle(Vec2::ZERO, 55.0f, 0, 28, Color4F(0.8f, 0.08f, 0.05f, 0.15f));
    // Middle ring
    auraContainer->drawCircle(Vec2::ZERO, 45.0f, 0, 24, false, Color4F(1.0f, 0.15f, 0.08f, 0.3f));
    // Inner close ring
    auraContainer->drawCircle(Vec2::ZERO, 30.0f, 0, 18, false, Color4F(1.0f, 0.25f, 0.12f, 0.25f));

    this->addChild(auraContainer, -5);

    // Pulsing animation
    auto scaleUp = EaseSineInOut::create(ScaleTo::create(0.7f, 1.2f));
    auto scaleDown = EaseSineInOut::create(ScaleTo::create(0.7f, 0.85f));
    auto pulse = RepeatForever::create(Sequence::create(scaleUp, scaleDown, nullptr));
    auraContainer->runAction(pulse);

    // Enrage burst flash
    auto flashNode = DrawNode::create();
    flashNode->setPosition(Vec2::ZERO);
    flashNode->drawSolidCircle(Vec2::ZERO, 60.0f, 0, 20, Color4F(1.0f, 0.2f, 0.05f, 0.4f));
    this->addChild(flashNode, 150);
    auto flashFade = Sequence::create(
        Spawn::create(ScaleTo::create(0.5f, 2.5f), FadeOut::create(0.5f), nullptr),
        RemoveSelf::create(), nullptr);
    flashNode->runAction(flashFade);
}

void BossMonster::performSpecialAttack()
{
    CCLOG("BossMonster performs special AOE attack!");

    _attackCooldown = _attackCooldownMax;

    int originalDamage = getAttackDamage();
    setAttackDamage(originalDamage * 2);
    attack();
    setAttackDamage(originalDamage);
}

void BossMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    _targetPlayer->takeDamage(_attackDamage);
}

void BossMonster::playAttackEffect()
{
    // Boss attack: massive crimson shockwave with debris
    createBurstEffect(this, Vec2::ZERO,
        Color4F(1.0f, 0.12f, 0.05f, 0.9f),   // inner: blood red
        Color4F(1.0f, 0.4f, 0.15f, 0.7f),     // outer: fire orange
        22, 55.0f, 0.5f);

    // Three expanding shockwave rings
    for (int ring = 0; ring < 3; ring++)
    {
        auto waveRing = DrawNode::create();
        waveRing->setPosition(Vec2::ZERO);
        waveRing->drawCircle(Vec2::ZERO, 25.0f, 0, 24, false,
            Color4F(1.0f, 0.3f, 0.1f, 0.5f - ring * 0.12f));
        this->addChild(waveRing, 90);

        auto delay = DelayTime::create(ring * 0.06f);
        auto expand = ScaleTo::create(0.4f, 3.0f + ring * 0.5f);
        auto fade = FadeOut::create(0.4f);
        waveRing->runAction(Sequence::create(delay,
            Spawn::create(expand, fade, nullptr),
            RemoveSelf::create(), nullptr));
    }

    // Radial spike particles
    for (int i = 0; i < 12; i++)
    {
        auto spike = DrawNode::create();
        float angle = i * M_PI / 6;
        spike->setPosition(Vec2::ZERO);
        spike->drawSolidCircle(Vec2(cosf(angle) * 35.0f, sinf(angle) * 35.0f),
            5.0f, 0, 6, Color4F(1.0f, 0.5f, 0.1f, 0.8f));
        this->addChild(spike, 100);
        auto flyOut = MoveBy::create(0.35f, Vec2(cosf(angle) * 40.0f, sinf(angle) * 40.0f));
        auto fadeOut = FadeOut::create(0.35f);
        spike->runAction(Sequence::create(
            Spawn::create(flyOut, fadeOut, nullptr),
            RemoveSelf::create(), nullptr));
    }

    // Heavy slam shake
    float curScale = getScale();
    auto slam = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.05f, curScale * 1.35f)),
        ScaleTo::create(0.04f, curScale * 0.8f),
        ScaleTo::create(0.05f, curScale * 1.2f),
        EaseBackIn::create(ScaleTo::create(0.2f, curScale)),
        nullptr);
    this->runAction(slam);
}

void BossMonster::die()
{
    if (!isRoleAlive()) return;

    CCLOG("Boss defeated! Player gained %d exp!", _expReward);

    setCollisionEnabled(false);

    // Death explosion - massive multi-layer burst
    createBurstEffect(this, Vec2::ZERO,
        Color4F(1.0f, 0.5f, 0.1f, 0.9f),
        Color4F(1.0f, 0.8f, 0.3f, 0.7f),
        25, 70.0f, 0.6f);

    // Delayed second burst
    auto delayedBurst = CallFunc::create([this]() {
        createBurstEffect(this, Vec2::ZERO,
            Color4F(1.0f, 0.6f, 0.2f, 0.7f),
            Color4F(0.9f, 0.7f, 0.1f, 0.5f),
            18, 50.0f, 0.5f);
    });

    // Death shrink
    auto delay = DelayTime::create(0.12f);
    auto fadeOut = ScaleTo::create(0.35f, 0.05f);
    auto dieCallback = CallFunc::create([this]() {
        Enemy::die();
    });
    auto removeBoss = RemoveSelf::create();
    this->runAction(Sequence::create(delay, delayedBurst, fadeOut, dieCallback, removeBoss, nullptr));
}

void BossMonster::takeDamage(int damage)
{
    takeDamage(damage, DamageType::Normal, nullptr);
}

void BossMonster::takeDamage(int damage, DamageType damageType, Role* attacker)
{
    // Bosses use real HP-based damage instead of the hit-count system
    if (!isAlive || !isObjectActive() || damage <= 0 || !canTakeDamage())
        return;

    setHp(getHp() - damage);

    if (getHp() <= 0)
    {
        setHp(0);
        die();
        return;
    }

    hurtCooldownTimer = hurtCooldown;
    playHitEffect();
}
