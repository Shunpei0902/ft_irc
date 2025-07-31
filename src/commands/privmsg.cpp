/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:19:32 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 17:28:29 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void privmsg(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
    {
        return; // クライアントが見つからない場合は何もしない
    }

    // 少なくとも2つのパラメータが必要（ターゲットとメッセージ）
    if (msg.params.empty() || msg.trailing.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PRIVMSG"));
        return;
    }

    std::string message = msg.trailing;
    std::string target_str = msg.params[0];

    // カンマでターゲットを分割
    std::stringstream ss(target_str);
    std::string target;
    std::set<std::string> processed; // 重複を避けるためのセット　例：複数のチャンネルやニックネームに送信する場合

    while (std::getline(ss, target, ','))
    {
        if (target.empty() || processed.count(target))
        {
            continue; // 空または重複ターゲットをスキップ
        }
        processed.insert(target);

        // チャンネル宛
        if (target[0] == '#')
        {
            target = target.substr(1); // チャンネル名の先頭の # を削除
            Channel *channel = server->getChannel(target);
            if (!channel)
            {
                server->addToClientBuffer(client_fd, ERR_NOSUCHCHANNEL(client->getNickname(), target));
                continue; // チャンネルが存在しない場合はエラーを返し、次のターゲットへ
            }
            // channel->sendMessage(client, message);
            // channel->broadcast(server, ":" + client->getNickname() + " PRIVMSG " + target + " :" + message);
            target = "#" + target; // チャンネル名を元に戻す
            channel->broadcast(server, RPL_PRIVMSG(client->getNickname(), client->getUsername(), target, message));
        }
        // ユーザー宛
        else
        {
            Client *recipient = server->getClientByNickname(target);
            if (!recipient)
            {
                server->addToClientBuffer(client_fd, ERR_NOSUCHNICK(client->getNickname(), target));
                continue;
            }
            server->addToClientBuffer(recipient->getFd(), ":" + client->getNickname() + " PRIVMSG " + target + " :" + message);
            server->addToClientBuffer(client_fd, RPL_PRIVMSG(client->getNickname(), client->getUsername(), target, message));
        }
    }
}