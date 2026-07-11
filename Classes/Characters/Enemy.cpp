#include "Enemy.h"

USING_NS_CC;

Enemy::Enemy()
    : _targetPlayer(nullptr)
    , _attackDamage(10)
    , _attackRange(50.0f)
    , _expReward(50)
    , _hitCount(0)
    , _hitsToDie(3)
    , _attackCooldown(0.0f)
    , _attackCooldownMax(1.0f)
    , _idleAction(nullptr)
    , _idleBaseScale(1.0f)
    , _isAttacking(false)
    , _telegraphTimer(0.0f)
    , _projectileCooldown(0.0f)
    , _projectileCooldownMax(2.5f)
{
}

Enemy::~Enemy()
{
    stopIdleAnimation();
}

bool Enemy::initEnemy(const std::string& name,
    const std::string& imagePath,
    const cocos2d::Vec2& startPosition,
    int maxHp,
    float speed,
    int defense,
    int attackDamage,
    float attackRange,
    int expReward)
{
    if (!Role::initRole(name, GameObjectType::Enemy, imagePath, startPosition, maxHp, speed, defense))
        return false;

    _attackDamage = attackDamage;
    _attackRange = attackRange;
    _expReward = expReward;
    _hitCount = 0;
    _hitsToDie = 3;
    _attackCooldown = 0.0f;
    hurtCooldown = 0.08f;
    hurtCooldownTimer = 0.0f;
    _idleAction = nullptr;
    _idleBaseScale = 1.0f;
    _isAttacking = false;
    _telegraphTimer = 0.0f;
    return true;
}

void Enemy::setTargetPlayer(Player* player)
{
    _targetPlayer = player;
}

Player* Enemy::getTargetPlayer() const
{
    return _targetPlayer;
}

void Enemy::setAttackDamage(int damage)
{
    _attackDamage = damage;
}

int Enemy::getAttackDamage() const
{
    return _attackDamage;
}

void Enemy::setAttackRange(float range)
{
    _attackRange = range;
}

float Enemy::getAttackRange() const
{
    return _attackRange;
}

void Enemy::setExpReward(int exp)
{
    _expReward = exp;
}

int Enemy::getExpReward() const
{
    return _expReward;
}

int Enemy::getHitsToDieForPlayer(Player* player) const
{
    if (player == nullptr)
    {
        return 3;
    }

    switch (player->getCurrentMood())
    {
    case MoodType::Excited:
        return 2;
    case MoodType::Exhausted:
    case MoodType::Fear:
    case MoodType::Panic:
        return 4;
    case MoodType::Normal:
    case MoodType::Focus:
    case MoodType::Calm:
    case MoodType::Irritable:
    default:
        return 3;
    }
}

cocos2d::Rect Enemy::getCollisionBox() const
{
    return GameObject::getCollisionBox();
}

void Enemy::takeDamage(int damage)
{
    takeDamage(damage, DamageType::Normal, nullptr);
}

void Enemy::takeDamage(int damage, DamageType damageType, Role* attacker)
{
    if (!isAlive || !isObjectActive() || damage <= 0 || !canTakeDamage())
    {
        return;
    }

    Player* moodPlayer = dynamic_cast<Player*>(attacker);
    if (moodPlayer == nullptr)
    {
        moodPlayer = _targetPlayer;
    }

    _hitsToDie = getHitsToDieForPlayer(moodPlayer);
    ++_hitCount;
    int hitsLeft = _hitsToDie - _hitCount;
    if (hitsLeft <= 0)
    {
        hp = 0;
        die();
        return;
    }

    int newHp = (maxHp * hitsLeft + _hitsToDie - 1) / _hitsToDie;
    if (newHp < 1) newHp = 1;
    setHp(newHp);
    hurtCooldownTimer = hurtCooldown;

    // Play hit effect
    playHitEffect();
}

void Enemy::applyKnockback(const cocos2d::Vec2& bulletDir)
{
    // Don't stack knockbacks — wait for previous one to finish
    if (_isKnockedBack) return;

    float spd = getSpeed();
    _isKnockedBack = true;
    _knockbackDir = bulletDir;
    _knockbackVelocity = bulletDir * spd * 4.0f;     // 4× max speed in bullet dir
    _knockbackAccelMag = spd * 8.0f;                 // enough to cancel 4× speed in 0.5s
}

void Enemy::die()
{
    if (!isRoleAlive()) return;

    stopIdleAnimation();

    if (_targetPlayer != nullptr && _targetPlayer->isRoleAlive())
    {
        _targetPlayer->addExp(_expReward);
    }

    Role::die();
}

void Enemy::updateEnemy(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
        return;

    // --- Knockback processing ---
    if (_isKnockedBack)
    {
        // Apply acceleration toward player to counter the knockback
        if (_targetPlayer && _targetPlayer->isRoleAlive())
        {
            cocos2d::Vec2 toPlayer = _targetPlayer->getPosition() - getPosition();
            toPlayer.normalize();
            _knockbackVelocity += toPlayer * _knockbackAccelMag * dt;
        }

        // Apply knockback velocity
        setPosition(getPosition() + _knockbackVelocity * dt);

        // Knockback ends when velocity reverses relative to original direction
        if (_knockbackVelocity.dot(_knockbackDir) <= 0.0f)
        {
            _isKnockedBack = false;
            _knockbackVelocity = cocos2d::Vec2::ZERO;
        }

        return; // skip normal movement while in knockback
    }

    if (hurtCooldownTimer > 0.0f)
    {
        hurtCooldownTimer -= dt;
        if (hurtCooldownTimer < 0.0f)
        {
            hurtCooldownTimer = 0.0f;
        }
    }

    // Update attack telegraph timer
    if (_isAttacking)
    {
        _telegraphTimer -= dt;
        if (_telegraphTimer <= 0.0f)
        {
            _isAttacking = false;
            // Execute the actual attack after telegraph
            attack();
            playAttackEffect();
        }
        return; // Don't move during telegraph
    }

    // Update attack cooldown
    if (_attackCooldown > 0.0f)
        _attackCooldown -= dt;

    // Execute movement
    move(dt);

    // Check attack range
    if (_targetPlayer != nullptr && _targetPlayer->isRoleAlive() && !_isAttacking)
    {
        float dist = getPosition().distance(_targetPlayer->getPosition());
        if (dist <= _attackRange && _attackCooldown <= 0.0f)
        {
            // Start attack telegraph instead of instant attack
            playAttackTelegraph();
            _attackCooldown = _attackCooldownMax;
        }
    }
}

// ==================== Idle Animation (Brotato-style) ====================

void Enemy::startIdleAnimation()
{
    if (_idleAction != nullptr) return;

    _idleBaseScale = getScale();

    // Brotato-style organic bobbing:
    // - Smooth scale breathing with EaseSineInOut
    // - Gentle rotation wobble
    // - Vertical float
    // - Staggered timing for natural feel

    auto scaleUp = EaseSineInOut::create(ScaleTo::create(0.55f, _idleBaseScale * 1.05f));
    auto scaleDown = EaseSineInOut::create(ScaleTo::create(0.55f, _idleBaseScale * 0.95f));
    auto scaleSeq = Sequence::create(scaleUp, scaleDown, nullptr);

    auto rotRight = EaseSineInOut::create(RotateTo::create(0.7f, 3.5f));
    auto rotLeft = EaseSineInOut::create(RotateTo::create(0.7f, -3.5f));
    auto rotSeq = Sequence::create(rotRight, rotLeft, nullptr);

    auto moveUp = EaseSineInOut::create(MoveBy::create(0.45f, Vec2(0, 4)));
    auto moveDown = EaseSineInOut::create(MoveBy::create(0.45f, Vec2(0, -4)));
    auto moveSeq = Sequence::create(moveUp, moveDown, nullptr);

    auto spawn = Spawn::create(scaleSeq, rotSeq, moveSeq, nullptr);
    auto repeat = RepeatForever::create(spawn);
    repeat->setTag(999);
    this->runAction(repeat);
    _idleAction = repeat;
}

void Enemy::stopIdleAnimation()
{
    if (_idleAction != nullptr)
    {
        this->stopAction(_idleAction);
        _idleAction = nullptr;
    }
    // Restore base state
    if (_idleBaseScale > 0.0f)
    {
        setScale(_idleBaseScale);
    }
    setRotation(0.0f);
}

// ==================== Attack Telegraph ====================

void Enemy::playAttackTelegraph()
{
    _isAttacking = true;
    _telegraphTimer = 0.28f; // 280ms warning before attack hits

    // Expanding warning circle at enemy's position (ground indicator)
    auto warningRing = DrawNode::create();
    warningRing->setPosition(Vec2::ZERO);
    warningRing->drawCircle(Vec2::ZERO, 10.0f, 0, 24, false, Color4F(1.0f, 0.15f, 0.1f, 0.5f));
    warningRing->setOpacity(200);
    this->addChild(warningRing, 50);

    auto expand = ScaleTo::create(0.28f, 3.0f);
    auto fadeRing = FadeOut::create(0.28f);
    auto ringSpawn = Spawn::create(expand, fadeRing, nullptr);
    auto removeRing = RemoveSelf::create();
    warningRing->runAction(Sequence::create(ringSpawn, removeRing, nullptr));

    // Flash monster red to warn player
    auto tintRed = TintTo::create(0.07f, 255, 60, 60);
    auto tintWhite = TintTo::create(0.07f, 255, 255, 255);
    auto tintRed2 = TintTo::create(0.07f, 255, 40, 40);
    auto tintNormal = TintTo::create(0.07f, 255, 255, 255);
    auto flashSeq = Sequence::create(tintRed, tintWhite, tintRed2, tintNormal, nullptr);
    this->runAction(flashSeq);

    // Scale pulse as warning
    float curScale = getScale();
    auto squash = ScaleTo::create(0.07f, curScale * 0.9f);
    auto stretch = ScaleTo::create(0.07f, curScale * 1.15f);
    auto restore = ScaleTo::create(0.14f, curScale);
    auto pulseSeq = Sequence::create(squash, stretch, restore, nullptr);
    this->runAction(pulseSeq);
}

// ==================== Attack Effect Helper ====================

// Creates a burst of particles radiating from a point
static void createBurstEffect(cocos2d::Node* parent, const cocos2d::Vec2& pos,
    const cocos2d::Color4F& innerColor, const cocos2d::Color4F& outerColor,
    int particleCount, float radius, float duration)
{
    auto burstNode = DrawNode::create();
    burstNode->setPosition(pos);

    // Inner solid circle
    burstNode->drawSolidCircle(Vec2::ZERO, radius * 0.3f, 0, 16, innerColor);

    // Outer expanding ring
    burstNode->drawCircle(Vec2::ZERO, radius * 0.5f, 0, 20, false,
        Color4F(innerColor.r, innerColor.g, innerColor.b, 0.6f));

    // Radial particles
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

    // Expand and fade
    auto scaleUp = ScaleTo::create(duration * 0.5f, 2.0f);
    auto fadeOut = FadeOut::create(duration);
    auto removeSelf = RemoveSelf::create();
    burstNode->runAction(Sequence::create(
        Spawn::create(scaleUp, fadeOut, nullptr),
        removeSelf, nullptr));
}

// ==================== Attack Effect ====================

void Enemy::playAttackEffect()
{
    // Default melee burst: red/orange explosion
    createBurstEffect(this, Vec2::ZERO,
        Color4F(1.0f, 0.3f, 0.1f, 0.8f),   // inner: bright red-orange
        Color4F(1.0f, 0.6f, 0.2f, 0.6f),   // outer: orange sparkles
        14, 35.0f, 0.4f);

    // Scale pop with overshoot
    float curScale = getScale();
    auto pop = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.08f, curScale * 1.25f)),
        EaseBackIn::create(ScaleTo::create(0.2f, curScale)),
        nullptr);
    this->runAction(pop);
}

// ==================== Hit Effect ====================

void Enemy::playHitEffect()
{
    // ── Layer 1: Red tint flash ──
    // Remember original color and flash red → back to white
    Color3B originalColor = this->getColor();
    this->setColor(Color3B(255, 100, 80));  // instant red-orange flash
    auto restoreColor = CallFunc::create([this, originalColor]() {
        if (this != nullptr && this->isObjectActive()) {
            this->setColor(originalColor);
        }
    });
    auto flashDelay = DelayTime::create(0.05f);
    auto flashSeq = Sequence::create(flashDelay, restoreColor, nullptr);
    this->runAction(flashSeq);

    // ── Layer 2: Expanding shockwave ring ──
    auto shockwave = DrawNode::create();
    shockwave->setPosition(Vec2::ZERO);
    // Outer glow ring
    shockwave->drawCircle(Vec2::ZERO, 12.0f, 0, 24, false,
        Color4F(1.0f, 0.85f, 0.3f, 0.7f));
    // Inner bright ring
    shockwave->drawCircle(Vec2::ZERO, 8.0f, 0, 20, false,
        Color4F(1.0f, 1.0f, 1.0f, 0.9f));
    this->addChild(shockwave, 199);

    auto expandRing = ScaleTo::create(0.25f, 3.5f);
    auto fadeRing = FadeOut::create(0.25f);
    auto removeRing = RemoveSelf::create();
    shockwave->runAction(Sequence::create(
        Spawn::create(expandRing, fadeRing, nullptr),
        removeRing, nullptr));

    // ── Layer 3: Multi-layered hit particles ──
    auto particleNode = DrawNode::create();
    particleNode->setPosition(Vec2::ZERO);

    // Inner layer: white-hot core sparks (fast, short-lived)
    for (int i = 0; i < 10; i++)
    {
        float angle = (i * 2.0f * M_PI) / 10 + (rand() % 60 - 30) * M_PI / 180.0f;
        float r = 10.0f + (rand() % 12);
        Vec2 pt = Vec2(cosf(angle), sinf(angle)) * r;
        float size = 2.5f + (rand() % 4);
        particleNode->drawDot(pt, size,
            Color4F(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // Outer layer: orange/yellow sparks (slower, wider spread)
    for (int i = 0; i < 14; i++)
    {
        float angle = (i * 2.0f * M_PI) / 14 + (rand() % 50 - 25) * M_PI / 180.0f;
        float r = 16.0f + (rand() % 20);
        Vec2 pt = Vec2(cosf(angle), sinf(angle)) * r;
        float size = 3.0f + (rand() % 5);
        float brightness = 0.6f + (rand() % 40) / 100.0f;
        particleNode->drawDot(pt, size,
            Color4F(1.0f, 0.7f + brightness * 0.3f, 0.1f + brightness * 0.2f, 0.85f));
    }

    // Extra debris particles (tiny, random angles)
    for (int i = 0; i < 6; i++)
    {
        float angle = CCRANDOM_0_1() * 2.0f * M_PI;
        float r = 20.0f + (rand() % 25);
        Vec2 pt = Vec2(cosf(angle), sinf(angle)) * r;
        particleNode->drawDot(pt, 1.5f + (rand() % 3),
            Color4F(1.0f, 0.85f, 0.4f, 0.7f));
    }

    this->addChild(particleNode, 200);

    // Particles fly outward and fade
    auto scaleUp = ScaleTo::create(0.22f, 1.6f);
    auto fadeOut = FadeOut::create(0.22f);
    auto removeParticles = RemoveSelf::create();
    particleNode->runAction(Sequence::create(
        Spawn::create(scaleUp, fadeOut, nullptr),
        removeParticles, nullptr));

    // ── Layer 4: Position jitter / screen shake ──
    Vec2 originalPos = this->getPosition();
    auto jitterRight = MoveTo::create(0.015f, originalPos + Vec2(3, 1));
    auto jitterLeft = MoveTo::create(0.015f, originalPos + Vec2(-3, -1));
    auto jitterUp = MoveTo::create(0.015f, originalPos + Vec2(1, 2));
    auto jitterDown = MoveTo::create(0.015f, originalPos + Vec2(-1, -2));
    auto settlePos = MoveTo::create(0.04f, originalPos);
    auto jitterSeq = Sequence::create(
        jitterRight, jitterLeft, jitterUp, jitterDown, settlePos, nullptr);
    this->runAction(jitterSeq);

    // ── Layer 5: Knockback squash & stretch ──
    float curScale = getScale();
    auto squash = ScaleTo::create(0.03f, curScale * 0.82f);
    auto bounce = ScaleTo::create(0.07f, curScale * 1.08f);
    auto settle = ScaleTo::create(0.10f, curScale);
    this->runAction(Sequence::create(squash, bounce, settle, nullptr));
}

// ==================== Time Scaling ====================

float Enemy::getTimeScaleFactor(float elapsedTime)
{
    // Monsters get progressively stronger as time passes.
    // Growth accelerates: every 30s adds ~0.7x multiplier, plus
    // an extra quadratic boost for long survival runs.
    // 30s → 1.7x   60s → 2.5x   120s → 4.3x   180s → 6.4x
    // 300s → 10x   600s → 16x (cap)
    float linearPart = 1.0f + (elapsedTime / 30.0f) * 0.7f;
    float quadraticPart = 1.0f + (elapsedTime * elapsedTime) / 72000.0f;
    float factor = linearPart * 0.6f + quadraticPart * 0.4f;
    if (factor > 16.0f) factor = 16.0f;
    if (factor < 1.0f) factor = 1.0f;
    return factor;
}

// ==================== Enemy Projectile System ====================

void Enemy::fireProjectile(const Vec2& direction, float speed,
    const Color4F& glowColor, const Color4F& trailColor,
    int damage, float radius, float lifetime)
{
    EnemyProjectile proj;
    proj.position = this->getPosition();
    proj.direction = direction;
    proj.direction.normalize();
    proj.speed = speed;
    proj.damage = damage;
    proj.lifetime = lifetime;
    proj.elapsed = 0.0f;
    proj.active = true;
    proj.radius = radius;

    // === Create impressive projectile visual ===
    auto container = Node::create();
    container->setPosition(proj.position);

    // Draw all layers on a DrawNode
    auto drawNode = DrawNode::create();
    drawNode->setPosition(Vec2::ZERO);

    // Layer 1: Large outer aura (soft glow)
    drawNode->drawSolidCircle(Vec2::ZERO, radius * 2.5f, 0, 16,
        Color4F(glowColor.r * 0.5f, glowColor.g * 0.5f, glowColor.b * 0.5f, 0.18f));
    // Layer 2: Medium glow ring
    drawNode->drawSolidCircle(Vec2::ZERO, radius * 1.6f, 0, 12,
        Color4F(glowColor.r * 0.7f, glowColor.g * 0.7f, glowColor.b * 0.7f, 0.3f));
    // Layer 3: Main body
    drawNode->drawSolidCircle(Vec2::ZERO, radius, 0, 14, glowColor);
    // Layer 4: Inner bright ring (energy edge)
    drawNode->drawCircle(Vec2::ZERO, radius * 0.85f, 0, 16, false,
        Color4F(glowColor.r * 1.2f, glowColor.g * 1.2f, glowColor.b * 1.2f, 0.85f));
    // Layer 5: Hot core
    drawNode->drawSolidCircle(Vec2::ZERO, radius * 0.45f, 0, 8,
        Color4F(std::min(1.0f, glowColor.r * 1.5f), std::min(1.0f, glowColor.g * 1.5f),
                std::min(1.0f, glowColor.b * 1.5f), 0.95f));
    // Layer 6: White-hot center
    drawNode->drawSolidCircle(Vec2::ZERO, radius * 0.2f, 0, 4,
        Color4F(1.0f, 1.0f, 1.0f, 0.85f));
    // Layer 7: Sparkle highlight
    drawNode->drawSolidCircle(Vec2(radius * 0.25f, radius * 0.3f), radius * 0.25f, 0, 4,
        Color4F(1.0f, 1.0f, 1.0f, 0.8f));

    container->addChild(drawNode);

    // Add trail ring that expands behind projectile
    auto trailRing = DrawNode::create();
    trailRing->setPosition(Vec2::ZERO);
    trailRing->drawCircle(Vec2::ZERO, radius * 1.3f, 0, 20, false, trailColor);
    container->addChild(trailRing, -1);

    proj.node = container;

    // Pulsing animation on the projectile
    auto pulseUp = ScaleTo::create(0.3f, 1.15f);
    auto pulseDown = ScaleTo::create(0.3f, 0.9f);
    auto pulse = RepeatForever::create(Sequence::create(pulseUp, pulseDown, nullptr));
    container->runAction(pulse);

    // Add to the same parent layer as the enemy
    Node* parent = this->getParent();
    if (parent)
    {
        parent->addChild(container, 15); // Above enemies, below UI
    }

    _projectiles.push_back(proj);

    // === Muzzle flash at enemy position ===
    auto muzzleFlash = DrawNode::create();
    muzzleFlash->setPosition(Vec2::ZERO);
    // Expanding ring from muzzle
    muzzleFlash->drawSolidCircle(Vec2::ZERO, radius * 2.5f, 0, 12,
        Color4F(glowColor.r, glowColor.g, glowColor.b, 0.5f));
    muzzleFlash->drawSolidCircle(Vec2::ZERO, radius * 1.2f, 0, 8,
        Color4F(1.0f, 1.0f, 1.0f, 0.7f));
    // Side sparkles at muzzle
    for (int i = 0; i < 8; i++)
    {
        float a = i * M_PI / 4;
        Vec2 pt = Vec2(cosf(a), sinf(a)) * radius * 2.0f;
        muzzleFlash->drawDot(pt, radius * 0.35f,
            Color4F(glowColor.r, glowColor.g, glowColor.b, 0.7f));
    }
    this->addChild(muzzleFlash, 150);
    auto muzzleAnim = Sequence::create(
        Spawn::create(ScaleTo::create(0.25f, 3.0f), FadeOut::create(0.25f), nullptr),
        RemoveSelf::create(), nullptr);
    muzzleFlash->runAction(muzzleAnim);
}

void Enemy::fireProjectileAtPlayer(float speed,
    const Color4F& glowColor, const Color4F& trailColor,
    int damage, float radius, float lifetime)
{
    if (_targetPlayer == nullptr) return;

    Vec2 dir = _targetPlayer->getPosition() - this->getPosition();
    dir.normalize();
    fireProjectile(dir, speed, glowColor, trailColor, damage, radius, lifetime);
}

void Enemy::updateProjectiles(float dt)
{
    for (auto& proj : _projectiles)
    {
        if (!proj.active) continue;

        proj.elapsed += dt;
        if (proj.elapsed >= proj.lifetime)
        {
            proj.active = false;
            if (proj.node)
            {
                // Fade out and remove
                proj.node->runAction(Sequence::create(
                    FadeOut::create(0.15f),
                    RemoveSelf::create(),
                    nullptr));
                proj.node = nullptr;
            }
            continue;
        }

        // Move projectile
        proj.position += proj.direction * proj.speed * dt;
        if (proj.node)
        {
            proj.node->setPosition(proj.position);

            // Rotate for visual interest
            proj.node->setRotation(proj.node->getRotation() + dt * 120.0f);

            // Trail particles only every 3 frames to reduce draw calls
            static int trailFrameSkip = 0;
            trailFrameSkip++;
            if (trailFrameSkip % 3 == 0)
            {
                auto trailDot = DrawNode::create();
                trailDot->setPosition(Vec2::ZERO);
                float trailAlpha = 0.4f;
                trailDot->drawSolidCircle(Vec2::ZERO, proj.radius * 1.2f, 0, 4,
                    Color4F(1.0f, 0.9f, 0.7f, trailAlpha));
                proj.node->addChild(trailDot, -1);

                auto trailAnim = Sequence::create(
                    Spawn::create(
                        FadeOut::create(0.3f),
                        ScaleTo::create(0.3f, 0.1f),
                        nullptr),
                    RemoveSelf::create(),
                    nullptr);
                trailDot->runAction(trailAnim);
            }

            // Extra sparkle occasionally (reduced frequency)
            if (CCRANDOM_0_1() < 0.05f)
            {
                auto spark = DrawNode::create();
                spark->setPosition(Vec2(
                    (CCRANDOM_0_1() - 0.5f) * proj.radius * 2,
                    (CCRANDOM_0_1() - 0.5f) * proj.radius * 2));
                spark->drawDot(Vec2::ZERO, proj.radius * 0.25f,
                    Color4F(1.0f, 1.0f, 1.0f, 0.8f));
                proj.node->addChild(spark, 10);
                spark->runAction(Sequence::create(
                    Spawn::create(
                        FadeOut::create(0.2f),
                        MoveBy::create(0.2f, Vec2(
                            (CCRANDOM_0_1() - 0.5f) * proj.radius * 3,
                            (CCRANDOM_0_1() - 0.5f) * proj.radius * 3)),
                        nullptr),
                    RemoveSelf::create(),
                    nullptr));
            }
        }
    }
}

void Enemy::cleanupProjectiles()
{
    _projectiles.erase(
        std::remove_if(_projectiles.begin(), _projectiles.end(),
            [](const EnemyProjectile& p) { return !p.active; }),
        _projectiles.end()
    );
}
