/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 17:12:33 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 08:44:54 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

static void modeUser(Server *server, Client *client, ParsedMessage &msg)
{
    std::string nickname = msg.params[0];

    // 他人のモードは変更できない
    if (nickname != client->getNickname())
    {
        server->addToClientBuffer(client->getFd(), ERR_USERSDONTMATCH(client->getNickname()));
        return; // 他人のモードを変更しようとした場合はエラーを返す
    }

    if (msg.params.size() == 1)
    {
        // 現在のユーザーモードを表示
        if (client->getModes().empty())
            server->addToClientBuffer(client->getFd(), RPL_UMODEIS(client->getNickname(), ""));
        else
        {
            // std::string modes = "+" + client->getModes();
            std::set<char> modes = client->getModes();
            std::string modes_msg = "+" + std::string(modes.begin(), modes.end());
            server->addToClientBuffer(client->getFd(), RPL_UMODEIS(client->getNickname(), modes_msg));
        }
        // server->addToClientBuffer(client->getFd(), RPL_UMODEIS(client->getNickname(), client->getModes()));
        return;
    }

    std::string mode_str = msg.params[1];
    if (mode_str[0] != '+' && mode_str[0] != '-')
    {
        server->addToClientBuffer(client->getFd(), ERR_UMODEUNKNOWNFLAG(client->getNickname()));
        return; // モードの先頭が '+' または '-' でない場合はエラーを返す
    }

    bool add_mode = true;
    // std::string applied_modes;
    // size_t param_index = 2; // params[0]=channel, params[1]=modes, params[2]以降=引数

    for (size_t i = 0; i < mode_str.size(); ++i)
    {
        if (mode_str[i] == '+')
            add_mode = true;
        else if (mode_str[i] == '-')
            add_mode = false;
        else
        {
            if (mode_str[i] == 'i')
            {
                if (add_mode)
                {
                    if (!client->hasMode('i'))
                    {
                        client->addMode('i');
                        // applied_modes += "+i";
                    }
                }
                else
                {
                    if (client->hasMode('i'))
                    {
                        client->removeMode('i');
                        // applied_modes += "-i";
                    }
                }
            }
            else if (mode_str[i] == 'o')
            {
                if (!add_mode)
                {
                    if (client->hasMode('o'))
                    {
                        client->removeMode('o');
                        // applied_modes += "-o";
                    }
                }
                else
                {
                    // 通常ユーザー自身は +o はできない → 無視
                    server->addToClientBuffer(client->getFd(), ERR_UMODEUNKNOWNFLAG(client->getNickname()));
                }
            }
            else
            {
                server->addToClientBuffer(client->getFd(), ERR_UMODEUNKNOWNFLAG(client->getNickname()));
            }
        }
    }

    // 実行者に結果を返す
    // if (!applied_modes.empty())
    // {
    // server->addToClientBuffer(client->getFd(), MODE_USERMSG(client->getNickname(), applied_modes));
    server->addToClientBuffer(client->getFd(), MODE_USERMSG(client->getNickname(), mode_str));
    // }
}

static void modeChannel(Server *server, Client *client, ParsedMessage &msg)
{
    std::string channel_name = msg.params[0];
    if (channel_name[0] == '#' || channel_name[0] == '&')
        channel_name = channel_name.substr(1); // チャンネル名の先頭の # を削除
    Channel *channel = server->getChannel(channel_name);
    if (!channel)
    {
        server->addToClientBuffer(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickname(), channel_name));
        return; // チャンネルが存在しない場合はエラーを返す
    }

    std::set<char> modes = channel->getModes();
    if (!channel->hasClient(*client))
    {
        server->addToClientBuffer(client->getFd(), ERR_NOTONCHANNEL(client->getNickname(), channel_name));
        return; // クライアントがチャンネルに参加していない場合はエラーを返す
    }

    if (msg.params.size() == 1)
    {
        // 現在のモード表示
        if (channel->getModes().empty())
            server->addToClientBuffer(client->getFd(), RPL_CHANNELMODEIS(client->getNickname(), channel_name, ""));
        else
        {

            std::string modes_msg = "+" + std::string(modes.begin(), modes.end());
            server->addToClientBuffer(client->getFd(), RPL_CHANNELMODEIS(client->getNickname(), channel_name, modes_msg));
        }
        return; // モードが設定されていない場合はメッセージを返す
    }

    if (!channel->isOperator(client->getNickname()))
    {
        server->addToClientBuffer(client->getFd(), ERR_CHANOPRIVSNEEDED(client->getNickname(), channel_name));
        return; // オペレーターでない場合はエラーを返す
    }

    std::string mode_str = msg.params[1];

    if (mode_str[0] != '+' && mode_str[0] != '-')
    {
        server->addToClientBuffer(client->getFd(), ERR_UMODEUNKNOWNFLAG(client->getNickname()));
        return; // モードの先頭が '+' または '-' でない場合はエラーを返す
    }

    bool add_mode = true;
    size_t param_index = 2; // params[0]=channel, params[1]=modes, params[2]以降=引数

    for (size_t i = 0; i < mode_str.size(); ++i)
    {
        if (mode_str[i] == '+')
            add_mode = true;
        else if (mode_str[i] == '-')
            add_mode = false;
        else
        {
            char mode = mode_str[i];
            if (mode == 'k' || mode == 'l' || mode == 'o')
            {
                if (add_mode)
                {
                    if (param_index >= msg.params.size())
                    {
                        server->addToClientBuffer(client->getFd(), ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
                        continue;
                    }
                    std::string param = msg.params[param_index++];
                    if (mode == 'k')
                        channel->setPassword(param);
                    else if (mode == 'l')
                    {
                        int limit;
                        std::istringstream iss(param);
                        if (!(iss >> limit) || limit < 0)
                        {
                            server->addToClientBuffer(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickname(), channel_name, "l", param));
                            continue; // 無効な数値の場合はエラーを返す
                        }
                        channel->setUserLimit(limit);
                    }
                    else if (mode == 'o')
                        channel->addOperator(param);
                }
                else
                {
                    if (mode == 'k')
                        channel->removePassword();
                    else if (mode == 'l')
                        channel->setUserLimit(-1); // 制限なしに設定
                    else if (mode == 'o')
                    {
                        if (param_index >= msg.params.size())
                        {
                            server->addToClientBuffer(client->getFd(), ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
                            continue;
                        }
                        std::string param = msg.params[param_index++];
                        channel->removeOperator(param);
                    }
                }
            }
            else if (mode == 'i' || mode == 't')
            {
                if (add_mode)
                    channel->addMode(mode);
                else
                    channel->removeMode(mode);
            }
        }
    }

    // 変更を全員に通知
    std::string mode_message = MODE_CHANNELMSG(channel_name, mode_str);
    channel->broadcast(server, mode_message);
}

void mode(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
        return; // クライアントが見つからない場合は何もしない

    if (msg.params.empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
        return; // 引数が足りない場合はエラーを返す
    }

    std::string target = msg.params[0];
    if (target[0] == '#') // チャンネル名の場合
        modeChannel(server, client, msg);
    else // ニックネームの場合
        modeUser(server, client, msg);
}