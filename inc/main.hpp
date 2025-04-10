/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 23:20:07 by sasano            #+#    #+#             */
/*   Updated: 2025/03/28 02:14:08 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <netinet/in.h>
#include <thread>
#include <mutex>

#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()

#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color

// #define PORT 6667 // IRCの標準ポート

class Client //-> class for client
{
    private:
        int socket;
        private:
        int Fd; //-> client file descriptor
        std::string IPadd; //-> client ip address
        std::string nickname;
        std::string username;
        std::string hostname;
        std::vector<std::string> channels;
    public:
        Client(){}; //-> default constructor
        Client(int socket, int Fd, std::string IPadd) : socket(socket), Fd(Fd), IPadd(IPadd) {}
        int get_socket() const { return socket; }
        int getFd() const { return Fd; } //-> getter for fd
        void setFd(int fd){Fd = fd;} //-> setter for fd
        void setIpAdd(std::string ipadd){IPadd = ipadd;} //-> setter for ipadd
        std::string getIPadd() const { return IPadd; }
        std::string getNickname() const { return nickname; }
        std::string getUsername() const { return username; }
        std::string getHostname() const { return hostname; }
        const std::vector<std::string>& getChannels() const { return channels; }
        void setNickname(const std::string& nickname) { this->nickname = nickname; }
        void setUsername(const std::string& username) { this->username = username; }
        void setHostname(const std::string& hostname) { this->hostname = hostname; }
        void joinChannel(const std::string& channel) { channels.push_back(channel); }
        void partChannel(const std::string& channel) {
            channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
        }
};

class Server //-> class for server
{
    private:
        int Port; //-> server port
        int SerSocketFd; //-> server socket file descriptor
        static bool Signal; //-> static boolean for signal
        std::vector<Client> clients; //-> vector of clients
        std::vector<struct pollfd> fds; //-> vector of pollfd
        std::unordered_map<std::string, std::vector<int>> channels;
        std::mutex client_mutex;
    public:
        Server(){SerSocketFd = -1;} //-> default constructor
        ~Server(); //-> destructor

        void ServerInit(); //-> server initialization
        void SerSocket(); //-> server socket creation
        void AcceptNewClient(); //-> accept new client
        void ReceiveNewData(int fd); //-> receive new data from a registered client
        static void SignalHandler(int signum); //-> signal handler
	    void CloseFds(); //-> close file descriptors
	    void ClearClients(int fd); //-> clear clients
}