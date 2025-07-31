/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cap.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 11:45:28 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 11:50:08 by sasano           ###   ########.fr       */
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
    // if (subcmd == "LS")
    // {
    //     // CAP LS コマンドの処理
    //     std::string capabilities = "multi-prefix";
    //     server->addToClientBuffer(client_fd, RPL_CAPABILITIES(client->getNickname(), capabilities));
    // }
    // else if (subcmd == "ACK")
    // {
    //     // CAP ACK コマンドの処理
    //     if (msg.params.size() < 2)
    //     {
    //         server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "CAP ACK"));
    //         return; // 引数が足りない場合はエラーを返す
    //     }
    //     std::string capability = msg.params[1];
    //     client->addCapability(capability);
    //     server->addToClientBuffer(client_fd, RPL_CAPACK(client->getNickname(), capability));
    // }
    // else if (subcmd == "NAK")
    // {
    //     // CAP NAK コマンドの処理
    //     if (msg.params.size() < 2)
    //     {
    //         server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "CAP NAK"));
    //         return; // 引数が足りない場合はエラーを返す
    //     }
    //     std::string capability = msg.params[1];
    //     client->removeCapability(capability);
    //     server->addToClientBuffer(client_fd, RPL_CAPNAK(client->getNickname(), capability));
    // }
    // else
    // {
    //     server->addToClientBuffer(client_fd, ERR_UNKNOWNCAPABILITY(client->getNickname(), subcmd));
    // }
}