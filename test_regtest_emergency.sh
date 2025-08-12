#!/bin/bash
# Bitcoin Classic Emergency Difficulty Adjustment Regtest Integration Test
# This script tests the emergency rules in a controlled regtest environment

set -e

echo "=========================================="
echo "Bitcoin Classic Emergency Rules Test"
echo "=========================================="

# Configuration
DATADIR="./regtest_emergency_test"
BITCOIND="./src/bitcoind"
BITCOIN_CLI="./src/bitcoin-cli"

# Check if binaries exist
if [ ! -f "$BITCOIND" ]; then
    echo "❌ bitcoind not found at $BITCOIND"
    echo "Please build Bitcoin Classic first: make"
    exit 1
fi

if [ ! -f "$BITCOIN_CLI" ]; then
    echo "❌ bitcoin-cli not found at $BITCOIN_CLI"
    echo "Please build Bitcoin Classic first: make"
    exit 1
fi

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    if [ -f "$DATADIR/regtest/bitcoind.pid" ]; then
        $BITCOIN_CLI -datadir="$DATADIR" -regtest stop 2>/dev/null || true
        sleep 2
    fi
    rm -rf "$DATADIR"
}

# Set trap for cleanup
trap cleanup EXIT

# Create test directory
mkdir -p "$DATADIR"

echo "Starting bitcoind in regtest mode..."
$BITCOIND -datadir="$DATADIR" -regtest -daemon -server \
    -rpcuser=test -rpcpassword=test -rpcport=18443 \
    -port=18444 -connect=0 -listen=0

# Wait for bitcoind to start
sleep 3

# Test basic functionality
echo "Testing basic RPC connectivity..."
BLOCK_COUNT=$($BITCOIN_CLI -datadir="$DATADIR" -regtest -rpcuser=test -rpcpassword=test getblockcount)
echo "✅ Current block count: $BLOCK_COUNT"

# Generate some blocks to get past the initial difficulty setup
echo "Generating initial blocks..."
$BITCOIN_CLI -datadir="$DATADIR" -regtest -rpcuser=test -rpcpassword=test generate 150

# Get current difficulty
CURRENT_DIFF=$($BITCOIN_CLI -datadir="$DATADIR" -regtest -rpcuser=test -rpcpassword=test getdifficulty)
echo "✅ Current difficulty: $CURRENT_DIFF"

# Get current block info
BLOCK_INFO=$($BITCOIN_CLI -datadir="$DATADIR" -regtest -rpcuser=test -rpcpassword=test getblockchaininfo)
echo "✅ Blockchain info retrieved"

echo ""
echo "=========================================="
echo "Emergency Rules Test Instructions"
echo "=========================================="
echo ""
echo "To test emergency rules manually:"
echo "1. Stop the daemon: $BITCOIN_CLI -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test stop"
echo "2. Modify block timestamps in the data directory to simulate delays"
echo "3. Restart and observe difficulty adjustments"
echo ""
echo "Or use the following commands to inspect the current state:"
echo ""
echo "Get difficulty:"
echo "$BITCOIN_CLI -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test getdifficulty"
echo ""
echo "Get latest block:"
echo "$BITCOIN_CLI -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test getbestblockhash"
echo ""
echo "Get block info:"
echo "$BITCOIN_CLI -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test getblock \$(bitcoin-cli -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test getbestblockhash)"
echo ""
echo "Generate more blocks:"
echo "$BITCOIN_CLI -datadir=\"$DATADIR\" -regtest -rpcuser=test -rpcpassword=test generate 1"
echo ""

# Keep the daemon running for manual testing
echo "Regtest environment is ready!"
echo "Data directory: $DATADIR"
echo "Press Ctrl+C to stop and cleanup"

# Wait for user interrupt
while true; do
    sleep 1
done
