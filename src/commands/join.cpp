/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:48:10 by sasano            #+#    #+#             */
/*   Updated: 2025/08/04 16:02:21 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

// 有効なチャンネル名かどうかをチェックする関数
static bool isValidChannelName(const std::string &name)
{
    if (name.empty() || name.length() > 200)
        return false;
    if (name[0] != '#' && name[0] != '&')
        return false;

    for (size_t i = 1; i < name.length(); ++i)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == '\a')
        {
            return false;
        }
    }
    return true;
}

void join(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (!msg.trailing.empty())
    {
        msg.params.push_back(msg.trailing);
        msg.trailing.clear(); // トレーリングメッセージを引数に追加
    }

    // JOIN コマンドの引数が空の場合はエラーを返す
    if (msg.params.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
        return;
    }

    if (msg.params[0] == "0")
    {
        // 引数が "0" の場合、全てのチャンネルからPARTする
        const std::map<std::string, Channel *> &channels = client->getChannels();
        for (std::map<std::string, Channel *>::const_iterator it = channels.begin(); it != channels.end();)
        // for (const auto &pair : client->getChannels())
        {
            // Channel *channel = pair.second;
            // std::string part_message = ":" + client->getNickname() + " PART " + channel->getName();
            // server->addToClientBuffer(client_fd, part_message);
            // QUIT による PART を模倣する
            ParsedMessage part_msg;
            part_msg.command = "PART";
            part_msg.params.push_back(it->second->getName());
            part_msg.trailing.clear();
            ++it; // 次のチャンネルへ進む
            part(server, client_fd, part_msg);
        }
        return; // 全てのチャンネルからPARTしたので終了
    }

    std::string nick = client->getNickname();
    // カンマで区切られたチャンネル名とキーを取得
    std::vector<std::string> channels = split(msg.params[0], ',');
    std::vector<std::string> keys;
    if (msg.params.size() > 1)
    {
        keys = split(msg.params[1], ',');
    }
    std::set<std::string> processed; // 重複を避けるためのセット

    // 各チャンネルに対して処理を行う
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channel_name = channels[i];

        if (processed.find(channel_name) != processed.end())
        {
            continue; // 既に処理済みのチャンネルはスキップ
        }
        processed.insert(channel_name); // チャンネル名をセットに追加

        if (isValidChannelName(channel_name) == false)
        {
            server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
            continue; // 無効なチャンネル名はスキップ
        }
        // 先頭の"#"や"&"を除去
        if (channel_name[0] == '#' || channel_name[0] == '&')
        {
            channel_name = channel_name.substr(1);
        }

        // チャンネルが存在しない場合は新規作成
        Channel *channel = server->getChannel(channel_name);
        if (!channel)
        {
            channel = new Channel(channel_name);
            server->addChannel(channel);
        }
        // チャンネルに参加しているか確認
        if (channel->hasClient(*client))
        {
            server->addToClientBuffer(client_fd, ERR_ALREADYJOINED(nick, channel_name));
            continue; // 既に参加している場合はスキップ
        }
        // チャンネルのパスワードが設定されている場合、キーを確認
        if (!channel->getPassword().empty())
        {
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (key != channel->getPassword())
            {
                server->addToClientBuffer(client_fd, ERR_BADCHANNELKEY(nick, channel_name));
                continue; // パスワードが一致しない場合はスキップ
            }
        }
        // チャンネルのユーザー制限を確認
        if (channel->getUserLimit() != -1 && channel->getClients().size() >= static_cast<size_t>(channel->getUserLimit()))
        {
            server->addToClientBuffer(client_fd, ERR_CHANNELISFULL(nick, channel_name));
            continue; // チャンネルが満員の場合はスキップ
        }
        // // チャンネルにバンされているか確認
        // if (channel->isBanned(nick))
        // {
        //     server->addToClientBuffer(client_fd, ERR_BANNEDFROMCHAN(nick, channel_name));
        //     continue; // バンされている場合はスキップ
        // }
        // Invite-onlyモードのチャンネルに参加する場合、オペレーターからの招待が必要
        if (channel->hasMode('i') && !channel->isInvited(nick))
        {
            server->addToClientBuffer(client_fd, ERR_INVITEONLYCHAN(nick, channel_name));
            continue; // 招待制のチャンネルに参加する場合はスキップ
        }

        // JOIN処理
        channel->addClient(*client);
        if (channel->getOperators().empty())
        {
            channel->addOperator(nick); // 最初の参加者をオペレーターにする
        }

        // JOIN通知
        // std::string join_message = ":" + nick + " JOIN " + channel_name;
        // server->addToClientBuffer(client_fd, join_message);
        // 他のメンバーにJOIN通知
        // for (std::map<std::string, Client *>::const_iterator it = channel->getClients().begin();
        //      it != channel->getClients().end(); ++it)
        // {
        //     if (it->second->getFd() != client_fd)
        //     {
        //         server->addToClientBuffer(it->second->getFd(), join_msg);
        //     }
        // }
        channel->broadcast(server, RPL_JOIN(nick, channel_name));

        // トピックがあれば送信
        if (!channel->getTopic().empty())
        {
            server->addToClientBuffer(client_fd, RPL_TOPIC(nick, channel_name, channel->getTopic()));
        }
        else
        {
            server->addToClientBuffer(client_fd, RPL_NOTOPIC(nick, channel_name));
        }
    }
}
