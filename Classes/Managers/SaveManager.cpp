#include "SaveManager.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>

SaveManager* SaveManager::s_instance = nullptr;

SaveManager* SaveManager::getInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new SaveManager();
    }
    return s_instance;
}

SaveManager::SaveManager()
    : m_saveFilePath("player_records.dat")
{
}

const std::string& SaveManager::getSaveFilePath() const
{
    return m_saveFilePath;
}

bool SaveManager::isValidPlayerId(int playerId) const
{
    return playerId >= 0 && playerId < MAX_PLAYER_COUNT;
}

PlayerRecord SaveManager::makeEmptyRecord(int playerId) const
{
    PlayerRecord record;
    std::memset(&record, 0, sizeof(PlayerRecord));

    record.isUsed = 0;
    record.playerId = playerId;
    record.rankLevel = static_cast<int>(PlayerRank::Bronze);
    record.rankPoints = 0;

    return record;
}

PlayerRecord SaveManager::makeNewPlayerRecord(int playerId, const char* playerName) const
{
    PlayerRecord record;
    std::memset(&record, 0, sizeof(PlayerRecord));

    record.isUsed = 1;
    record.playerId = playerId;
    copyPlayerName(record.playerName, playerName);

    record.highScore = 0;
    record.bestSurvivalTime = 0;
    record.bestProgress = 0;
    record.bestKills = 0;

    record.totalGames = 0;
    record.totalWins = 0;
    record.totalKills = 0;
    record.totalSurvivalTime = 0;

    record.rankLevel = static_cast<int>(PlayerRank::Bronze);
    record.rankPoints = 0;

    record.unlockedAchievements = 0;

    record.lastScore = 0;
    record.lastSurvivalTime = 0;
    record.lastProgress = 0;
    record.lastKills = 0;
    record.lastResult = static_cast<int>(GameResult::Lose);
    copyPlayerName(record.lastPlayedTime, "N/A");

    return record;
}

void SaveManager::copyPlayerName(char dest[32], const char* src) const
{
    std::memset(dest, 0, 32);

    if (src == nullptr)
    {
        std::strncpy(dest, "Player", 31);
        return;
    }

    std::strncpy(dest, src, 31);
    dest[31] = '\0';
}

void SaveManager::fillCurrentTime(char dest[20]) const
{
    std::memset(dest, 0, 20);

    std::time_t now = std::time(nullptr);
    std::tm localTime;

#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::strftime(dest, 20, "%Y-%m-%d %H:%M", &localTime);
    dest[19] = '\0';
}

bool SaveManager::initSaveFile()
{
    // Check if file exists and has enough size.
    {
        std::ifstream input(m_saveFilePath, std::ios::binary);
        if (input.is_open())
        {
            input.seekg(0, std::ios::end);
            std::streamoff fileSize = input.tellg();
            const std::streamoff expectedSize =
                static_cast<std::streamoff>(MAX_PLAYER_COUNT) * sizeof(PlayerRecord);

            if (fileSize >= expectedSize)
            {
                return true;
            }
        }
    }

    // Create or rebuild save file with empty records.
    std::ofstream output(m_saveFilePath, std::ios::binary | std::ios::trunc);
    if (!output.is_open())
    {
        std::cerr << "[SaveManager] Failed to create save file: "
                  << m_saveFilePath << std::endl;
        return false;
    }

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
    {
        PlayerRecord emptyRecord = makeEmptyRecord(i);
        output.write(reinterpret_cast<const char*>(&emptyRecord), sizeof(PlayerRecord));
    }

    output.close();
    return true;
}

bool SaveManager::createOrResetPlayer(int playerId, const char* playerName)
{
    if (!isValidPlayerId(playerId))
    {
        std::cerr << "[SaveManager] Invalid playerId: " << playerId << std::endl;
        return false;
    }

    if (!initSaveFile())
    {
        return false;
    }

    PlayerRecord record = makeNewPlayerRecord(playerId, playerName);
    return savePlayerRecord(record);
}

bool SaveManager::loadPlayerRecord(int playerId, PlayerRecord& outRecord)
{
    if (!isValidPlayerId(playerId))
    {
        std::cerr << "[SaveManager] Invalid playerId: " << playerId << std::endl;
        return false;
    }

    if (!initSaveFile())
    {
        return false;
    }

    std::ifstream input(m_saveFilePath, std::ios::binary);
    if (!input.is_open())
    {
        std::cerr << "[SaveManager] Failed to open save file for reading: "
                  << m_saveFilePath << std::endl;
        return false;
    }

    // Random read: directly jump to the target player's byte offset.
    std::streamoff offset =
        static_cast<std::streamoff>(playerId) * sizeof(PlayerRecord);

    input.seekg(offset, std::ios::beg);
    input.read(reinterpret_cast<char*>(&outRecord), sizeof(PlayerRecord));

    bool ok = input.good();
    input.close();

    return ok;
}

bool SaveManager::savePlayerRecord(const PlayerRecord& record)
{
    if (!isValidPlayerId(record.playerId))
    {
        std::cerr << "[SaveManager] Invalid playerId: " << record.playerId << std::endl;
        return false;
    }

    if (!initSaveFile())
    {
        return false;
    }

    std::fstream file(m_saveFilePath,
                      std::ios::binary | std::ios::in | std::ios::out);

    if (!file.is_open())
    {
        std::cerr << "[SaveManager] Failed to open save file for writing: "
                  << m_saveFilePath << std::endl;
        return false;
    }

    // Random write: directly jump to the target player's byte offset.
    std::streamoff offset =
        static_cast<std::streamoff>(record.playerId) * sizeof(PlayerRecord);

    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(PlayerRecord));

    bool ok = file.good();
    file.close();

    return ok;
}

void SaveManager::updateRecordWithGameData(PlayerRecord& record,
                                           const GameResultData& gameData) const
{
    record.isUsed = 1;
    record.playerId = gameData.playerId;
    copyPlayerName(record.playerName, gameData.playerName);

    // Update accumulated data.
    record.totalGames += 1;
    record.totalKills += gameData.kills;
    record.totalSurvivalTime += gameData.survivalTime;

    if (gameData.result == static_cast<int>(GameResult::Win))
    {
        record.totalWins += 1;
    }

    // Update best data for leaderboard.
    if (gameData.score > record.highScore)
    {
        record.highScore = gameData.score;
    }

    if (gameData.survivalTime > record.bestSurvivalTime)
    {
        record.bestSurvivalTime = gameData.survivalTime;
    }

    if (gameData.progress > record.bestProgress)
    {
        record.bestProgress = gameData.progress;
    }

    if (gameData.kills > record.bestKills)
    {
        record.bestKills = gameData.kills;
    }

    // Update rank points and rank level.
    int earnedPoints = calculateEarnedRankPoints(gameData);
    record.rankPoints += earnedPoints;
    record.rankLevel = static_cast<int>(calculateRankFromPoints(record.rankPoints));

    // Update last game overview.
    record.lastScore = gameData.score;
    record.lastSurvivalTime = gameData.survivalTime;
    record.lastProgress = gameData.progress;
    record.lastKills = gameData.kills;
    record.lastResult = gameData.result;
    fillCurrentTime(record.lastPlayedTime);

    // Update achievements.
    updateAchievements(record);
}

void SaveManager::updateAchievements(PlayerRecord& record) const
{
    // bit 0: First Win
    if (record.totalWins >= 1)
    {
        record.unlockedAchievements |= (1 << 0);
    }

    // bit 1: Kill 100 enemies in total
    if (record.totalKills >= 100)
    {
        record.unlockedAchievements |= (1 << 1);
    }

    // bit 2: Reach 10000 high score
    if (record.highScore >= 10000)
    {
        record.unlockedAchievements |= (1 << 2);
    }

    // bit 3: Reach 100 assignment progress
    if (record.bestProgress >= 100)
    {
        record.unlockedAchievements |= (1 << 3);
    }

    // bit 4: Become King rank
    if (record.rankLevel >= static_cast<int>(PlayerRank::King))
    {
        record.unlockedAchievements |= (1 << 4);
    }
}

bool SaveManager::updatePlayerAfterGame(const GameResultData& gameData)
{
    if (!isValidPlayerId(gameData.playerId))
    {
        std::cerr << "[SaveManager] Invalid playerId: " << gameData.playerId << std::endl;
        return false;
    }

    if (!initSaveFile())
    {
        return false;
    }

    PlayerRecord record;
    bool loaded = loadPlayerRecord(gameData.playerId, record);

    if (!loaded || record.isUsed == 0)
    {
        record = makeNewPlayerRecord(gameData.playerId, gameData.playerName);
    }

    updateRecordWithGameData(record, gameData);

    return savePlayerRecord(record);
}

std::vector<PlayerRecord> SaveManager::loadAllPlayerRecords()
{
    std::vector<PlayerRecord> records;

    if (!initSaveFile())
    {
        return records;
    }

    std::ifstream input(m_saveFilePath, std::ios::binary);
    if (!input.is_open())
    {
        std::cerr << "[SaveManager] Failed to open save file for reading all records: "
                  << m_saveFilePath << std::endl;
        return records;
    }

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
    {
        PlayerRecord record;
        input.read(reinterpret_cast<char*>(&record), sizeof(PlayerRecord));

        if (!input.good())
        {
            break;
        }

        if (record.isUsed == 1)
        {
            records.push_back(record);
        }
    }

    input.close();
    return records;
}

std::vector<PlayerRecord> SaveManager::loadLeaderboardByHighScore(int maxCount)
{
    std::vector<PlayerRecord> records = loadAllPlayerRecords();

    std::sort(records.begin(), records.end(),
              [](const PlayerRecord& a, const PlayerRecord& b)
              {
                  if (a.highScore != b.highScore)
                  {
                      return a.highScore > b.highScore;
                  }
                  return a.rankPoints > b.rankPoints;
              });

    if (maxCount > 0 && static_cast<int>(records.size()) > maxCount)
    {
        records.resize(maxCount);
    }

    return records;
}

std::vector<PlayerRecord> SaveManager::loadLeaderboardByRankPoints(int maxCount)
{
    std::vector<PlayerRecord> records = loadAllPlayerRecords();

    std::sort(records.begin(), records.end(),
              [](const PlayerRecord& a, const PlayerRecord& b)
              {
                  if (a.rankPoints != b.rankPoints)
                  {
                      return a.rankPoints > b.rankPoints;
                  }
                  return a.highScore > b.highScore;
              });

    if (maxCount > 0 && static_cast<int>(records.size()) > maxCount)
    {
        records.resize(maxCount);
    }

    return records;
}

const char* SaveManager::getRankName(int rankLevel) const
{
    return rankToString(static_cast<PlayerRank>(rankLevel));
}

const char* SaveManager::getRankChineseName(int rankLevel) const
{
    return rankToChineseString(static_cast<PlayerRank>(rankLevel));
}
