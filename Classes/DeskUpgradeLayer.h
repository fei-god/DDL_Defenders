#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <functional>

USING_NS_CC;

class DeskUpgradeLayer : public cocos2d::Layer
{
public:
    static DeskUpgradeLayer* create();
    virtual bool init() override;

    // Dismiss tracking — prevent re-open until zone exit
    static bool canShowPanel() { return _canShowPanel; }
    static void resetCanShow() { _canShowPanel = true; }

    // Callbacks — owner sets these to sync game state
    void setOnClose(std::function<void()> callback) { _onCloseCb = callback; }
    void setOnWeaponChanged(std::function<void(int,int)> cb) { _onWeaponChangedCb = cb; }
    void setOnUpgrade(std::function<void(int)> cb) { _onUpgradeCb = cb; }

    // Sync from game
    void setEquippedWeapons(int id1, int id2);
    void setUpgradePoints(int pts);

private:
    cocos2d::Node* rootNode = nullptr;

    // State
    int _selectedCardIdx = -1;
    int _selectedAttrIdx = -1;
    int _equipSlot1 = -1;
    int _equipSlot2 = -1;
    int _upgradePts = 0;

    // Dismiss tracking
    static bool _canShowPanel;
    std::function<void()> _onCloseCb;
    std::function<void(int,int)> _onWeaponChangedCb;
    std::function<void(int)> _onUpgradeCb;

    // Card references
    Node* _cardNodes[6] = {nullptr};
    Node* _attrItemNodes[5] = {nullptr};
    Sprite* _cardFrames[6] = {nullptr};

    // Buttons & labels
    ui::Button* _btnEquip = nullptr;
    ui::Button* _btnUpgrade = nullptr;
    Label* _pointLabel = nullptr;

    void adaptScale();

    void createPanel();
    void createTitles();
    void createWeapons();
    void createAttributes();
    void createButtons();

    Node* createWeaponCard(
        const std::string& iconPath,
        const std::string& name,
        const Vec2& position,
        int cardIndex
    );

    void createAttributeItem(
        const std::string& fullText,
        const Vec2& position,
        const std::string& costText,
        int attrIndex
    );

    // Interaction
    void onCardClicked(int cardIndex);
    void onAttrClicked(int attrIndex);
    void onEquipClicked();
    void onUpgradeClicked();
    void onCloseClicked();

    void selectCard(int cardIndex);
    void deselectCard(int cardIndex);
    void selectAttr(int attrIndex);
    void deselectAttr(int attrIndex);
    void refreshEquipButton();
    void refreshPointLabel();
    void refreshCardFrames();

    void fitWidth(Node* node, float width);
    void fitSize(Node* node, float width, float height);
};
