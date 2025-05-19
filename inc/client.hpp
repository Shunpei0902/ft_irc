/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:24:02 by sasano            #+#    #+#             */
/*   Updated: 2025/04/10 17:36:03 by sasano           ###   ########.fr       */
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

class Client //-> class for client
{
    private:
        // int socket;
        int Fd; //-> client file descriptor
        struct in_addr IPadd; //-> client ip address
        std::string nickname;
        std::string username;
        std::string hostname;
        std::vector<std::string> channels;
    public:
        Client(){}; //-> default constructor
        // Client(int Fd, std::string IPadd) : Fd(Fd), IPadd(IPadd) {};
        // Client(int Fd, std::string IPadd, std::string nickname, std::string username, std::string hostname)
        //     : Fd(Fd), IPadd(IPadd), nickname(nickname), username(username), hostname(hostname) {};
        // Client(int socket, int Fd, std::string IPadd) : socket(socket), Fd(Fd), IPadd(IPadd) {}
        // int Getsocket() const { return socket; }
        int GetFd() const { return Fd; } //-> getter for fd
        void SetFd(int fd){Fd = fd;} //-> setter for fd
        void SetIpAdd(std::string ipadd){IPadd = ipadd;} //-> setter for ipadd
        std::string GetIPadd() const { return IPadd; }
        std::string GetNickname() const { return nickname; }
        std::string GetUsername() const { return username; }
        std::string GetHostname() const { return hostname; }
        const std::vector<std::string>& GetChannels() const { return channels; }
        void SetNickname(const std::string& nickname) { this->nickname = nickname; }
        void SetUsername(const std::string& username) { this->username = username; }
        void SetHostname(const std::string& hostname) { this->hostname = hostname; }
        void JoinChannel(const std::string& channel) { channels.push_back(channel); }
        // void PartChannel(const std::string& channel) {
        //     channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
        // }
};
