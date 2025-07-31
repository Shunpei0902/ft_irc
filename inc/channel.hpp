/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 19:24:37 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 09:00:20 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
// #include <unordered_set>
#include <memory>

#include "irc.hpp"
// #include "client.hpp"
// #include "server.hpp"

class Server;
class Client;

class Channel
{
private:
    std::string _name;     // チャンネル名 (#foo など)
    std::string _topic;    // トピック
    std::string _password; // +k モード用のキー
    int _userLimit;        // +l モードの制限人数（-1なら制限なし）

    std::map<std::string, Client *> _clients; // ニックネーム → Client
    std::set<std::string> _operators;         // オペレータ（ニックネーム）
    std::set<std::string> _banList;           // BANされてるニックネーム
    std::set<std::string> _inviteList;        // 招待されたニックネーム（+i モード用）

    std::set<char> _modes; // 有効なモード (+k, +l, +i, +b など)
    bool _inviteOnly;      // +i モード（招待制）

public:
    Channel(const std::string &name);
    ~Channel() {}

    // 基本情報
    const std::string &getName() const;
    const std::string &getTopic() const;
    void setTopic(const std::string &topic);

    // クライアント操作
    void addClient(Client &client);
    void removeClient(Client &client);
    bool hasClient(const Client &client) const;

    // オペレータ管理
    void addOperator(const std::string &nickname);
    void removeOperator(const std::string &nickname);
    bool isOperator(const std::string &nickname) const;

    // パスワード管理
    void setPassword(const std::string &password);
    const std::string &getPassword() const;
    void removePassword(); // パスワードを削除

    // モード管理
    void addMode(char mode);
    void removeMode(char mode);
    bool hasMode(char mode) const;
    std::set<char> getModes() const;

    // BAN管理
    void ban(const std::string &nickname);
    void unban(const std::string &nickname);
    bool isBanned(const std::string &nickname) const;

    // 招待管理
    void addInvite(const std::string &nickname);
    void removeInvite(const std::string &nickname);
    bool isInviteOnly() const;
    bool isInvited(const std::string &nickname) const;

    // 容量制限
    void setUserLimit(int limit);
    int getUserLimit() const;

    // メンバー一覧取得
    // std::map<int, Client *> getClients() const;
    std::map<std::string, Client *> getClients() const;
    // std::map<std::string, Client *> getOperators() const;
    std::set<std::string> getOperators() const;

    // その他
    bool empty() const;
    void broadcast(Server *server, const std::string &message); // チャンネル内の全員にメッセージを送信
};
