#!/bin/bash

# Multi-client IRC integration tests
# Tests interaction between multiple irssi clients

SERVER_BINARY="../ircserv"
PORT="6667"
PASSWORD="testpass"
SERVER_PID=""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

start_server() {
    echo -e "${YELLOW}Starting IRC server...${NC}"
    $SERVER_BINARY $PORT $PASSWORD &
    SERVER_PID=$!
    sleep 3
    
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}✗ Failed to start server${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Server started${NC}"
}

stop_server() {
    if [ -n "$SERVER_PID" ] && kill -0 $SERVER_PID 2>/dev/null; then
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null
        echo -e "${GREEN}✓ Server stopped${NC}"
    fi
}

# Test channel communication between multiple clients
test_channel_communication() {
    echo -e "${YELLOW}Testing channel communication between clients...${NC}"
    
    local test_channel="#multichat"
    local test_message="Hello from client1!"
    local log1="/tmp/irc_test_client1.log"
    local log2="/tmp/irc_test_client2.log"
    
    # Clean up old logs
    rm -f $log1 $log2
    
    # Start first client
    expect -c "
        set timeout 20
        log_file $log1
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick client1\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        
        expect \"*$test_channel*\"
        sleep 3
        send \"/msg $test_channel $test_message\\r\"
        sleep 3
        send \"/quit\\r\"
        expect eof
    " &
    
    local client1_pid=$!
    sleep 5
    
    # Start second client
    expect -c "
        set timeout 15
        log_file $log2
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick client2\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        
        expect \"*$test_channel*\"
        sleep 5
        send \"/quit\\r\"
        expect eof
    " &
    
    local client2_pid=$!
    
    # Wait for both clients to finish
    wait $client1_pid
    wait $client2_pid
    
    # Check if client2 received client1's message
    if [ -f "$log2" ] && grep -q "$test_message" "$log2"; then
        echo -e "${GREEN}✓ Channel communication test passed${NC}"
        rm -f $log1 $log2
        return 0
    else
        echo -e "${RED}✗ Channel communication test failed${NC}"
        echo -e "${BLUE}Client1 log:${NC}"
        [ -f "$log1" ] && cat "$log1" | tail -10
        echo -e "${BLUE}Client2 log:${NC}"
        [ -f "$log2" ] && cat "$log2" | tail -10
        rm -f $log1 $log2
        return 1
    fi
}

# Test private message between clients
test_private_messages() {
    echo -e "${YELLOW}Testing private messages between clients...${NC}"
    
    local test_message="Private hello!"
    local log1="/tmp/irc_test_priv1.log"
    local log2="/tmp/irc_test_priv2.log"
    
    rm -f $log1 $log2
    
    # Start receiver client first
    expect -c "
        set timeout 20
        log_file $log2
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick receiver\\r\"
        
        expect \"*Welcome*\"
        sleep 8
        send \"/quit\\r\"
        expect eof
    " &
    
    local receiver_pid=$!
    sleep 5
    
    # Start sender client
    expect -c "
        set timeout 15
        log_file $log1
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick sender\\r\"
        
        expect \"*Welcome*\"
        sleep 2
        send \"/msg receiver $test_message\\r\"
        sleep 2
        send \"/quit\\r\"
        expect eof
    " &
    
    local sender_pid=$!
    
    wait $sender_pid
    wait $receiver_pid
    
    # Check if receiver got the private message
    if [ -f "$log2" ] && grep -q "$test_message" "$log2"; then
        echo -e "${GREEN}✓ Private message test passed${NC}"
        rm -f $log1 $log2
        return 0
    else
        echo -e "${RED}✗ Private message test failed${NC}"
        echo -e "${BLUE}Sender log:${NC}"
        [ -f "$log1" ] && cat "$log1" | tail -10
        echo -e "${BLUE}Receiver log:${NC}"
        [ -f "$log2" ] && cat "$log2" | tail -10
        rm -f $log1 $log2
        return 1
    fi
}

# Test nickname conflicts
test_nick_conflicts() {
    echo -e "${YELLOW}Testing nickname conflicts...${NC}"
    
    local log1="/tmp/irc_test_nick1.log"
    local log2="/tmp/irc_test_nick2.log"
    
    rm -f $log1 $log2
    
    # Start first client with specific nick
    expect -c "
        set timeout 15
        log_file $log1
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick samenick\\r\"
        
        expect \"*Welcome*\"
        sleep 5
        send \"/quit\\r\"
        expect eof
    " &
    
    local client1_pid=$!
    sleep 3
    
    # Start second client with same nick
    expect -c "
        set timeout 12
        log_file $log2
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick samenick\\r\"
        
        expect {
            \"*433*\" {
                send \"/nick samenick2\\r\"
                expect \"*Welcome*\"
                sleep 2
                send \"/quit\\r\"
                expect eof
                exit 0
            }
            \"*Welcome*\" {
                send \"/quit\\r\"
                expect eof
                exit 1
            }
            timeout {
                exit 1
            }
        }
    " &
    
    local client2_pid=$!
    
    wait $client1_pid
    wait $client2_pid
    local result=$?
    
    if [ $result -eq 0 ]; then
        echo -e "${GREEN}✓ Nickname conflict test passed${NC}"
        rm -f $log1 $log2
        return 0
    else
        echo -e "${RED}✗ Nickname conflict test failed${NC}"
        rm -f $log1 $log2
        return 1
    fi
}

# Test simultaneous channel operations
test_channel_operations() {
    echo -e "${YELLOW}Testing simultaneous channel operations...${NC}"
    
    local test_channel="#operations"
    
    # Multiple clients performing different operations
    expect -c "
        set timeout 15
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick op1\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        sleep 2
        send \"/topic $test_channel :Test channel topic\\r\"
        sleep 2
        send \"/quit\\r\"
        expect eof
    " &
    
    expect -c "
        set timeout 15
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick op2\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        sleep 3
        send \"/part $test_channel Leaving now\\r\"
        sleep 1
        send \"/quit\\r\"
        expect eof
    " &
    
    wait
    
    echo -e "${GREEN}✓ Channel operations test completed${NC}"
    return 0
}

# Test error handling - connection drops
test_connection_drops() {
    echo -e "${YELLOW}Testing connection drop handling...${NC}"
    
    local test_channel="#droptest"
    
    # Start a client that will be killed
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick dropclient\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        expect \"*$test_channel*\"
        
        # Force quit without proper QUIT command
        exit 0
    " &
    
    local drop_pid=$!
    sleep 5
    
    # Kill the expect process to simulate connection drop
    kill $drop_pid 2>/dev/null
    wait $drop_pid 2>/dev/null
    
    sleep 2
    
    # Test that server is still responsive
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick testclient\\r\"
        
        expect \"*Welcome*\"
        send \"/join $test_channel\\r\"
        expect \"*$test_channel*\"
        send \"/quit\\r\"
        expect eof
        
        puts \"${GREEN}✓ Connection drop test passed${NC}\"
        exit 0
    "
    
    return $?
}

cleanup() {
    stop_server
}

trap cleanup EXIT

main() {
    if [ ! -f "$SERVER_BINARY" ]; then
        echo -e "${RED}✗ Server binary not found at $SERVER_BINARY${NC}"
        exit 1
    fi
    
    if ! command -v expect &> /dev/null; then
        echo -e "${RED}✗ expect not found. Install with: brew install expect${NC}"
        exit 1
    fi
    
    if ! command -v irssi &> /dev/null; then
        echo -e "${RED}✗ irssi not found${NC}"
        exit 1
    fi
    
    start_server
    
    local passed=0
    local total=0
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Multi-Client IRC Integration Tests${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    ((total++))
    if test_channel_communication; then ((passed++)); fi
    sleep 2
    
    ((total++))
    if test_private_messages; then ((passed++)); fi
    sleep 2
    
    ((total++))
    if test_nick_conflicts; then ((passed++)); fi
    sleep 2
    
    ((total++))
    if test_channel_operations; then ((passed++)); fi
    sleep 2
    
    ((total++))
    if test_connection_drops; then ((passed++)); fi
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Tests completed: ${passed}/${total} passed${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    return $((total - passed))
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit $?
fi