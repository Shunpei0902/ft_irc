/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 19:25:39 by sasano            #+#    #+#             */
/*   Updated: 2025/07/30 23:11:21 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"
#include "server.hpp"
#include "client.hpp"

Client::Client() : _fd(-1), _ipAdd(""), _nickname(""), _username(""),
                   _realname(""), _connexion_password(false),
                   _hasNick(false), _hasUser(false), _registrationDone(false),
                   _to_deconnect(false), _pass_flag(false) {}
Client::Client(int fd, const std::string &ipadd) : _fd(fd), _ipAdd(ipadd)
{
    _nickname = "";
    _username = "";
    _realname = "";
    _modes.clear();
    _channels.clear();
    _connexion_password = false;
    _hasNick = false;
    _hasUser = false;
    _registrationDone = false;
    _to_deconnect = false;
    _pass_flag = false; // パスワード接続フラグ（PASSコマンドによる）
}
Client::~Client() {}

int Client::getFd() const
{
    return _fd;
}

void Client::setFd(int newfd) { _fd = newfd; }

void Client::setIpAdd(const std::string &ipadd) { _ipAdd = ipadd; }

std::string Client::getIpAdd() const { return _ipAdd; }

std::string Client::getNickname() const { return _nickname; }
void Client::setNickname(const std::string &nick) { _nickname = nick; }

std::string Client::getUsername() const { return _username; }
void Client::setUsername(const std::string &user) { _username = user; }

std::string Client::getRealname() const { return _realname; }
void Client::setRealname(const std::string &realname) { _realname = realname; }

// std::string Client::getHostname() const { return _hostname; }
// void Client::setHostname(const std::string& host) { _hostname = host; }

const std::set<char> &Client::getModes() const { return _modes; }
void Client::addMode(char mode)
{
    if (_modes.find(mode) == _modes.end()) // モードがまだ追加されていない場合のみ追加
    {
        _modes.insert(mode);
    }
}
void Client::removeMode(char mode)
{
    // _modes.erase(mode); // C++17 以降、std::string の erase メソッドで文字を削除できます
    std::set<char>::iterator it = _modes.find(mode);
    //  it = std::remove(_modes.begin(), _modes.end(), mode);
    if (it != _modes.end())
    {
        _modes.erase(it, _modes.end());
    }
}
bool Client::hasMode(char mode) const
{
    return _modes.find(mode) != _modes.end();
}

bool &Client::getConnexionPassword() { return (_connexion_password); }
bool &Client::hasNick() { return (_hasNick); }
bool &Client::hasUser() { return (_hasUser); }
bool &Client::isRegistrationDone() { return (_registrationDone); }
bool &Client::getDeconnexionStatus() { return (_to_deconnect); }
bool &Client::getPassFlag() { return (_pass_flag); } // パスワード接続フラグ（PASSコマンドによる）

bool Client::isInChannel(Channel *channel) const
{
    // チャンネルがクライアントのチャンネルマップに存在するか確認
    return _channels.find(channel->getName()) != _channels.end();
    // return _channels.find(channel) != _channels.end();
}

void Client::addChannel(Channel *channel)
{
    _channels[channel->getName()] = channel;
}
void Client::removeChannel(Channel *channel)
{
    _channels.erase(channel->getName());
}

const std::map<std::string, Channel *> &Client::getChannels() const { return _channels; }