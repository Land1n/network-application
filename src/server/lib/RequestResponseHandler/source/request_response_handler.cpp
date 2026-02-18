#include <boost/json.hpp>

#include <sstream> 
#include "request_response_handler.hpp"
#include "signal_data_packet.hpp"

#include <boost/type_index.hpp>

namespace json = boost::json;

std::string RequestResponseHandler::getIp()
{
    std::ostringstream oss;
    oss << "[" << socket->remote_endpoint().address().to_string()
        << ":" << socket->remote_endpoint().port() << "]";
    return oss.str();
}

void RequestResponseHandler::serializeData(){}

SignalDataPacket RequestResponseHandler::parseData()
{
    char* arr = data.data();
    std::cout << "Client " << getIp() << " parse data..."; 
    json::value request_json = json::parse(arr);

    SignalDataPacket sdp(
        request_json.at("sizePacket").as_int64(),
        request_json.at("dataLenght").as_int64(),
        request_json.at("centralFrequency").as_int64()
    );
    sdp.setData(request_json.at("data").as_array());
    std::cout << "successful" << std::endl; 
    return sdp;
}
void RequestResponseHandler::sendData()
{}

bool RequestResponseHandler::searchData(uint32_t magic_patern = 0xABAB000B)
{
    uint32_t magic;
    std::cout << "Client " << getIp() << " search magic_patern..."; 
    while (is_working->load() && magic != magic_patern)
    {
        try{
            boost::asio::read(*socket,boost::asio::buffer(&magic,4));
        } catch(const std::exception& e){
            // std::cerr << "Client" << getIp() << "searchData error (magic_patern) : " << e.what() << std::endl;
            return false;
        }
    }
    std::cout << "successful" << std::endl;; 
    std::cout << "Client " << getIp() << " search json_len..."; 
    uint32_t json_len;
    try{
        uint32_t net_len;
        boost::asio::read(*socket, boost::asio::buffer(&net_len, 4));
        json_len = ntohl(net_len);
    }   catch (std::exception& e) {
        std::cerr << "Client" << getIp() << "searchData error (net_len) : " << e.what() << std::endl;
        return false;
    }
    std::cout << "successful" << std::endl;  
    std::cout << "Client " << getIp() << " search successful" << std::endl; 
    data.resize(json_len);
    return true;
}


void RequestResponseHandler::processingData()
{

    while (is_working->load())
    {
        try
        {
            boost::system::error_code error;

            if (!searchData()) {
                // Если ошибка, вероятно клиент отключился
                error = boost::asio::error::eof;
            }
            size_t len = socket->read_some(boost::asio::buffer(data), error);

            if (!error){
                // std::cout << std::string_view(data.data(), len) << std::endl;
                SignalDataPacket sdp = parseData();
                std::cout << "sdp.data.at(0): " << sdp.data.at(0) << std::endl;
                std::cout << "type(sdp.data.at(0)) : " << boost::typeindex::type_id_with_cvr<decltype(sdp.data.at(0))>().pretty_name() << std::endl;
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
            std::cerr << "Client " << getIp () << " processing error: " << e.what() << std::endl;
        }
        
    }

}
