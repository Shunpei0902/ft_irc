#!/bin/bash

# Stress tests for ft_irc server
# Tests server resilience under various stress conditions

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
    echo -e "${YELLOW}Starting IRC server for stress testing...${NC}"
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

# Test multiple simultaneous connections
test_concurrent_connections() {
    echo -e "${YELLOW}Testing concurrent connections (20 clients)...${NC}"
    
    local success_count=0
    local pids=()
    
    # Start 20 clients simultaneously
    for i in {1..20}; do
        (
            response=$(timeout 10 bash -c "
                {
                    echo 'PASS $PASSWORD'
                    echo 'NICK stressuser$i'  
                    echo 'USER stressuser$i 0 * :Stress User $i'
                    sleep 1
                    echo 'JOIN #stresstest'
                    sleep 1
                    echo 'PRIVMSG #stresstest :Hello from client $i'
                    sleep 1
                    echo 'QUIT :Stress test complete'
                } | nc 127.0.0.1 $PORT
            ")
            
            if echo "$response" | grep -q "001.*Welcome"; then
                echo "Client $i: SUCCESS" > /tmp/stress_result_$i
            else
                echo "Client $i: FAILED" > /tmp/stress_result_$i
            fi
        ) &
        pids+=($!)
    done
    
    # Wait for all clients to complete
    for pid in "${pids[@]}"; do
        wait $pid
    done
    
    # Count successes
    for i in {1..20}; do
        if [ -f "/tmp/stress_result_$i" ] && grep -q "SUCCESS" "/tmp/stress_result_$i"; then
            ((success_count++))
        fi
        rm -f "/tmp/stress_result_$i" 2>/dev/null
    done
    
    echo -e "${BLUE}Concurrent connections result: $success_count/20 successful${NC}"
    
    if [ $success_count -ge 18 ]; then
        echo -e "${GREEN}✓ Concurrent connections test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Concurrent connections test failed${NC}"
        return 1
    fi
}

# Test flood of messages
test_message_flood() {
    echo -e "${YELLOW}Testing message flood handling...${NC}"
    
    # Start a receiver client
    expect -c "
        set timeout 30
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick floodreceiver\\r\"
        
        expect \"*Welcome*\"
        send \"/join #floodtest\\r\"
        
        expect \"*#floodtest*\"
        sleep 15
        send \"/quit\\r\"
        expect eof
    " &
    
    local receiver_pid=$!
    sleep 5
    
    # Start flood client
    timeout 20 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK floodsender'
            echo 'USER floodsender 0 * :Flood User'
            sleep 1
            echo 'JOIN #floodtest'
            sleep 1
            
            # Send 100 messages rapidly
            for i in {1..100}; do
                echo 'PRIVMSG #floodtest :Flood message $i'
            done
            
            echo 'QUIT :Flood complete'
        } | nc 127.0.0.1 $PORT
    " > /dev/null 2>&1 &
    
    wait $receiver_pid
    
    # Test if server is still responsive
    response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK postflood'
            echo 'USER postflood 0 * :Post Flood User'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "001.*Welcome"; then
        echo -e "${GREEN}✓ Message flood test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Message flood test failed${NC}"
        return 1
    fi
}

# Test rapid connect/disconnect
test_rapid_connections() {
    echo -e "${YELLOW}Testing rapid connect/disconnect...${NC}"
    
    local success_count=0
    
    for i in {1..10}; do
        response=$(timeout 5 bash -c "
            {
                echo 'PASS $PASSWORD'
                echo 'NICK rapid$i'
                echo 'USER rapid$i 0 * :Rapid User $i'
                echo 'QUIT :Rapid disconnect'
            } | nc 127.0.0.1 $PORT
        ")
        
        if echo "$response" | grep -q "001.*Welcome"; then
            ((success_count++))
        fi
        
        # Very brief pause
        sleep 0.1
    done
    
    echo -e "${BLUE}Rapid connections result: $success_count/10 successful${NC}"
    
    if [ $success_count -ge 8 ]; then
        echo -e "${GREEN}✓ Rapid connections test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Rapid connections test failed${NC}"
        return 1
    fi
}

# Test large message handling  
test_large_messages() {
    echo -e "${YELLOW}Testing large message handling...${NC}"
    
    # Create a large message (close to IRC limit of ~512 chars)
    local large_msg="This is a very long message that tests the server's ability to handle messages approaching the IRC protocol limit of 512 characters including the command prefix and CRLF termination. This message should be properly handled by the server without causing any issues or crashes. The server must maintain stability even with maximum length messages from multiple clients simultaneously."
    
    response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK largetest'
            echo 'USER largetest 0 * :Large Test User'
            sleep 1
            echo 'JOIN #largetest'
            sleep 1
            echo 'PRIVMSG #largetest :$large_msg'
            sleep 1
            echo 'QUIT :Large test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "001.*Welcome"; then
        echo -e "${GREEN}✓ Large message test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Large message test failed${NC}"
        return 1
    fi
}

cleanup() {
    # Clean up any remaining processes
    pkill -f "nc.*127.0.0.1.*$PORT" 2>/dev/null || true
    pkill -f irssi 2>/dev/null || true
    stop_server
}

trap cleanup EXIT

main() {
    if [ ! -f "$SERVER_BINARY" ]; then
        echo -e "${RED}✗ Server binary not found at $SERVER_BINARY${NC}"
        exit 1
    fi
    
    start_server
    
    local passed=0
    local total=0
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}IRC Server Stress Tests${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    ((total++))
    if test_concurrent_connections; then ((passed++)); fi
    sleep 3
    
    ((total++))
    if test_message_flood; then ((passed++)); fi
    sleep 3
    
    ((total++))
    if test_rapid_connections; then ((passed++)); fi
    sleep 3
    
    ((total++))
    if test_large_messages; then ((passed++)); fi
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Stress tests completed: ${passed}/${total} passed${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    return $((total - passed))
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit $?
fi