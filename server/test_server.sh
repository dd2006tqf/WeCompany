#!/bin/bash

# WeCompany Server Test Script
# This script tests the TCP server connection

SERVER_HOST="${1:-127.0.0.1}"
SERVER_PORT="${2:-8888}"

echo "======================================="
echo "WeCompany Server Connection Test"
echo "======================================="
echo "Server: $SERVER_HOST:$SERVER_PORT"
echo ""

# Test 1: Check if server port is open
echo "Test 1: Checking if server is listening..."
if command -v nc &> /dev/null; then
    timeout 2 nc -zv $SERVER_HOST $SERVER_PORT 2>&1
    if [ $? -eq 0 ]; then
        echo "✓ Server is listening on port $SERVER_PORT"
    else
        echo "✗ Cannot connect to server on port $SERVER_PORT"
        exit 1
    fi
else
    echo "Warning: netcat (nc) not found, skipping port check"
fi

echo ""

# Test 2: Send test connection
echo "Test 2: Testing TCP connection..."
if command -v telnet &> /dev/null; then
    (echo "test"; sleep 1) | telnet $SERVER_HOST $SERVER_PORT 2>&1 | head -5
else
    echo "Warning: telnet not found, skipping connection test"
fi

echo ""
echo "======================================="
echo "Test completed"
echo "======================================="
