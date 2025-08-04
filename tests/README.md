# IRC Server Integration Tests

Automated test suite for the ft_irc server using real IRC clients (irssi) and netcat.

## Overview

This test suite provides comprehensive integration testing for your IRC server implementation using actual IRC clients rather than mock implementations. It includes:

- **Basic protocol tests** using netcat
- **Real client tests** using irssi 
- **Multi-client interaction tests**
- **Authentication and security tests**

## Prerequisites

### Required Tools
- `irssi` - IRC client for integration tests
- `expect` - For automating interactive programs
- `netcat` (`nc`) - For basic protocol tests
- `bash` - Shell for test scripts

### Installation on macOS
```bash
brew install irssi expect
```

### Installation on Linux (Ubuntu/Debian)
```bash
sudo apt-get install irssi expect netcat-openbsd
```

## Test Structure

```
tests/
├── README.md                 # This file
├── test_runner.py           # Python test runner (alternative)
├── expect_tests.sh          # Main irssi-based tests
├── nc_tests.sh              # Basic netcat protocol tests
├── multi_client_test.sh     # Multi-client interaction tests
└── test_results.json        # Generated test results (if using Python runner)
```

## Running Tests

### 1. Compile Your Server First
```bash
cd /path/to/ft_irc
make
```

### 2. Run Basic Protocol Tests (Fastest)
```bash
cd tests
./nc_tests.sh
```

### 3. Run Full Integration Tests with irssi
```bash
cd tests
./expect_tests.sh
```

### 4. Run Multi-Client Tests
```bash
cd tests
./multi_client_test.sh
```

### 5. Run All Tests with Python Runner
```bash
cd tests
python3 test_runner.py
```

## Test Categories

### Basic Protocol Tests (`nc_tests.sh`)
- ✅ User registration (PASS, NICK, USER)
- ✅ Channel operations (JOIN, PART)
- ✅ Message sending (PRIVMSG)
- ✅ PING/PONG handling
- ✅ Password validation and rejection
- ✅ Partial command handling (RFC compliance)
- ✅ Operator command privileges (TOPIC)

### Integration Tests (`expect_tests.sh`)
- ✅ Full client connection with irssi
- ✅ Channel joining and messaging
- ✅ PING/PONG automatic handling
- ✅ Password authentication and rejection (464 error)
- ✅ Multi-client scenarios
- ✅ Operator privileges (TOPIC, MODE)
- ✅ RFC 2812 compliance (numeric replies)

### Multi-Client Tests (`multi_client_test.sh`)
- ✅ Channel communication between clients
- ✅ Private messages between users
- ✅ Nickname conflict resolution (433 error)
- ✅ Simultaneous channel operations
- ✅ Connection drop handling and server stability

## Customizing Tests

### Server Configuration
Edit the variables at the top of each test script:

```bash
SERVER_BINARY="../ircserv"    # Path to your server binary
PORT="6667"                   # Server port
PASSWORD="testpass"           # Server password
```

### Adding New Tests

#### For netcat tests (`nc_tests.sh`):
```bash
test_your_feature() {
    echo -e "${YELLOW}Testing your feature...${NC}"
    
    local response=$(timeout 10 bash -c "
        {
            echo 'PASS $PASSWORD'
            echo 'NICK testuser'
            echo 'USER testuser 0 * :Test User'
            echo 'YOUR_COMMAND parameters'
            sleep 1
            echo 'QUIT :Test complete'
        } | nc 127.0.0.1 $PORT
    ")
    
    if echo "$response" | grep -q "EXPECTED_RESPONSE"; then
        echo -e "${GREEN}✓ Your feature test passed${NC}"
        return 0
    else
        echo -e "${RED}✗ Your feature test failed${NC}"
        return 1
    fi
}
```

#### For irssi tests (`expect_tests.sh`):
```bash
test_your_feature() {
    echo -e "${YELLOW}Testing your feature...${NC}"
    
    expect -c "
        set timeout 10
        spawn irssi --noconnect
        
        expect \"*irssi*\"
        send \"/connect 127.0.0.1 $PORT $PASSWORD\\r\"
        
        expect \"*Welcome*\"
        send \"/your_command parameters\\r\"
        
        expect {
            \"*expected_response*\" {
                send \"/quit\\r\"
                puts \"${GREEN}✓ Your feature test passed${NC}\"
                exit 0
            }
            timeout {
                puts \"${RED}✗ Your feature test failed${NC}\"
                exit 1
            }
        }
    "
    
    return $?
}
```

## Understanding Test Results

### Success Output
```
✓ Server started
✓ Registration test passed
✓ JOIN test passed
✓ PRIVMSG test passed
✓ PING/PONG test passed
======================================
Tests completed: 5/5 passed
======================================
```

### Failure Output
```
✗ Registration test failed
Response: ERROR :Password incorrect
======================================
Tests completed: 4/5 passed
======================================
```

## Compliance with ft_irc Requirements

These tests are designed to verify compliance with:

### 42 School Technical Requirements
- **Single poll() usage**: Tests verify server handles multiple connections with one poll() call
- **Non-blocking I/O**: Connection handling tests verify proper fcntl() usage
- **No errno handling**: Tests don't rely on EAGAIN-based retry logic
- **Memory leak prevention**: Connection drop tests verify proper cleanup

### RFC 2812 IRC Protocol Compliance
- **Numeric replies**: Tests verify proper 001 Welcome, 464 Password incorrect, 433 Nickname in use
- **Message format**: All tests use proper IRC message format with \r\n endings  
- **Command authentication**: Tests verify PASS must come before NICK/USER
- **Channel operations**: Tests verify JOIN, PART, PRIVMSG broadcasting
- **Operator privileges**: Tests verify TOPIC, MODE, KICK require channel operator status

### Evaluation Criteria Covered
- ✅ Basic networking and connection handling
- ✅ Multiple concurrent client support
- ✅ Partial command and connection drop resilience
- ✅ Channel operations and message broadcasting
- ✅ Operator command privileges
- ✅ Password authentication enforcement

## Troubleshooting

### Common Issues

#### 1. Server Won't Start
```
✗ Failed to start server
```
**Solutions:**
- Check if server binary exists: `ls -la ../ircserv`
- Ensure server compiles: `make` in parent directory
- Check if port is already in use: `lsof -i :6667`

#### 2. Connection Timeout
```
✗ Connection timeout
```
**Solutions:**
- Increase timeout values in test scripts
- Check server logs for errors
- Verify server is accepting connections: `netstat -an | grep 6667`

#### 3. Tests Hang
**Solutions:**
- Kill hanging processes: `pkill -f irssi` or `pkill -f ircserv`
- Check for zombie processes: `ps aux | grep irssi`
- Restart terminal session

#### 4. Permission Denied
```
bash: ./expect_tests.sh: Permission denied
```
**Solution:**
```bash
chmod +x expect_tests.sh
chmod +x nc_tests.sh  
chmod +x multi_client_test.sh
```

### Debugging Failed Tests

#### Enable Verbose Output
Add debugging to test scripts:
```bash
# At the top of the script, add:
set -x  # Enable debug mode

# Or run with debug:
bash -x ./expect_tests.sh
```

#### Check Server Output
Run server manually to see error messages:
```bash
cd ..
./ircserv 6667 testpass
```

#### Test Manual Connection
```bash
# Test with netcat manually:
nc 127.0.0.1 6667
PASS testpass
NICK testuser
USER testuser 0 * :Test User
```

#### Test with Real IRC Client
```bash
# Connect with irssi manually:
irssi
/connect 127.0.0.1 6667 testpass
```

## Extending the Test Suite

### Adding Performance Tests
```bash
test_performance() {
    echo "Testing server under load..."
    
    # Start multiple clients simultaneously
    for i in {1..10}; do
        (
            echo -e "PASS $PASSWORD\nNICK user$i\nUSER user$i 0 * :User $i\nQUIT" | nc 127.0.0.1 $PORT
        ) &
    done
    
    wait
    echo "✓ Performance test completed"
}
```

### Adding Protocol Compliance Tests
```bash
test_rfc_compliance() {
    echo "Testing RFC 1459 compliance..."
    
    # Test specific RFC requirements
    # Add your RFC compliance tests here
}
```

## Test Reports

The Python test runner generates detailed JSON reports:

```json
{
  "test": "basic_connection",
  "result": {
    "nickname": "testuser1",
    "commands": ["/sleep 2"],
    "success": true,
    "output": "Welcome to the IRC server...",
    "error": ""
  }
}
```

## Contributing

When adding new tests:

1. Follow the existing naming convention
2. Add appropriate error handling
3. Include timeouts to prevent hanging
4. Update this README with new test descriptions
5. Test your tests! Ensure they pass and fail appropriately

## IRC Protocol Reference

Useful IRC commands for testing:

- `PASS <password>` - Set connection password
- `NICK <nickname>` - Set nickname
- `USER <username> <hostname> <servername> <realname>` - User registration
- `JOIN <channel>` - Join channel
- `PART <channel> [<reason>]` - Leave channel
- `PRIVMSG <target> <message>` - Send message
- `PING <token>` - Ping server
- `PONG <token>` - Pong response
- `QUIT [<reason>]` - Disconnect

## License

This test suite is provided as-is for educational purposes as part of the 42 school ft_irc project.