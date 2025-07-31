/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 14:38:50 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 08:51:03 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void kick(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (msg.params.size() < 2)
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "KICK"));
        return; // 引数が足りない場合はエラーを返す
    }

    std::string channel_name = msg.params[0];
    std::string target_nick = msg.params[1];
    std::string comment = (msg.params.size() > 2) ? msg.params[2] : client->getNickname();

    if (channel_name[0] == '#' || channel_name[0] == '&')
        channel_name = channel_name.substr(1); // チャンネル名の先頭の # を削除
    Channel *channel = server->getChannel(channel_name);
    if (!channel)
    {
        server->addToClientBuffer(client_fd, ERR_NOSUCHCHANNEL(client->getNickname(), channel_name));
        return; // チャンネルが存在しない場合はエラーを返す
    }

    if (!channel->hasClient(*client))
    {
        server->addToClientBuffer(client_fd, ERR_NOTONCHANNEL(client->getNickname(), channel_name));
        // ERR_USERNOTINCHANNEL(requester_nick, target_nick, channel_name)
        return; // クライアントがチャンネルに参加していない場合はエラーを返す
    }

    if (!channel->isOperator(client->getNickname()))
    {
        server->addToClientBuffer(client_fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channel_name));
        return; // オペレーターでない場合はエラーを返す
    }

    Client *target_client = server->getClientByNickname(target_nick);
    if (!target_client || !channel->hasClient(*target_client))
    {
        server->addToClientBuffer(client_fd, ERR_USERNOTINCHANNEL(client->getNickname(), target_nick, channel_name));
        return; // 対象ユーザーがチャンネルにいない場合はエラーを返す
    }

    // KICKメッセージをチャンネル内の全員に送信
    // std::string kick_message = ":" + client->getNickname() + " KICK " + channel_name + " " + target_nick + " :" + comment;

    // すべてのクライアントに通知（自分も含む）
    std::map<std::string, Client *> members = channel->getClients();
    for (std::map<std::string, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        server->addToClientBuffer(it->second->getFd(), RPL_KICK(client->getNickname(), channel_name, target_nick, comment));
    }

    if (channel->isOperator(target_nick))
    {
        channel->removeOperator(target_nick); // 対象がオペレーターならオペレーターを削除
    }
    // 対象ユーザーをチャンネルから削除
    channel->removeClient(*target_client);
}
