# Difficulty Adjustment and Chain Stability

This document describes the enhanced difficulty adjustment system implemented to solve chain stability issues and prevent reorganization attacks.

## Problem Statement

The original Bitcoin difficulty adjustment algorithm has several vulnerabilities:

1. **Chain Stuck Problem**: When miners with high hash power join the network, difficulty increases. When they suddenly leave, remaining miners cannot produce blocks fast enough, causing the chain to be stuck for days or weeks.

2. **Reorganization Attack Vulnerability**: A stuck chain is vulnerable to reorg attacks where an attacker can privately mine blocks and then use rented hash power to push their private chain to become the main chain.

## Solution: Bellcoin v3 Style Difficulty Adjustment

### Key Features

1. **Averaging Window**: Uses a 17-block averaging window instead of 2016 blocks, making the system much more responsive to hash rate changes.

2. **Asymmetric Adjustment Limits**: 
   - Maximum increase: 16% per adjustment
   - Maximum decrease: 32% per adjustment
   - This asymmetry allows faster recovery when miners leave

3. **Emergency Difficulty Rules**: If a block takes more than 6x the target time (60 minutes for 10-minute blocks), the next block can be mined at minimum difficulty, preventing permanent chain stalls.

4. **Dampened Adjustment**: Uses a dampening factor to smooth out rapid changes and prevent oscillation.

### Configuration Parameters

- `nNewPowDiffHeight`: Block height at which the new algorithm activates (122290 for mainnet)
- `nPowAveragingWindow`: Number of blocks to average (17)
- `nPowMaxAdjustDown`: Maximum percentage decrease (32%)
- `nPowMaxAdjustUp`: Maximum percentage increase (16%)
- `nPostBlossomPowTargetSpacing`: Target block time in seconds (600 = 10 minutes)

### Algorithm Details

1. **Target Calculation**: The new target is calculated based on the average of the last 17 blocks' difficulties.

2. **Time Adjustment**: The actual time span is dampened using the formula:
   ```
   adjusted_timespan = averaging_window_timespan + (actual_timespan - averaging_window_timespan) / 4
   ```

3. **Limit Application**: The adjustment is clamped to the maximum allowed percentage changes.

4. **Emergency Rule**: If the time since the last block exceeds 6x target spacing, minimum difficulty is allowed.

### Chain Stability Monitoring

The system includes monitoring functions to detect:

- Chain stuck conditions (slow block times)
- Potential reorganization attacks (suspicious timing patterns)
- Network hash rate estimation
- Emergency difficulty activation needs

### Benefits

1. **Prevents Chain Stalls**: Emergency rules ensure the chain never gets permanently stuck
2. **Faster Recovery**: Asymmetric limits allow quicker difficulty reduction when miners leave
3. **Attack Resistance**: Shorter adjustment window reduces the window for reorganization attacks
4. **Stability**: Dampening prevents wild oscillations in difficulty

### Backward Compatibility

The system maintains full backward compatibility:
- Old algorithm is used for blocks before `nNewPowDiffHeight`
- New algorithm is used for blocks at or after `nNewPowDiffHeight`
- All existing blocks remain valid

### Testing

Comprehensive tests are included to verify:
- Basic difficulty adjustment behavior
- Emergency difficulty activation
- Adjustment limit enforcement
- Algorithm switching at the activation height
- Difficulty transition validation

## Usage

The new system activates automatically at the configured block height. No user intervention is required. The system will:

1. Monitor chain stability continuously
2. Apply emergency rules when needed
3. Log stability metrics for analysis
4. Validate all difficulty transitions

For development and testing, the activation height can be configured in the chain parameters.
