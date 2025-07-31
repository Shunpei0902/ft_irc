/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:24:02 by sasano            #+#    #+#             */
/*   Updated: 2025/07/30 23:11:33 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "irc.hpp"
#include "color.hpp"
// #include "channel.hpp"

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
#include <string>
// #include <unordered_set>
#include <algorithm>

class Channel;

class Client
{
private:
    int _fd;            // クライアントのファイルディスクリプタ
    std::string _ipAdd; // IPアドレス
    std::string _nickname;
    std::string _username;
    std::string _realname; // 実名（REALNAMEはIRCでは一般的ではない）
    std::set<char> _modes; // ユーザーモード（"i", "o"のみ）
    // std::string _hostname;
    std::map<std::string, Channel *> _channels; // クライアントが参加しているチャンネルのマップ <channel_name, Channel*>

    // 初期情報の登録フラグ
    bool _connexion_password; // パスワード接続フラグ
    bool _hasNick;            // NICKコマンドによるニックネーム登録フラグ
    bool _hasUser;            // USERコマンドによるユーザー名登録フラグ
    bool _registrationDone;   // 登録完了フラグ
    bool _to_deconnect;       // 切断フラグ
    bool _pass_flag;          // パスワード接続フラグ（PASSコマンドによる）

public:
    Client();
    Client(int id, const std::string &ip);
    ~Client();

    int getFd() const;
    void setFd(int newFd);

    std::string getIpAdd() const;
    void setIpAdd(const std::string &ipadd);

    std::string getNickname() const;
    void setNickname(const std::string &nick);
    std::string getUsername() const;
    void setUsername(const std::string &user);
    std::string getRealname() const;
    void setRealname(const std::string &realname);

    // const std::string &hasModes() const;
    const std::set<char> &getModes() const;
    void addMode(char mode);
    void removeMode(char mode);
    bool hasMode(char mode) const;

    // const std::string& getHostname() const ;
    // void setHostname(const std::string& host);

    bool &getConnexionPassword();
    bool &hasNick();
    bool &hasUser();
    bool &isRegistrationDone();
    bool &getDeconnexionStatus();
    bool &getPassFlag(); // パスワード接続フラグ（PASSコマンドによる）

    // チャンネル関連
    bool isInChannel(Channel *channel) const;
    const std::map<std::string, Channel *> &getChannels() const;
    void addChannel(Channel *channel);
    void removeChannel(Channel *channel);
};
