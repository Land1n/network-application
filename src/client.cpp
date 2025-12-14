#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

void chat_client(const std::string& server_ip) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip), 12345));

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
            size_t length = socket.read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) break;
            else if (error) throw boost::system::system_error(error);

            std::cout << "Server: " << data << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    std::string server_ip;
    std::cout << "Enter server IP: ";
    std::cin >> server_ip;
    std::cin.ignore(); // Ignore leftover newline from std::cin
    chat_client(server_ip);
    return 0;
}
