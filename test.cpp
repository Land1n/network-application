#include <iostream>

#include <boost/json.hpp>

#include <vector>
#include <typeinfo>
namespace json = boost::json;

int main() 
{

	std::string data = R"({"command" : "parse", "data":[1,2,3,4,5]})";
	json::value json_val = json::parse(data);

	std::cout << typeid(json_val.at("data").as_array().size()).name() << std::endl;
	for(int i = 0; i < 5;++i)
		std::cout << json_val.at("data").at(i);
	return 0;
}
