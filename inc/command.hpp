/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 20:13:25 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 11:49:07 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "irc.hpp"
#include "numerical_replies.hpp"
#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

class Server;

void pass(Server *server, int client_fd, ParsedMessage &msg);
void nick(Server *server, int client_fd, ParsedMessage &msg);
void user(Server *server, int client_fd, ParsedMessage &msg);
void join(Server *server, int client_fd, ParsedMessage &msg);
void part(Server *server, int client_fd, ParsedMessage &msg);
void privmsg(Server *server, int client_fd, ParsedMessage &msg);
void ping(Server *server, int client_fd, ParsedMessage &msg);
// void kill(Server *server, int client_fd, ParsedMessage &msg);
// void oper(Server *server, int client_fd, ParsedMessage &msg);
void mode(Server *server, int client_fd, ParsedMessage &msg);
// void who(Server *server, int client_fd, ParsedMessage& msg);
// void whois(Server *server, int client_fd, ParsedMessage& msg);
void topic(Server *server, int client_fd, ParsedMessage &msg);
void kick(Server *server, int client_fd, ParsedMessage &msg);
void invite(Server *server, int client_fd, ParsedMessage &msg);
void quit(Server *server, int client_fd, ParsedMessage &msg);
void cap(Server *server, int client_fd, ParsedMessage &msg);

// その他のコマンドもここに追加可能
// 例: void kick(Server *server, int client_fd, const ParsedMessage& msg);