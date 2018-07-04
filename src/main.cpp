
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

//#include <RF24/RF24.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/time_facet.hpp>

#include <boost/log/utility/setup.hpp>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>

#include "shared_mutex/shared_mutex.h"

#include "config.hpp"
#include "log.hpp"
#include "sh_mutex.hpp"


using namespace rscs::gcs;

auto _slg = make_logger("main");


typedef std::function<void(std::vector<uint8_t> & )> rcv_handler_t;

void do_receive(boost::asio::ip::udp::socket & socket, std::vector<uint8_t> & buffer, rcv_handler_t & on_receive)
{
    using namespace boost;

    static const size_t max_udp_packet_size = 65535;
    buffer.resize(max_udp_packet_size);
    socket.async_receive(asio::buffer(buffer), [&](const boost::system::error_code & err, size_t received){

        if (err == asio::error::operation_aborted)
        {
            LOG_INFO << "read operation aborted. stopping receiving cycle";
            return;
        }
        else if (err)
        {
            LOG_ERROR << "socket receive error :" << err << " : " << err.message();
            do_receive(socket, buffer, on_receive);
            return;
        }

        LOG_DEBUG << "got udp packet. size = " << received;
        buffer.resize(received);
        on_receive(buffer);
        do_receive(socket, buffer, on_receive);
    });
}



int main(int argc, const char ** argv)
{
    // ====================================================================================================
    // ====================================================================================================
    rscs::gcs::config c;

    try
    {
        c.load(argc, argv);

        LOG_INFO << "log lines";

        std::stringstream log_cfgs;
        for (auto & line : c.log.program_log_cfg)
        {
            LOG_INFO << "log cfg line = " << line;
            log_cfgs << line << std::endl;
        }

        init_log();
        boost::log::init_from_stream(log_cfgs);
    }
    catch (std::exception & e)
    {
        LOG_ERROR << "config load error: " << e.what() << std::endl;
        std::stringstream usage;
        c.print_usage(usage);
        LOG_INFO << usage.str();

        return EXIT_FAILURE;
    }

    std::unique_ptr<RF24> radio;
    sh_mutex mtx;
    mtx.open(c.rf.sh_mtx_name);


    // ====================================================================================================
    // ====================================================================================================
    LOG_INFO << "setting up radio";
    try
    {
        if (c.rf.module_id == rf24_module_id_t::FIRST)
            radio.reset(new RF24(24, BCM2835_SPI_CS1));
        else if (c.rf.module_id == rf24_module_id_t::SECOND)
            radio.reset(new RF24(22, BCM2835_SPI_CS2));
        else if (c.rf.module_id == rf24_module_id_t::UNISAT_DEV)
            radio.reset(new RF24(22, BCM2835_SPI_CS0));
        else
            throw std::invalid_argument("invalid rf24 module id? its impasiburu!!1");

        std::unique_lock<sh_mutex> lock(mtx);
        LOG_INFO << "shared mutex locked";
        radio->begin();
        radio->enableDynamicPayloads();
        if (c.rf.auto_ack)
        {
            LOG_INFO << "ACKs enabled";
            radio->enableAckPayload();
        }
        else
        {
            LOG_WARN << "acks disabled";
            radio->setAutoAck(false);
        }

        //radio->setChannel(NRF_CHANNEL);
        //radio->setAddressWidth(5);
        //radio->setDataRate(RF24_1MBPS);
        //radio->setCRCLength(RF24_CRC_16);
        radio->setChannel(c.rf.rf_channel);
        radio->setAddressWidth(c.rf.address_width);
        radio->setDataRate(c.rf.data_rate);
        radio->setCRCLength(c.rf.crc_length);

        //radio->openWritingPipe(0xAAAAAAAAAA);
        //radio->openReadingPipe(0, 0xAAAAAAAAAA);
        //radio->openReadingPipe(1, 0xBBBBBBBBBB);
        radio->openWritingPipe(c.rf.write_pipe);
        radio->openReadingPipe(0, c.rf.read_pipe);
        radio->openReadingPipe(1, c.rf.write_pipe);

        radio->startListening();

        LOG_INFO << "radio setted up";
        radio->printDetails();
    }
    catch (std::exception & e)
    {
        LOG_ERROR << "error on radio confgure: " << e.what();
        return EXIT_FAILURE;
    }


    // ====================================================================================================
    // ====================================================================================================
    LOG_INFO << "configuring binlog";
    std::ofstream binlog_uplink;
    std::ofstream binlog_downlink;
    try
    {
        using namespace boost::filesystem;
        typedef boost::filesystem::path path;

        path dirpath = c.log.data_log_dir;
        create_directories(dirpath);

        std::stringstream sstr;
        auto * facet = new boost::posix_time::time_facet("%Y%m%dT%H%M%S");
        sstr.imbue(std::locale(std::locale(), facet));
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        sstr << now;
        std::string now_str = sstr.str();

        auto upath = dirpath / (c.log.data_log_prefix + now_str + "_up.blog");
        auto dpath = dirpath / (c.log.data_log_prefix + now_str + "_down.blog");

        LOG_INFO << "using binlog files: " << upath << ", " << dpath;
        binlog_uplink.open(upath.string().c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
        binlog_downlink.open(dpath.string().c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
    }
    catch (std::exception & e)
    {
        LOG_ERROR << "cant configure binlog: " << e.what();
        return EXIT_FAILURE;
    }

    // ====================================================================================================
    // ====================================================================================================
    LOG_INFO << "configuring network";
    using namespace boost;
    typedef asio::ip::udp::socket socket_t;
    typedef asio::ip::udp::resolver resolver_t;

    bool has_network = !c.net.target_host.empty();
    asio::io_service io;
    socket_t socket(io);
    std::vector<uint8_t> socket_rx_buffer;
    std::deque<uint8_t> uplink_queue;
    bool io_stop_flag = false;
    rcv_handler_t rcv_handler = [&](std::vector<uint8_t> & buffer){
        std::copy(buffer.begin(), buffer.end(), std::back_inserter(uplink_queue));
    };

    if (has_network)
    {

        try {
            std::string host = c.net.target_host;
            std::string port = std::to_string(c.net.target_port);
            LOG_INFO << "target-host: " << host << ", target-port: " << port;

            resolver_t resolver(io);
            resolver_t::query query(asio::ip::udp::v4(), host, port, asio::ip::udp::resolver::query::numeric_service);
            asio::ip::udp::endpoint target_endpoint = *resolver.resolve(query);
            socket.connect(target_endpoint);

            if (c.net.uplink_listen)
            {
                do_receive(socket, socket_rx_buffer, rcv_handler);
                LOG_INFO << "listening for uplink data because config said so";
            }
            else
            {
                LOG_INFO << "uplink data ignored, because config said so";
            }
        }
        catch (std::exception & e) {
            std::cout << "ERROR: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        LOG_INFO << "no network used since host is not setted up";
    }


    // ====================================================================================================
    // ====================================================================================================
    LOG_INFO << "configuring signal set";

    asio::signal_set signal_set(io);
    signal_set.add(SIGINT);
    signal_set.add(SIGTERM);

    signal_set.async_wait([&](const boost::system::error_code & e, int signal){
        if (e == asio::error::operation_aborted)
        {
            LOG_INFO << "SIGNAL WAITING ABORTED";
            return;
        }
        else if (e)
        {
            LOG_ERROR << "error in set awaits";
            return;
        }

        LOG_INFO << "got signal " << signal;
        io_stop_flag = true;
        socket.cancel();
    });


    // ====================================================================================================
    // ====================================================================================================
    LOG_INFO << "entering main loop";
    uint64_t downlink_bytes_counter = 0;
    uint64_t uplink_bytes_counter = 0;
    typedef std::chrono::steady_clock clock_t;
    auto last_report_timepoint = clock_t::now();

    try
    {
        size_t ack_payloads_availible = 3; // можно класть не больше чем столькоs
        uint8_t rx_pipeno;
        while(!io_stop_flag) {
            {
                // процессим радио
                uint8_t downlink_buffer[32];

                std::unique_lock<sh_mutex> lock(mtx);
                if( radio->available(&rx_pipeno) ) {
                    size_t len = radio->getDynamicPayloadSize();
                    radio->read(downlink_buffer, len);
                    lock.unlock(); // чтобы не держать слишком долгоs

                    LOG_TRACE << "got packet. pipeno = " << (int)rx_pipeno << " len = " << len;

                    binlog_downlink.write((char*)downlink_buffer, len);
                    binlog_downlink.flush();
                    if (has_network)
                    {
                        system::error_code err;
                        socket.send(asio::buffer(downlink_buffer, len), err);
                        if (err)
                            LOG_ERROR << "cant send data :" <<  err << ":" << err.message();

                    }
                    // мы получили пакет, значит пайлоад ушел с ним. отражаем в счетчике
                    if (ack_payloads_availible++ > 3)
                        ack_payloads_availible = 3;

                    downlink_bytes_counter += len;
                }
            }

            // процессим сеть (если есть) и сигналы
            io.poll();

            // если что-то получен, то оно появится в uplink очереди
            uint8_t uplink_buffer[32];

            if (uplink_queue.size() && ack_payloads_availible > 0)
            {
                LOG_DEBUG << "found uplink packet in queue";
                size_t len = std::min(sizeof(uplink_buffer), uplink_queue.size());
                auto copy_end = std::next(uplink_queue.begin(), len);
                std::copy(uplink_queue.begin(), copy_end, std::begin(uplink_buffer));
                uplink_queue.erase(uplink_queue.begin(), copy_end);

                {
                    std::unique_lock<sh_mutex> lock(mtx);
                    radio->writeAckPayload(rx_pipeno, uplink_buffer, len);
                }
                // теперь пайлоадов можно класть меньше
                ack_payloads_availible--;

                binlog_uplink.write((char*)uplink_buffer, len);

                LOG_TRACE << "placed ack payload of size = " << len;
                uplink_bytes_counter += len;
            }


            auto now = clock_t::now();
            if (now - last_report_timepoint >= std::chrono::milliseconds(c.report_delay_ms))
            {
                LOG_INFO << "uplink bytes counter: " << uplink_bytes_counter;
                LOG_INFO << "downlink bytes counter: " << downlink_bytes_counter;
                last_report_timepoint = now;
            }
        }
    }
    catch (std::exception & e)
    {
        LOG_ERROR << "error in main loop: " << e.what();
        io_stop_flag = true;
    }


    // ждем завешения всех асинхронных операций
    boost::system::error_code err;
    if (socket.is_open())
    {
        socket.cancel(err);
        if (err)
            LOG_ERROR << "socket cancel error: " << err << ":" << err.message();
    }

    err = boost::system::error_code();
    signal_set.cancel(err);
    if (err)
        LOG_ERROR << "signal set error:" << err << ":" << err.message();

    io.run();

    {
        std::unique_lock<sh_mutex> lock(mtx);
        radio->stopListening();
        LOG_INFO << "radio stopped";
    }

    LOG_INFO << "terminated gracefully";
    return EXIT_SUCCESS;
}

