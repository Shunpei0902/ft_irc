/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 16:42:59 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 01:32:51 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void ping(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
    {
        return; // クライアントが見つからない場合は何もしない
    }

    std::string ping_token;
    
    // Check for token in params first, then in trailing
    if (!msg.params.empty())
    {
        ping_token = msg.params[0];
    }
    else if (!msg.trailing.empty())
    {
        ping_token = msg.trailing;
    }
    else
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PING"));
        return;
    }

    // PONG メッセージを送信 (RFC format: PONG server :token)
    std::string response = ":localhost PONG localhost :" + ping_token + "\r\n";
    server->addToClientBuffer(client_fd, response);
}