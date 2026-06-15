#pragma once
#ifndef __ASSET_PATHS_H__
#define __ASSET_PATHS_H__

#include "platform/CCFileUtils.h"
#include <string>

namespace AssetPaths
{
    inline std::string resolve(const std::string& preferredPath)
    {
        auto* fileUtils = cocos2d::FileUtils::getInstance();
        if (fileUtils->isFileExist(preferredPath))
        {
            return preferredPath;
        }

        const std::string doublePng = preferredPath + ".png";
        if (fileUtils->isFileExist(doublePng))
        {
            return doublePng;
        }

        const std::string jpgPath = preferredPath.substr(0, preferredPath.find_last_of('.')) + ".jpg";
        if (fileUtils->isFileExist(jpgPath))
        {
            return jpgPath;
        }

        const std::string jpegPath = preferredPath.substr(0, preferredPath.find_last_of('.')) + ".jpeg";
        if (fileUtils->isFileExist(jpegPath))
        {
            return jpegPath;
        }

        return "";
    }

    inline bool exists(const std::string& preferredPath)
    {
        return !resolve(preferredPath).empty();
    }
}

#endif
