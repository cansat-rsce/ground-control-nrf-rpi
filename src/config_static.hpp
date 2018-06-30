/*
 * config_static.hpp
 *
 *  Created on: 21 июн. 2018 г.
 *      Author: snork
 */

#ifndef CONFIG_STATIC_HPP_
#define CONFIG_STATIC_HPP_

#ifndef RSCS_GCS_ALLOW_REAL_NRF
    #ifdef __arm__
        #define RSCS_GCS_ALLOW_REAL_NRF 1
    #endif
#endif


#define RSCS_GCS_NRF_FIRST_CSN_PIN 0    // TODO реальные цифры
#define RSCS_GCS_NRF_FIRST_CE_PIN 0     // TODO реальные цифры

#define RSCS_GCS_NRF_SECOND_CSN_PIN 0   // TODO реальные цифры
#define RSCS_GCS_NRF_SECOND_CE_PIN 0    // TODO реальные цифры



#endif /* CONFIG_STATIC_HPP_ */
