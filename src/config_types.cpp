/*
 * cfg_types_io.cpp
 *
 *  Created on: Jun 13, 2018
 *      Author: snork
 */

#include "config.hpp"
#include "config_types.hpp"


std::istream & operator>>(std::istream & stream, rf24_pa_dbm_e & value)
{
    std::string value_string;
    stream >> value_string;

    if (value_string == "RF24_PA_MIN")
        value = RF24_PA_MIN;
    else if (value_string == "RF24_PA_LOW")
        value = RF24_PA_LOW;
    else if (value_string == "RF24_PA_HIGH")
        value = RF24_PA_HIGH;
    else if (value_string == "RF24_PA_MAX")
        value = RF24_PA_MAX;
    else
        throw std::invalid_argument("invalid value of pa_level \"" + value_string + "\"");

    return stream;
}



std::istream & operator>>(std::istream & stream, rf24_datarate_e & value)
{
    std::string value_string;
    stream >> value_string;

    if (value_string == "RF24_1MBPS")
        value = RF24_1MBPS;
    else if (value_string == "RF24_2MBPS")
        value = RF24_2MBPS;
    else if (value_string == "RF24_250KBPS")
        value = RF24_250KBPS;
    else
        throw std::invalid_argument("invalid data_rate value: \"" + value_string + "\"");

    return stream;
}


std::istream & operator>>(std::istream & stream, rf24_crclength_e & value)
{
    std::string value_string;
    stream >> value_string;

    if (value_string == "RF24_CRC_DISABLED")
        value = RF24_CRC_DISABLED;
    else if (value_string == "RF24_CRC_8")
        value = RF24_CRC_8;
    else if (value_string == "RF24_CRC_16")
        value = RF24_CRC_16;
    else
        throw std::invalid_argument("invalid value of crc_lenght field");

    return stream;
}

namespace rscs { namespace gcs {

    std::istream & operator>>(std::istream & stream, rscs::gcs::rf24_pipe_t  & value)
    {
        std::string value_str;
        stream >> value_str;

        value.value = std::stoull(value_str, nullptr, 16);
        return stream;
    }


    std::ostream & operator<<(std::ostream & stream, const rscs::gcs::rf24_pipe_t  & value)
    {
        std::exception_ptr error;
        auto flags = stream.flags();

        try { stream << std::hex << "0x" << value.value; }
        catch (...) { error = std::current_exception(); }

        stream.flags(std::move(flags));

        if (error)
            std::rethrow_exception(error);

        return stream;
    }



    std::istream & operator>>(std::istream & stream, rscs::gcs::rf24_module_id_t & value)
    {
        std::string value_str;
        stream >> value_str;

        if ("FIRST" == value_str)
            value = rscs::gcs::rf24_module_id_t::FIRST;
        else if ("SECOND" == value_str)
            value = rscs::gcs::rf24_module_id_t::SECOND;
        else
            throw std::invalid_argument("invalid rf24 module id. Allowed values: FIRST, SECOND");

        return stream;
    }


    std::ostream & operator<<(std::ostream & stream, const rscs::gcs::rf24_module_id_t & value)
    {
        switch (value)
        {
        case rscs::gcs::rf24_module_id_t::FIRST:
            stream << "FIRST";
            break;
        case rscs::gcs::rf24_module_id_t::SECOND:
            stream << "SECOND";
            break;
        default:
            stream << "INVALID(" << (int)value << ")";
            break;
        }

        return stream;
    }

}}
