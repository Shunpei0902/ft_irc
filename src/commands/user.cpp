/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 19:09:18 by sasano            #+#    #+#             */
/*   Updated: 2025/07/29 18:34:25 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command.hpp"

static bool isValidUsername(const std::string &username)
{
    // ユーザー名の検証ロジックを実装
    // ここでは、ユーザー名が空でないことを確認するだけの簡単なチェックを行う
    return !username.empty();
}

void user(Server *server, int client_fd, ParsedMessage &msg)
{
    Client *client = server->getClient(client_fd);
    if (!client)
    {
        return; // クライアントが見つからない場合は何もしない
    }

    // PASS コマンドで接続パスワードが設定されていない場合はエラーを返す
    if (!client->getConnexionPassword())
    {
        server->addToClientBuffer(client_fd, ERR_NOTREGISTERED(client->getNickname()));
        return;
    }

    // 登録が完了している場合は、USER コマンドを受け付けない
    if (client->isRegistrationDone())
    {
        server->addToClientBuffer(client_fd, ERR_ALREADYREGISTERED(client->getNickname()));
        return;
    }

    // USER コマンドの引数が4つ未満の場合はエラーを返す
    //irssi はデフォルトで4つ全てを返す
    // IRC protocol: USER <username> <hostname> <servername> <realname>
    if (msg.params.size() < 3 || msg.params[0].empty())
    {
        server->addToClientBuffer(client_fd, ERR_NEEDMOREPARAMS(client->getNickname(), "USER"));
        return;
    }

    // ユーザー名、リアル名を設定
    std::string username = msg.params[0];
    // std::string hostname = msg.params[1];  // 通常は無視される
    // std::string servername = msg.params[2]; // 通常は無視される
    std::string realname = msg.params.size() > 3 ? msg.params[3] : ""; // 4つ目の引数があればリアル名として使用
    if (!msg.trailing.empty())
    {
        // トレーリングメッセージがある場合は、リアル名として使用
        realname = msg.trailing;
    }

    // ユーザー名が有効かどうかを確認
    if (!isValidUsername(username))
    {
        server->addToClientBuffer(client_fd, ERR_ERRONEUSNICKNAME(client->getNickname(), username));
        return;
    }

    // クライアントの登録情報を更新
    client->setUsername(username);
    client->setRealname(realname);
    client->hasUser() = true; // USER コマンドによるユーザー名登録フラグを立てる
}
