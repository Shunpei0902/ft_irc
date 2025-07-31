/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:23:26 by sasano            #+#    #+#             */
/*   Updated: 2025/07/31 16:14:04 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"
#include "channel.hpp"
#include "server.hpp"
#include "color.hpp"

bool Server::_signal = false;

Server::Server() : _port(-1), _serSocketFd(-1)
{
	// コンストラクタの初期化リストでメンバ変数を初期化
	_signal = false; // シグナルフラグを初期化
	_password = "";	 // パスワードを空に初期化
	// _operators.clear(); // サーバーオペレーターのベクターをクリア
	_fds.clear();		   // pollfdのベクターを空に初期化
	_clients.clear();	   // クライアントのマップを空に初期化
	_channels.clear();	   // チャンネルのマップを空に初期化
	_send_buffers.clear(); // 送信バッファを空に初期化
	_recv_buffers.clear(); // 受信バッファを空に初期化
}
Server::~Server() {}

std::map<int, Client *> Server::getClients()
{
	return _clients;
}

std::map<std::string, Channel *> Server::getChannels()
{
	return _channels;
}

// ポート番号取得
int Server::getPort() const { return _port; }

// ipアドレス取得
struct in_addr Server::getIpAdd() const
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getsockname(_serSocketFd, (struct sockaddr *)&addr, &len);
	return addr.sin_addr;
}

// 　クライアント取得
Client *Server::getClient(int fd)
{
	// std::cout << "NNN" << std::endl;
	// for (size_t i = 0; i < _clients.size(); i++)
	// {
	// 	std::cout << "_clients[i]->getFd() : " << "fd : " << fd << std::endl;
	// 	if (_clients[i]->getFd() == fd)
	// 	{
	// 		std::cout << "NAN" << std::endl;
	// 		return _clients[i];
	// 	}
	// }
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		return it->second; // クライアントが見つかった場合はそのポインタを返す
	}
	return NULL;
}

// クライアントのニックネームでクライアントを取得
Client *Server::getClientByNickname(const std::string &nickname)
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
		{
			return it->second; // ニックネームが一致するクライアントを返す
		}
	}
	return NULL;
}

// クライアント削除
void Server::clearClients(int fd)
{
	std::map<int, Client *>::iterator it_client = _clients.find(fd);
	if (it_client == _clients.end())
	{
		std::cout << "Client not found for fd: " << fd << std::endl;
		return; // クライアントが見つからない場合は何もしない
	}
	// クライアントのチャンネルからクライアントを削除
	std::map<std::string, Channel *> channels = it_client->second->getChannels();
	for (std::map<std::string, Channel *>::iterator it_channel = channels.begin(); it_channel != channels.end(); ++it_channel)
	{
		Channel *channel = it_channel->second;
		channel->removeClient(*it_client->second); // チャンネルからクライアントを削除
		if (channel->getClients().empty())
		{
			removeChannel(channel->getName()); // チャンネルが空なら削除
		}
	}
	// クライアントのファイルディスクリプタを閉じる
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break;
		}
	}
	close(fd); // クライアントのソケットを閉じる
	// クライアントの情報を削除
	delete it_client->second; // クライアントのメモリを解放
	_clients.erase(fd);		  // クライアントの情報を削除
	_send_buffers.erase(fd);  // クライアントの送信バッファを削除
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
}

void Server::addChannel(Channel *channel)
{
	// チャンネルをサーバーに追加
	_channels[channel->getName()] = channel;
}

Channel *Server::getChannel(const std::string &channel_name)
{
	// チャンネル名でチャンネルを取得
	std::map<std::string, Channel *>::iterator it = _channels.find(channel_name);
	if (it != _channels.end())
	{
		return it->second;
	}
	return NULL; // チャンネルが見つからない場合はnullを返す
}

void Server::removeChannel(const std::string &channel_name)
{
	// チャンネル名でチャンネルを削除
	std::map<std::string, Channel *>::iterator it = _channels.find(channel_name);
	if (it != _channels.end())
	{
		std::cout << RED << "Channel <" << channel_name << "> Removed" << WHI << std::endl;
		Channel *channel = it->second;
		_channels.erase(it); // チャンネルを削除
		delete channel;		 // チャンネルのメモリを解放
	}
	else
	{
		std::cout << RED << "Channel <" << channel_name << "> Not Found" << WHI << std::endl;
	}
}

// シグナルハンドラ
void Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl
			  << "Signal Received!" << std::endl;
	_signal = true; // サーバーを停止するためのフラグを立てる
}

void Server::clearChannels()
{
	// 全てのチャンネルを削除
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end();)
	{
		std::cout << RED << "Channel <" << it->first << "> Cleared" << WHI << std::endl;
		delete it->second; // チャンネルのメモリを解放
		std::map<std::string, Channel *>::iterator toErase = it;
		++it;					  // 次のイテレータを先に取得しておく
		_channels.erase(toErase); // チャンネルを削除
	}
	std::cout << RED << "All channels cleared" << WHI << std::endl;
}

// 全てのファイルディスクリプタを閉じる関数
void Server::closeFds()
{
	// クライアントソケットを閉じる
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end();)
	{
		if (it->second)
		{
			std::cout << RED << "Client <" << it->second->getFd() << "> Disconnected" << WHI << std::endl;
			close(it->second->getFd()); // クライアントのソケットを閉じる
			delete it->second;			// クライアントのメモリを解放
		}
		// it = _clients.erase(it); // eraseは次のイテレータを返すのでそれを使うc++11
		// eraseの返り値はvoidなので、イテレータを別途管理する必要がある
		std::map<int, Client *>::iterator toErase = it;
		++it; // 次のイテレータを先に取得しておく
		_clients.erase(toErase);
	}
	// サーバーソケットを閉じる
	if (_serSocketFd != -1)
	{
		close(_serSocketFd);
		std::cout << RED << "Server <" << _serSocketFd << "> Disconnected" << WHI << std::endl;
	}
}

void Server::setPassword(const std::string &password)
{
	_password = password; //-> set the server password
}

const std::string &Server::getPassword() const
{
	return _password; //-> get the server password
}

// サーバーソケットを作成し、ポートとアドレスを設定する関数
void Server::serSocket()
{
	struct sockaddr_in add;			   // IPv4用のソケットアドレスを格納
	struct pollfd newPoll;			   // poll() 用にソケットとその監視対象イベントを保持する
	add.sin_family = AF_INET;		   // IPv4 (AF_INET) を使用。
	add.sin_port = htons(this->_port); // ポート番号をネットワークバイトオーダー（ビッグエンディアン）に変換
	// add.sin_addr.s_addr = INADDR_ANY;  // 任意のネットワークインターフェースで待ち受けることを意味する（0.0.0.0）
	const char *ip_address = "127.0.0.1"; // ← 任意のアドレスに変更可能
	if (inet_pton(AF_INET, ip_address, &add.sin_addr) <= 0)
		throw(std::runtime_error("Invalid IP address"));

	// ソケットを作成
	_serSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create a socket using IPv4 and TCP
	if (_serSocketFd == -1)							//-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if (setsockopt(_serSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_serSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_serSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_serSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	newPoll.fd = _serSocketFd; //-> add the server socket to the pollfd
	newPoll.events = POLLIN;   //-> set the event to POLLIN for reading data
	newPoll.revents = 0;	   //-> set the revents to 0
	_fds.push_back(newPoll);   //-> add the server socket to the pollfd
}

// サーバー起動
void Server::serverInit(const char *port, const char *password, struct tm *timeinfo)
{
	// ポート番号を設定
	_port = std::atoi(port);
	if (_port <= 0 || _port > 65535)
	{
		throw std::runtime_error("Invalid port number");
	}

	// パスワードを設定
	_password = password;

	std::cout << "---- SERVER ----" << std::endl;
	std::cout << "Server started at: " << asctime(timeinfo);
	std::cout << "Port: " << _port << std::endl;
	std::cout << "Password: " << _password << std::endl;
	std::cout << "----------------" << std::endl;
	// サーバーソケットを作成
	serSocket();

	std::cout << GRE << "Server <" << _serSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	std::cout << "IPaddress: " << inet_ntoa(getIpAdd()) << std::endl;
	std::cout << "Port: " << _port << std::endl;
	std::cout << "----------------" << std::endl;
	std::cout << "Server is running..." << std::endl;
	std::cout << "Press Ctrl + C to stop the server" << std::endl;

	// シグナルを受け取るまでループ
	while (!_signal)
	{
		// poll() の直前で POLLOUT を必要に応じてセット
		for (size_t i = 1; i < _fds.size(); ++i)
		{
			if (!getSendBuffer(_fds[i].fd).empty())
				_fds[i].events |= POLLOUT; // 書き込み可能イベントを監視
			else
				_fds[i].events &= ~POLLOUT; // 書き込み不要なら解除
		}

		// pollシステムコールで接続要求やクライアントからの受信を監視
		if ((poll(&_fds[0], _fds.size(), -1) == -1) && !_signal)
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < _fds.size(); i++) //-> check all file descriptors
		{
			if (_fds[i].revents & POLLIN) //-> check if there is data to read
			{
				if (_fds[i].fd == _serSocketFd)
					acceptNewClient(); //-> accept new client
				else
					// ReceiveNewData(fds[i].fd); //-> receive new data from a registered client
					handleSocketReadable(_fds[i].fd); //-> handle the socket readable
			}
			if (_fds[i].revents & POLLOUT) //-> check if there is data to write
				sendBuffer(_fds[i].fd);
		}
	}
	clearChannels(); //-> delete all channels when the server stops
	closeFds();		 //-> close the file descriptors when the server stops
}

// 新しいクライアントを受け入れる関数
void Server::acceptNewClient()
{
	Client cli;
	struct sockaddr_in cliadd;
	struct pollfd newPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(_serSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}

	newPoll.fd = incofd;						//-> add the client socket to the pollfd
	newPoll.events = POLLIN;					//-> set the event to POLLIN for reading data
	newPoll.revents = 0;						//-> set the revents to 0
	cli.setFd(incofd);							//-> set the client file descriptor
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	// _clients.push_back(cli);					//-> add the client to the vector of clients
	Client *newClient = new Client(incofd, inet_ntoa(cliadd.sin_addr)); //-> add the client to the map of clients
	_clients.insert(std::make_pair(incofd, newClient));					//-> insert the client into the map of clients
	// _clients[incofd]->setIpAdd(inet_ntoa(cliadd.sin_addr)); //
	_fds.push_back(newPoll); //-> add the client socket to the pollfd
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
	std::string welcome = _welcomemsg();
	if (send(incofd, welcome.c_str(), welcome.length(), 0) == -1)
		std::cout << "send() error " << std::endl;
	// std::cout << "[" << currentDateTime() << "]: new connection from "
	// 		  << inet_ntoa(((struct sockaddr_in *)&remotaddr)->sin_addr)
	// 		  << " on socket " << newfd << std::endl;
}

// 受信したソケットが読み取り可能な場合の処理
// 受信したデータをバッファに追加し、\r\nで分割して処理する
void Server::handleSocketReadable(int client_fd)
{
	char buf[1024];
	memset(buf, 0, sizeof(buf));

	ssize_t bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);

	if (bytes <= 0)
	{							 //-> check if the client disconnected
		clearClients(client_fd); //-> clear the client
		return;
	}

	buf[bytes] = '\0';
	// recv_buffer += buf;
	_recv_buffers[client_fd] += std::string(buf); //-> add the received data to the buffer

	// "\r\n" を "\n" に置換して正規化
	size_t pos;
	while ((pos = _recv_buffers[client_fd].find("\r\n")) != std::string::npos)
	{
		_recv_buffers[client_fd].replace(pos, 2, "\n");
	}

	// "\n"が無い場合は何もしない

	// 受信バッファから1行ずつ処理（"\n" 区切り）
	while ((pos = _recv_buffers[client_fd].find('\n')) != std::string::npos)
	{
		std::string line = _recv_buffers[client_fd].substr(0, pos);
		_recv_buffers[client_fd].erase(0, pos + 1);

		// 念のため末尾の '\r' を除去
		if (!line.empty() && line[line.size() - 1] == '\r')
		{
			line.erase(line.size() - 1);
		}

		if (!line.empty())
		{
			handleClientMessage(line, client_fd);
		}
	}
}

// void Server::sendBuffers()
// {
// 	for (std::map<int, std::string>::iterator it = _send_buffers.begin(); it != _send_buffers.end(); ++it)
// 	{
// 		int client_fd = it->first;
// 		std::string &buffer = it->second;

// 		if (!buffer.empty())
// 		{
// 			ssize_t sent = send(client_fd, buffer.c_str(), buffer.length(), 0);
// 			if (sent == -1)
// 			{
// 				std::cerr << RED << "Failed to send to client <" << client_fd << ">" << WHI << std::endl;
// 				clearClients(client_fd); // エラー時にクライアントを切断しても良い
// 			}
// 			else
// 			{
// 				buffer.erase(0, sent); // 送信済み分を削除
// 			}
// 		}
// 	}
// }

void Server::sendBuffer(int client_fd)
{
	// クライアントの送信バッファにメッセージを追加
	std::map<int, std::string>::iterator it = _send_buffers.find(client_fd);
	if (it != _send_buffers.end())
	{
		std::string &buffer = it->second;
		if (!buffer.empty())
		{
			ssize_t sent = send(client_fd, buffer.c_str(), buffer.length(), 0);
			if (sent == -1)
			{
				std::cerr << RED << "Failed to send to client <" << client_fd << ">" << WHI << std::endl;
				clearClients(client_fd); // エラー時にクライアントを切断しても良い
			}
			else
			{
				buffer.erase(0, sent); // 送信済み分を削除
			}
		}
	}
}

void Server::addToClientBuffer(int client_fd, const std::string &message)
{
	// クライアントのバッファにメッセージを追加
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	if (it != _clients.end())
	{
		_send_buffers[client_fd] += message; // クライアントの送信バッファにメッセージを追加
	}
}

std::string Server::getSendBuffer(int client_fd) const
{
	// クライアントの送信バッファを取得
	std::map<int, std::string>::const_iterator it = _send_buffers.find(client_fd);
	if (it != _send_buffers.end())
	{
		return it->second; // クライアントの送信バッファを返す
	}
	return ""; // バッファが存在しない場合は空文字列を返す
}

std::string Server::_welcomemsg(void)
{
	std::string welcome = RED;
	welcome.append("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\n");
	welcome.append("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\n");
	welcome.append("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗\n");
	welcome.append("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝\n");
	welcome.append("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\n");
	welcome.append(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\n");
	welcome.append(BLUE);
	welcome.append("You need to login so you can start chatting OR you can send HELP to see how :) \n");
	welcome.append(RESET);
	return (welcome);
};