/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 18:35:23 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 02:36:34 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void quit(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない}

    // QUIT メッセージを全クライアントに送信
    // message のトレーリング部分が空でない場合は、QUIT メッセージとして使用
    std::string quit_msg = msg.trailing.empty() ? "Client Quit" : msg.trailing;
    // std::string quit_message = ":" + client->getNickname() + " QUIT :" + quit_msg;

    const std::map<int, Client *> &clients = server->getClients();
    for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        // クライアントにメッセージを送信
        server->addToClientBuffer(it->second->getFd(), RPL_QUIT(client->getNickname(), quit_msg));
    }
    // クライアントが参加しているチャンネルからクライアントを削除
    const std::map<std::string, Channel *> &channels = client->getChannels();
    for (std::map<std::string, Channel *>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        // クライアントをチャンネルから削除
        // QUIT による PART を模倣する
        ParsedMessage part_msg;
        part_msg.command = "PART";
        part_msg.params.push_back(it->second->getName());
        // part_msg.trailing = quit_msg;
        part(server, client_fd, part_msg);
    }
    // クライアントをサーバーから切断
    server->clearClients(client_fd);
}