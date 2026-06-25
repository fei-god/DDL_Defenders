#include "ThesisBoss.h"

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

ThesisBoss* ThesisBoss::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    ThesisBoss* boss = new (std::nothrow) ThesisBoss();
    if (boss && boss->initThesisBoss(imagePath, startPosition, target, waveLevel))
    {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

bool ThesisBoss::initThesisBoss(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    if (!BossMonster::initBossMonster(imagePath, startPosition, target, waveLevel))
    {
        return false;
    }

    setObjectName("ThesisBoss");
    addTag("Boss");
    addTag("Thesis");
    return true;
}

void ThesisBoss::move(float dt)
{
    BossMonster::move(dt);
}

void ThesisBoss::attack()
{
    BossMonster::attack();
    if (getTargetPlayer())
    {
        getTargetPlayer()->changeMood(MoodType::Exhausted, 1.5f);
    }
}

void ThesisBoss::playAttackEffect()
{
    // Thesis Boss: dark academic burst - deep purple/black with glowing pages
    createBurstEffect(this, Vec2::ZERO,
        Color4F(0.25f, 0.08f, 0.45f, 0.85f),  // inner: deep violet
        Color4F(0.5f, 0.2f, 0.7f, 0.7f),       // outer: purple sparks
        20, 55.0f, 0.5f);

    // Floating "page" particles (small rectangular dots flying outward)
    for (int i = 0; i < 12; i++)
    {
        auto page = DrawNode::create();
        float angle = (i * 2.0f * M_PI) / 12;
        page->setPosition(Vec2(cosf(angle) * 20.0f, sinf(angle) * 20.0f));

        // Small rectangular "page" shape
        Vec2 rect[4] = {
            Vec2(-3, -4), Vec2(3, -4), Vec2(3, 4), Vec2(-3, 4)
        };
        page->drawSolidPoly(rect, 4, Color4F(0.6f, 0.4f, 0.8f, 0.7f));
        page->setRotation(CCRANDOM_0_1() * 360.0f);
        this->addChild(page, 100);

        auto flyOut = MoveBy::create(0.5f, Vec2(cosf(angle) * 45.0f, sinf(angle) * 45.0f));
        auto spin = RotateBy::create(0.5f, 180.0f + CCRANDOM_0_1() * 180.0f);
        auto fade = FadeOut::create(0.5f);
        page->runAction(Sequence::create(
            Spawn::create(flyOut, spin, fade, nullptr),
            RemoveSelf::create(), nullptr));
    }

    // Dark ink burst rings
    for (int ring = 0; ring < 3; ring++)
    {
        auto inkRing = DrawNode::create();
        inkRing->setPosition(Vec2::ZERO);
        inkRing->drawCircle(Vec2::ZERO, 20.0f + ring * 8.0f, 0, 24, false,
            Color4F(0.3f, 0.1f, 0.5f, 0.55f - ring * 0.13f));
        this->addChild(inkRing, 90);

        auto delay = DelayTime::create(ring * 0.07f);
        auto expand = ScaleTo::create(0.45f, 3.5f + ring * 0.5f);
        auto fade = FadeOut::create(0.45f);
        inkRing->runAction(Sequence::create(delay,
            Spawn::create(expand, fade, nullptr),
            RemoveSelf::create(), nullptr));
    }

    // Overwhelming pressure: scale distortion
    float curScale = getScale();
    auto distort = Sequence::create(
        ScaleTo::create(0.04f, curScale * 1.35f, curScale * 0.8f),
        ScaleTo::create(0.04f, curScale * 0.8f, curScale * 1.2f),
        ScaleTo::create(0.06f, curScale * 1.2f, curScale * 0.9f),
        EaseBackIn::create(ScaleTo::create(0.22f, curScale)),
        nullptr);
    this->runAction(distort);
}
