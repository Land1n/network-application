#ifndef CLIENTCREATOR_H
#define CLIENTCREATOR_H

#include <memory>
#include <ostream>

namespace Network {
class Client;
struct ClientCreatorParams {
	std::string hostname{};
	int32_t port{0};
	uint32_t reconnectTimeoutMs{0};
	uint32_t logLevel{0};
	uint32_t unitMsgLenForReadFromSocket{128};
};
inline std::ostream& operator<<(std::ostream& str, const ClientCreatorParams& cfg)
{
	return (str << "timeout: " << cfg.reconnectTimeoutMs << " ms");
}

class ClientCreator {
public:
	virtual ~ClientCreator() = default;

	virtual std::shared_ptr<Client> create(const ClientCreatorParams& clientCreatorParams) = 0;
};
} // namespace Network

#endif // CLIENTCREATOR_H
