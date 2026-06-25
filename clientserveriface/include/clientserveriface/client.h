#ifndef CLIENT_H
#define CLIENT_H

#include <functional>
#include <cstddef>

namespace Network {

class Client {
public:
	using ConnChangeHandler = std::function<void()>;
	using ReadHandler       = std::function<void(const void*, size_t)>;

	virtual ~Client() = default;

	virtual void start()                            = 0;
	virtual void stop()                             = 0;
	virtual void write(const void* data, size_t sz) = 0;
	virtual void disconnect()                       = 0;

	void setNewConnectionHandler(ConnChangeHandler h)
	{
		newHandler = h;
	}

	void setCloseConnectionHandler(ConnChangeHandler h)
	{
		closeHandler = h;
	}

	void setReadHandler(ReadHandler h)
	{
		readHandler = h;
	}

protected:
	ConnChangeHandler newHandler;
	ConnChangeHandler closeHandler;
	ReadHandler readHandler;
};
} // namespace Network

#endif // CLIENT_H
