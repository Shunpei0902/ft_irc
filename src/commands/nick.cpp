/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 16:25:38 by sasano            #+#    #+#             */
/*   Updated: 2025/07/29 18:42:25 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

// NICK コマンドの実装

// クライアントのニックネームが使用中かどうかを確認する
static bool isNicknameInUse(Server *server, std::string &nickname)
{
    std::map<int, Client *> clients = server->getClients();
    for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    // for (const Client &client : server->getClients()) //使えない（C++11)
    {
        if (it->second->getNickname() == nickname)
        {
            return true; // ニックネームが使用中
        }
    }
    return false; // 使用中ではない
}

// ニックネームが有効かどうかを確認する
static bool isValidNickname(const std::string &nickname)
{

    // 先頭は英字でなければならない
    if (nickname.empty() || !isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '_' || nickname[0] == '[' || nickname[0] == ']' || nickname[0] == '\\' || nickname[0] == '`' || nickname[0] == '{' || nickname[0] == '}' || nickname[0] == '^')
    {
        return false; // 無効なニックネーム
    }
    // ニックネームの長さが1文字以上9文字以下であることを確認
    if (nickname.length() < 1 || nickname.length() > 9)
    {
        return false; // 無効なニックネーム
    }
    // ニックネームに使用できない文字が含まれていないことを確認
    for (size_t i = 1; i < nickname.length(); ++i)
    {
        if (!isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_' && nickname[i] != '[' && nickname[i] != ']' && nickname[i] != '\\' && nickname[i] != '`' && nickname[i] != '{' && nickname[i] != '}' && nickname[i] != '^')
        {
            return false; // 無効なニックネーム
        }
    }
    return true; // 有効なニックネーム
}

void nick(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
    {
        return; // クライアントが見つからない場合は何もしない
    }
    // PASS コマンドで接続パスワードが設定されていない場合はエラーを返す
    // NICK コマンドは、クライアントが接続パスワードを設定した後にのみ使用できる
    if (!client->getConnexionPassword())
    {
        // server->addToClientBuffer(client_fd, ERR_PASSWDMISMATCH(client->getNickname()));
        server->addToClientBuffer(client_fd, ERR_NOTREGISTERED(client->getNickname()));
        return;
    }
    // NICK コマンドの引数が空の場合はエラーを返す
    if (msg.params.empty())
    {
        if (msg.trailing.empty())
        {
            server->addToClientBuffer(client_fd, ERR_NONICKNAMEGIVEN(client->getNickname()));
            return;
        }
        // トレーリングメッセージがある場合は、params に追加
        msg.params.push_back(msg.trailing);
        msg.trailing.clear(); // トレーリングメッセージは params に移動したのでクリア
    }
    std::string new_nick = msg.params[0];

    // ニックネームが有効かどうかを確認
    if (!isValidNickname(new_nick))
    {
        server->addToClientBuffer(client_fd, ERR_ERRONEUSNICKNAME(client->getNickname(), new_nick));
        return;
    }
    // すでに使用されているニックネームかどうかを確認
    if (isNicknameInUse(server, new_nick))
    {
        server->addToClientBuffer(client_fd, ERR_NICKNAMEINUSE(client->getNickname(), new_nick));
        return;
    }
    // 既存のニックネームを更新
    std::string old_nick = client->getNickname();
    client->setNickname(new_nick);

    // クライアントの登録情報を更新
    if (!client->hasNick())
    {
        client->hasNick() = true; // NICK コマンドによるニックネーム登録フラグを設定
    }

    // NICK コマンドの応答を送信
    server->addToClientBuffer(client_fd, RPL_NICK(old_nick, client->getUsername(), new_nick));
    // チャンネル内の全クライアントに新しいニックネームを通知
    std::map<std::string, Channel *> channels = client->getChannels();
    if (channels.empty())
    {
        return; // クライアントが参加しているチャンネルがない場合は何もしない
    }
    // チャンネルごとにクライアントに新しいニックネームを通知
    // for (Channel *channel : client->getChannels())
    for (std::map<std::string, Channel *>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel *channel = it->second;
        if (!channel->hasClient(*client))
        {
            continue; // クライアントがチャンネルに参加していない場合はスキップ
        }

        // チャンネル内の全クライアントに新しいニックネームを通知
        // std::string nick_change_message = ":" + old_nick + " NICK " + new_nick;
        // channel->broadcast(server, nick_change_message);
        channel->broadcast(server, RPL_NICK(old_nick, client->getUsername(), new_nick));
    }
}