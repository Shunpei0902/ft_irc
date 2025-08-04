#!/bin/bash

# Simple IRC tests using netcat (nc)
# Lightweight alternative for basic protocol testing

SERVER_BINARY="../ircserv"
PORT="6667"
PASSWORD="testpass"
SERVER_PID=""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

# Test basic connection and registration
test_registration() {
    echo -e "${YELLOW}Testing registration...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK testuser'
            echo 'USER testuser 0 * :Test User'
            sleep 2
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "001.*Welcome"; then
        echo -e "${GREEN}✓ Registration test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Registration test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test JOIN command
test_join() {
    echo -e "${YELLOW}Testing JOIN command...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK testuser2'
            echo 'USER testuser2 0 * :Test User'
            sleep 1
            echo 'JOIN #testchan'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "JOIN.*#testchan"; then
        echo -e "${GREEN}✓ JOIN test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ JOIN test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test PRIVMSG command
test_privmsg() {
    echo -e "${YELLOW}Testing PRIVMSG command...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK testuser3'
            echo 'USER testuser3 0 * :Test User'
            sleep 1
            echo 'JOIN #testchan'
            sleep 1
            echo 'PRIVMSG #testchan :Hello World'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "PRIVMSG.*Hello World"; then
        echo -e "${GREEN}✓ PRIVMSG test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ PRIVMSG test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test PING/PONG
test_ping() {
    echo -e "${YELLOW}Testing PING/PONG...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK testuser4'
            echo 'USER testuser4 0 * :Test User'
            sleep 1
            echo 'PING :test123'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "PONG.*test123"; then
        echo -e "${GREEN}✓ PING/PONG test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ PING/PONG test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test invalid password - must reject before any commands
test_invalid_password() {
    echo -e "${YELLOW}Testing invalid password rejection...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS wrongpassword'
            echo 'NICK testuser5'
            echo 'USER testuser5 0 * :Test User'
            sleep 2
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    # Should get 464 ERR_PASSWDMISMATCH and NOT get 001 Welcome
    if echo "$response" | grep -qE "(464|Password incorrect)" && ! echo "$response" | grep -q "001.*Welcome"; then
        echo -e "${GREEN}✓ Invalid password test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Invalid password test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test partial commands - server should handle incomplete messages
test_partial_commands() {
    echo -e "${YELLOW}Testing partial command handling...${NC}"
    
    # Send partial command without \r\n (use shorter valid nickname)
    local response=$(timeout 5 bash -c "
        {
            printf 'PASS $PASSWORD\r\n'
            printf 'NICK partial1'
            sleep 2
            printf '\r\n'
            printf 'USER partial1 0 * :Test User\r\n'
            sleep 1
            printf 'QUIT :Test complete\r\n'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "001.*Welcome"; then
        echo -e "${GREEN}✓ Partial command test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Partial command test failed${NC}"
        echo "Response: $response"
        return 1
    fi
}

# Test operator privileges - KICK command
test_operator_commands() {
    echo -e "${YELLOW}Testing operator commands...${NC}"
    
    # First connect as operator (channel creator)
    local response=$(timeout 15 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK operator1'
            echo 'USER operator1 0 * :Operator User'
            sleep 1
            echo 'JOIN #optest'
            sleep 1
            # Try to set topic (operator privilege)
            echo 'TOPIC #optest :Operator set topic'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    # Look for topic response (332 is RPL_TOPIC)
    if echo "$response" | grep -q "332.*#optest.*Operator set topic"; then
        echo -e "${GREEN}✓ Operator commands test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Operator commands test failed${NC}"
        echo "Response: $response"
        return 1
    fi
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
    
    if ! command -v nc &> /dev/null; then
        echo -e "${RED}✗ netcat (nc) not found${NC}"
        exit 1
    fi
    
    start_server
    
    local passed=0
    local total=0
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Basic IRC Protocol Tests (netcat)${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    ((total++))
    if test_registration; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_join; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_privmsg; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_ping; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_invalid_password; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_partial_commands; then ((passed++)); fi
    sleep 1
    
    ((total++))
    if test_operator_commands; then ((passed++)); fi
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Tests completed: ${passed}/${total} passed${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    return $((total - passed))
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit $?
fi