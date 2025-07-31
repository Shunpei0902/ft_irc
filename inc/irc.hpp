/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 23:20:07 by sasano            #+#    #+#             */
/*   Updated: 2025/07/30 15:56:46 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_HPP
#define IRC_HPP

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
#include <map>
#include <set>

#define SUCCESS 0
#define FAILURE -1

#define REQUIRED_INFO_COUNT 2 // NICK, USERコマンドによる情報登録は必要
// #define PORT 6667 // IRCの標準ポート

struct ParsedMessage
{
    std::string prefix;              // 送信者情報（例: :nick!user@host）
    std::string command;             // IRCコマンド名
    std::vector<std::string> params; // コマンドの引数
    std::string trailing;            // コマンドの後ろに続くメッセージ
};

std::vector<std::string> split(const std::string &str, char delimiter);

#endif // IRC_HPP
