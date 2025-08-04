/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 15:28:49 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 19:00:04 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void topic(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (!msg.trailing.empty())
    {
        msg.params.push_back(msg.trailing);
        msg.trailing.clear(); // トレーリングメッセージを引数に追加
    }
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

    // Check if we have topic content to set (either in params[1] or trailing)
    bool has_topic_content = (msg.params.size() >= 2) || !msg.trailing.empty();
    
    if (!has_topic_content)
    {
        // トピックを取得してクライアントに送信 (no topic provided, just display current)
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
        // トピックを設定 (check if user is operator when topic mode is set)
        if (channel->hasMode('t') && !channel->isOperator(client->getNickname()))
        {
            server->addToClientBuffer(client_fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channel_name));
            return; // オペレーターでない場合はエラーを返す
        }
        
        // Get topic from params[1] or trailing message
        std::string new_topic;
        if (msg.params.size() >= 2)
        {
            new_topic = msg.params[1];
        }
        else if (!msg.trailing.empty())
        {
            new_topic = msg.trailing;
        }
        else
        {
            new_topic = ""; // Empty topic
        }
        
        channel->setTopic(new_topic);

        // トピック設定のメッセージをチャンネル内の全員に送信
        channel->broadcast(server, RPL_TOPIC(client->getNickname(), channel_name, ":" + new_topic));
        // const std::map<std::string, Client> &members = channel->getClients();
        // for (const auto &member : members)
        // {
        //     server->addToClientBuffer(member.second.getFd(), topic_message);
        // }
    }
}