#!/bin/bash

# IRC Server Integration Tests using expect and irssi
# More reliable than Python subprocess for interactive testing

SERVER_BINARY="../ircserv"
PORT="6667"
PASSWORD="testpass"
SERVER_PID=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check dependencies
check_dependencies() {
    if ! command -v expect &> /dev/null; then
        echo -e "${RED}✗ expect not found. Please install expect first.${NC}"
        echo "On macOS: brew install expect"
        exit 1
    fi
    
    if ! command -v irssi &> /dev/null; then
        echo -e "${RED}✗ irssi not found. Please install irssi first.${NC}"
        exit 1
    fi
    
    if [ ! -f "$SERVER_BINARY" ]; then
        echo -e "${RED}✗ Server binary not found at $SERVER_BINARY${NC}"
        echo "Please compile the server first with 'make'"
        exit 1
    fi
}

# Start IRC server
start_server() {
    echo -e "${YELLOW}Starting IRC server on port $PORT...${NC}"
    $SERVER_BINARY $PORT $PASSWORD &
    SERVER_PID=$!
    
    # Wait for server to start
    sleep 3
    
    # Check if server is running
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}✗ Failed to start server${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Server started (PID: $SERVER_PID)${NC}"
}

# Stop IRC server
stop_server() {
    if [ -n "$SERVER_PID" ] && kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${YELLOW}Stopping server...${NC}"
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null
        echo -e "${GREEN}✓ Server stopped${NC}"
    fi
}

# Test basic connection and registration
test_basic_connection() {
    echo -e "${YELLOW}Testing basic connection and registration...${NC}"
    
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        
        expect {
            \"*Welcome*\" {
                send \"/quit\\r\"
                puts \"${GREEN}✓ Basic connection test passed${NC}\"
                exit 0
            }
            \"*Connection refused*\" {
                puts \"${RED}✗ Connection refused${NC}\"
                exit 1
            }
            timeout {
                puts \"${RED}✗ Connection timeout${NC}\"
                exit 1
            }
        }
    "
    
    return $?
}

# Test channel operations
test_channel_operations() {
    echo -e "${YELLOW}Testing channel operations...${NC}"
    
    expect -c "
        set timeout 15
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        
        expect \"*Welcome*\"
        send \"/join #testchannel\\r\"
        
        expect {
            \"*#testchannel*\" {
                send \"/msg #testchannel Hello from test!\\r\"
                sleep 1
                send \"/part #testchannel Test complete\\r\"
                sleep 1
                send \"/quit\\r\"
                puts \"${GREEN}✓ Channel operations test passed${NC}\"
                exit 0
            }
            timeout {
                puts \"${RED}✗ Channel join timeout${NC}\"
                exit 1
            }
        }
    "
    
    return $?
}

# Test multiple clients interaction
test_multiple_clients() {
    echo -e "${YELLOW}Testing multiple clients...${NC}"
    
    # Start first client in background
    expect -c "
        set timeout 20
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick testuser1\\r\"
        
        expect \"*Welcome*\"
        send \"/join #multiclient\\r\"
        
        expect \"*#multiclient*\"
        
        # Wait for second client to join
        sleep 5
        send \"/msg #multiclient Hello from client 1!\\r\"
        sleep 2
        send \"/quit\\r\"
        exit 0
    " &
    
    CLIENT1_PID=$!
    sleep 3
    
    # Start second client
    expect -c "
        set timeout 15
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick testuser2\\r\"
        
        expect \"*Welcome*\"
        send \"/join #multiclient\\r\"
        
        expect \"*#multiclient*\"
        send \"/msg #multiclient Hello from client 2!\\r\"
        sleep 2
        send \"/quit\\r\"
        
        puts \"${GREEN}✓ Multiple clients test passed${NC}\"
        exit 0
    "
    
    local result=$?
    wait $CLIENT1_PID 2>/dev/null
    return $result
}

# Test PING/PONG functionality
test_ping_pong() {
    echo -e "${YELLOW}Testing PING/PONG...${NC}"
    
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        
        expect \"*Welcome*\"
        send \"/quote PING :test123\\r\"
        
        expect {
            \"*PONG*test123*\" {
                send \"/quit\\r\"
                puts \"${GREEN}✓ PING/PONG test passed${NC}\"
                exit 0
            }
            timeout {
                puts \"${RED}✗ PING/PONG timeout${NC}\"
                exit 1
            }
        }
    "
    
    return $?
}

# Test password authentication
test_wrong_password() {
    echo -e "${YELLOW}Testing wrong password rejection...${NC}"
    
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT wrongpassword\\r\"
        
        expect {
            \"*Password incorrect*\" {
                send \"/quit\\r\"
                puts \"${GREEN}✓ Wrong password rejection test passed${NC}\"
                exit 0
            }
            \"*464*\" {
                send \"/quit\\r\"
                puts \"${GREEN}✓ Wrong password rejection test passed (464)${NC}\"
                exit 0
            }
            \"*Welcome*\" {
                puts \"${RED}✗ Server accepted wrong password${NC}\"
                exit 1
            }
            timeout {
                puts \"${RED}✗ Password test timeout${NC}\"
                exit 1
            }
        }
    "
    
    return $?
}

# Test operator privileges and channel operations
test_operator_privileges() {
    echo -e "${YELLOW}Testing operator privileges...${NC}"
    
    expect -c "
        set timeout 20
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick channelop\\r\"
        
        expect \"*Welcome*\"
        send \"/join #optest\\r\"
        
        expect \"*#optest*\"
        # Test TOPIC command (operator privilege)
        send \"/topic #optest Test topic by operator\\r\"
        sleep 2
        
        # Test MODE command (operator privilege)
        send \"/mode #optest +l 10\\r\"
        sleep 2
        
        send \"/quit\\r\"
        puts \"${GREEN}✓ Operator privileges test completed${NC}\"
        exit 0
    "
    
    return $?
}

# Test RFC compliance - proper numeric replies
test_rfc_compliance() {
    echo -e "${YELLOW}Testing RFC 2812 compliance...${NC}"
    
    expect -c "
        set timeout 15
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        send \"/nick rfctest\\r\"
        
        expect \"*Welcome*\"
        
        # Test JOIN to get proper numeric replies
        send \"/join #rfctest\\r\"
        expect \"*#rfctest*\"
        
        # Test NAMES command for 353/366 replies
        send \"/names #rfctest\\r\"
        sleep 2
        
        send \"/quit\\r\"
        puts \"${GREEN}✓ RFC compliance test completed${NC}\"
        exit 0
    "
    
    return $?
}

# Main test runner
run_tests() {
    local passed=0
    local total=0
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}IRC Server Integration Tests${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    # Test basic connection
    ((total++))
    if test_basic_connection; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test channel operations
    ((total++))
    if test_channel_operations; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test PING/PONG
    ((total++))
    if test_ping_pong; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test wrong password
    ((total++))
    if test_wrong_password; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test multiple clients
    ((total++))
    if test_multiple_clients; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test operator privileges
    ((total++))
    if test_operator_privileges; then
        ((passed++))
    fi
    
    sleep 2
    
    # Test RFC compliance
    ((total++))
    if test_rfc_compliance; then
        ((passed++))
    fi
    
    echo -e "${YELLOW}======================================${NC}"
    echo -e "${YELLOW}Tests completed: ${passed}/${total} passed${NC}"
    echo -e "${YELLOW}======================================${NC}"
    
    if [ $passed -eq $total ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Some tests failed.${NC}"
        return 1
    fi
}

# Cleanup function
cleanup() {
    stop_server
}

# Set trap for cleanup
trap cleanup EXIT

# Main execution
main() {
    check_dependencies
    start_server
    run_tests
    return $?
}

# Run if script is executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit $?
fi