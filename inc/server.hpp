/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:23:43 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 12:38:18 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "irc.hpp"
// #include "client.hpp" //-> include client.hpp
// #include "channel.hpp"
// #include "command.hpp"

#include <iostream>
#include <string>
// #include <unordered_map>
#include <sstream>
#include <cstring>
#include <netinet/in.h>
// #include <thread>
// #include <mutex>

#include <vector>       //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h>  //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h>      //-> for fcntl()
#include <unistd.h>     //-> for close()
#include <arpa/inet.h>  //-> for inet_ntoa()
#include <poll.h>       //-> for poll()
#include <csignal>      //-> for signal()

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

// struct server_op
// {
//     std::string name;
//     std::string host;
//     std::string password;
// };

class Channel; // Forward declaration of Channel class
class Client;  // Forward declaration of Client class

class Server //-> class for server
{
private:
    int _port;                                  //-> server port
    int _serSocketFd;                           //-> server socket file descriptor
    static bool _signal;                        //-> static boolean for signal
    std::vector<struct pollfd> _fds;            //-> vector of pollfd
    std::map<int, Client *> _clients;           //-> vector of clients
    std::map<std::string, Channel *> _channels; // channel name → Channel*
    std::map<int, std::string> _send_buffers;   // fd → message buffer
    std::map<int, std::string> _recv_buffers;   // 受信バッファ
    std::string _password;
    // std::vector<server_op> _operators; //-> vector of server operators

public:
    // コンストラクタ
    Server();
    ~Server();

    // サーバー初期化→起動
    // void serverInit(const std::string &port, const std::string &password, const std::tm *timeinfo); //-> initialize the server
    void serverInit(const char *port, const char *password, struct tm *timeinfo);
    // ソケット通信関連
    int getPort() const;                      //-> getter for port
    struct in_addr getIpAdd() const;          //-> getter for ip address
    void serSocket();                         //-> server socket creation
    void handleSocketReadable(int client_fd); //-> handle socket readable
    static void signalHandler(int signum);    //-> signal handler
    void closeFds();                          //-> close file descriptors

    void setPassword(const std::string &password); //-> set server password
    const std::string &getPassword() const;        //-> get server password

    // メッセージ解析
    void handleClientRegistrationCommand(std::map<int, Client *> &client, int client_fd, ParsedMessage &msg);
    void handleClientMessage(const std::string &line, int client_fd);
    void executeCommand(ParsedMessage &msg, int client_fd); //-> execute command

    // クライアント関連
    void acceptNewClient();    //-> accept new client
    void clearClients(int fd); //-> clear clients
    Client *getClient(int fd); //-> get client by file descriptor
    // void removeClient(int client_fd); //-> remove client by file descriptor
    // void addClient(const Client& client); //-> add client to server
    Client *getClientByNickname(const std::string &nickname); //-> get client by nickname
    std::map<int, Client *> getClients();                     //-> get all clients

    // チャンネル関連
    void addChannel(Channel *channel);                    //-> add channel to server
    Channel *getChannel(const std::string &channel_name); //-> get channel by name
    std::map<std::string, Channel *> getChannels();       //-> get all channels
    void removeChannel(const std::string &channel_name);  //-> remove channel by name
    void clearChannels();                                 //-> clear all channels

    // メッセージ送信バッファ
    void addToClientBuffer(int client_fd, const std::string &message); //-> add message to client buffer
    void sendBuffer(int fd);                                           //-> send buffered messages to clients
    std::string getSendBuffer(int client_fd) const;                    //-> get client buffer
    // std::string getCientBuffer(int client_fd) const;                   //-> get client buffer
    // void clearClientBuffer(int client_fd);                             //-> clear client buffer
    // void sendBufferedMessages(); //-> send buffered messages to clients
    std::string _welcomemsg(void);
};