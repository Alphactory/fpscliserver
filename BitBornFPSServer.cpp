//
// Created by eighty on 1/5/21.
//

#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;



int main()
{
    // Data initialization //
    string mapFileName = "map.txt";
    int listenPort = 5080;

    wstring wmap;
    // Create Map of world space # = wall block, . = space
    char validWalls[2] = { '#', '@' }; // Valid flavours of walls
    //create a file object to read text from into the map
    fstream mapFile;
    mapFile.open("../"+mapFileName, ios::in);
    if (!mapFile.is_open()){
        cout <<"could not find map file"<<endl;
    }else{
        string tmp;
        string map;
        while (getline(mapFile, tmp)) {
            map += tmp;
        }
        wstring temp_wmap(map.begin(), map.end());
        wmap = temp_wmap;
    }

    // NETWORK STUFF
    // Create a socket (IPv4, TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockaddr{};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(listenPort); // htons is necessary to convert a number to
    // network byte order
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port "<<listenPort<<". errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(sockfd, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Grab a connection from the queue
    auto addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (connection < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // MAIN LOOP//
    while(true){
        // Read from the connection
        int size = 100;
        char* buffer = new char[size]();
        auto bytesRead = read(connection, buffer, size);
        string message;
        for(int i=0; i<size; i++){
            char thischar = buffer[i];
            if (thischar != '\n'){
                message += thischar;
            }
            else{
                break;
            }
        }
        cout << "The message was: " << message << endl;
        if (message == "quit"){
            break;
        }
        else if (message=="map"){
            // Send a message to the connection
            string response(wmap.begin(), wmap.end());
            send(connection, response.c_str(), response.size(), 0);
            cout<<"sent map"<<endl;
        }
        else{
            string response = "invalid command supplied\n";
            send(connection, response.c_str(), response.size(), 0);
        }


    }

    // Close the connections
    close(connection);
    close(sockfd);

}
