#!/bin/bash

SERVER_BINARY="../ircserv"
PORT="6667"
PASSWORD="testpass"

echo "Starting server..."
$SERVER_BINARY $PORT $PASSWORD &
SERVER_PID=$!
sleep 3

echo "Testing TOPIC command..."

response=$(timeout 10 bash -c "
{
    echo 'PASS testpass'
    echo 'NICK topictest'
    echo 'USER topictest 0 * :Topic Test'
    sleep 1
    echo 'JOIN #topictest'
    sleep 1
    echo 'TOPIC #topictest :My test topic'
    sleep 2
    echo 'QUIT'
} | nc 127.0.0.1 $PORT
")

echo "=== FULL RESPONSE ==="
echo "$response"
echo "====================="

echo "Checking for TOPIC responses..."
echo "$response" | grep -i topic || echo "No TOPIC found"
echo "$response" | grep "332" || echo "No 332 found"

kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null