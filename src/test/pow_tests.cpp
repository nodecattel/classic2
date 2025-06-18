// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "chainparams.h"
#include "pow.h"
#include "random.h"
#include "util.h"
#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1262152739;  // Block #32255
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1d00d86a);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1233061996;  // Block #2015
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1d00ffff);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1279297671;  // Block #68543
    pindexLast.nBits = 0x1c05a3f4;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1c0168fd);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1269211443;  // Block #46367
    pindexLast.nBits = 0x1c387f6f;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1d00e1fd);
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus();

    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : NULL;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1269211443 + i * params.nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[GetRand(10000)];
        CBlockIndex *p2 = &blocks[GetRand(10000)];
        CBlockIndex *p3 = &blocks[GetRand(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, params);
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

// Helper function to create a mock block index for new tests
CBlockIndex* CreateMockBlockIndex(int height, uint32_t nBits, int64_t nTime, CBlockIndex* pprev = nullptr)
{
    CBlockIndex* pindex = new CBlockIndex();
    pindex->nHeight = height;
    pindex->nBits = nBits;
    pindex->nTime = nTime;
    pindex->pprev = pprev;
    return pindex;
}

BOOST_AUTO_TEST_CASE(new_difficulty_algorithm_basic)
{
    SelectParams(CBaseChainParams::MAIN);
    Consensus::Params params = Params().GetConsensus();

    // Set up parameters for new algorithm
    params.nNewPowDiffHeight = 100;
    params.nPowAveragingWindow = 17;
    params.nPowMaxAdjustDown = 32;
    params.nPowMaxAdjustUp = 16;
    params.nPostBlossomPowTargetSpacing = 60;

    // Create a chain of blocks with consistent timing
    CBlockIndex* pindexPrev = nullptr;
    std::vector<CBlockIndex*> blocks;

    uint32_t initialBits = 0x1d00ffff; // Some reasonable difficulty
    int64_t baseTime = 1000000;

    // Create 20 blocks with perfect timing (60 seconds apart)
    for (int i = 0; i < 20; i++) {
        CBlockIndex* pindex = CreateMockBlockIndex(
            150 + i, // Height > nNewPowDiffHeight
            initialBits,
            baseTime + i * 60, // Perfect 60-second spacing
            pindexPrev
        );
        blocks.push_back(pindex);
        pindexPrev = pindex;
    }

    // Test that difficulty remains stable with perfect timing
    CBlockHeader header;
    header.nTime = baseTime + 20 * 60;

    uint32_t newBits = GetNextWorkRequiredNew(blocks.back(), &header, params);

    // With perfect timing, difficulty should remain approximately the same
    arith_uint256 oldTarget, newTarget;
    oldTarget.SetCompact(initialBits);
    newTarget.SetCompact(newBits);

    // Allow for small variations due to rounding
    BOOST_CHECK(newTarget <= oldTarget * 105 / 100); // No more than 5% increase
    BOOST_CHECK(newTarget >= oldTarget * 95 / 100);  // No more than 5% decrease

    // Clean up
    for (auto* block : blocks) {
        delete block;
    }
}

BOOST_AUTO_TEST_CASE(emergency_difficulty_activation)
{
    SelectParams(CBaseChainParams::MAIN);
    Consensus::Params params = Params().GetConsensus();

    params.nNewPowDiffHeight = 100;
    params.nPostBlossomPowTargetSpacing = 60;

    // Create a block
    CBlockIndex* pindex = CreateMockBlockIndex(150, 0x1d00ffff, 1000000);

    // Create a header with timestamp 7 minutes (420 seconds) after last block
    // This is > 6 * 60 = 360 seconds, so should trigger emergency difficulty
    CBlockHeader header;
    header.nTime = 1000000 + 420;

    uint32_t newBits = GetNextWorkRequiredNew(pindex, &header, params);
    uint32_t maxBits = UintToArith256(params.powLimit).GetCompact();

    // Should return maximum difficulty (minimum work)
    BOOST_CHECK_EQUAL(newBits, maxBits);

    delete pindex;
}

BOOST_AUTO_TEST_CASE(permitted_difficulty_transition_new_algo)
{
    SelectParams(CBaseChainParams::MAIN);
    Consensus::Params params = Params().GetConsensus();

    params.nNewPowDiffHeight = 100;
    params.nPowMaxAdjustDown = 32;
    params.nPowMaxAdjustUp = 16;
    params.fPowAllowMinDifficultyBlocks = false;

    uint32_t oldBits = 0x1d00ffff;
    arith_uint256 oldTarget;
    oldTarget.SetCompact(oldBits);

    // Test valid adjustment within limits
    arith_uint256 validTarget = oldTarget * 110 / 100; // 10% increase (within 32% limit)
    uint32_t validBits = validTarget.GetCompact();

    BOOST_CHECK(PermittedDifficultyTransition(params, 150, oldBits, validBits));

    // Test invalid adjustment beyond limits
    arith_uint256 invalidTarget = oldTarget * 150 / 100; // 50% increase (beyond 32% limit)
    uint32_t invalidBits = invalidTarget.GetCompact();

    BOOST_CHECK(!PermittedDifficultyTransition(params, 150, oldBits, invalidBits));
}

BOOST_AUTO_TEST_SUITE_END()
