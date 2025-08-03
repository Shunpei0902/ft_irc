#!/usr/bin/env python3
"""
IRC Server Integration Test Runner
Tests ft_irc server using real IRC clients (irssi)
"""

import subprocess
import time
import os
import signal
import tempfile
import threading
from typing import List, Optional, Dict, Any
import json


class IRCServerTest:
    def __init__(self, server_binary: str = "./ircserv", port: int = 6667, password: str = "testpass"):
        self.server_binary = server_binary
        self.port = port
        self.password = password
        self.server_process: Optional[subprocess.Popen] = None
        self.test_results: List[Dict[str, Any]] = []
        
    def start_server(self) -> bool:
        """Start the IRC server"""
        try:
            # Start server in background
            self.server_process = subprocess.Popen(
                [self.server_binary, str(self.port), self.password],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                preexec_fn=os.setsid  # Create new process group
            )
            
            # Give server time to start
            time.sleep(2)
            
            # Check if server is still running
            if self.server_process.poll() is None:
                print(f"✓ Server started on port {self.port}")
                return True
            else:
                stdout, stderr = self.server_process.communicate()
                print(f"✗ Server failed to start")
                print(f"stdout: {stdout.decode()}")
                print(f"stderr: {stderr.decode()}")
                return False
                
        except Exception as e:
            print(f"✗ Failed to start server: {e}")
            return False
    
    def stop_server(self):
        """Stop the IRC server"""
        if self.server_process:
            try:
                # Send SIGTERM to process group
                os.killpg(os.getpgid(self.server_process.pid), signal.SIGTERM)
                self.server_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                # Force kill if needed
                os.killpg(os.getpgid(self.server_process.pid), signal.SIGKILL)
            except Exception as e:
                print(f"Warning: Error stopping server: {e}")
            
            self.server_process = None
            print("✓ Server stopped")
    
    def create_irssi_config(self, nickname: str, temp_dir: str) -> str:
        """Create temporary irssi config file"""
        config_content = f'''
servers = (
  {{
    address = "127.0.0.1";
    chatnet = "testnet";
    port = "{self.port}";
    password = "{self.password}";
    autoconnect = "yes";
  }}
);

chatnets = {{
  testnet = {{
    type = "IRC";
    nick = "{nickname}";
    username = "{nickname}";
    realname = "Test User";
  }};
}};

settings = {{
  core = {{
    real_name = "Test User";
    user_name = "{nickname}";
    nick = "{nickname}";
  }};
  "fe-text" = {{ actlist_sort = "refnum"; }};
}};
'''
        config_file = os.path.join(temp_dir, "config")
        with open(config_file, "w") as f:
            f.write(config_content)
        return config_file
    
    def run_irssi_test(self, nickname: str, commands: List[str], timeout: int = 10) -> Dict[str, Any]:
        """Run irssi with specified commands and capture output"""
        result = {
            "nickname": nickname,
            "commands": commands,
            "success": False,
            "output": "",
            "error": ""
        }
        
        with tempfile.TemporaryDirectory() as temp_dir:
            try:
                # Create config file
                config_file = self.create_irssi_config(nickname, temp_dir)
                
                # Create command script
                script_content = "\\n".join(commands + ["/quit"])
                
                # Run irssi with commands
                cmd = [
                    "irssi",
                    "--home", temp_dir,
                    "--config", config_file,
                    "--noconnect"
                ]
                
                process = subprocess.Popen(
                    cmd,
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                
                # Send commands
                input_commands = "\\n".join([
                    f"/connect 127.0.0.1 {self.port} {self.password}",
                    *commands,
                    "/quit"
                ])
                
                stdout, stderr = process.communicate(input=input_commands, timeout=timeout)
                
                result["output"] = stdout
                result["error"] = stderr
                result["success"] = process.returncode == 0
                
            except subprocess.TimeoutExpired:
                process.kill()
                result["error"] = "Test timed out"
            except Exception as e:
                result["error"] = str(e)
        
        return result
    
    def test_basic_connection(self) -> bool:
        """Test basic server connection"""
        print("Testing basic connection...")
        
        result = self.run_irssi_test("testuser1", [
            "/sleep 2"  # Give time for connection
        ], timeout=15)
        
        self.test_results.append({
            "test": "basic_connection",
            "result": result
        })
        
        success = result["success"] and "Welcome" in result["output"]
        print(f"{'✓' if success else '✗'} Basic connection test")
        return success
    
    def test_channel_operations(self) -> bool:
        """Test channel join/part operations"""
        print("Testing channel operations...")
        
        result = self.run_irssi_test("testuser2", [
            "/sleep 2",
            "/join #testchannel",
            "/sleep 1",
            "/msg #testchannel Hello channel!",
            "/sleep 1",
            "/part #testchannel"
        ], timeout=20)
        
        self.test_results.append({
            "test": "channel_operations", 
            "result": result
        })
        
        success = result["success"]
        print(f"{'✓' if success else '✗'} Channel operations test")
        return success
    
    def test_private_messages(self) -> bool:
        """Test private message functionality"""
        print("Testing private messages...")
        
        # This test requires multiple clients, simplified for now
        result = self.run_irssi_test("testuser3", [
            "/sleep 2",
            "/msg testuser4 Hello there!",
            "/sleep 1"
        ], timeout=15)
        
        self.test_results.append({
            "test": "private_messages",
            "result": result
        })
        
        success = result["success"]
        print(f"{'✓' if success else '✗'} Private messages test")
        return success
    
    def run_all_tests(self) -> bool:
        """Run all tests"""
        print("=" * 50)
        print("Starting IRC Server Integration Tests")
        print("=" * 50)
        
        if not self.start_server():
            return False
        
        try:
            # Give server more time to fully initialize
            time.sleep(3)
            
            tests = [
                self.test_basic_connection,
                self.test_channel_operations,
                self.test_private_messages
            ]
            
            passed = 0
            for test in tests:
                if test():
                    passed += 1
                time.sleep(2)  # Pause between tests
            
            print("=" * 50)
            print(f"Tests completed: {passed}/{len(tests)} passed")
            print("=" * 50)
            
            return passed == len(tests)
            
        finally:
            self.stop_server()
    
    def save_results(self, filename: str = "test_results.json"):
        """Save test results to file"""
        with open(filename, "w") as f:
            json.dump(self.test_results, f, indent=2)
        print(f"Test results saved to {filename}")


def main():
    """Main test runner"""
    import argparse
    
    parser = argparse.ArgumentParser(description="IRC Server Integration Tests")
    parser.add_argument("--server", default="./ircserv", help="Path to IRC server binary")
    parser.add_argument("--port", type=int, default=6667, help="Server port")
    parser.add_argument("--password", default="testpass", help="Server password")
    
    args = parser.parse_args()
    
    # Check if server binary exists
    if not os.path.exists(args.server):
        print(f"Error: Server binary '{args.server}' not found")
        print("Please compile the server first with 'make'")
        return 1
    
    # Check if irssi is available
    if subprocess.run(["which", "irssi"], capture_output=True).returncode != 0:
        print("Error: irssi not found. Please install irssi first.")
        return 1
    
    tester = IRCServerTest(args.server, args.port, args.password)
    success = tester.run_all_tests()
    tester.save_results()
    
    return 0 if success else 1


if __name__ == "__main__":
    exit(main())