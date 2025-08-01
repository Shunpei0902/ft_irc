/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 15:28:49 by sasano            #+#    #+#             */
/*   Updated: 2025/07/30 19:15:28 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void topic(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (msg.params.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "TOPIC"));
        return; // 引数が足りない場合はエラーを返す
    }

    std::string channel_name = msg.params[0];
    if (channel_name[0] == '#' || channel_name[0] == '&')
    {
        channel_name = channel_name.substr(1); // 先頭の"#"や"&"を除去
    }
    Channel *channel = server->getChannel(channel_name);
    if (!channel)
    {
        server->addToClientBuffer(client_fd, ERR_NOSUCHCHANNEL(client->getNickname(), channel_name));
        return; // チャンネルが存在しない場合はエラーを返す
    }

    if (msg.params.size() == 1)
    {
        // トピックを取得してクライアントに送信
        std::string topic = channel->getTopic();
        if (topic.empty())
        {
            server->addToClientBuffer(client_fd, RPL_NOTOPIC(client->getNickname(), channel_name));
        } // トピックが設定されていない場合はメッセージを返す
        else
        {
            server->addToClientBuffer(client_fd, RPL_TOPIC(client->getNickname(), channel_name, topic));
        }
    }
    else
    {
        if (!channel->hasClient(*client))
        {
            server->addToClientBuffer(client_fd, ERR_NOTONCHANNEL(client->getNickname(), channel_name));
            return; // クライアントがチャンネルに参加していない場合はエラーを返す
        }
        // トピックを設定
        if (channel->hasMode('t') && !channel->isOperator(client->getNickname()))
        {
            server->addToClientBuffer(client_fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channel_name));
            return; // オペレーターでない場合はエラーを返す
        }
        std::string new_topic = msg.params[1];
        channel->setTopic(new_topic);

        // トピック設定のメッセージをチャンネル内の全員に送信
        channel->broadcast(server, RPL_TOPIC(client->getNickname(), channel_name, new_topic));
        // const std::map<std::string, Client> &members = channel->getClients();
        // for (const auto &member : members)
        // {
        //     server->addToClientBuffer(member.second.getFd(), topic_message);
        // }
    }
}