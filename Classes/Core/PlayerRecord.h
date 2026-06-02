#pragma once

#include <cstring>

// Player rank.
// Use int values for binary file compatibility.
enum class PlayerRank
{
    Bronze = 0,
    Silver = 1,
    Gold = 2,
    Platinum = 3,
    Diamond = 4,
    King = 5
};

// Game result.
enum class GameResult
{
    Lose = 0,
    Win = 1
};

// Runtime game result data.
// This struct is used to pass one game's result to SaveManager.
struct GameResultData
{
    int playerId;
    char playerName[32];

    int score;
    int survivalTime;
    int progress;
    int kills;
    int result;

    GameResultData()
    {
        playerId = 0;
        std::memset(playerName, 0, sizeof(playerName));

        score = 0;
        survivalTime = 0;
        progress = 0;
        kills = 0;
        result = static_cast<int>(GameResult::Lose);
    }
};

// Fixed-size player record.
// This struct is stored directly in binary file.
// Do not use std::string here.
struct PlayerRecord
{
    int isUsed;

    int playerId;
    char playerName[32];

    // Leaderboard data
    int highScore;
    int bestSurvivalTime;
    int bestProgress;
    int bestKills;

    // Accumulated data
    int totalGames;
    int totalWins;
    int totalKills;
    int totalSurvivalTime;

    // Rank system
    int rankLevel;
    int rankPoints;

    // Achievement bitmask
    int unlockedAchievements;

    // Last game data
    int lastScore;
    int lastSurvivalTime;
    int lastProgress;
    int lastKills;
    int lastResult;

    char lastPlayedTime[20];

    PlayerRecord()
    {
        isUsed = 0;

        playerId = 0;
        std::memset(playerName, 0, sizeof(playerName));

        highScore = 0;
        bestSurvivalTime = 0;
        bestProgress = 0;
        bestKills = 0;

        totalGames = 0;
        totalWins = 0;
        totalKills = 0;
        totalSurvivalTime = 0;

        rankLevel = static_cast<int>(PlayerRank::Bronze);
        rankPoints = 0;

        unlockedAchievements = 0;

        lastScore = 0;
        lastSurvivalTime = 0;
        lastProgress = 0;
        lastKills = 0;
        lastResult = static_cast<int>(GameResult::Lose);

        std::memset(lastPlayedTime, 0, sizeof(lastPlayedTime));
    }
};

// Calculate rank from rank points.
// This name must match SaveManager.cpp.
inline PlayerRank calculateRankFromPoints(int rankPoints)
{
    if (rankPoints >= 10000)
    {
        return PlayerRank::King;
    }

    if (rankPoints >= 7000)
    {
        return PlayerRank::Diamond;
    }

    if (rankPoints >= 4000)
    {
        return PlayerRank::Platinum;
    }

    if (rankPoints >= 2000)
    {
        return PlayerRank::Gold;
    }

    if (rankPoints >= 1000)
    {
        return PlayerRank::Silver;
    }

    return PlayerRank::Bronze;
}

// Convert rank to English text.
// This name must match SaveManager.cpp.
inline const char* rankToString(PlayerRank rank)
{
    switch (rank)
    {
    case PlayerRank::Bronze:
        return "Bronze";
    case PlayerRank::Silver:
        return "Silver";
    case PlayerRank::Gold:
        return "Gold";
    case PlayerRank::Platinum:
        return "Platinum";
    case PlayerRank::Diamond:
        return "Diamond";
    case PlayerRank::King:
        return "King";
    default:
        return "Unknown";
    }
}

// To avoid encoding problems, this version also returns English text.
// Later, UI students can translate these words into Chinese on the screen.
inline const char* rankToChineseString(PlayerRank rank)
{
    return rankToString(rank);
}

// Calculate earned rank points from one game's result.
// This name and parameter must match SaveManager.cpp.
inline int calculateEarnedRankPoints(const GameResultData& game)
{
    int points = 0;

    points += game.score / 10;
    points += game.survivalTime / 5;
    points += game.progress * 5;
    points += game.kills * 20;

    if (game.result == static_cast<int>(GameResult::Win))
    {
        points += 300;
    }

    if (points < 0)
    {
        points = 0;
    }

    return points;
}