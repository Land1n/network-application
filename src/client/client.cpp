#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <thread>

#include <../core/include/request_response_handler.hpp>

using boost::asio::ip::tcp;

namespace json = boost::json;

void chat_client(const std::string& server_ip) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip), 8888));

        std::cout << "Connected to server at " << server_ip << std::endl;

        char data[1024];
	std::vector<int> vec = {0};
        while (true) {

	    boost::system::error_code error;
            std::string command;
            std::cout << "Enter command : ";
	    std::cin >> command;	
	    RequestResponseHandler rrh;
	    std::string request_str = rrh.serializeData(MessageData(command,vec));
	    boost::asio::write(socket, boost::asio::buffer(request_str), error);

            if (error) throw boost::system::system_error(error);

            // Read server response
            std::memset(data, 0, sizeof(data));
            if (error == boost::asio::error::eof) break;
            else if (error) throw boost::system::system_error(error);
	    
	    
	    size_t l = socket.read_some(boost::asio::buffer(data),error);
	    
	    MessageData md = rrh.parseData(data);
	    if (md.command == "pong")
		    ++vec[0];
	    std::string response = rrh.serializeData(md);
	    std::cout << "Server: " << response << std::endl;
	
	}
    } catch (std::exception& e) {
        std::cerr << "Exception: "<< "\n" << e.what() << std::endl;
    }
}

int main() {
    std::string server_ip;
    chat_client("127.0.0.1");
    return 0;
}
