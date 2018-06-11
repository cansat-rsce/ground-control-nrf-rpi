/*
 * config.hpp
 *
 *  Created on: 12 июн. 2018 г.
 *      Author: snork
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

#include <RF24/RF24.h>

namespace rscs
{
	namespace gcs
	{
		class config
		{
		public:
			static void x()
			{
				RF24_PA_MIN;
			}


			void load_from_file(const std::string & file_path);

			uint8_t address_width;
			uint8_t retries_delay;
			uint8_t retries_count;
			uint8_t rf_channel;

			rf24_pa_dbm_e pa_level;
			rf24_datarate_e data_rate;
			rf24_crclength_e crc_length;




		};

	}
}



#endif /* CONFIG_HPP_ */
