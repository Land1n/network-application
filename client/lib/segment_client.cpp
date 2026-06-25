#include "segment_client/segment_client.h"

#include <memory>
#include <iostream>

#include "message/information_message.h"
#include "message/raw_message.h"
#include "message/signal_message.h"

SegmentClient::SegmentClient(const std::string& address, PortInt port) : address(address), port(port)
{
	session = std::make_unique<Session>(ioContext.getIOContext(), mode);
}
SegmentClient::~SegmentClient()
{
	stop();
}
void SegmentClient::start()
{
	ErrorHandler::check_error(error_code{}, "SegmentClient::start", true);
	isWork = true;
	if(magicNumber)
		session->setMagicNumber(magicNumber);
	connect();
	session->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
		if(msg != nullptr) {
			auto raw = dynamic_cast<RawMessage*>(msg.get());
			/// + TODO: huina
			if(raw != nullptr) {
				std::string raw_msg(reinterpret_cast<const char*>(raw->getData().data()), raw->getData().size());
				Logger::getInstance().log(LogLevel::Info, "SegmentClient::read", "RESPONSE to string : " + raw_msg);
			}
		}
	});
	mainWorker.addTask([this]() {
		while(isWork.load()) {
			session->read();
		}
	});
}
void SegmentClient::disconnect()
{
	session->disconnect();
}
void SegmentClient::write(const void* data, size_t sz)
{
	std::string type(reinterpret_cast<const char*>(data), sz);
	if(type == "signal" || type == "information") {
		session->write(std::make_unique<Message>(type, Transaction::Request));
	}
	else {
		const auto* bytes = static_cast<const uint8_t*>(data);
		std::vector<uint8_t> raw_bytes(bytes, bytes + sz);
		auto raw_msg = std::make_unique<RawMessage>("raw", Transaction::Response, raw_bytes);

		session->write(std::move(raw_msg));
	}
}
void SegmentClient::setIOMode(IOMode m)
{
	mode = m;
}
void SegmentClient::setMagicNumber(MagicInt n)
{
	magicNumber = n;
}
bool SegmentClient::getIsWork()
{
	return isWork.load();
}

void SegmentClient::stop()
{
	isWork = false;

	disconnect();
	mainWorker.flush();
	ErrorHandler::check_error(error_code{}, "SegmentClient::stop", true);
}
void SegmentClient::connect()
{
	if(readHandler)
		session->setOnReadHandler([this](size_t, const void* data, size_t sz) {
			readHandler(data, sz);
		});
	session->setOnConnect([this](error_code ec) {
		if(ec)
			stop();
	});
	session->connect(address, port);
}