#include "PhoneMonster.h"

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

PhoneMonster* PhoneMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    PhoneMonster* monster = new (std::nothrow) PhoneMonster();
    if (monster && monster->initPhoneMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool PhoneMonster::initPhoneMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    int scaledHp = 20 + waveLevel * 3;
    float scaledSpeed = 145.0f + waveLevel * 2.0f;
    int scaledAtk = 6 + waveLevel * 1;
    int scaledExp = 35 + waveLevel * 5;
    bool ok = initEnemy("PhoneMonster", imagePath, startPosition,
        scaledHp, scaledSpeed, 0, scaledAtk, 46.0f, scaledExp);
    if (!ok) return false;

    setTargetPlayer(target);
    _attackCooldownMax = 0.65f;
    _zigzagTimer = 0.0f;
    _zigzagSign = 1.0f;
    return true;
}

void PhoneMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    _zigzagTimer -= dt;
    if (_zigzagTimer <= 0.0f)
    {
        _zigzagTimer = 0.35f;
        _zigzagSign = -_zigzagSign;
    }

    Vec2 dir = _targetPlayer->getPosition() - getPosition();
    if (dir.lengthSquared() < 0.0001f) return;
    dir.normalize();

    Vec2 side(-dir.y, dir.x);
    Vec2 blended = (dir + side * 0.35f * _zigzagSign).getNormalized();
    setDirection(blended);
    setPosition(getPosition() + blended * getSpeed() * dt);

    // Shoot fast purple signal bolts frequently
    if (_projectileCooldown > 0.0f)
        _projectileCooldown -= dt;
    if (_projectileCooldown <= 0.0f)
    {
        // PhoneMonster shoots rapid bolts
        _projectileCooldownMax = 0.8f + CCRANDOM_0_1() * 0.4f; // Fast fire rate
        fireProjectileAtPlayer(350.0f,  // fast!
            Color4F(0.55f, 0.2f, 0.95f, 0.75f),  // glow: electric purple
            Color4F(0.7f, 0.45f, 1.0f, 0.5f),    // trail: violet
            _attackDamage / 3, 7.0f, 2.0f);
        _projectileCooldown = _projectileCooldownMax;
    }

    // Zigzag trail - electric purple dots
    if (CCRANDOM_0_1() < 0.25f)
    {
        auto trail = DrawNode::create();
        trail->setPosition(Vec2::ZERO);
        trail->drawSolidCircle(Vec2::ZERO, 5.0f, 0, 6,
            Color4F(0.6f, 0.2f, 0.95f, 0.55f));
        trail->drawSolidCircle(Vec2::ZERO, 3.0f, 0, 4,
            Color4F(0.8f, 0.5f, 1.0f, 0.7f));
        this->addChild(trail, -1);
        auto fade = FadeOut::create(0.35f);
        auto shrink = ScaleTo::create(0.35f, 0.2f);
        trail->runAction(Sequence::create(
            Spawn::create(fade, shrink, nullptr),
            RemoveSelf::create(), nullptr));
    }
}

void PhoneMonster::attack()
{
    if (_targetPlayer == nullptr) return;

    _targetPlayer->takeDamage(_attackDamage);
    _targetPlayer->changeMood(MoodType::Irritable, 2.0f);
}

void PhoneMonster::playAttackEffect()
{
    // Phone ring wave: purple/magenta electric burst with signal rings
    createBurstEffect(this, Vec2::ZERO,
        Color4F(0.45f, 0.15f, 0.9f, 0.8f),   // inner: deep purple
        Color4F(0.7f, 0.4f, 1.0f, 0.65f),     // outer: bright violet sparks
        12, 32.0f, 0.4f);

    // Concentric signal rings expanding outward
    for (int ring = 0; ring < 3; ring++)
    {
        auto ringNode = DrawNode::create();
        ringNode->setPosition(Vec2::ZERO);
        ringNode->drawCircle(Vec2::ZERO, 14.0f, 0, 20, false,
            Color4F(0.6f, 0.3f, 1.0f, 0.6f - ring * 0.15f));
        this->addChild(ringNode, 95);

        auto delay = DelayTime::create(ring * 0.06f);
        auto expand = ScaleTo::create(0.4f, 2.5f + ring * 0.5f);
        auto fade = FadeOut::create(0.4f);
        ringNode->runAction(Sequence::create(delay,
            Spawn::create(expand, fade, nullptr),
            RemoveSelf::create(), nullptr));
    }

    // Quick electric jitter
    float curScale = getScale();
    auto zap = Sequence::create(
        ScaleTo::create(0.03f, curScale * 1.08f, curScale * 0.92f),
        ScaleTo::create(0.03f, curScale * 0.94f, curScale * 1.06f),
        ScaleTo::create(0.03f, curScale * 1.05f, curScale * 0.95f),
        ScaleTo::create(0.15f, curScale),
        nullptr);
    this->runAction(zap);
}
