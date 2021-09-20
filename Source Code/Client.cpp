#include "Client.h"
#include "Board.h"
#include "Game.h"
#include "Const.h"

int main()
{
	std::string ipAddress = "209.97.169.233";	//IP Address of the server
	int port = 14003;							//Listening port # on the server

	//Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		std::cout << "Can't start Winsock, Error #" << wsResult << std::endl;
		return 0;
	}

	//Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Can't create socket, Error #" << WSAGetLastError << std::endl;
		WSACleanup();
		return 0;
	}

	//Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		std::cout << "Can't connect to server, Error #" << WSAGetLastError << std::endl;
		closesocket(sock);
		WSACleanup();
		return 0;
	}

	//Do-while loop to send and receive data
	char buffer[2048];
	std::string _historyMoves;
	bool gameOver = false;

	while (true)
	{
		ZeroMemory(buffer, 2048);
		int bytesReceived = recv(sock, buffer, 2048, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			perror("receive");
			break;
		}

		if (bytesReceived == 0)
		{
			std::cout << "Server disconnected" << std::endl;
			break;
		}

		//Grab information from server
		std::string _message(buffer);

		Game game;
		game.Setup(_message, gameOver);

		if (!gameOver)
		{
			std::string _myMove = game.Play();

			send(sock, _myMove.c_str(), _myMove.size(), 0);

			_historyMoves += _myMove + " ";
			std::cout << "\n-----------------------------------------------------------------------\n";
			std::cout << "history: " << _historyMoves << std::endl;
			std::cout << "-----------------------------------------------------------------------\n\n";
		}
		else
			break;
	}

	//Gracefully close down everything
	closesocket(sock);
	WSACleanup();
}