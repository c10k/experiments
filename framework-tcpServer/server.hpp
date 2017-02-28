#include "socket.hpp"
#include <vector>
#include <functional>
#include <thread>
#include <poll.h>


namespace pollServer {


enum class events {
	IN     = POLLIN,
	OUT    = POLLOUT,
	PRI    = POLLPRI,
	RDHUP  = POLLRDHUP,
	ERR    = POLLERR,
	HANGUP = POLLHUP,
	INVAL  = POLLNVAL
};

inline constexpr events operator|(events a, events b) noexcept
{
	return static_cast<events>(static_cast<int>(a) | static_cast<int>(b));
}

class server {
private:
	net::Socket listenSocket;
	// std::vector<net::Socket> connectedSockets;
	std::thread mainServerThread;
	std::vector<struct pollfd> connectedSockets;  // this vector size should be no maximum than RLIMIT_NOFILE

	void process()
	{
		// collection of all poll fd structures and manage them using poll.
		for (;;) {

			if (connectedSockets.size() == 0) break;

			auto res = poll(connectedSockets.data(), connectedSockets.size(), -1);

			if (res == -1) {
				auto currErrNo = errno;
				throw std::runtime_error(net::methods::getErrorMsg());
			}

			for (auto &eachConnectedSocket : connectedSockets) {
				if !(eachConnectedSocket.revents & pollServer::events::INVAL)
					{
						if (eachConnectedSocket.fd == listenSocket.getSocket()) {
							// accept connections here and push_back them to
							// connectedSockets vector with their events set for
							// input, error, hangup.
							auto peer = listenSocket.accept();
							struct pollfd peerObject;
							peerObject.fd     = peer.getSocket();
							peerObject.events = pollServer::events::IN
							  | pollServer::events::ERR
							  | pollServer::events::HANGUP
							  | pollServer::events::INVAL;

							connectedSockets.push_back(peerObject);

						} else {
							// check the current socket's revent for input,
							// error, hangup and call functions accordingly.
							if (eachConnectedSocket.revents & pollServer::events::IN)
								OnRequest();
							else if (eachConnectedSocket.revents & pollServer::events::ERR)
								OnError();
							else if (eachConnectedSocket.revents & pollServer::events::HANGUP)
								OnClose();
						}
					}
				else {
					// if socket has been closed then remove it from vector
					// outside this loop, after noting it's index here OR
					// instead of that, make the fd member -1 and when connectedSockets
					// size reaches RLIMIT_NOFILE then only traverse connectedSockets
					// and remove all those objects where fd is -1.
				}
			}
		}
	}

public:
	std::function<void(net::Socket &)> OnAccept;
	std::function<void(net::Socket &)> OnRequest;
	std::function<void(net::Socket &)> OnError;
	std::function<void()> OnClose;

	server(SF::domain _domain, SF::type _type, const int _proto = 0)
	    : listenSocket(SF::domain _domain, SF::type _type, const int _proto = 0)
	{
		struct pollfd listenSockObj;
		listenSockObj.fd     = listenSocket.getSocket();
		listenSockObj.events = pollServer::events::IN | pollServer::events::ERR
		  | pollServer::events::INVAL;

		connectedSockets.push_back(listenSockObj);
	}

	server(net::Socket _s) : listenSocket(_s) {}

	run(const char _addr[], const int _port, const int _q = SOMAXCONN)
	{

		try {

			listenSocket.start(const char _addr[], const int _port, const int _q);
			mainServerThread(&process);

		} catch (...) {
			throw;
		}
	}

	~server()
	{
		listenSocket.close();
		mainServerThread.join();
		for (auto &eachConnectedSocket : connectedSockets)
			eachConnectedSocket.close();
	}
};
}