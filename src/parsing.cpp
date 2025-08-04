/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 20:12:10 by sasano            #+#    #+#             */
/*   Updated: 2025/08/04 15:35:19 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "numerical_replies.hpp"
#include "server.hpp"
#include "client.hpp"
#include "command.hpp"

// 文字列 input を空白文字で分割し、vector にして返す。
//  : より前の部分（コマンドやパラメータ）を解析するため
static std::vector<std::string> split(const std::string &input)
{
	if (input.empty())
	{
		return std::vector<std::string>(); // 空の文字列は空のベクターを返す
	}
	// input 文字列から 文字列ストリームを生成
	std::stringstream ss(input);
	std::string word;
	std::vector<std::string> tokens;
	// ストリームから空白（スペース、タブ、改行）で区切られた単語を読み取り、tokens ベクターに追加
	while (ss >> word)
	{
		tokens.push_back(word);
	}
	return tokens;
}

// 受信したメッセージを解析して、コマンド名、引数、トレーリングメッセージを取得する
// 例: :dan!d@localhost PRIVMSG #chan :Hello
// raw はクライアントから受け取った1行のメッセージ
static ParsedMessage parseMessage(const std::string &raw)
{
	ParsedMessage msg;
	std::string subster = raw;
	if (raw.empty())
	{
		return msg; // 空のメッセージはそのまま返す
	}
	// prefix の抽出
	if (raw[0] == ':')
	{
		size_t prefix_end = raw.find(' ');
		if (prefix_end != std::string::npos)
		{
			msg.prefix = raw.substr(1, prefix_end - 1);
			subster = raw.substr(prefix_end + 1);
		}
	}

	// トレーリングの抽出
	size_t pos = raw.find(" :"); // トレーリングメッセージの開始位置を探す
	if (pos != std::string::npos)
	{
		msg.trailing = subster.substr(pos + 2); // トレーリングメッセージ
		subster = subster.substr(0, pos);		// トレーリングメッセージを除いた部分
	}
	else
		msg.trailing = ""; // トレーリングメッセージがない場合
	// コマンド名と引数を分割
	std::vector<std::string> tokens = split(subster);
	if (!tokens.empty())
	{
		msg.command = tokens[0];
		msg.params.assign(tokens.begin() + 1, tokens.end());
	}

	return msg;
}

// コマンド実行処理
void Server::executeCommand(ParsedMessage &msg, int client_fd)
{
	if (msg.command.empty())
		return;

	typedef void (*CommandFunc)(Server *, int, ParsedMessage &);
	// コマンドを実行するための関数ポインタのマップ
	static std::map<std::string, CommandFunc> commandMap;
	if (commandMap.empty())
	{
		commandMap["INVITE"] = invite;
		commandMap["JOIN"] = join;
		commandMap["KICK"] = kick;
		// commandMap["KILL"] = kill;
		// commandMap["LIST"] = list;
		commandMap["MODE"] = mode;
		// commandMap["MOTD"] = motd;
		// commandMap["NAMES"] = names;
		commandMap["NICK"] = nick;
		// commandMap["NOTICE"] = notice;
		// commandMap["OPER"] = oper;
		commandMap["PART"] = part;
		commandMap["PASS"] = pass;
		commandMap["PING"] = ping;
		commandMap["PRIVMSG"] = privmsg;
		commandMap["QUIT"] = quit;
		commandMap["TOPIC"] = topic;
		commandMap["USER"] = user;
	}

	Client *client = getClient(client_fd);
	if (!client)
	{
		std::cout << "Client not found for fd: " << client_fd << std::endl;
		return; // クライアントが見つからない場合は何もしない
	}

	// コマンドがマップに存在するか確認

	std::map<std::string, CommandFunc>::const_iterator it = commandMap.find(msg.command);
	if (it != commandMap.end())
	{
		// コマンドが見つかった場合、対応する関数を呼び出す
		it->second(this, client_fd, msg);
	}
	else
	{
		// 未知のコマンドに対するエラーメッセージ送信
		addToClientBuffer(client_fd, ERR_UNKNOWNCOMMAND(client->getNickname(), msg.command));
	}
}

void Server::handleClientRegistrationCommand(std::map<int, Client *> &client, int client_fd, ParsedMessage &msg)
{
	// クライアントの情報を取得
	std::map<int, Client *>::iterator it = client.find(client_fd);
	if (it == client.end())
	{
		// クライアントが見つからない場合は何もしない
		return;
	}
	if (msg.command == "CAP")
	{
		cap(this, client_fd, msg);
	}
	else if (msg.command == "NICK")
	{
		nick(this, client_fd, msg);
	}
	else if (msg.command == "USER")
	{
		user(this, client_fd, msg);
	}
	else if (msg.command == "PASS")
	{
		pass(this, client_fd, msg);
		if (it->second->getPassFlag())
			it->second->getConnexionPassword() = true; // パスワード接続フラグを立てる
		else
			it->second->getConnexionPassword() = false; // パスワード接続フラグを下げる
	}
	else
	{
		// 登録が完了していない状態での未知のコマンド
		addToClientBuffer(client_fd, ERR_NOTREGISTERED(it->second->getNickname()));
		std::cout << "Unknown command during registration: " << msg.command << std::endl;
		return;
	}
}

static void sendClientRegistration(Server *server, int client_fd, std::map<int, Client *>::iterator &it)
{
	time_t now = time(NULL);
	// クライアントに登録情報を送信
	server->addToClientBuffer(client_fd, RPL_WELCOME(user_id(it->second->getNickname(), it->second->getUsername()), it->second->getNickname()));
	server->addToClientBuffer(client_fd, RPL_YOURHOST(it->second->getNickname(), "localhost", "ft_irc"));
	server->addToClientBuffer(client_fd, RPL_CREATED(it->second->getNickname(), static_cast<std::string>(ctime(&now)).substr(0, 24)));
	server->addToClientBuffer(client_fd, RPL_MYINFO(it->second->getNickname(), "localhost", "ft_irc", "", "", ""));
	std::cout << "Client registration complete for fd: " << client_fd << std::endl;
}

// クライアントからの1行を解析 -> コマンドを実行
void Server::handleClientMessage(const std::string &line, int client_fd)
{
	ParsedMessage msg = parseMessage(line);
	if (msg.command.empty())
		return; // コマンドが空の場合は何もしない

	// クライアントの情報を取得
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	if (it == _clients.end())
	{
		// クライアントが見つからない場合は何もしない
		return;
	}

	std::cout << "Received command: " << msg.command << " from client fd: " << client_fd << std::endl;
	// 登録が完了していない場合の処理（NICK/USERによる認証）
	if (it->second->isRegistrationDone() == false)
	{
		// クライアントの初期コマンドを処理
		// クライアント情報収集中（NICKとUSERの取得）
		// if (it->second.hasAllInfo() == false)
		if (it->second->hasNick() == false || it->second->hasUser() == false)
		{
			// クライアントの初期コマンドを処理
			// コマンドを解析して、NICK, USERコマンドによる情報をクライアント構造体に格納
			handleClientRegistrationCommand(_clients, client_fd, msg);
		}
		// 全情報取得後のWELCOME処理
		// 情報がそろっていて WELCOME をまだ送っていなければ
		if (it->second->hasNick() == true && it->second->hasUser() == true)
		{
			// クライアントに登録情報を送信
			// 001 〜 004 のサーバーメッセージを送信
			sendClientRegistration(this, client_fd, it);
			it->second->isRegistrationDone() = true; // 登録完了フラグを立てる
		}
	}
	else
		executeCommand(msg, client_fd);
}