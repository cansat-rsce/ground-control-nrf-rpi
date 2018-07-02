/*
 * log.h
 *
 *  Created on: 9 июн. 2018 г.
 *      Author: snork
 */

#ifndef COMMON_LOG_HPP_
#define COMMON_LOG_HPP_

#include <cstddef>
#include <ostream>

#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>


namespace rscs { namespace gcs {
    enum severity_level
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    std::ostream& operator<< (std::ostream& strm, const severity_level & level);
    std::istream & operator>> (std::istream & strm, severity_level & level);
}}


BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", ::rscs::gcs::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)


#define LOG_TRACE       BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::TRACE)
#define LOG_DEBUG       BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::DEBUG)
#define LOG_INFO        BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::INFO)
#define LOG_WARN        BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::WARN)
#define LOG_ERROR       BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::ERROR)
#define LOG_CRITICAL    BOOST_LOG_SEV(_slg, ::rscs::gcs::severity_level::CRITICAL)


namespace rscs { namespace gcs {

    typedef boost::log::sources::severity_channel_logger_mt<severity_level, std::string> logger_t;


    template <class DESCENDANT>
    class logable
    {
    protected:
        logable(const std::string & channel_name)
            :_slg(boost::log::keywords::channel = channel_name)
        {}

        logger_t _slg;
    };


    logger_t make_logger(const std::string & channel_name);
    void init_log();
}}

#endif
