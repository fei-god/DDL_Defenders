#include "SleepyMonster.h"
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

SleepyMonster* SleepyMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    SleepyMonster* monster = new SleepyMonster();
    if (monster && monster->initSleepyMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool SleepyMonster::initSleepyMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    int scaledHp = 20 + waveLevel * 4;
    float scaledSpeed = 82.0f + waveLevel * 2.5f;
    int scaledAtk = 7 + waveLevel * 1;
    int scaledExp = 20 + waveLevel * 5;
    bool ok = initEnemy("SleepyMonster",
        imagePath,
        startPosition,
        scaledHp,
        scaledSpeed,
        0,
        scaledAtk,
        35.0f,
        scaledExp);
    if (!ok) return false;

    setTargetPlayer(target);
    _pauseTimer = 0.7f;
    _isPausing = false;
    changeRandomDirection();
    return true;
}

void SleepyMonster::changeRandomDirection()
{
    float angle = CCRANDOM_0_1() * 2 * M_PI;
    _randomDirection = Vec2(cos(angle), sin(angle));
    _randomDirection.normalize();
}

void SleepyMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    if (_isPausing)
    {
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            _isPausing = false;
            changeRandomDirection();
        }
        else
        {
            Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
            if (dirToPlayer.lengthSquared() > 0.0001f)
            {
                dirToPlayer.normalize();
                setDirection(dirToPlayer);
                setPosition(getPosition() + dirToPlayer * getSpeed() * 0.35f * dt);
            }
            return;
        }
    }
    else
    {
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            _isPausing = true;
            _pauseTimer = 0.12f + CCRANDOM_0_1() * 0.18f;
            return;
        }

        Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
        dirToPlayer.normalize();
        Vec2 blendedDir = (_randomDirection * 0.18f + dirToPlayer * 0.82f);
        blendedDir.normalize();
        setDirection(blendedDir);

        Vec2 newPos = getPosition() + blendedDir * getSpeed() * dt;
        setPosition(newPos);
    }

    // Shoot slow green sleep bubble occasionally
    if (_projectileCooldown > 0.0f)
        _projectileCooldown -= dt;
    if (_projectileCooldown <= 0.0f)
    {
        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
        if (distToPlayer > 60.0f && distToPlayer < 300.0f)
        {
            fireProjectileAtPlayer(140.0f,  // slow!
                Color4F(0.2f, 0.7f, 0.4f, 0.7f),    // glow: sleepy green
                Color4F(0.4f, 0.85f, 0.6f, 0.45f),  // trail: soft teal
                _attackDamage / 2, 10.0f, 3.5f);
            _projectileCooldown = _projectileCooldownMax + CCRANDOM_0_1() * 1.0f;
        }
    }

    if (CCRANDOM_0_1() < 0.01f)
    {
        changeRandomDirection();
    }
}

void SleepyMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    _targetPlayer->takeDamage(_attackDamage);
}

void SleepyMonster::playAttackEffect()
{
    // Sleepy burst: dreamy green-teal wave with "ZZZ" particles
    createBurstEffect(this, Vec2::ZERO,
        Color4F(0.2f, 0.65f, 0.35f, 0.8f),   // inner: forest green
        Color4F(0.4f, 0.85f, 0.55f, 0.65f),  // outer: soft teal sparks
        12, 30.0f, 0.4f);

    // ZZZ floating text effect (three expanding rings)
    for (int ring = 0; ring < 3; ring++)
    {
        auto zNode = DrawNode::create();
        zNode->setPosition(Vec2(0, 10 + ring * 6));
        zNode->drawSolidCircle(Vec2::ZERO, 6.0f - ring * 1.5f, 0, 8,
            Color4F(0.5f, 0.9f, 0.7f, 0.5f - ring * 0.12f));
        this->addChild(zNode, 100);

        auto delay = DelayTime::create(ring * 0.08f);
        auto fadeAndFloat = Spawn::create(
            FadeOut::create(0.5f),
            MoveBy::create(0.5f, Vec2(0, 20)),
            ScaleTo::create(0.5f, 2.0f),
            nullptr);
        zNode->runAction(Sequence::create(delay, fadeAndFloat, RemoveSelf::create(), nullptr));
    }

    float curScale = getScale();
    auto pulse = Sequence::create(
        EaseSineInOut::create(ScaleTo::create(0.12f, curScale * 1.1f)),
        EaseSineInOut::create(ScaleTo::create(0.25f, curScale)),
        nullptr);
    this->runAction(pulse);
}
