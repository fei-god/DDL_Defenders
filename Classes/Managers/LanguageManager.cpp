#include "LanguageManager.h"
#include "cocos2d.h"
#include <cstdio>

USING_NS_CC;

LanguageManager* LanguageManager::_instance = nullptr;

LanguageManager* LanguageManager::getInstance()
{
    if (!_instance)
    {
        _instance = new LanguageManager();
        _instance->initStrings();
    }
    return _instance;
}

void LanguageManager::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}

LanguageManager::LanguageManager()
    : _currentLanguage(Language::SIMPLIFIED_CHINESE)
{
}

LanguageManager::~LanguageManager()
{
}

void LanguageManager::setLanguage(Language lang)
{
    _currentLanguage = lang;
}

LanguageManager::Language LanguageManager::getLanguage() const
{
    return _currentLanguage;
}

int LanguageManager::getLanguageIndex() const
{
    return static_cast<int>(_currentLanguage);
}

std::string LanguageManager::getLanguageName() const
{
    auto it = _strings.find("lang_self");
    if (it != _strings.end())
    {
        auto it2 = it->second.find(_currentLanguage);
        if (it2 != it->second.end())
            return it2->second;
    }
    return "Unknown";
}

std::string LanguageManager::getString(const std::string& key) const
{
    auto it = _strings.find(key);
    if (it != _strings.end())
    {
        auto it2 = it->second.find(_currentLanguage);
        if (it2 != it->second.end())
            return it2->second;
    }
    return "[" + key + "]";
}

std::string LanguageManager::getStringF(const std::string& key, float value) const
{
    std::string fmt = getString(key);
    char buf[128];
    snprintf(buf, sizeof(buf), fmt.c_str(), value);
    return std::string(buf);
}

std::vector<std::string> LanguageManager::getAvailableLanguageNames() const
{
    std::vector<std::string> names;
    for (int i = 0; i < 2; ++i)
    {
        Language lang = static_cast<Language>(i);
        auto it = _strings.find("lang_self");
        if (it != _strings.end())
        {
            auto it2 = it->second.find(lang);
            if (it2 != it->second.end())
                names.push_back(it2->second);
        }
    }
    return names;
}

std::vector<LanguageManager::Language> LanguageManager::getAvailableLanguages()
{
    std::vector<Language> langs;
    for (int i = 0; i < 2; ++i)
        langs.push_back(static_cast<Language>(i));
    return langs;
}

LanguageManager::Language LanguageManager::intToLanguage(int idx)
{
    if (idx < 0 || idx >= 2)
        return Language::SIMPLIFIED_CHINESE;
    return static_cast<Language>(idx);
}

// ============================================================================
//  All localised strings
// ============================================================================
void LanguageManager::initStrings()
{
    using L = Language;

    // ---- Language self-names ----
    _strings["lang_self"][L::SIMPLIFIED_CHINESE] = u8"简体中文";
    _strings["lang_self"][L::ENGLISH]            = "English";
    _strings["lang_self"][L::JAPANESE]           = u8"日本語";
    _strings["lang_self"][L::KOREAN]             = u8"한국어";

    // ---- Main Menu ----
    _strings["start_game"][L::SIMPLIFIED_CHINESE] = u8"开始游戏";
    _strings["start_game"][L::ENGLISH]            = "Start Game";
    _strings["start_game"][L::JAPANESE]           = u8"ゲーム開始";
    _strings["start_game"][L::KOREAN]             = u8"게임 시작";

    _strings["settings"][L::SIMPLIFIED_CHINESE] = u8"设置";
    _strings["settings"][L::ENGLISH]            = "Settings";
    _strings["settings"][L::JAPANESE]           = u8"設定";
    _strings["settings"][L::KOREAN]             = u8"설정";

    _strings["exit_game"][L::SIMPLIFIED_CHINESE] = u8"退出游戏";
    _strings["exit_game"][L::ENGLISH]            = "Exit Game";
    _strings["exit_game"][L::JAPANESE]           = u8"ゲーム終了";
    _strings["exit_game"][L::KOREAN]             = u8"게임 종료";

    _strings["mainmenu_hint"][L::SIMPLIFIED_CHINESE] = u8"WASD 移动 | 鼠标点击 射击";
    _strings["mainmenu_hint"][L::ENGLISH]            = "WASD Move | Mouse Click Shoot";
    _strings["mainmenu_hint"][L::JAPANESE]           = u8"WASD 移動 | マウスクリック 射撃";
    _strings["mainmenu_hint"][L::KOREAN]             = u8"WASD 이동 | 마우스 클릭 발사";

    // ---- Game Over ----
    _strings["gameover_title"][L::SIMPLIFIED_CHINESE] = u8"游戏结束";
    _strings["gameover_title"][L::ENGLISH]            = "Game Over";
    _strings["gameover_title"][L::JAPANESE]           = u8"ゲームオーバー";
    _strings["gameover_title"][L::KOREAN]             = u8"게임 오버";

    _strings["survival_time_fmt"][L::SIMPLIFIED_CHINESE] = u8"存活时间: %.1fs";
    _strings["survival_time_fmt"][L::ENGLISH]            = "Survival Time: %.1fs";
    _strings["survival_time_fmt"][L::JAPANESE]           = u8"生存時間: %.1fs";
    _strings["survival_time_fmt"][L::KOREAN]             = u8"생존 시간: %.1fs";

    _strings["restart"][L::SIMPLIFIED_CHINESE] = u8"重新开始";
    _strings["restart"][L::ENGLISH]            = "Restart";
    _strings["restart"][L::JAPANESE]           = u8"リスタート";
    _strings["restart"][L::KOREAN]             = u8"다시 시작";

    _strings["back_to_title"][L::SIMPLIFIED_CHINESE] = u8"返回标题";
    _strings["back_to_title"][L::ENGLISH]            = "Back to Title";
    _strings["back_to_title"][L::JAPANESE]           = u8"タイトルに戻る";
    _strings["back_to_title"][L::KOREAN]             = u8"타이틀로 돌아가기";

    // ---- Pause ----
    _strings["pause_title"][L::SIMPLIFIED_CHINESE] = u8"暂停";
    _strings["pause_title"][L::ENGLISH]            = "Paused";
    _strings["pause_title"][L::JAPANESE]           = u8"一時停止";
    _strings["pause_title"][L::KOREAN]             = u8"일시 정지";

    _strings["resume"][L::SIMPLIFIED_CHINESE] = u8"继续游戏";
    _strings["resume"][L::ENGLISH]            = "Resume";
    _strings["resume"][L::JAPANESE]           = u8"再開";
    _strings["resume"][L::KOREAN]             = u8"계속하기";

    // ---- Settings ----
    _strings["settings_title"][L::SIMPLIFIED_CHINESE] = u8"设置";
    _strings["settings_title"][L::ENGLISH]            = "Settings";
    _strings["settings_title"][L::JAPANESE]           = u8"設定";
    _strings["settings_title"][L::KOREAN]             = u8"설정";

    _strings["resolution"][L::SIMPLIFIED_CHINESE] = u8"分辨率";
    _strings["resolution"][L::ENGLISH]            = "Resolution";
    _strings["resolution"][L::JAPANESE]           = u8"解像度";
    _strings["resolution"][L::KOREAN]             = u8"해상도";

    _strings["key_bindings"][L::SIMPLIFIED_CHINESE] = u8"键位设置";
    _strings["key_bindings"][L::ENGLISH]            = "Key Bindings";
    _strings["key_bindings"][L::JAPANESE]           = u8"キー設定";
    _strings["key_bindings"][L::KOREAN]             = u8"키 설정";

    _strings["language"][L::SIMPLIFIED_CHINESE] = u8"语言";
    _strings["language"][L::ENGLISH]            = "Language";
    _strings["language"][L::JAPANESE]           = u8"言語";
    _strings["language"][L::KOREAN]             = u8"언어";

    _strings["back"][L::SIMPLIFIED_CHINESE] = u8"返回";
    _strings["back"][L::ENGLISH]            = "Back";
    _strings["back"][L::JAPANESE]           = u8"戻る";
    _strings["back"][L::KOREAN]             = u8"뒤로";

    _strings["restart_hint"][L::SIMPLIFIED_CHINESE] = u8"重启后生效";
    _strings["restart_hint"][L::ENGLISH]            = "Takes effect after restart";
    _strings["restart_hint"][L::JAPANESE]           = u8"再起動後に適用";
    _strings["restart_hint"][L::KOREAN]             = u8"재시작 후 적용";

    _strings["press_key"][L::SIMPLIFIED_CHINESE] = u8"按下新按键...";
    _strings["press_key"][L::ENGLISH]            = "Press new key...";
    _strings["press_key"][L::JAPANESE]           = u8"新しいキーを押す...";
    _strings["press_key"][L::KOREAN]             = u8"새 키를 누르세요...";

    _strings["rebind"][L::SIMPLIFIED_CHINESE] = u8"改键";
    _strings["rebind"][L::ENGLISH]            = "Rebind";
    _strings["rebind"][L::JAPANESE]           = u8"変更";
    _strings["rebind"][L::KOREAN]             = u8"변경";

    _strings["move_up"][L::SIMPLIFIED_CHINESE] = u8"上移";
    _strings["move_up"][L::ENGLISH]            = "Move Up";
    _strings["move_up"][L::JAPANESE]           = u8"上に移動";
    _strings["move_up"][L::KOREAN]             = u8"위로 이동";

    _strings["move_down"][L::SIMPLIFIED_CHINESE] = u8"下移";
    _strings["move_down"][L::ENGLISH]            = "Move Down";
    _strings["move_down"][L::JAPANESE]           = u8"下に移動";
    _strings["move_down"][L::KOREAN]             = u8"아래로 이동";

    _strings["move_left"][L::SIMPLIFIED_CHINESE] = u8"左移";
    _strings["move_left"][L::ENGLISH]            = "Move Left";
    _strings["move_left"][L::JAPANESE]           = u8"左に移動";
    _strings["move_left"][L::KOREAN]             = u8"왼쪽으로 이동";

    _strings["move_right"][L::SIMPLIFIED_CHINESE] = u8"右移";
    _strings["move_right"][L::ENGLISH]            = "Move Right";
    _strings["move_right"][L::JAPANESE]           = u8"右に移動";
    _strings["move_right"][L::KOREAN]             = u8"오른쪽으로 이동";

    _strings["fire_key"][L::SIMPLIFIED_CHINESE] = u8"射击";
    _strings["fire_key"][L::ENGLISH]            = "Fire";
    _strings["fire_key"][L::JAPANESE]           = u8"射撃";
    _strings["fire_key"][L::KOREAN]             = u8"발사";

    _strings["pause_key"][L::SIMPLIFIED_CHINESE] = u8"暂停";
    _strings["pause_key"][L::ENGLISH]            = "Pause";
    _strings["pause_key"][L::JAPANESE]           = u8"一時停止";
    _strings["pause_key"][L::KOREAN]             = u8"일시 정지";

    _strings["mouse_left"][L::SIMPLIFIED_CHINESE] = u8"鼠标左键";
    _strings["mouse_left"][L::ENGLISH]            = "Left Mouse";
    _strings["mouse_left"][L::JAPANESE]           = u8"マウス左";
    _strings["mouse_left"][L::KOREAN]             = u8"마우스 왼쪽";

    // ---- Generic ----
    _strings["display_mode"][L::SIMPLIFIED_CHINESE] = u8"显示模式";
    _strings["display_mode"][L::ENGLISH]            = "Display Mode";
    _strings["display_mode"][L::JAPANESE]           = u8"表示モード";
    _strings["display_mode"][L::KOREAN]             = u8"디스플레이 모드";

    _strings["windowed"][L::SIMPLIFIED_CHINESE] = u8"窗口";
    _strings["windowed"][L::ENGLISH]            = "Windowed";
    _strings["windowed"][L::JAPANESE]           = u8"ウィンドウ";
    _strings["windowed"][L::KOREAN]             = u8"창";

    _strings["borderless"][L::SIMPLIFIED_CHINESE] = u8"无边框";
    _strings["borderless"][L::ENGLISH]            = "Borderless";
    _strings["borderless"][L::JAPANESE]           = u8"ボーダーレス";
    _strings["borderless"][L::KOREAN]             = u8"테두리 없음";

    _strings["fullscreen"][L::SIMPLIFIED_CHINESE] = u8"全屏";
    _strings["fullscreen"][L::ENGLISH]            = "Fullscreen";
    _strings["fullscreen"][L::JAPANESE]           = u8"フルスクリーン";
    _strings["fullscreen"][L::KOREAN]             = u8"전체 화면";

    _strings["apply"][L::SIMPLIFIED_CHINESE] = u8"应用";
    _strings["apply"][L::ENGLISH]            = "Apply";
    _strings["apply"][L::JAPANESE]           = u8"適用";
    _strings["apply"][L::KOREAN]             = u8"적용";

    _strings["confirm"][L::SIMPLIFIED_CHINESE] = u8"确定";
    _strings["confirm"][L::ENGLISH]            = "OK";
    _strings["confirm"][L::JAPANESE]           = u8"確認";
    _strings["confirm"][L::KOREAN]             = u8"확인";
    // Clean Chinese/English overrides used by the current game UI.
    _strings["lang_self"][L::SIMPLIFIED_CHINESE] = u8"简体中文";
    _strings["lang_self"][L::ENGLISH] = "English";

    _strings["start_game"][L::SIMPLIFIED_CHINESE] = u8"开始游戏";
    _strings["start_game"][L::ENGLISH] = "Start Game";
    _strings["settings"][L::SIMPLIFIED_CHINESE] = u8"设置";
    _strings["settings"][L::ENGLISH] = "Settings";
    _strings["exit_game"][L::SIMPLIFIED_CHINESE] = u8"退出游戏";
    _strings["exit_game"][L::ENGLISH] = "Exit Game";
    _strings["mainmenu_hint"][L::SIMPLIFIED_CHINESE] = u8"WASD 移动 | O 攻击";
    _strings["mainmenu_hint"][L::ENGLISH] = "WASD Move | O Attack";

    _strings["gameover_title"][L::SIMPLIFIED_CHINESE] = u8"游戏结束";
    _strings["gameover_title"][L::ENGLISH] = "Game Over";
    _strings["survival_time_fmt"][L::SIMPLIFIED_CHINESE] = u8"生存时间: %.1fs";
    _strings["survival_time_fmt"][L::ENGLISH] = "Survival Time: %.1fs";
    _strings["restart"][L::SIMPLIFIED_CHINESE] = u8"重新开始";
    _strings["restart"][L::ENGLISH] = "Restart";
    _strings["back_to_title"][L::SIMPLIFIED_CHINESE] = u8"返回首页";
    _strings["back_to_title"][L::ENGLISH] = "Back to Menu";

    _strings["pause_title"][L::SIMPLIFIED_CHINESE] = u8"暂停";
    _strings["pause_title"][L::ENGLISH] = "Paused";
    _strings["resume"][L::SIMPLIFIED_CHINESE] = u8"继续游戏";
    _strings["resume"][L::ENGLISH] = "Resume";

    _strings["settings_title"][L::SIMPLIFIED_CHINESE] = u8"设置";
    _strings["settings_title"][L::ENGLISH] = "Settings";
    _strings["resolution"][L::SIMPLIFIED_CHINESE] = u8"分辨率";
    _strings["resolution"][L::ENGLISH] = "Resolution";
    _strings["display_mode"][L::SIMPLIFIED_CHINESE] = u8"显示模式";
    _strings["display_mode"][L::ENGLISH] = "Display Mode";
    _strings["windowed"][L::SIMPLIFIED_CHINESE] = u8"窗口";
    _strings["windowed"][L::ENGLISH] = "Windowed";
    _strings["borderless"][L::SIMPLIFIED_CHINESE] = u8"无边框";
    _strings["borderless"][L::ENGLISH] = "Borderless";
    _strings["fullscreen"][L::SIMPLIFIED_CHINESE] = u8"全屏";
    _strings["fullscreen"][L::ENGLISH] = "Fullscreen";
    _strings["apply"][L::SIMPLIFIED_CHINESE] = u8"应用";
    _strings["apply"][L::ENGLISH] = "Apply";
    _strings["key_bindings"][L::SIMPLIFIED_CHINESE] = u8"按键设置";
    _strings["key_bindings"][L::ENGLISH] = "Key Bindings";
    _strings["language"][L::SIMPLIFIED_CHINESE] = u8"语言";
    _strings["language"][L::ENGLISH] = "Language";
    _strings["back"][L::SIMPLIFIED_CHINESE] = u8"返回";
    _strings["back"][L::ENGLISH] = "Back";
    _strings["restart_hint"][L::SIMPLIFIED_CHINESE] = u8"重启后生效";
    _strings["restart_hint"][L::ENGLISH] = "Takes effect after restart";
    _strings["press_key"][L::SIMPLIFIED_CHINESE] = u8"按下新按键...";
    _strings["press_key"][L::ENGLISH] = "Press new key...";
    _strings["rebind"][L::SIMPLIFIED_CHINESE] = u8"改键";
    _strings["rebind"][L::ENGLISH] = "Rebind";
    _strings["move_up"][L::SIMPLIFIED_CHINESE] = u8"向上移动";
    _strings["move_up"][L::ENGLISH] = "Move Up";
    _strings["move_down"][L::SIMPLIFIED_CHINESE] = u8"向下移动";
    _strings["move_down"][L::ENGLISH] = "Move Down";
    _strings["move_left"][L::SIMPLIFIED_CHINESE] = u8"向左移动";
    _strings["move_left"][L::ENGLISH] = "Move Left";
    _strings["move_right"][L::SIMPLIFIED_CHINESE] = u8"向右移动";
    _strings["move_right"][L::ENGLISH] = "Move Right";
    _strings["fire_key"][L::SIMPLIFIED_CHINESE] = u8"攻击";
    _strings["fire_key"][L::ENGLISH] = "Attack";
    _strings["pause_key"][L::SIMPLIFIED_CHINESE] = u8"暂停";
    _strings["pause_key"][L::ENGLISH] = "Pause";
    _strings["mouse_left"][L::SIMPLIFIED_CHINESE] = "O";
    _strings["mouse_left"][L::ENGLISH] = "O";
    _strings["confirm"][L::SIMPLIFIED_CHINESE] = u8"确定";
    _strings["confirm"][L::ENGLISH] = "OK";

    // ---- Story Mode / Save System ----
    _strings["story_mode"][L::SIMPLIFIED_CHINESE] = u8"故事模式";
    _strings["story_mode"][L::ENGLISH] = "Story Mode";

    _strings["new_game"][L::SIMPLIFIED_CHINESE] = u8"新游戏";
    _strings["new_game"][L::ENGLISH] = "New Game";

    _strings["load_game"][L::SIMPLIFIED_CHINESE] = u8"读取存档";
    _strings["load_game"][L::ENGLISH] = "Load Game";

    _strings["save_game"][L::SIMPLIFIED_CHINESE] = u8"保存进度";
    _strings["save_game"][L::ENGLISH] = "Save Progress";

    _strings["saved_hint"][L::SIMPLIFIED_CHINESE] = u8"已保存！";
    _strings["saved_hint"][L::ENGLISH] = "Saved!";

    _strings["auto_save"][L::SIMPLIFIED_CHINESE] = u8"自动存档";
    _strings["auto_save"][L::ENGLISH] = "Auto Save";

    _strings["manual_save"][L::SIMPLIFIED_CHINESE] = u8"手动存档";
    _strings["manual_save"][L::ENGLISH] = "Manual Save";

    _strings["no_saves"][L::SIMPLIFIED_CHINESE] = u8"没有存档数据";
    _strings["no_saves"][L::ENGLISH] = "No save data";

    _strings["save_full"][L::SIMPLIFIED_CHINESE] = u8"存档已满，请删除旧存档";
    _strings["save_full"][L::ENGLISH] = "Save slots full. Delete old saves.";

    _strings["delete_save_confirm"][L::SIMPLIFIED_CHINESE] = u8"确定删除此存档？";
    _strings["delete_save_confirm"][L::ENGLISH] = "Delete this save?";

    _strings["load"][L::SIMPLIFIED_CHINESE] = u8"载入";
    _strings["load"][L::ENGLISH] = "Load";

    _strings["level_info_fmt"][L::SIMPLIFIED_CHINESE] = u8"第 %d 关";
    _strings["level_info_fmt"][L::ENGLISH] = "Level %d";
}
