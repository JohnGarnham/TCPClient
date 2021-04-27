// File: $Id: tcp-project1.C,v 1.5 2007/04/16 02:45:17 jeg3600 Exp jeg3600 $
// Author: John Garnham
// Description: A TCP client for transferring data 
// Revisions:
// $Log: tcp-project1.C,v $
// Revision 1.5  2007/04/16 02:45:17  jeg3600
// Bug fixed. Now connects to hosts other than itself.
//
// Revision 1.4  2007/04/13 05:53:02  jeg3600
// Should be ready for final submission
//
// Revision 1.3  2007/04/12 17:09:41  jeg3600
// Cleaner. Fixed error messages.
//
// Revision 1.2  2007/04/12 04:05:55  jeg3600
// Fixed retrieve command
//
// Revision 1.1  2007/04/12 00:50:18  jeg3600
// Initial revision
//
// Revision 1.4  2007/04/11 07:45:19  jeg3600
// Accurately sends an ADD request
//
// Revision 1.3  2007/04/11 07:31:54  jeg3600
// Connects to the server and detects connection errors successfully. Reads in input for ADD.
//
// Revision 1.2  2007/04/11 03:50:17  jeg3600
// Initial version
//
// Revision 1.1  2007/04/10 17:58:13  jeg3600
// Initial revision
//
//

#include <iostream>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_MIN 1024
#define PORT_MAX 65535

// Structure for data structure to pass to/from server
typedef struct {
  int command;      // 0 for add, 1 for retrieve
  int id;           // Sequence number
  char name[32];    // Name
  int age;          // Age
} data_record;


int main(int argc, char*  argv[]) {

  
  // hostname for the socket
  char* hostname;  

  // port to connect to on server
  int port;        

  // socket handler
  int sock;

  // host
  struct hostent *hostent;

  // server data structure
  struct sockaddr_in server;

  // input from the user
  std::string in;

  // Client MUST accept two values: hostname and port
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " hostname port" << std::endl;
    exit(0);
  }
  
  // hostname is the first argument. port is the second argument.
  hostname = argv[1];
  port = atoi(argv[2]);
  if ( port < PORT_MIN || port > PORT_MAX) {
    std::cerr << port << ": invalid port number" << std::endl;
    exit(0);
  }

  // Create TCP (SOCK_STREAM) socket
  sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock < 0) { 
    std::cerr << "socket: " << strerror(errno) << std::endl;
    exit(0);
  }

  // DNS look-up
  hostent = gethostbyname( hostname );
  if ( hostent == NULL) {
    std::cerr << "gethostbyname: " << strerror(errno) << std::endl;
    exit(0);
  }

  // Set up server data structure
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = *(unsigned long*) hostent->h_addr;

  // Attemp to connect to the server
  if ( connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
    std::cerr << "connect: " << strerror(errno) << std::endl;
    exit(0);
  }


  // data record to send to the server
  data_record to;

  do {

    // data record to retrieve from server
    data_record from;

    // reset to and from
    to.command = -1;
    to.id = 0;
    from.command = -1;
    from.id = 0;

    // Prompt user for command
    std::cout << "Enter command (0 for Add, 1 for Retrieve, 2 to Quit):";
    std::cin >> in;

    to.command = atoi(in.c_str());

    if (to.command == 0) {

      // Add a data entry

      // Grab the ID from the user
      std::cout << "Enter id (integer):";
      std::cin >> in;
      to.id = atoi(in.c_str());

      // Grab the name
      std::cout << "Enter name (up to 32 char):";
      std::cin >> to.name;

      // Grab the age
      std::cout << "Enter age (integer):";
      std::cin >> in;
      to.age = atoi(in.c_str());

      // Send the data record to the server
      if ( send(sock, &to, sizeof(to), 0) <= 0) {
	std::cerr << "send: " << strerror(errno) << std::endl;
	close(sock);
	exit(0);
      }

      // Fetch the returned status code. 0 = success. 1 = fail.
      if ( read(sock, (char*) &from, sizeof(from.command)) <= 0) {
	std::cerr << "read: " << strerror(errno) << std::endl;
	close(sock);
	exit(0);
      }

      // Tell the user whether or not the add was successful or not
      if (from.command == 0) {
	std::cout << "ID " << to.id << " added successfully" << std::endl;
      } else if (from.command == 1) {
	std::cout << "ID " << to.id << " already exists" << std::endl;
      } else {

	std::cerr << "Error: corrupted data." << std::endl;
      }
      
    } else if (to.command == 1) {

      // Retrieve a data entry

      // Grab the ID
      std::cout << "Enter id (integer):";
      std::cin >> in;

      to.id = atoi(in.c_str());

      // Send the ID to the server
      // Only send command and ID (first ... bytes)
      if ( send(sock, &to, sizeof(to.command) + sizeof(to.id), 0) <= 0) {
	std::cerr << "send: " << strerror(errno) << std::endl;
	close(sock);
	exit(0);
      }

      // Receive the data
      if ( read(sock, (char*) &from, sizeof(from)) <= 0) {
	std::cerr << "read: " << strerror(errno) << std::endl;
	close(sock);
	exit(0);
      }

      // Print the data
      if (from.command == 0) {
	std::cout << "ID: " << from.id << std::endl;
	std::cout << "Name: " << from.name << std::endl;
	std::cout << "Age: " << from.age << std::endl;
      } else if (from.command == 1) {
	std::cout << "ID " << to.id << " does not exist." <<
	  std::endl;
      } else {
	std::cerr << "Error: corrupted data." << std::endl;
      }

    }

  } while (to.command != 2);            // 2 means quit

  // Close the socket when done
  close(sock);
  
  return 0;

}
