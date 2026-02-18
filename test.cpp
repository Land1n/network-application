#include <iostream>
#include <vector>
#include <complex>
#include <string>
#include <algorithm>
#include <boost/json.hpp>

namespace json = boost::json;

template<typename T>
struct DataPacket {
    std::string type;           
    std::string format;        
    unsigned int length;         
    std::vector<T> data;       

    DataPacket(const std::string& t = "", const std::string& fmt = "")
        : type(t), format(fmt), length(0) 
    {}
};

struct SignalDataPacket : DataPacket<std::complex<double>> {
    unsigned int centralFrequency;  

    SignalDataPacket(unsigned int freq = 0)
        : DataPacket<std::complex<double>>("signal", "std::complex<double>"),
          centralFrequency(freq)
    {}
};

size_t allocated_size(const std::string& s) {
    return s.capacity() + 1; 
}

template<typename T>
size_t allocated_size(const std::vector<T>& v) {
    return v.capacity() * sizeof(T);
}


size_t total_size(const SignalDataPacket& p) {
    size_t total = sizeof(SignalDataPacket);              
    total += allocated_size(p.type);                        
    total += allocated_size(p.format);                    
    total += allocated_size(p.data);                        
    return total;
}

json::array complex_to_json(const std::complex<double>& c) {
    json::array arr;
    arr.push_back(c.real());
    arr.push_back(c.imag());
    return arr;
}

int main() {
    SignalDataPacket packet(1);
    for (int i = 0; i < 100;i++)
            packet.data.push_back(std::complex<double>(i+i-i*i, i-2*i));
    // packet.data.push_back(std::complex<double>(3.0, 4.0));

    json::object response;
    response["type"] = "signal";
    response["centralFrequency"] = 5000;
    response["lenght"] = 500;

    json::array array_data;
    for (const auto& c : packet.data) {
        array_data.push_back(complex_to_json(c));
    }
    response["data"] = std::move(array_data);
    
    std::array<char,1785> js_str;
    for (int i = 0; i < js_str.max_size();i++)
        js_str[i] = json::serialize(response).at(i);
    
        // std::cout << js_str << std::endl;
    std::string pattern_magic = "lenght";
    auto it = std::search(js_str.begin(), js_str.end(),
        pattern_magic.begin(), pattern_magic.end()
    );
    std::vector<char> q;
    for(size_t i = pattern_magic.length()+2; *(it+i)!=',';i++)q.push_back(*(it+i));
    // size_t i = 1;
    // while (*(it+i)!=',')
    // {
    //     q.push_back(*(it+i));
    //     i++;
    // }
    
    std::string qw(q.begin(),q.end());

    json::value js_qw = json::parse(json::serialize(response));

    std::cout << js_qw << std::endl;
    // std::cout << json::serialize(response).length()<< std::endl;
    // std::cout << "json::serialize(response).size() = " << json::serialize(response).size() << std::endl;

    //std::cout << "Total allocated memory = " << total_size(packet) << " bytes\n";
    //std::cout << sizeof(packet);
    return 0;
}



