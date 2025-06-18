// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainstability.h"
#include "chain.h"
#include "arith_uint256.h"
#include "util.h"
#include "utiltime.h"
#include <algorithm>
#include <vector>

bool IsChainStuck(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    if (!pindexLast) return false;
    
    // Check if we're using the new difficulty algorithm
    if (pindexLast->nHeight < params.nNewPowDiffHeight) {
        // For old algorithm, check if last block took more than 4x target time
        int64_t nTimeDiff = GetTime() - pindexLast->GetBlockTime();
        return nTimeDiff > params.nPowTargetSpacing * 4;
    } else {
        // For new algorithm, check if last block took more than 3x target time
        int64_t nTimeDiff = GetTime() - pindexLast->GetBlockTime();
        return nTimeDiff > params.nPostBlossomPowTargetSpacing * 3;
    }
}

bool DetectPotentialReorgAttack(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    if (!pindexLast || pindexLast->nHeight < 100) return false;
    
    // Look for suspicious patterns in recent blocks
    const CBlockIndex* pindex = pindexLast;
    std::vector<int64_t> blockTimes;
    std::vector<uint32_t> difficulties;
    
    // Collect data from last 20 blocks
    for (int i = 0; i < 20 && pindex; i++) {
        blockTimes.push_back(pindex->GetBlockTime());
        difficulties.push_back(pindex->nBits);
        pindex = pindex->pprev;
    }
    
    if (blockTimes.size() < 10) return false;
    
    // Check for rapid succession of blocks (possible private mining)
    int rapidBlocks = 0;
    for (size_t i = 1; i < blockTimes.size(); i++) {
        int64_t timeDiff = blockTimes[i-1] - blockTimes[i];
        int64_t targetSpacing = (pindexLast->nHeight >= params.nNewPowDiffHeight) ? 
                               params.nPostBlossomPowTargetSpacing : params.nPowTargetSpacing;
        
        if (timeDiff < targetSpacing / 3) {
            rapidBlocks++;
        }
    }
    
    // If more than 30% of recent blocks came too quickly, it might be an attack
    return rapidBlocks > (blockTimes.size() * 3 / 10);
}

double EstimateNetworkHashRate(const CBlockIndex* pindexLast, const Consensus::Params& params, int nBlocks)
{
    if (!pindexLast || pindexLast->nHeight < nBlocks) return 0.0;
    
    const CBlockIndex* pindexFirst = pindexLast;
    arith_uint256 totalWork = 0;
    
    // Go back nBlocks
    for (int i = 0; i < nBlocks && pindexFirst->pprev; i++) {
        arith_uint256 blockWork;
        blockWork.SetCompact(pindexFirst->nBits);
        totalWork += blockWork;
        pindexFirst = pindexFirst->pprev;
    }
    
    int64_t timeDiff = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    if (timeDiff <= 0) return 0.0;
    
    // Convert work to hash rate estimate
    if (nBlocks == 0) return 0.0;

    arith_uint256 avgWork = totalWork / nBlocks;
    // Convert to double by getting the compact representation and converting
    double workDouble = avgWork.GetCompact();
    double hashRate = workDouble / timeDiff * nBlocks;

    return hashRate;
}

bool ShouldActivateEmergencyDifficulty(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    if (!pindexLast) return false;
    
    // Only for new difficulty algorithm
    if (pindexLast->nHeight < params.nNewPowDiffHeight) return false;
    
    // Check if last block took more than 6x target time
    int64_t nTimeDiff = GetTime() - pindexLast->GetBlockTime();
    return nTimeDiff > params.nPostBlossomPowTargetSpacing * 6;
}

void LogChainStabilityMetrics(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    if (!pindexLast) return;
    
    bool isStuck = IsChainStuck(pindexLast, params);
    bool potentialAttack = DetectPotentialReorgAttack(pindexLast, params);
    double hashRate = EstimateNetworkHashRate(pindexLast, params);
    bool emergencyNeeded = ShouldActivateEmergencyDifficulty(pindexLast, params);
    
    int64_t timeSinceLastBlock = GetTime() - pindexLast->GetBlockTime();
    int64_t targetSpacing = (pindexLast->nHeight >= params.nNewPowDiffHeight) ? 
                           params.nPostBlossomPowTargetSpacing : params.nPowTargetSpacing;
    
    LogPrintf("Chain Stability Metrics: Height=%d, TimeSinceLastBlock=%ds (target=%ds), "
              "HashRate=%.2e H/s, Stuck=%s, PotentialAttack=%s, EmergencyNeeded=%s\n",
              pindexLast->nHeight, timeSinceLastBlock, targetSpacing, hashRate,
              isStuck ? "YES" : "NO", potentialAttack ? "YES" : "NO", 
              emergencyNeeded ? "YES" : "NO");
}
