/*
 * cfg_types_io.h
 *
 *  Created on: Jun 13, 2018
 *      Author: snork
 */

#ifndef CONFIG_TYPES_HPP_
#define CONFIG_TYPES_HPP_

#include <istream>
#include <ostream>

#include "config_static.hpp"

#if RSCS_GCS_ALLOW_REAL_NRF == 1
#include <RF24/RF24.h>
#else
/**
 * Power Amplifier level.
 *
 * For use with setPALevel()
 */
typedef enum { RF24_PA_MIN = 0,RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, RF24_PA_ERROR } rf24_pa_dbm_e ;

/**
 * Data rate.  How fast data moves through the air.
 *
 * For use with setDataRate()
 */
typedef enum { RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS } rf24_datarate_e;

/**
 * CRC Length.  How big (if any) of a CRC is included.
 *
 * For use with setCRCLength()
 */
typedef enum { RF24_CRC_DISABLED = 0, RF24_CRC_8, RF24_CRC_16 } rf24_crclength_e;

#endif



namespace rscs { namespace gcs
{


    //! позволяет указать пайп
    class rf24_pipe_t
    {
    public:
        rf24_pipe_t(): value() {}
        rf24_pipe_t(const uint64_t & value_): value(value_) {}

        operator uint64_t & () { return value; }
        operator const uint64_t & () const { return value; }

        uint64_t value;
    };


    //! позволяет указывать конкретный модуль, подключенный к наземной станции
    class rf24_module_id_t
    {
    public:
        enum Value { FIRST, SECOND };

        rf24_module_id_t(): value() {}
        rf24_module_id_t(const Value & value_): value(value_) {}

        operator Value&() { return value; }
        operator const Value&() const { return value; }

        Value value;
    };

}}


std::istream & operator>>(std::istream & stream, rf24_pa_dbm_e & value);
std::istream & operator>>(std::istream & stream, rf24_datarate_e & value);
std::istream & operator>>(std::istream & stream, rf24_crclength_e & value);

namespace rscs { namespace gcs {

    std::istream & operator>>(std::istream & stream, rf24_pipe_t & value);
    std::ostream & operator<<(std::ostream & stream, const rf24_pipe_t & value);


    std::istream & operator>>(std::istream & stream, rf24_module_id_t & value);
    std::ostream & operator<<(std::ostream & stream, const rf24_module_id_t & value);
}}

#endif /* CONFIG_TYPES_HPP_ */
