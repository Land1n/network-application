#include <client_session.hpp>
#include <boost/json.hpp>

namespace json = boost::json;

ClientSession::ClientSession(boost::asio::io_context &io_context, tcp::acceptor &acceptor) : socket(io_context)
{
        try{
                acceptor.accept(this->socket);
                this->address = this->socket.remote_endpoint().address().to_string();
                this->port = this->socket.remote_endpoint().port();
                std::cout << "Client connected: " << this->address << " : " << this->port << std::endl;
        } catch (std::exception &e){
                std::cerr << "Exeption : " << e.what() << std::endl;

        }

}

void ClientSession::sendData(const std::string& command, const std::string& data)
{

        json::object response;
	response["command"] = command;
        response["data"] = data;
        std::string response_str = json::serialize(response);
        boost::asio::write(socket, boost::asio::buffer(response_str));
}

void ClientSession::processingCommand(const char* data, std::function<void(const std::string&,const std::string&)> callback)
{
        json::value request_json = json::parse(data);
        std::string command = request_json.as_object()["command"].as_string().c_str();
        if (command == "getData"){
                callback(command,data);
        } else if (command == "viewData") {
                std::cout << "Client [ " << this->address << ":" << this->port
<< " ] data = " << json::serialize(request_json) << std::endl;
        }
        else {
                callback("error","Unknown command");
        }

}

void ClientSession::startSession()
{
	try{
		
		while(true)
		{
			std::memset(this->data, 0, sizeof(this->data));
			boost::system::error_code error;
			

			size_t bytes_read = socket.read_some(boost::asio::buffer(this->data), error);
			if (error == boost::asio::error::eof)
                	{
                        	std::cout << "Client disconnected: " << this->address << ":" << this->port << std::endl;
                        	break;
                	}
            		else if (error) 
				throw boost::system::system_error(error);
			
			this->processingCommand(this->data,[this](const std::string& command, const std::string& data){
                		this->sendData(command, data);
            		});

		}
	} catch (std::exception &e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}



