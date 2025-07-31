/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 19:12:50 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 08:59:28 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"
#include "server.hpp"
#include "channel.hpp"

Channel::Channel(const std::string &name)
    : _name(name), _topic(""), _password(""), _userLimit(-1), _inviteOnly(false) {}

// 基本情報
const std::string &Channel::getName() const
{
    return _name;
}
const std::string &Channel::getTopic() const
{
    return _topic;
}
void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

// クライアント操作
void Channel::addClient(Client &client)
{
    _clients[client.getNickname()] = &client;
    client.addChannel(this); // クライアントのチャンネルリストに追加
}
void Channel::removeClient(Client &client)
{
    _clients.erase(client.getNickname());
    _operators.erase(client.getNickname());
    client.removeChannel(this); // クライアントのチャンネルリストから削除
}
bool Channel::hasClient(const Client &client) const
{
    return _clients.find(client.getNickname()) != _clients.end();
}

// オペレータ管理
void Channel::addOperator(const std::string &nickname)
{
    _operators.insert(nickname);
}
void Channel::removeOperator(const std::string &nickname)
{
    _operators.erase(nickname);
}
bool Channel::isOperator(const std::string &nickname) const
{
    return _operators.find(nickname) != _operators.end();
}

// パスワード管理
void Channel::setPassword(const std::string &password)
{
    _password = password;
    addMode('k'); // パスワードが設定された場合、+k モードを追加
}
const std::string &Channel::getPassword() const
{
    return _password;
}

void Channel::removePassword()
{
    _password = "";  // パスワードを空にする
    removeMode('k'); // パスワードが削除された場合、+k モードを削除
}

// モード管理
void Channel::addMode(char mode)
{
    if (_modes.find(mode) == _modes.end())
    {
        _modes.insert(mode); // モードがまだ存在しない場合のみ追加
    }
    if (mode == 'i')
    {
        _inviteOnly = true; // +i モードが追加された場合、招待制に設定
    }
}
void Channel::removeMode(char mode)
{
    std::set<char>::iterator it = _modes.find(mode);
    if (it != _modes.end())
    {
        _modes.erase(it, _modes.end());
    }
    if (mode == 'i')
    {
        _inviteOnly = false; // +i モードが削除された場合、招待制を解除
    }
}
bool Channel::hasMode(char mode) const
{
    return _modes.find(mode) != _modes.end();
}

std::set<char> Channel::getModes() const
{
    return _modes;
}

// BAN管理
void Channel::ban(const std::string &nickname)
{
    _banList.insert(nickname);
    addMode('b'); // +b モードを追加
}
void Channel::unban(const std::string &nickname)
{
    _banList.erase(nickname);
    if (_banList.empty())
    {
        removeMode('b'); // BANリストが空になった場合、+b モードを削除
    }
}
bool Channel::isBanned(const std::string &nickname) const
{
    return _banList.find(nickname) != _banList.end();
}

void Channel::addInvite(const std::string &nickname)
{
    _inviteList.insert(nickname);
}
void Channel::removeInvite(const std::string &nickname)
{
    _inviteList.erase(nickname);
}
bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}
bool Channel::isInvited(const std::string &nickname) const
{
    return _inviteList.find(nickname) != _inviteList.end();
}

// 容量制限
void Channel::setUserLimit(int limit)
{
    _userLimit = limit;
    if (limit > 0)
        addMode('l'); // +l モードを追加
    else
        removeMode('l'); // 制限なしの場合、+l モードを削除
}
int Channel::getUserLimit() const
{
    return _userLimit;
}

// メンバー一覧取得
std::map<std::string, Client *> Channel::getClients() const
{
    return _clients;
}

std::set<std::string> Channel::getOperators() const
{
    return _operators;
}
// その他
bool Channel::empty() const
{
    return _clients.empty();
}

void Channel::broadcast(Server *server, const std::string &message)
{
    const std::map<std::string, Client *> &members = this->getClients();
    for (std::map<std::string, Client *>::const_iterator member = members.begin(); member != members.end(); ++member)
    {
        server->addToClientBuffer(member->second->getFd(), message);
    }
}
