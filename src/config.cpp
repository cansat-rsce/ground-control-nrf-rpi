/*
 * config.cpp
 *
 *  Created on: 13 июн. 2018 г.
 *      Author: snork
 */

#include "config.hpp"

#include <boost/program_options/config.hpp>
#include <boost/program_options/cmdline.hpp>

namespace rscs { namespace gcs
{
    namespace po = boost::program_options;

    config::config()
        : _general_opts("general opts"), _rf_opts("rf24 opts"), _network_opts("network opts"), _log_opts("log opts")
    {
        _rf_opts.add_options()
            ("auto-ack", po::value(&rf.auto_ack)->default_value(false),
                    "enable auto ack")

            ("module-id", po::value(&rf.module_id)->required(),
                    "module id. values FIRST and SECOND")

            ("sh-mtx-name", po::value(&rf.sh_mtx_name)->default_value("rscs-gcs-rf-mtx"),
                    "shared mutex to allow two isntances of rf running")

            ("read-pipe", po::value(&rf.read_pipe)->required(),
                    "read pipe in hexadec")

            ("write-pipe", po::value(&rf.write_pipe)->required(),
                    "write pipe in hexadec")

            ("address-width", po::value(&rf.address_width)->required(),
                    "from 3 to 5 bytes")

            ("retries-delay", po::value(&rf.retries_delay)->required(),
                    "How long to wait between each retry, in multiples of 250us,"
                    " max is 15.  0 means 250us, 15 means 4000us")

            ("retries-count", po::value(&rf.retries_count)->required(),
                    "How many retries before giving up, max 15")

            ("rf-channel", po::value(&rf.rf_channel),
                    "Which RF channel to communicate on, 0-125")

            ("pa-level", po::value(&rf.pa_level)->required(),
                "Power Amplifier (PA) level to one of four levels: "
                "RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX "
                "The power levels correspond to the following output levels respectively: "
                "NRF24L01: -18dBm, -12dBm,-6dBM, and 0dBm")

            ("data-rate", po::value(&rf.data_rate)->required(),
                    "speed RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps")

            ("crc-length", po::value(&rf.crc_length)->required(),
                    "length RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit "
                    "and RF24_CRC_DISABLED for disabled CRC")
        ;


        _general_opts.add_options()
            ("help,h", po::value(&_help_requested)->default_value(false)->implicit_value(true),
                    "print help and exit")

            ("config-file", po::value(&_cfg_file_path),
                "config file path")
        ;

        _network_opts.add_options()
            ("target-host", po::value(&net.target_host)->default_value(""), "target host to send data")
            ("target-port", po::value(&net.target_port)->default_value(0), "target port to send data")
            ("uplink-listen", po::value(&net.uplink_listen)->default_value(true),
                    "listen to uplink commanands")
        ;


        _log_opts.add_options()
            ("report-delay", po::value(&report_delay_ms)->default_value(5000), "delay between reports (in ms)")
            ("data-log-dir", po::value(&log.data_log_dir)->default_value("datalog"), "catalogue to store data log files")
            ("data-log-prefix", po::value(&log.data_log_prefix),
                    "prefix for data log file names. Useful to distinguish different users")
            ("program-log-cfg", po::value(&log.program_log_cfg)->composing(), "boost log configuration file-like strings")
        ;

        _cfg_file_opts.add(_rf_opts).add(_network_opts).add(_log_opts);
        _all_opts.add(_general_opts).add(_cfg_file_opts);
    }


    void config::load(int argc, const char ** argv)
    {
        // Первым проходом смотрим просто - запросили ли помощь или нужно парсить конфиг файл
        po::command_line_parser parser(argc, argv);
        parser.options(_general_opts);
        parser.allow_unregistered();
        auto parsed_opts = parser.run();

        po::variables_map vm;
        po::store(parsed_opts, vm);
        po::notify(vm);

        if (_help_requested)
            throw std::runtime_error("help requested");


        vm = po::variables_map();
        // окей, поехали парсить реальные опции
        if (!_cfg_file_path.empty())
        {
            auto parsed_opts = po::parse_config_file<char>(_cfg_file_path.c_str(), _cfg_file_opts);
            po::store(parsed_opts, vm);
        }

        // Теперь из командной строки
        auto super_giga_opts = _all_opts;
        parsed_opts = po::parse_command_line<char>(argc, argv, super_giga_opts);

        po::store(parsed_opts, vm);
        po::notify(vm);

        // TODO: валидация по типам
    }


    void config::print_usage(std::ostream & stream)
    {
        stream << _all_opts << std::endl;
    }

}}


