#!/bin/bash

# Master test runner - runs all IRC server tests
# Provides a single command to run the complete test suite

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# Test configuration
SERVER_BINARY="../ircserv"
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

print_banner() {
    echo -e "${PURPLE}
╔══════════════════════════════════════════════════════════════╗
║                 IRC Server Test Suite                        ║
║              Comprehensive Integration Testing               ║
╚══════════════════════════════════════════════════════════════╝${NC}"
}

print_section() {
    echo -e "\n${BLUE}▓▓▓ $1 ▓▓▓${NC}"
}

check_prerequisites() {
    print_section "Checking Prerequisites"
    
    local missing=0
    
    # Check server binary
    if [ ! -f "$SERVER_BINARY" ]; then
        echo -e "${RED}✗ Server binary not found at $SERVER_BINARY${NC}"
        echo -e "${YELLOW}  Please run 'make' in the parent directory first${NC}"
        ((missing++))
    else
        echo -e "${GREEN}✓ Server binary found${NC}"
    fi
    
    # Check required tools
    local tools=("nc" "expect" "irssi" "python3")
    for tool in "${tools[@]}"; do
        if command -v "$tool" &> /dev/null; then
            echo -e "${GREEN}✓ $tool found${NC}"
        else
            echo -e "${RED}✗ $tool not found${NC}"
            case $tool in
                "expect"|"irssi")
                    echo -e "${YELLOW}  Install with: brew install $tool${NC}"
                    ;;
                "nc")
                    echo -e "${YELLOW}  netcat should be pre-installed${NC}"
                    ;;
                "python3")
                    echo -e "${YELLOW}  Install Python 3${NC}"
                    ;;
            esac
            ((missing++))
        fi
    done
    
    if [ $missing -gt 0 ]; then
        echo -e "\n${RED}Missing $missing prerequisites. Please install them first.${NC}"
        return 1
    fi
    
    echo -e "${GREEN}All prerequisites satisfied!${NC}"
    return 0
}

run_test_suite() {
    local test_script="$1"
    local test_name="$2"
    local passed=0
    local failed=0
    
    print_section "$test_name"
    
    if [ ! -f "$test_script" ]; then
        echo -e "${RED}✗ Test script not found: $test_script${NC}"
        return 1
    fi
    
    # Make script executable
    chmod +x "$test_script"
    
    # Run the test
    if "$test_script"; then
        echo -e "${GREEN}✓ $test_name completed successfully${NC}"
        return 0
    else
        echo -e "${RED}✗ $test_name failed${NC}"
        return 1
    fi
}

run_python_tests() {
    print_section "Python Test Runner"
    
    if [ -f "$TEST_DIR/test_runner.py" ]; then
        python3 "$TEST_DIR/test_runner.py" --server "$SERVER_BINARY"
        return $?
    else
        echo -e "${YELLOW}Python test runner not found, skipping...${NC}"
        return 0
    fi
}

generate_summary() {
    local total_suites=$1
    local passed_suites=$2
    
    echo -e "\n${PURPLE}
╔══════════════════════════════════════════════════════════════╗
║                        TEST SUMMARY                          ║
╚══════════════════════════════════════════════════════════════╝${NC}"
    
    echo -e "${BLUE}Test Suites Run: $total_suites${NC}"
    echo -e "${GREEN}Passed: $passed_suites${NC}"
    echo -e "${RED}Failed: $((total_suites - passed_suites))${NC}"
    
    if [ $passed_suites -eq $total_suites ]; then
        echo -e "\n${GREEN}🎉 ALL TESTS PASSED! Your IRC server is working correctly.${NC}"
        return 0
    else
        echo -e "\n${RED}⚠️  Some tests failed. Check the output above for details.${NC}"
        return 1
    fi
}

cleanup() {
    # Kill any remaining server processes
    pkill -f "ircserv.*6667" 2>/dev/null || true
    # Kill any remaining irssi processes
    pkill -f irssi 2>/dev/null || true
    echo -e "\n${YELLOW}Cleanup completed${NC}"
}

main() {
    local quick_mode=false
    local verbose=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -q|--quick)
                quick_mode=true
                shift
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [OPTIONS]"
                echo "Options:"
                echo "  -q, --quick    Run only basic tests (faster)"
                echo "  -v, --verbose  Enable verbose output"
                echo "  -h, --help     Show this help message"
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                echo "Use -h or --help for usage information"
                exit 1
                ;;
        esac
    done
    
    # Set verbose mode
    if [ "$verbose" = true ]; then
        set -x
    fi
    
    # Setup cleanup trap
    trap cleanup EXIT
    
    print_banner
    
    # Check prerequisites
    if ! check_prerequisites; then
        exit 1
    fi
    
    cd "$TEST_DIR"
    
    local total_suites=0
    local passed_suites=0
    
    # Test suite definitions
    declare -a test_suites=(
        "nc_tests.sh:Basic Protocol Tests (netcat)"
        "expect_tests.sh:Integration Tests (irssi)"
    )
    
    # Add multi-client tests unless in quick mode
    if [ "$quick_mode" = false ]; then
        test_suites+=("multi_client_test.sh:Multi-Client Tests")
    fi
    
    # Run each test suite
    for suite in "${test_suites[@]}"; do
        IFS=':' read -r script_name suite_name <<< "$suite"
        ((total_suites++))
        
        if run_test_suite "$script_name" "$suite_name"; then
            ((passed_suites++))
        fi
        
        # Brief pause between test suites
        sleep 2
    done
    
    # Run Python tests if not in quick mode
    if [ "$quick_mode" = false ]; then
        ((total_suites++))
        if run_python_tests; then
            ((passed_suites++))
        fi
    fi
    
    # Generate final summary
    generate_summary $total_suites $passed_suites
    return $?
}

# Show usage if no arguments and script is being run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]] && [[ $# -eq 0 ]]; then
    echo -e "${YELLOW}IRC Server Test Suite${NC}"
    echo -e "${YELLOW}Usage: $0 [OPTIONS]${NC}"
    echo -e "${YELLOW}Run with -h or --help for more information${NC}"
    echo -e "${YELLOW}Run with no options to execute all tests${NC}"
    echo ""
fi

# Execute main function if script is run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit $?
fi