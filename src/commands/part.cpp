/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:48:27 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 07:34:35 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void part(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない}

    if (!msg.trailing.empty())
    {
        msg.params.push_back(msg.trailing);
        msg.trailing.clear(); // トレーリングメッセージを引数に追加
    }

    // PART コマンドの引数が空の場合はエラーを返す
    if (msg.params.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PART"));
        return;
    }

    // チャンネル名のリスト取得
    std::vector<std::string> channels = split(msg.params[0], ',');

    // PARTメッセージの理由（共通）を取得
    std::string part_msg;
    if (msg.params.size() > 1)
        part_msg = msg.params[1];
    else if (!msg.trailing.empty())
        part_msg = msg.trailing;

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channel_name = channels[i];
        if (channel_name[0] == '#' || channel_name[0] == '&')
        {
            channel_name = channel_name.substr(1); // チャンネル名の先頭の # または & を削除
        }

        // チャンネルが存在するか確認
        Channel *channel = server->getChannel(channel_name);
        if (!channel)
        {
            server->addToClientBuffer(client_fd, ERR_NOSUCHCHANNEL(client->getNickname(), channel_name));
            continue;
        }

        // クライアントがチャンネルに参加しているか確認
        if (!channel->hasClient(*client))
        {
            server->addToClientBuffer(client_fd, ERR_NOTONCHANNEL(client->getNickname(), channel_name));
            continue;
        }

        // PARTメッセージの生成
        // std::string part_message = ":" + client->getNickname() + " PART " + channel_name;
        // if (!part_msg.empty())
        // {
        //     part_message += " :" + part_msg; // トレーリングメッセージがあれば追加
        // }

        // すべてのクライアントに通知（自分も含む）
        const std::map<std::string, Client *> members = channel->getClients();
        for (std::map<std::string, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
        {
            server->addToClientBuffer(it->second->getFd(), RPL_PART(client->getNickname(), channel_name, part_msg));
        }

        // クライアントをチャンネルから削除
        channel->removeClient(*client);
        channel->removeOperator(client->getNickname()); // オペレーターからも削除
        if (channel->empty())
        {
            server->removeChannel(channel_name); // チャンネルが空になったら削除
        }
    }
}