#include "DeskUpgradeLayer.h"
#include "Core/AssetPaths.h"

USING_NS_CC;

bool DeskUpgradeLayer::_canShowPanel = true;

DeskUpgradeLayer* DeskUpgradeLayer::create()
{
    auto layer = new DeskUpgradeLayer();
    if (layer && layer->init())
    {
        layer->autorelease();
        return layer;
    }
    CC_SAFE_DELETE(layer);
    return nullptr;
}

bool DeskUpgradeLayer::init()
{
    if (!Layer::init())
        return false;

    rootNode = Node::create();
    rootNode->setContentSize(Size(1672, 941));
    rootNode->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto visibleSize = Director::getInstance()->getVisibleSize();
    rootNode->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
    this->addChild(rootNode);

    adaptScale();

    createPanel();
    createTitles();
    createWeapons();
    createAttributes();
    createButtons();

    return true;
}

void DeskUpgradeLayer::adaptScale()
{
    auto size = Director::getInstance()->getVisibleSize();

    float scaleX = size.width / 1672.0f;
    float scaleY = size.height / 941.0f;
    float scale = std::min(scaleX, scaleY);

    rootNode->setScale(scale);
}

void DeskUpgradeLayer::createPanel()
{
    auto panel = Sprite::create(AssetPaths::resolve("art/ui/upgrade/ui_panel_main.png"));
    if (!panel) return;
    panel->setPosition(Vec2(836, 470));
    fitSize(panel, 1672, 941);
    rootNode->addChild(panel, 0);
}

void DeskUpgradeLayer::createTitles()
{
    auto weaponTitle = Sprite::create(AssetPaths::resolve("art/ui/upgrade/ui_title_weapon.png"));
    if (weaponTitle) {
        weaponTitle->setPosition(Vec2(475, 785));
        weaponTitle->setScale(0.243f);
        rootNode->addChild(weaponTitle, 5);
    }

    auto attributeTitle = Sprite::create(AssetPaths::resolve("art/ui/upgrade/ui_title_attribute.png"));
    if (attributeTitle) {
        attributeTitle->setPosition(Vec2(1197, 785));
        attributeTitle->setScale(0.243f);
        rootNode->addChild(attributeTitle, 5);
    }
}

// ====== WEAPONS ======

void DeskUpgradeLayer::createWeapons()
{
    std::vector<std::string> names =
    {
        "咖啡枪",
        "咖啡激光",
        "键盘冲击波",
        "键盘武器",
        "台灯激光",
        "咖啡爆破"
    };

    std::vector<std::string> icons =
    {
        "weapon/coffee_gun_sprite.png",
        "weapon/coffee_bullet_sprite.png",
        "weapon/keyboard_wave_sprite.png",
        "weapon/keyboard_weapon_sprite.png",
        "weapon/desk_lamp_weapon_sprite.png",
        "weapon/coffee_blast_sprite.png"
    };

    Vec2 weaponPos[6] =
    {
        Vec2(270, 605),
        Vec2(495, 605),
        Vec2(720, 605),

        Vec2(270, 315),
        Vec2(495, 315),
        Vec2(720, 315)
    };

    for (int i = 0; i < 6; i++)
    {
        auto card = createWeaponCard(icons[i], names[i], weaponPos[i], i);
        if (card) {
            rootNode->addChild(card, 3);
        }
    }
}

Node* DeskUpgradeLayer::createWeaponCard(
    const std::string& iconPath,
    const std::string& name,
    const Vec2& position,
    int cardIndex
)
{
    auto node = Node::create();
    node->setPosition(position);
    _cardNodes[cardIndex] = node;

    std::string framePath = AssetPaths::resolve("art/ui/upgrade/weapon_slot_normal.png");
    auto frame = Sprite::create(framePath);
    if (frame) {
        fitSize(frame, 205, 295);
        node->addChild(frame);
        _cardFrames[cardIndex] = frame;
    }

    std::string iconRes = AssetPaths::resolve(iconPath);
    auto icon = Sprite::create(iconRes);
    if (icon) {
        float maxIconSize = 100.0f;
        float srcW = icon->getContentSize().width;
        float srcH = icon->getContentSize().height;
        float iconScale = maxIconSize / std::max(srcW, srcH);
        icon->setScale(iconScale);
        icon->setPosition(Vec2(0, 18));
        node->addChild(icon);
    }

    auto label = Label::createWithSystemFont(name, "Arial", 20);
    label->setColor(Color3B(232, 216, 176));
    label->setPosition(Vec2(0, -82));
    node->addChild(label);

    // Touch listener
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this, cardIndex](Touch* touch, Event*) -> bool {
        if (!_cardNodes[cardIndex]) return false;
        Rect rect(_cardNodes[cardIndex]->getPositionX() - 100,
                  _cardNodes[cardIndex]->getPositionY() - 125,
                  200, 250);
        Vec2 localPos = _cardNodes[cardIndex]->getParent()->convertToNodeSpace(touch->getLocation());
        if (rect.containsPoint(localPos)) {
            onCardClicked(cardIndex);
            return true;
        }
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, node);

    return node;
}

void DeskUpgradeLayer::onCardClicked(int cardIndex)
{
    if (_selectedCardIdx == cardIndex) {
        // Deselect
        deselectCard(cardIndex);
        _selectedCardIdx = -1;
    } else {
        // Select new
        if (_selectedCardIdx >= 0) {
            deselectCard(_selectedCardIdx);
        }
        _selectedCardIdx = cardIndex;
        selectCard(cardIndex);
    }
    refreshEquipButton();
}

void DeskUpgradeLayer::selectCard(int cardIndex)
{
    if (!_cardNodes[cardIndex]) return;
    _cardNodes[cardIndex]->setScale(1.1f);
}

void DeskUpgradeLayer::deselectCard(int cardIndex)
{
    if (!_cardNodes[cardIndex]) return;
    _cardNodes[cardIndex]->setScale(1.0f);
}

// ====== ATTRIBUTES ======

void DeskUpgradeLayer::createAttributes()
{
    _pointLabel = Label::createWithSystemFont("升级点数：0", "Arial", 34);
    _pointLabel->setColor(Color3B(255, 216, 107));
    _pointLabel->setPosition(Vec2(1210, 700));
    rootNode->addChild(_pointLabel, 5);

    std::vector<Vec2> pos =
    {
        Vec2(1200, 625),
        Vec2(1200, 530),
        Vec2(1200, 435),
        Vec2(1200, 340),
        Vec2(1200, 245)
    };

    createAttributeItem("攻击 +2",       pos[0], "1pt", 0);
    createAttributeItem("生命上限 +10",  pos[1], "1pt", 1);
    createAttributeItem("速度 +10",      pos[2], "1pt", 2);
    createAttributeItem("能量恢复 +10%", pos[3], "1pt", 3);
    createAttributeItem("弹幕 +1",       pos[4], "1pt", 4);
}

void DeskUpgradeLayer::createAttributeItem(
    const std::string& fullText,
    const Vec2& position,
    const std::string& costText,
    int attrIndex
)
{
    auto node = Node::create();
    node->setPosition(position);
    rootNode->addChild(node, 4);
    _attrItemNodes[attrIndex] = node;

    auto bg = Sprite::create(AssetPaths::resolve("art/ui/upgrade/attribute_upgrade_bar.png"));
    if (bg) {
        fitSize(bg, 658, 91);
        node->addChild(bg);
    }

    auto textLabel = Label::createWithSystemFont(fullText, "Arial", 25);
    textLabel->setColor(Color3B(232, 216, 176));
    textLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    textLabel->setPosition(Vec2(-145, 0));
    node->addChild(textLabel);

    auto costLabel = Label::createWithSystemFont(costText, "Arial", 25);
    costLabel->setColor(Color3B(255, 216, 107));
    costLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    costLabel->setPosition(Vec2(195, 0));
    node->addChild(costLabel);

    // Touch listener
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this, attrIndex](Touch* touch, Event*) -> bool {
        if (!_attrItemNodes[attrIndex]) return false;
        Rect rect(_attrItemNodes[attrIndex]->getPositionX() - 330,
                  _attrItemNodes[attrIndex]->getPositionY() - 45,
                  660, 90);
        Vec2 localPos = _attrItemNodes[attrIndex]->getParent()->convertToNodeSpace(touch->getLocation());
        if (rect.containsPoint(localPos)) {
            onAttrClicked(attrIndex);
            return true;
        }
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, node);
}

void DeskUpgradeLayer::onAttrClicked(int attrIndex)
{
    if (_selectedAttrIdx == attrIndex) {
        deselectAttr(attrIndex);
        _selectedAttrIdx = -1;
    } else {
        if (_selectedAttrIdx >= 0) {
            deselectAttr(_selectedAttrIdx);
        }
        _selectedAttrIdx = attrIndex;
        selectAttr(attrIndex);
    }
}

void DeskUpgradeLayer::selectAttr(int attrIndex)
{
    if (!_attrItemNodes[attrIndex]) return;
    _attrItemNodes[attrIndex]->setScale(1.1f);
}

void DeskUpgradeLayer::deselectAttr(int attrIndex)
{
    if (!_attrItemNodes[attrIndex]) return;
    _attrItemNodes[attrIndex]->setScale(1.0f);
}

// ====== BUTTONS ======

void DeskUpgradeLayer::createButtons()
{
    std::vector<std::string> names = { "装备", "升级", "关闭" };
    std::vector<Vec2> pos =
    {
        Vec2(560, 90),
        Vec2(836, 90),
        Vec2(1110, 90)
    };

    std::string btnPath = AssetPaths::resolve("art/ui/upgrade/button_large.png");

    for (int i = 0; i < 3; i++)
    {
        auto button = ui::Button::create(btnPath);
        if (!button) continue;
        fitSize(button, 338, 104);
        button->setPosition(pos[i]);
        button->setTitleText(names[i]);
        button->setTitleFontSize(234);
        button->setTitleColor(Color3B(232, 216, 176));
        rootNode->addChild(button, 5);

        if (i == 0) {
            _btnEquip = button;
            button->addClickEventListener([this](Ref*) { onEquipClicked(); });
        } else if (i == 1) {
            _btnUpgrade = button;
            button->addClickEventListener([this](Ref*) { onUpgradeClicked(); });
        } else {
            button->addClickEventListener([this](Ref*) { onCloseClicked(); });
        }
    }
}

// ====== GAME LOGIC ======

void DeskUpgradeLayer::onEquipClicked()
{
    if (_selectedCardIdx < 0) return;

    // Check if selected card is already equipped
    if (_equipSlot1 == _selectedCardIdx || _equipSlot2 == _selectedCardIdx) {
        // Unequip
        if (_equipSlot1 == _selectedCardIdx) {
            _equipSlot1 = -1;
        } else {
            _equipSlot2 = -1;
        }
    } else {
        // Equip
        if (_equipSlot1 < 0) {
            _equipSlot1 = _selectedCardIdx;
        } else if (_equipSlot2 < 0) {
            _equipSlot2 = _selectedCardIdx;
        } else {
            // Both full → eject slot 2, shift 1→2, new→1
            _equipSlot2 = _equipSlot1;
            _equipSlot1 = _selectedCardIdx;
        }
    }
    refreshCardFrames();
    refreshEquipButton();

    // Notify owner
    if (_onWeaponChangedCb) {
        _onWeaponChangedCb(_equipSlot1, _equipSlot2);
    }
}

void DeskUpgradeLayer::onUpgradeClicked()
{
    if (_selectedAttrIdx < 0 || _upgradePts <= 0) return;
    if (_onUpgradeCb) {
        _onUpgradeCb(_selectedAttrIdx);
    }
}

void DeskUpgradeLayer::setEquippedWeapons(int id1, int id2)
{
    _equipSlot1 = id1;
    _equipSlot2 = id2;
    refreshCardFrames();
}

void DeskUpgradeLayer::setUpgradePoints(int pts)
{
    _upgradePts = pts;
    refreshPointLabel();
}

void DeskUpgradeLayer::onCloseClicked()
{
    _canShowPanel = false;
    if (_onCloseCb) {
        _onCloseCb();
    }
}

void DeskUpgradeLayer::refreshEquipButton()
{
    if (!_btnEquip) return;
    if (_selectedCardIdx >= 0 &&
        (_equipSlot1 == _selectedCardIdx || _equipSlot2 == _selectedCardIdx)) {
        _btnEquip->setTitleText("取消装备");
    } else {
        _btnEquip->setTitleText("装备");
    }
}

void DeskUpgradeLayer::refreshPointLabel()
{
    if (!_pointLabel) return;
    char buf[64];
    snprintf(buf, sizeof(buf), "升级点数：%d", _upgradePts);
    _pointLabel->setString(buf);
}

void DeskUpgradeLayer::refreshCardFrames()
{
    for (int i = 0; i < 6; i++)
    {
        if (!_cardFrames[i]) continue;
        bool equipped = (_equipSlot1 == i || _equipSlot2 == i);
        std::string framePath = AssetPaths::resolve(
            equipped ? "art/ui/upgrade/weapon_slot_selected.png"
                     : "art/ui/upgrade/weapon_slot_normal.png");
        auto newFrame = Sprite::create(framePath);
        if (newFrame) {
            fitSize(newFrame, 205, 295);
            _cardFrames[i]->setTexture(newFrame->getTexture());
        }
    }
}

// ====== UTILITY ======

void DeskUpgradeLayer::fitWidth(Node* node, float width)
{
    if (!node) return;
    float scale = width / node->getContentSize().width;
    node->setScale(scale);
}

void DeskUpgradeLayer::fitSize(Node* node, float width, float height)
{
    if (!node) return;
    auto size = node->getContentSize();
    node->setScaleX(width / size.width);
    node->setScaleY(height / size.height);
}
