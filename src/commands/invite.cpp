/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 15:01:30 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 08:53:07 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void invite(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (msg.params.size() < 2)
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE"));
        return; // 引数が足りない場合はエラーを返す
    }

    std::string target_nick = msg.params[0];
    std::string channel_name = msg.params[1];
    if (channel_name[0] == '#' || channel_name[0] == '&')
        channel_name = channel_name.substr(1); // チャンネル名の先頭の # を削除
    std::cout << "invite: " << target_nick << " to " << channel_name << std::endl;
    Channel *channel = server->getChannel(channel_name);
    if (!channel)
    {
        server->addToClientBuffer(client_fd, ERR_NOSUCHCHANNEL(client->getNickname(), channel_name));
        return; // チャンネルが存在しない場合はエラーを返す
    }

    if (!channel->hasClient(*client))
    {
        server->addToClientBuffer(client_fd, ERR_NOTONCHANNEL(client->getNickname(), channel_name));
        return; // クライアントがチャンネルに参加していない場合はエラーを返す
    }

    // チャンネルが招待制であり、クライアントがオペレーターでない場合はエラーを返す
    if (channel->isInviteOnly() && !channel->isOperator(client->getNickname()))
    {
        server->addToClientBuffer(client_fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channel_name));
        return; // オペレーターでない場合はエラーを返す
    }

    Client *target_client = server->getClientByNickname(target_nick);
    if (!target_client)
    {
        server->addToClientBuffer(client_fd, ERR_NOSUCHNICK(client->getNickname(), target_nick));
        return; // 対象ユーザーが存在しない場合はエラーを返す
    }

    if (channel->hasClient(*target_client))
    {
        server->addToClientBuffer(client_fd, ERR_USERONCHANNEL(client->getNickname(), target_nick, channel_name));
        return; // 対象ユーザーが既にチャンネルにいる場合はエラーを返す
    }

    // チャンネルの招待リストに対象ユーザーを追加
    channel->addInvite(target_nick);

    // クライアントに成功メッセージを送信
    server->addToClientBuffer(client_fd, RPL_INVITING(client->getNickname(), client->getNickname(), target_nick, channel_name));

    // INVITEメッセージを対象ユーザーに送信
    std::string invite_message = RPL_INVITE(client->getNickname(), target_nick, channel_name);
    server->addToClientBuffer(target_client->getFd(), invite_message);
}
