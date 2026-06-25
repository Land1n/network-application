#ifndef SERVER_H
#define SERVER_H

#include "connectionid.h"
#include <functional>

namespace Network {

class Server {
public:
	using ConnChangeHandler     = std::function<void(ConnectionId)>;
	using ReadHandler           = std::function<void(ConnectionId, const void*, size_t)>;
	using IdDistributionHandler = std::function<ConnectionId()>;

	virtual ~Server() = default;

	virtual void start()                                             = 0;
	virtual void stop()                                              = 0;
	virtual void write(ConnectionId id, const void* data, size_t sz) = 0;
	virtual void writeControl(ConnectionId id, const void* data, size_t sz)
	{
		write(id, data, sz);
	};
	virtual void writeData(ConnectionId id, const void* data, size_t sz)
	{
		write(id, data, sz);
	};
	virtual void disconnect(ConnectionId id) = 0;

	virtual void setNewConnectionHandler(ConnChangeHandler h)
	{
		newHandler = h;
	}

	virtual void setCloseConnectionHandler(ConnChangeHandler h)
	{
		closeHandler = h;
	}

	virtual void setReadHandler(ReadHandler h)
	{
		readHandler = h;
	}

	virtual void setIdDistributionHandler(IdDistributionHandler h) = 0;

protected:
	ConnChangeHandler newHandler;
	ReadHandler readHandler;
	ConnChangeHandler closeHandler;
};
} // namespace Network

#endif // SERVER_H
