// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_FASTBLOCK_H
#define BITCOIN_FASTBLOCK_H

#include "primitives/block.h"
#include "chain.h"
#include "consensus/params.h"

/** Fast block detection and discouragement system (non-consensus) */

/**
 * Check if a block came too quickly after the previous block
 * This is used for network-level discouragement, not consensus validation
 */
bool IsFastBlock(const CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& params);

/**
 * Calculate a "discouragement score" for fast blocks
 * Higher scores mean the block should be deprioritized in relay/mining
 * Returns 0 for normal blocks, higher values for faster blocks
 */
int GetFastBlockScore(const CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& params);

/**
 * Check if we should relay a block based on its timing
 * Returns true if block should be relayed normally
 * Returns false if block should be delayed or deprioritized
 */
bool ShouldRelayBlock(const CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& params);

/**
 * Get relay delay for fast blocks (in seconds)
 * Fast blocks get delayed relay to discourage them
 */
int GetRelayDelay(const CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& params);

#endif // BITCOIN_FASTBLOCK_H
