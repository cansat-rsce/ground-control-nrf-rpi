/*
 * config.hpp
 *
 *  Created on: 12 июн. 2018 г.
 *      Author: snork
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>
#include <istream>

#include <boost/program_options.hpp>

#include "config_types.hpp"

namespace rscs { namespace gcs
{

    class config
    {
    public:
        struct rf_module_config
        {
            rf24_module_id_t module_id;
            std::string sh_mtx_name;

            int address_width;
            int retries_delay;
            int retries_count;
            int rf_channel;

            rf24_pa_dbm_e pa_level;
            rf24_datarate_e data_rate;
            rf24_crclength_e crc_length;
            rf24_pipe_t write_pipe;
            rf24_pipe_t read_pipe;
        };


        struct network_config
        {
            std::string target_host;
            uint32_t target_port;
        };


        struct log_config
        {
            std::string data_log_dir;
            std::string data_log_prefix;
            std::vector<std::string> program_log_cfg;
        };

        network_config net;
        rf_module_config rf;
        log_config log;

        config();
        void load(int argc, const char ** argv);
        void print_usage(std::ostream & stream);

    private:
        bool _help_requested = false;
        std::string _cfg_file_path = "";

        boost::program_options::options_description _general_opts;
        boost::program_options::options_description _rf_opts;
        boost::program_options::options_description _network_opts;
        boost::program_options::options_description _log_opts;

        boost::program_options::options_description _all_opts;
        boost::program_options::options_description _cfg_file_opts;
    };
}}



#endif /* CONFIG_HPP_ */
