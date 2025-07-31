/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 14:20:43 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 18:17:00 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

// PASSコマンドのパラメータからパスワードを抽出する
static std::string getPasswordFromMessage(const ParsedMessage &msg)
{
    if (msg.params.empty())
    {
        return "";
    }

    std::string password = msg.params[0];

    // IRCメッセージ構文で ':' は末尾パラメータの印であり除去する
    if (!password.empty() && password[0] == ':')
    {
        password = password.substr(1);
    }

    return password;
}

void pass(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
    {
        return; // クライアントが見つからない場合は失敗
    }

    // 登録完了後にはPASSを受け入れない
    if (client->isRegistrationDone())
    {
        server->addToClientBuffer(client_fd, ERR_ALREADYREGISTERED(client->getNickname()));
        return;
    }
    // サーバーのパスワードが設定されていない場合は、PASS コマンドを無視
    if (server->getPassword().empty())
    {
        // server->addToClientBuffer(client_fd, RPL_PASS(client->getNickname()));
        client->getPassFlag() = true; // パスワード接続フラグを立てる
        return;                       // 成功として扱う
    }

    // クライアントの接続パスワードがすでに設定されている場合は、PASS コマンドを無視(リプライメッセージは送信しない)
    if (client->getConnexionPassword())
    {
        client->getPassFlag() = true; // パスワード接続フラグを立てる
        return;
    } // 成功として扱う

    // トレーリングメッセージがある場合は、params に追加
    if (msg.params.empty() && !msg.trailing.empty())
    {
        // トレーリングメッセージがある場合は、params に追加
        msg.params.push_back(msg.trailing);
        msg.trailing.clear(); // トレーリングメッセージは params に移動したのでクリア
    }

    // PASS コマンドの引数が空または複数ある場合はエラーを返す
    if (msg.params.empty() || msg.params.size() > 1)
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PASS"));
        return;
    }

    // メッセージからパスワードを取得
    std::string password = getPasswordFromMessage(msg);

    // サーバーのパスワードと一致するか確認
    if (password != server->getPassword())
    {
        server->addToClientBuffer(client_fd, ERR_PASSWDMISMATCH(client->getNickname()));
        return; // パスワードが一致しない場合は失敗
    }
    // パスワードが一致した場合、接続パスワードフラグを設定
    client->getConnexionPassword() = true;
    // server->addToClientBuffer(client_fd, RPL_PASS(client->getNickname()));
    client->getPassFlag() = true; // パスワード接続フラグを立てる
    return;                       // パスワードが一致した場合は成功
}