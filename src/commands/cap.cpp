/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cap.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 11:45:28 by sasano            #+#    #+#             */
/*   Updated: 2025/08/04 16:05:33 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

void cap(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (msg.params.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "CAP"));
        return; // 引数が足りない場合はエラーを返す
    }

    std::string subcmd = msg.params[0];
    if (subcmd == "LS")
    {
        // Capabilityを返す（今回は空でも可）
        std::string response = "CAP * LS :\r\n";        // 何もサポートしていないときは空でOK
        server->addToClientBuffer(client_fd, response); // クライアントに返信
    }
    else if (subcmd == "REQ")
    {
        // 受信したCAP REQに対する応答（無視も可能）
        server->addToClientBuffer(client_fd, "CAP * ACK :\r\n");
    }
    else if (subcmd == "END")
    {
        // Capability交渉の完了 → 特に処理不要な場合もある
    }
    return;
}