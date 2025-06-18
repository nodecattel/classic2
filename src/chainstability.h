// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINSTABILITY_H
#define BITCOIN_CHAINSTABILITY_H

#include "chain.h"
#include "consensus/params.h"
#include <stdint.h>

class CBlockIndex;

/** Chain stability monitoring and protection functions */

/**
 * Check if the chain is at risk of getting stuck due to slow block times
 * Returns true if the last few blocks took significantly longer than expected
 */
bool IsChainStuck(const CBlockIndex* pindexLast, const Consensus::Params& params);

/**
 * Detect potential reorganization attacks by analyzing competing chains
 * Returns true if there are suspicious timing patterns that might indicate an attack
 */
bool DetectPotentialReorgAttack(const CBlockIndex* pindexLast, const Consensus::Params& params);

/**
 * Calculate the effective hash rate based on recent block times and difficulties
 * Returns estimated hash rate in hashes per second
 */
double EstimateNetworkHashRate(const CBlockIndex* pindexLast, const Consensus::Params& params, int nBlocks = 120);

/**
 * Check if emergency difficulty rules should be activated
 * Returns true if conditions warrant emergency difficulty reduction
 */
bool ShouldActivateEmergencyDifficulty(const CBlockIndex* pindexLast, const Consensus::Params& params);

/**
 * Log chain stability metrics for monitoring
 */
void LogChainStabilityMetrics(const CBlockIndex* pindexLast, const Consensus::Params& params);

#endif // BITCOIN_CHAINSTABILITY_H
