/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 22:10:18 by sasano            #+#    #+#             */
/*   Updated: 2025/07/30 15:46:58 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int argc, char **argv)
{

	if (argc == 3)
	{
		Server ser;
		time_t rawtime;
		struct tm *timeinfo;

		// 現在のローカル時刻を timeinfo に取得
		time(&rawtime);
		timeinfo = localtime(&rawtime);

		try
		{
			// SignalHandler を設定して、サーバーの初期化と起動を行う
			signal(SIGINT, Server::signalHandler);		//-> catch the signal (ctrl + c)
			signal(SIGQUIT, Server::signalHandler);		//-> catch the signal (ctrl + \)
			ser.serverInit(argv[1], argv[2], timeinfo); //-> initialize the server

			// // 設定ファイルからサーバーの設定を読み込む
			// char filename[39] = "srcs/config/ManageServOperators.config";
			// ser.readFromConfigFile(filename);
		}
		catch (const std::exception &e)
		{
			ser.closeFds(); // 全てのファイルディスクリプタを閉じる
			std::cerr << e.what() << std::endl;
		}
		std::cout << "The Server Closed!" << std::endl;
		return (SUCCESS);
	}
	else
	{
		std::cout << "Correct usage is ./ircserv [port] [password] :)" << std::endl;
		return (FAILURE);
	}
}
