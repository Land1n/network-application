#ifndef SERVERCREATOR_H
#define SERVERCREATOR_H

#include <memory>
#include <ostream>
#include <optional>
#include <sstream>

namespace Network {
class Server;
struct ServerCreatorParams {
	int32_t port{0};
	int32_t interval{8000};
	bool multiConnect{true};
	uint32_t logLevel{0};
	std::optional<uint32_t> controlPort{std::nullopt};
};
inline std::ostream& operator<<(std::ostream& str, const ServerCreatorParams& cfg)
{
	std::stringstream oss;
	oss << "port: " << cfg.port << ", multiconnect: " << std::boolalpha << cfg.multiConnect
	    << ", interval: " << std::noboolalpha << cfg.interval;

	if(cfg.controlPort.has_value()) {
		oss << "controlPort(for dual server): " << cfg.controlPort.value();
	}
	return (str << oss.str());
}

class ServerCreator {
public:
	virtual ~ServerCreator() = default;

	virtual std::shared_ptr<Server> create(const ServerCreatorParams& serverFactoryParams) = 0;
};
} // namespace Network

#endif // SERVERCREATOR_H
