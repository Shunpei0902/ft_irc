/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:23:43 by sasano            #+#    #+#             */
/*   Updated: 2025/04/10 17:44:29 by sasano           ###   ########.fr       */
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

#include "client.hpp" //-> include client.hpp

#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color


// システムのヘッダーで定義されてる。二重定義
// struct sockaddr_in {
//     sa_family_t     sin_family; // アドレスファミリ (AF_INET for IPv4)
//     in_port_t       sin_port; // ポート番号
//     struct  in_addr sin_addr; // IPアドレス
//     char            sin_zero[8]; // 予約領域（使用しない）
// };

// struct in_addr {
// in_addr_t s_addr;  // IPv4アドレス
// };

// struct pollfd {
//     int     fd; //-> file descriptor
//     short   events;//-> requested events
//     short   revents;//-> returned events
// };

class Server //-> class for server
{
    private:
        int Port; //-> server port
        std::string Password; //-> server password
        int SerSocketFd; //-> server socket file descriptor
        static bool Signal; //-> static boolean for signal
        std::vector<Client> clients; //-> vector of clients
        std::vector<struct pollfd> fds; //-> vector of pollfd
        // std::unordered_map<std::string, std::vector<int> > channels;
        // std::mutex client_mutex;
    public:
        Server(){SerSocketFd = -1;} //-> default constructor
        Server(int port, std::string password) : Port(port), Password(password) {};
        // ~Server(); //-> destructor

        int GetPort() const { return Port; } //-> getter for port
        struct in_addr GetIpAdd() const { //-> getter for ip address
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            getsockname(SerSocketFd, (struct sockaddr *)&addr, &len);
            return addr.sin_addr;
        }
        void ServerInit(); //-> server initialization
        void SerSocket(); //-> server socket creation
        void AcceptNewClient(); //-> accept new client
        void ReceiveNewData(int fd); //-> receive new data from a registered client
        static void SignalHandler(int signum); //-> signal handler
	    void CloseFds(); //-> close file descriptors
	    void ClearClients(int fd); //-> clear clients
};
