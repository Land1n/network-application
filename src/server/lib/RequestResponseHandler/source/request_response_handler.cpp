#include "request_response_handler.hpp"

std::string RequestResponseHandler::getIp()
{
    std::ostringstream oss;
    oss << "[" << socket->remote_endpoint().address().to_string()
        << ":" << socket->remote_endpoint().port() << "]";
    return oss.str();
}

void searchData()
{
    
}
void serializeData()
{}
void sendData()
{}

void RequestResponseHandler::processingData()
{

    while (is_working->load())
    {
        try
        {
            boost::system::error_code error;
            size_t len = socket->read_some(boost::asio::buffer(data), error);
            if (!error){
                std::cout << std::string_view(data, len) << std::endl;
        	}
            else if (error == boost::asio::error::eof){
                std::cout << "Client " << getIp() << " disconnected" << std::endl;
                break;
            }
            else if (error) {
                throw boost::system::system_error(error);
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Client" << getIp () << "processing error:" << e.what() << std::endl;
        }
        
    }

}
