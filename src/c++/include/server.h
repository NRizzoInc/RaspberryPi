#ifndef RPI_SERVER_H
#define RPI_SERVER_H

// Standard Includes
#include <iostream>
#include <string>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Our Includes

// 3rd Party Includes

namespace RPI {

class TcpServer {
    public:
        /********************************************** Constructors **********************************************/
        TcpServer();
        virtual ~TcpServer();

        /********************************************* Getters/Setters *********************************************/

        /********************************************* Server Functions ********************************************/


    private:
        /******************************************** Private Variables ********************************************/

        /********************************************* Helper Functions ********************************************/

}; // end of TcpServer class


} // end of RPI namespace

#endif
