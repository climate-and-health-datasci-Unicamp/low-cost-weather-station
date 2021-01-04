#ifndef _MACROS_
#define _MACROS_

/*
Modifie this document in order to give the exactly 
location of the station and the date to initiate the measurements.
*/

#define HEADERSIZE 5

//Macros for date to initiate
#define year 2021
#define month 1
#define dayOfMonth 4
#define hour 9
#define minute 0
#define second 0

//Macros for Location
#define ID 6666
#define NAME "TESTES"
#define LAT -22.899874
#define LNG -47.093644
#define ELEVATION 695

const char* ssid = "xxxxx"; //Enter SSID
const char* password = "xxxxx"; //Enter Password
const char* server_ip = "xxxx";
const int port = 8080;

//const char* websockets_server = "52.224.66.43:8080/"; //server adress and port

#endif
