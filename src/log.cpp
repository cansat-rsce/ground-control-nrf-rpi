/*
 * log.cpp
 *
 *  Created on: 30 июн. 2018 г.
 *      Author: snork
 */

#include "log.hpp"

namespace rscs { namespace gcs {

    std::ostream& operator<< (std::ostream& strm, const severity_level & level)
    {
        static const char* strings[] =
        {
            "trace",
            "debug",
            "info",
            "warn",
            "error",
            "critical"
        };

        if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
            strm << strings[level];
        else
            strm << static_cast< int >(level);

        return strm;
    }


    std::istream & operator>>(std::istream & strm, severity_level & level)
    {
        int value;
        strm >> value;
        level = (severity_level)value;
        return strm;
    }


    logger_t make_logger(const std::string & channel_name)
    {
        return logger_t(boost::log::keywords::channel = channel_name);
    }


    void init_log()
    {
        boost::log::register_simple_formatter_factory<severity_level, char>("Severity");
        boost::log::register_simple_filter_factory<severity_level, char>("Severity");
        boost::log::add_common_attributes();
    }

}}

