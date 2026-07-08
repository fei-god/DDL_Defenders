# DDL Defenders — 开发进度总结

## 项目概述

cocos2d-x v4 C++ 俯视角自动射击/塔防游戏。玩家在宿舍（Hub）通过书桌升级属性、通过床回血，然后从门进入关卡战斗。

---

## 场景流程

```
故事模式 (3关): Hub → Scene1 图书馆 → Hub → Scene2 教室 → Hub → Scene3 办公室 → 主菜单 (DDL FINISHED!)
无尽模式:     Hub → 随机关卡 → Hub → 随机关卡 → Hub → ... (无限循环)
```

---

## 已实现的系统

### 战斗系统
- **双武器自动攻击**: WASD 移动，两把武器同时自动瞄准最近敌人开火
- **武器类型** (6种):
  | ID | 名称 | 特点 |
  |----|------|------|
  | 0 | 咖啡枪 CoffeeGun | 均衡，范围 560 |
  | 1 | 咖啡激光 CoffeeLaser | 远程，范围 760 |
  | 2 | 键盘冲击波 KeyboardWave | 扇形 3 发，范围 520 |
  | 3 | 键盘武器 KeyboardWeap | 穿透，范围 640 |
  | 4 | 台灯激光 DeskLampLaser | 远程穿透，范围 720 |
  | 5 | 咖啡爆破 CoffeeBlast | 近战，范围 185 |
- **武器能量系统**: 每把武器独立能量条，消耗后自动回复
- **武器冷却系统**: 受心情系统影响
- **武器专精** (`applyWeaponMasteryEffects`): 无尽模式成长加成
- **子弹池** (BulletPool): 对象池复用子弹

### 敌人系统
- **4 种敌人类型**:
  | 类型 | 类名 | 说明 |
  |------|------|------|
  | 0 | SleepyMonster | 基础近战怪 |
  | 1 | DDLMonster | DDL 怪 |
  | 2 | ThesisBoss | Boss（继承 BossMonster） |
  | 3 | PhoneMonster | 远程怪 |
- **Hit-count 死亡系统**: 敌人死亡由受击次数决定（非 HP），受玩家心情影响
- **敌人投射物**: 远程怪发射投射物，死后投射物进入 orphan 列表继续存在
- **Boss 波**: 每 3 波触发一次
- **碰撞箱缩小**: Enemy::getCollisionBox() 返回 1/12 的碰撞箱

### 波次系统 (WaveManager)
- **图书馆** (Scene 1): DDL怪兽 only，生存 90 秒 + 击杀 10 → 胜利
- **教室** (Scene 2): Sleepy+DDL 混搭，4 波 × 25 秒 → 清完第 4 波所有敌人 → 胜利
- **办公室** (Scene 3): ThesisBoss+PhoneMonster，`_thesisProgress >= 100%` → 胜利
- **无尽模式**: 时间驱动生成难度曲线，随机关卡循环
- **波次难度**: 随波次增加 spawn 速度、怪血量、怪伤害
- **敌人清理**: stopSpawn 后仍然清理死敌人（victory check 依赖 `aliveEnemies.empty()`）
- **通关回调**: `_allWavesClearedCallback` 在 WaveManager::update() 中被正确调用

### 胜利/延迟系统
- 触发胜利后停止生成、清怪（图书馆/办公室）、启动 5 秒倒计时
- 倒计时期间游戏逻辑冻结
- 故事模式通关 → 显示 "DDL FINISHED!"，跳转主菜单，存档停在 Level 3
- 其他情况 → 显示 "胜利！X秒后返回宿舍…"，跳转 Hub
- 变量: `_victoryDelayTimer`, `_victoryDelayActive`, `_goToMainMenu`

### 场景加载
- 基于 `_levelNumber` 决定下一关（非 `_sceneId + 1`）
- Hub 中按 E 进门显示目的地名称
- `KEY_CAPITAL_E` 也支持

### 模型缩放
- 敌人模型 3× (180px 普通 / 360px Boss)
- 玩家模型 2× (applySpriteFit 306×306)
- 武器模型 2× (setObjectScale 0.095→0.19)
- 子弹速度翻倍

### UI 系统
- **主界面**: MainMenuScene（故事/无尽/设置/排行榜）
- **故事模式**: StoryModeScene（存档管理）
- **Hub 面板**: 自动弹出书桌升级面板、床面板
- **战斗 HUD**: HP 条、武器能量、武器槽位、波次倒计时
- **暂停菜单**: 继续/设置/保存/返回，含武器面板和属性面板
- **设置**: 分辨率、显示模式、键位绑定、语言切换
- **排行榜**: LeaderboardScene

### 书桌升级面板 (DeskUpgradeLayer)

**独立类**: `Classes/DeskUpgradeLayer.h/.cpp` — 1672×941 设计空间，自动适配屏幕。

**UI 布局**:
- 左区域: 武器更换标题 + 3×2 武器卡牌网格
- 右区域: 角色属性标题 + 升级点数 + 5 条属性升级条
- 底部: 装备/升级/关闭 三个按钮

**武器卡牌交互**:
- 点击选中 → 放大 10%（1.1× scale）；点击另一张 → 切换选中；再点已选中 → 取消
- 选中卡牌后按「装备」→ 写入装备槽（优先 1 号位，再 2 号位）
- 选中已装备卡牌 → 按钮变为「取消装备」→ 卸下
- 两槽已满时装备新武器 → 2 号位弹出，1→2，新→1
- 槽位为空时允许不装备武器（不强制填充）
- 切换装备后立即更新玩家手上持有的武器模型

**属性升级交互**:
- 点击属性条选中 → 放大 10%；切换选中/取消同理
- 选中后按「升级」→ 消耗 1 点升级点数，实装属性加成
- 属性文本使用单 Label（如 "攻击 +2"），不拆分

**关闭按钮**:
- 关闭后遮罩+面板消失，标记 `canShowPanel=false`
- 在区域内不重复弹出，走出区域自动重置标记
- 通过回调触发 `GameScene::hideDeskPanel()`，避免悬空指针

**数据同步**:
- 装备变更通过 `setOnWeaponChanged` 回调通知 GameScene，实时更新 `_weaponLoadoutIds` 并持久化
- 属性升级通过 `setOnUpgrade` 回调通知 GameScene，应用实际属性加成
- 打开面板时通过 `setEquippedWeapons()` / `setUpgradePoints()` 同步当前游戏状态

**素材路径**: `Resources/art/ui/upgrade/`
- `ui_panel_main.png` — 主面板背景 (1672×941)
- `ui_title_weapon.png` / `ui_title_attribute.png` — 左右标题
- `weapon_slot_normal.png` / `weapon_slot_selected.png` — 卡牌选中/未选中
- `attribute_upgrade_bar.png` — 属性条背景
- `button_large.png` — 底部按钮

### 心情系统 (MoodSystem)
- 玩家心情影响：冷却倍率、伤害倍率、击打次数
- 环境区域（书桌/床/插座）影响心情值

### 存档系统 (SaveManager)
- UserDefault 持久化
- 存档内容: 等级、经验、升级点、武器配置、属性加成、解锁关卡

### 音频系统 (AudioManager)
- 保留的方法: playButtonClick, playPause, playPlayerHurt, playPlayerMonsterCollision, playPlayerDie, playPlayerLevelUp, playKeyboardAttack, playLaserAttack, playCoffeeAttack, playWeaponUpgrade, playRewardPickup, playUpgradeSelected, playEnemySpawn, playEnemyHit, playEnemyDie, playGameStart, playGameOver, playVictory, playBedEffect, playDeskEffect, playSocketEffect, BGM/音量控制

### 语言系统 (LanguageManager)
- 支持简体中文、英文（日韩占位，不可选）
- `mainmenu_hint` 已更新为当前操控: "WASD 移动 自动攻击 Esc 暂停"
- `getStringF` 保留（被 GameOverScene 使用）

---

## 关键类/文件

| 文件 | 职责 |
|------|------|
| `Classes/GameScene.h/.cpp` | 核心场景：Hub+战斗，最大文件 |
| `Classes/Managers/WaveManager.h/.cpp` | 波次生成、敌人管理 |
| `Classes/Managers/AudioManager.h/.cpp` | 音频统一接口 |
| `Classes/Managers/SaveManager.h/.cpp` | 存档读写 |
| `Classes/Managers/LanguageManager.h/.cpp` | 多语言 |
| `Classes/Managers/CollisionManager.h/.cpp` | 子弹-敌人碰撞 |
| `Classes/Weapons/Weapon.h/.cpp` | 武器基类 |
| `Classes/Weapons/CoffeeGun.cpp` 等 6 个 | 具体武器 |
| `Classes/Characters/Player.h/.cpp` | 玩家 |
| `Classes/Characters/Enemy.h/.cpp` | 敌人基类（hit-count 死亡系统） |
| `Classes/Characters/SleepyMonster.cpp` 等 4 个 | 具体敌人 |
| `Classes/Core/GameObject.h/.cpp` | 游戏对象基类 |
| `Classes/Core/Role.h/.cpp` | 角色基类（HP、伤害、无敌） |
| `Classes/Core/Bullet.h/.cpp` | 子弹 |
| `Classes/Core/BulletPool.h/.cpp` | 子弹对象池 |
| `Classes/Core/AssetPaths.h` | 素材路径解析 |
| `Classes/Core/PlayerRecord.h` | 玩家记录数据结构 |
| `Classes/DeskUpgradeLayer.h/.cpp` | 书桌升级面板（独立Layer，武器切换+属性升级） |
| `Classes/Scenes/MainMenuScene.cpp` 等 | 各场景 |
| `Classes/Systems/MoodSystem.h/.cpp` | 心情系统 |

---

## 已清理的死代码

以下内容已在本次重构中删除：
- `fireBullet()` / `applyAimWeaponDamage()` / `switchWeapon()` — O键手动攻击系统
- `_currentWeapon` / `_currentWeaponIndex` — 旧武器切换系统
- 16 个 AudioManager 未使用音效方法
- 3 个 SaveManager 未使用方法
- WaveManager: `_bossWaveCallback`, `_totalEnemiesThisWave`, `_enemiesSpawnedCount`, `_useTimeBasedSpawn`
- Weapon: `updateObject()`, `readyNow()`, 4 个死 getter
- GameObject: 11 个未使用虚方法, `interactable` 字段
- Role: 7 个 Brotato 式战斗方法
- Player: `enemySpeedModifier`, `onWaveStart/End`, `pickupExp/Material/Heal`
- HelloWorldScene / DDLTestScene / MoodEventDemoScene — 6 个死场景文件
- GameScene: `_lastWeaponSlotIndex`, `_nearPowerSocket`, debug `_environmentLabel` 覆写, `onPauseEquipWeapon/UnequipWeapon`, `applyWeaponMastery`, `goToAfterBattle`, `#include "VictoryScene.h"`, 冗余 `onMouseDown`, 冗余 `_playerVisual` 旋转, legacy 槽位编号

---

## 编译环境

- **平台**: Windows
- **引擎**: cocos2d-x v4
- **编译器**: Visual Studio 2022 (MSVC)
- **项目文件**: `proj.win32/DDL_Defenders.vcxproj`

---

## 待完成事项

1. 测试完整故事模式流程（含 DeskUpgradeLayer 的装备/属性在战斗中生效）
2. 测试无尽模式流程
3. DeskUpgradeLayer — 升级按钮目前无特殊反馈（可加音效/动画）
4. 心情系统实际影响武器冷却和伤害的数值平衡调整
5. 背包系统 — 目前武器背包仅用于存储未装备武器 ID，未在 UI 中展示
