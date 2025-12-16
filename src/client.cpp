#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

void chat_client(const std::string& server_ip) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip), 88888));

        std::cout << "Connected to server at " << server_ip << std::endl;

        char data[1024];
        while (true) {
            std::string message;
            std::cout << "You: ";
            std::getline(std::cin, message);

            boost::system::error_code error;
            boost::asio::write(socket, boost::asio::buffer(message), error);

            if (error) throw boost::system::system_error(error);

            // Read server response
            std::memset(data, 0, sizeof(data));
            if (error == boost::asio::error::eof) break;
            else if (error) throw boost::system::system_error(error);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    std::string server_ip;
    chat_client("127.0.0.1");
    return 0;
}
