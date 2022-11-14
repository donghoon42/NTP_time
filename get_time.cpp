#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <memory.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

#define Jan_int 1
#define Feb_int 2
#define Mar_int 3
#define Apr_int 4
#define May_int 5
#define Jun_int 6
#define Jul_int 7
#define Aug_int 8
#define Sep_int 9
#define Oct_int 10
#define Nov_int 11
#define Dec_int 12

void error( char* msg )
{
    perror( msg ); // Print the error message to stderr.

    exit( 0 ); // Quit the process.
}



typedef struct
{

  uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum;         // Eight bits. Stratum level of the local clock.
  uint8_t poll;            // Eight bits. Maximum interval between successive messages.
  uint8_t precision;       // Eight bits. Precision of the local clock.

  uint32_t rootDelay;      // 32 bits. Total round trip delay time.
  uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
  uint32_t refId;          // 32 bits. Reference clock identifier.

  uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
  uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

  uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
  uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

  uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
  uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

  uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
  uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

struct time_struct{
    int day;
    int month;
    int year;
};

void cvt_time( time_struct &time, std::string curr_time){
    std::string month = curr_time.substr(4,6).substr(0,3);
    std::string day = curr_time.substr(9,10);
    std::string year = curr_time.substr(20,24);
    time.day = std::stoi(day);
    if (month == "Jan"){
        time.month = Jan_int;
    }
    else if (month == "Feb"){
        time.month = Feb_int;
    }
    else if (month == "Mar"){
        time.month = Mar_int;
    }
    else if (month == "Apr"){
        time.month = Apr_int;
    }
    else if (month == "May"){
        time.month = May_int;
    }
    else if (month == "Jun"){
        time.month = Jun_int;
    }
    else if (month == "Jul"){
        time.month = Jul_int;
    }
    else if (month == "Aug"){
        time.month = Aug_int;
    }
    else if (month == "Sep"){
        time.month = Sep_int;
    }
    else if (month == "Oct"){
        time.month = Oct_int;
    }
    else if (month == "Nov"){
        time.month = Nov_int;
    }
    else if (month == "Dec"){
        time.month = Dec_int;
    }
    time.year = std::stoi(year);
    std::cout << "day: " << time.day << std::endl;
    std::cout << "month: " << time.month << std::endl;
    std::cout << "year: " << time.year << std::endl;
}

int main(int argc, char *argv[])
{
    int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.

    int portno = 123; // NTP UDP port number.
    // Create and zero out the packet. All 48 bytes worth.

    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    memset( &packet, 0, sizeof( ntp_packet ) );

// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.
    
    
    
    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

    if ( sockfd < 0 )
    perror( "ERROR opening socket" );

    server = gethostbyname("time.google.com"); // Convert URL to IP.

    if ( server == NULL )
    perror( "ERROR, no such host" );

    // Zero out the server address structure.

    bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.

    bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

    // Convert the port number integer to network big-endian style and save it to the server address structure.

    serv_addr.sin_port = htons( portno );


    // Call up the server using its IP address and port number.

    if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
    perror( "ERROR connecting" );

    // Send it the NTP packet it wants. If n == -1, it failed.

    n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

    if ( n < 0 )
    perror( "ERROR writing to socket" );

    // Wait and receive the packet back from the server. If n == -1, it failed.

    n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

    if ( n < 0 )
    perror( "ERROR reading from socket" );

    packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
    packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)

    time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

    printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );

    time_struct time;
    cvt_time( time,ctime( ( const time_t* ) &txTm ));
    
    time_struct preset_time;
    preset_time.day = 10;
    preset_time.month = 11;
    preset_time.year = 2022;

    std::cout << "preset date and time: " << preset_time.year <<"-"<<preset_time.month<<"-"<<preset_time.day <<std::endl;
    if (time.year == preset_time.year && time.month == preset_time.month && time.day >= preset_time.day){
        std::cout << "trial package expired..." << std::endl;
        return 0;
    }
    else{
        std::cout << "trial package not expired" << std::endl;
    }
    
    std::cout << "then... trial package available!!!\n";

    return 0;
}
