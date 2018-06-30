/*
 * sh_mutex.hpp
 *
 *  Created on: 30 июн. 2018 г.
 *      Author: snork
 */

#ifndef SHARED_MUTEX_SH_MUTEX_HPP_
#define SHARED_MUTEX_SH_MUTEX_HPP_

#include "shared_mutex/shared_mutex.h"

#include "log.hpp"

namespace rscs { namespace gcs {

    class sh_mutex: private logable<sh_mutex>
    {
    public:
        static const std::string default_name(){ return "rscs_gcs_rf24_mtx"; }

        sh_mutex()
            : logable<sh_mutex>("sh_mutex")
        {}

        sh_mutex(const std::string name)
            : logable<sh_mutex>("sh_mutex")
        {
            open(name);
        }

        ~sh_mutex()
        {
            unlock();
            close();
        }

        void open(const std::string & name = default_name())
        {
            if (_inited)
                throw std::runtime_error("mutex already inited");

            _mtx = shared_mutex_init(const_cast<char*>(name.c_str()));
            _inited = true;

            if (_mtx.created)
                LOG_DEBUG << "mutex was created";
            else
                LOG_DEBUG << "mutex was opened";
        }

        void close()
        {
            if (!_inited)
                return;

            int rc = shared_mutex_destroy(_mtx);
            if (rc != 0)
                LOG_ERROR << "cant properly close shared mtx";
            _inited = false;
        }

        void lock()
        {
            if (!_inited)
                throw std::runtime_error("mutex lock attempt while it was not inited");

            pthread_mutex_lock(_mtx.ptr);
        }

        void unlock()
        {
            if (!_inited)
                return;

            pthread_mutex_unlock(_mtx.ptr);
        }

    private:
        bool _inited = false;
        shared_mutex_t _mtx;
    };

}}

#endif /* SHARED_MUTEX_SH_MUTEX_HPP_ */
