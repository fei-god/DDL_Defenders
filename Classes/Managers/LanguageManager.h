#pragma once
#ifndef __LANGUAGE_MANAGER_H__
#define __LANGUAGE_MANAGER_H__

#include <string>
#include <map>
#include <vector>

class LanguageManager
{
public:
    enum class Language
    {
        SIMPLIFIED_CHINESE = 0,
        ENGLISH = 1,
        JAPANESE = 2,
        KOREAN = 3,

        COUNT
    };

    static LanguageManager* getInstance();
    static void destroyInstance();

    void setLanguage(Language lang);
    Language getLanguage() const;
    int getLanguageIndex() const;
    std::string getLanguageName() const;

    // Retrieve a localised string by key. Returns "[key]" if not found.
    std::string getString(const std::string& key) const;

    // Format a string with one float argument (for survival time etc.)
    std::string getStringF(const std::string& key, float value) const;


    // Convert a key-code integer (stored in UserDefault) to the enum
    static Language intToLanguage(int idx);

private:
    LanguageManager();
    ~LanguageManager();

    void initStrings();

    Language _currentLanguage;
    std::map<std::string, std::map<Language, std::string>> _strings;

    static LanguageManager* _instance;
};

#endif // __LANGUAGE_MANAGER_H__
