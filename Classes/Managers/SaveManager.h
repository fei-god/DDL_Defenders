#pragma once

#include "PlayerRecord.h"

#include <string>
#include <vector>

// ============================================================
// SaveManager.h
// ------------------------------------------------------------
// SaveManager handles binary random file processing for player
// profiles and leaderboard data.
// 
// Main course requirement points:
// 1. Binary file processing
// 2. Fixed-size struct PlayerRecord
// 3. Random reading with seekg()
// 4. Random writing/updating with seekp()
// ============================================================

class SaveManager
{
public:
    static SaveManager* getInstance();

    // Initialize save file. If the file does not exist, create it
    // and fill it with empty PlayerRecord slots.
    bool initSaveFile();

    // Create or reset one player record.
    bool createOrResetPlayer(int playerId, const char* playerName);

    // Read one player record randomly according to playerId.
    bool loadPlayerRecord(int playerId, PlayerRecord& outRecord);

    // Write one player record randomly according to record.playerId.
    bool savePlayerRecord(const PlayerRecord& record);

    // Update player profile after one game ends.
    // This is the main function GameScene should call.
    bool updatePlayerAfterGame(const GameResultData& gameData);

    // Load all valid player records.
    std::vector<PlayerRecord> loadAllPlayerRecords();

    // Load leaderboard records sorted by highScore from high to low.
    std::vector<PlayerRecord> loadLeaderboardByHighScore(int maxCount = 10);

    // Load leaderboard records sorted by rankPoints from high to low.
    std::vector<PlayerRecord> loadLeaderboardByRankPoints(int maxCount = 10);

    // Utility display functions.
    const char* getRankName(int rankLevel) const;
    const char* getRankChineseName(int rankLevel) const;

    // Save file path getter.
    const std::string& getSaveFilePath() const;

private:
    SaveManager();

    SaveManager(const SaveManager&) = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    bool isValidPlayerId(int playerId) const;

    PlayerRecord makeEmptyRecord(int playerId) const;
    PlayerRecord makeNewPlayerRecord(int playerId, const char* playerName) const;

    void updateRecordWithGameData(PlayerRecord& record, const GameResultData& gameData) const;
    void updateAchievements(PlayerRecord& record) const;
    void copyPlayerName(char dest[32], const char* src) const;
    void fillCurrentTime(char dest[20]) const;

private:
    static SaveManager* s_instance;

    // Keep this count fixed to make random file processing simple.
    // playerId should be in range [0, MAX_PLAYER_COUNT - 1].
    static const int MAX_PLAYER_COUNT = 100;

    // Save file name.
    // It will be created in the current working directory.
    // In Cocos2d-x, this is usually the running executable directory.
    std::string m_saveFilePath;
};
