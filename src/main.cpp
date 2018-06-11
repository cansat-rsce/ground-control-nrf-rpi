
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <RF24/RF24.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <boost/lexical_cast.hpp>

//TODO вынести в что-нибудь общее
#define NRF_CHANNEL		0x59
#define ADDR_ONBOARD	0xB0AFDB0AFD
#define ADDR_GCS 		0x57A7104313

int main(int argc, char ** argv)
{
	std::string addr_net;
	std::string port;
	std::string file;

	if(argc >= 2) addr_net = argv[1];
	else addr_net = "localhost";

	if(argc >= 3) port = argv[2];
	else port = "23728";

	if(argc >= 4) file = argv[3];
	else file = "radio.bin";

	using namespace std;
	using namespace boost;
	using udp = boost::asio::ip::udp;

	RF24 radio1(24, BCM2835_SPI_CS1); //, BCM2835_SPI_SPEED_8MHZ);

	radio1.begin();

	radio1.setChannel(NRF_CHANNEL);
	radio1.setAddressWidth(5);
	radio1.setDataRate(RF24_1MBPS);
	radio1.setCRCLength(RF24_CRC_16);
	radio1.enableDynamicPayloads();
	radio1.enableAckPayload();
	radio1.openWritingPipe(0xAAAAAAAAAA);
	radio1.openReadingPipe(0, 0xAAAAAAAAAA);
	radio1.openReadingPipe(1, 0xBBBBBBBBBB);
	radio1.startListening();

	uint8_t rx_pipeno;
	uint8_t buffer[32];

	try {
		asio::io_service io;
		udp::socket socket(io);
		udp::resolver resolver(io);
		udp::resolver::query query(udp::v4(), addr_net, port, udp::resolver::query::numeric_service);
		udp::endpoint target_endpoint = *resolver.resolve(query);
		socket.open(target_endpoint.protocol());

		time_t current_time = time(NULL);
		struct tm * time_tm = localtime(&current_time);
		char time_string[255];
		strftime(time_string, 255, "%c", time_tm);
		std::cout << std::string(time_string) << std::endl;
		ofstream log(file.c_str(), ios::out | ios::binary);
		//free(time_string);

		radio1.printDetails();

		while(true) {
			if( radio1.available(&rx_pipeno) ) {
				size_t len = radio1.getDynamicPayloadSize();
				radio1.read( buffer, len );
				std::cout << "Got packet! " << len << std::endl;

	            socket.send_to(asio::buffer(buffer, len), target_endpoint);

	            log.write((const char *)buffer, len);
	            log.flush();
			}
		}
	}

	catch (std::exception & e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}


	std::cout << std::endl;
	std::cout << std::endl;

	return 0;
}

