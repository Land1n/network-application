#include "signal_data_packet.hpp"


void SignalDataPacket::setData(json::array& arr)
{
    for (auto const& value : arr){
        if (value.is_array() && value.as_array().size() == 2) {
            auto& complex_arr = value.as_array();
            float real = complex_arr[0].as_double();  
            float imag = complex_arr[1].as_double();
            data.push_back(std::complex<float>(real, imag));
        } else {
            // Обработка ошибки: можно выбросить исключение или проигнорировать
        }
    }
}