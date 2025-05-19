/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasano <shunkotkg0141@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 02:23:26 by sasano            #+#    #+#             */
/*   Updated: 2025/04/10 17:51:37 by sasano           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <string>

void Server::ClearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < fds.size(); i++){ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
			{fds.erase(fds.begin() + i); break;}
	}
	for(size_t i = 0; i < clients.size(); i++){ //-> remove the client from the vector of clients
		if (clients[i].GetFd() == fd)
			{clients.erase(clients.begin() + i); break;}
	}

}

bool Server::Signal = false; //-> initialize the static boolean
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true; //-> set the static boolean to true to stop the server
}

void Server::CloseFds(){
	for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (SerSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
		close(SerSocketFd);
	}
}

void Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET; //-> set the address family to ipv4
	add.sin_port = htons(this->Port); //-> convert the port to network byte order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address

	SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if(SerSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if(setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
	fds.push_back(NewPoll); //-> add the server socket to the pollfd
}

void Server::ServerInit()
{
	// this->Port = 4444;
	SerSocket(); //-> create the server socket

	std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	std::cout << "IPaddress: " << inet_ntoa(GetIpAdd()) << std::endl;
	std::cout << "Port: " << GetPort() << std::endl;
	std::cout << "----------------" << std::endl;
	std::cout << "Server is running..." << std::endl;
	std::cout << "Press Ctrl + C to stop the server" << std::endl;

	while (Server::Signal == false) //-> run the server until the signal is received
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++) //-> check all file descriptors
		{
			if (fds[i].revents & POLLIN)//-> check if there is data to read
			{
				if (fds[i].fd == SerSocketFd)
					AcceptNewClient(); //-> accept new client
				else
					ReceiveNewData(fds[i].fd); //-> receive new data from a registered client
			}
		}
	}
	CloseFds(); //-> close the file descriptors when the server stops
}

void Server::AcceptNewClient()
{
	Client cli; //-> create a new client
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		{std::cout << "fcntl() failed" << std::endl; return;}

	NewPoll.fd = incofd; //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0

	Client::SetFd(incofd); //-> set the client file descriptor
	Client::SetIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli); //-> add the client to the vector of clients
	fds.push_back(NewPoll); //-> add the client socket to the pollfd

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    return prefix.size() <= str.size() && std::equal(prefix.begin(), prefix.end(), str.begin());
}

void Server::ReceiveNewData(int fd)
{
    char buff[1024]; //-> buffer for the received data
    memset(buff, 0, sizeof(buff)); //-> clear the buffer

    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data

    if(bytes <= 0){ //-> check if the client disconnected
        std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
        ClearClients(fd); //-> clear the client
        close(fd); //-> close the client socket
        return;
    }

    else { //-> print the received data
        buff[bytes] = '\0';
        std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;

        // Find the client with matching fd
        Client* targetClient = nullptr;
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].GetFd() == fd) {
                targetClient = &clients[i];
                break;
            }
        }

        if (!targetClient) {
            std::cout << RED << "Error: Client not found for fd " << fd << WHI << std::endl;
            return;
        }

        std::string command(buff);
        if (command.starts_with("/nick")) {
            std::string nick = command.substr(6);
            // Remove trailing whitespace/newline
            nick = nick.substr(0, nick.find_last_not_of(" \n\r\t") + 1);
            targetClient->SetNickname(nick);
            std::cout << "Client <" << fd << "> Nickname: " << nick << std::endl;
        }
        else if (command.starts_with("/join")) {
            std::string channel = command.substr(6);
            // Remove trailing whitespace/newline
            channel = channel.substr(0, channel.find_last_not_of(" \n\r\t") + 1);
            targetClient->JoinChannel(channel);
            std::cout << "Client <" << fd << "> Joined Channel: " << channel << std::endl;
        }
        else if (command.starts_with("/msg")) {
            std::string msg = command.substr(5);
            // Remove trailing whitespace/newline
            msg = msg.substr(0, msg.find_last_not_of(" \n\r\t") + 1);
            std::cout << "Client <" << fd << "> Message: " << msg << std::endl;
            // TODO: Implement message handling
        }
        else if (command.starts_with("/part")) {
            std::string channel = command.substr(6);
            // Remove trailing whitespace/newline
            channel = channel.substr(0, channel.find_last_not_of(" \n\r\t") + 1);
            // TODO: Implement PartChannel in Client class
            std::cout << "Client <" << fd << "> Left Channel: " << channel << std::endl;
        }
        else if (command.starts_with("/quit")) {
            std::string reason = command.substr(6);
            // Remove trailing whitespace/newline
            reason = reason.substr(0, reason.find_last_not_of(" \n\r\t") + 1);
            std::cout << "Client <" << fd << "> Quit Reason: " << reason << std::endl;
            ClearClients(fd);
            close(fd);
        }
        else if (command.starts_with("/who")) {
            std::string user = command.substr(5);
            // Remove trailing whitespace/newline
            user = user.substr(0, user.find_last_not_of(" \n\r\t") + 1);
            // TODO: Implement who functionality
            std::cout << "Client <" << fd << "> Who: " << user << std::endl;
        }
    }
}
