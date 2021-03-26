#include <ESP8266WiFi.h>

#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RtcDS3231.h>
#include <Adafruit_HTU21DF.h>
#include <SPI.h>
#include "macros.hpp"

#include <string>
#include <sstream>
#include <string.h>

#define SD_CS_PIN D8

using namespace std;

Adafruit_HTU21DF htu; //temperature and humidity sensor object
RtcDS3231<TwoWire> Rtc(Wire); //RTC object

#define sec 1000

void setup () 
{   
    htu.begin(); // Inicia sensor de temperatura e umidade                                  
    Rtc.Begin(); // Inicia RTC
    EEPROM.begin(4);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).
    SD.begin(SD_CS_PIN);//Inicializa cartao SD
    
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) Rtc.SetDateTime(compiled);
    if (!Rtc.GetIsRunning())  Rtc.SetIsRunning(true);

    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmOne);
    
    int ison = EEPROM.read(0);
    if (ison)
    {
        RtcDateTime now = Rtc.GetDateTime();
        RtcDateTime alarmTime = now+3600; // it adds 1h
        DS3231AlarmOne alarm1(
                alarmTime.Day(),
                alarmTime.Hour(),
                alarmTime.Minute(), 
                alarmTime.Second(),
                DS3231AlarmOneControl_MinutesSecondsMatch);
        Rtc.SetAlarmOne(alarm1);
    }
    else
    {
        delay(5*sec);

        File logFile = SD.open("log.txt", FILE_WRITE);
        // se o arquivo foi aberto corretamente, escreve os dados nele
        if (logFile)
        {

            logFile.println("Station ID,Name,Latitude,Longitude,Elevation,First Record");
            logFile.print(ID,DEC);logFile.print(",");
            logFile.print(NAME);logFile.print(",");
            logFile.print(LAT,DEC);logFile.print(",");
            logFile.print(LNG,DEC);logFile.print(",");
            logFile.print(ELEVATION,DEC);logFile.print(",");

            logFile.print(year, DEC);
            logFile.print('-');
            if (month < 10) logFile.print("0");
            logFile.print(month, DEC);
            logFile.print('-');
            if (dayOfMonth < 10) logFile.print("0");
            logFile.println(dayOfMonth, DEC);
            
            //fecha o arquivo após usá-lo
            logFile.close();
        }
        if (WiFi.status() == WL_CONNECTED){
            //Formatacao do dado
            stringstream datarow;
            datarow << ID << "," << NAME << "," << LAT << "," << LNG << "," << ELEVATION << ",";
            datarow << year << "-";
            if (month < 10) datarow << "0";
            datarow << month << "-";
            if (dayOfMonth < 10) datarow << "0";
            datarow << dayOfMonth;
            
            WiFiClient client;
            int con_att = 0;
            while (!client.connect(server_ip, port) && con_att < 30) {
                con_att++;
                delay(100);
            }
    
            if (con_att < 30){
                string msg = "INFO";
                msg.append(datarow.str());
                    
                int msg_size = msg.size();
                int dig=0, temp = msg_size;
                while (temp > 0){
                  temp = temp/10;
                  dig++;
                }
                
                stringstream streaming;
                streaming << "";
                for( int j = 0; j < HEADERSIZE - dig; j++) streaming << " ";
                streaming << msg_size << msg ;
                string schar = streaming.str();
                
                char buff[10];
                do{
                    if (client.connected())
                        client.print(&schar[0]);
                    client.read((uint8_t*)buff, 8);
                }while (strcmp(buff, "received")!=0 && client.available());
                client.stop();
            }
        }

        
        EEPROM.write(0, 1);//Esp8266 on
        EEPROM.commit();//Salva o dado na EEPROM.

        const RtcDateTime* now = new RtcDateTime(year, month, dayOfMonth, hour, minute, second);
        Rtc.SetDateTime(*now);
    
        RtcDateTime alarmTime = *now+3600; // it adds 1h
        DS3231AlarmOne alarm1(
                alarmTime.Day(),
                alarmTime.Hour(),
                alarmTime.Minute(), 
                alarmTime.Second(),
                DS3231AlarmOneControl_MinutesSecondsMatch);
        Rtc.SetAlarmOne(alarm1);
    }

    // throw away any old alarm state before we ran
    Rtc.LatchAlarmsTriggeredFlags();

    WiFi.begin((char *)ssid, (char*)password);
    char i = 0;
    //Wait for WIFI to connect or 30 seconds
    while (WiFi.status() != WL_CONNECTED && i < 30){
        delay(sec);
        i++;
    }
}

void loop ()
{
    float temp = htu.readTemperature(), hum = htu.readHumidity();
    RtcDateTime now = Rtc.GetDateTime();
    int yr = now.Year(), mn = now.Month(), dy = now.Day();
    int hr = now.Hour(), mt = now.Minute();
    
    //Formatacao do dado
    stringstream datarow;
    datarow << ID << ",";
    datarow <<  yr << "-";
    if (mn < 10) datarow << "0";
    datarow << mn << "-";
    if (dy < 10) datarow << "0";
    datarow << dy << ",";
    if (hr < 10) datarow << "0";
    datarow <<  hr << ":";
    if (mt < 10) datarow << "0";
    datarow << mt << ":";
    datarow << "00" << ",";
    datarow << temp << "," << hum;
    
    File dataFile = SD.open("weather_station.csv", FILE_WRITE);
    // se o arquivo foi aberto corretamente, escreve os dados nele
    if (dataFile) 
    {
        if (dataFile.size() == 0)
            dataFile.println("Station ID,Date,Time,Temperature (C),Humidity (%)");

        //Escrita no arquivo
        dataFile.print(ID, DEC);
        dataFile.print(",");
        dataFile.print(yr, DEC);
        dataFile.print('-');
        if (mn < 10) dataFile.print("0");
        dataFile.print(mn, DEC);
        dataFile.print('-');
        if (dy < 10) dataFile.print("0");
        dataFile.print(dy, DEC);
        dataFile.print(",");
        
        if (hr < 10) dataFile.print("0");
        dataFile.print(hr, DEC);
        dataFile.print(':');
        if (mt < 10) dataFile.print("0");
        dataFile.print(mt, DEC);
        dataFile.print(':');
        dataFile.print("00");

        dataFile.print(',');
        dataFile.print(temp);
        
        dataFile.print(",");
        dataFile.println(hum);
        
        //fecha o arquivo
        dataFile.close();
    }

    if (WiFi.status() == WL_CONNECTED){
        WiFiClient client;

        int con_att = 0;
        while (!client.connect(server_ip, port) && con_att < 30) {
            con_att++;
            delay(100);
        }

        if (con_att < 30){
            string msg = "DATA";
            msg.append(datarow.str());
            
            int msg_size = msg.size();
            int dig=0, temp = msg_size;
            while (temp > 0){
              temp = temp/10;
              dig++;
            }
        
            stringstream streaming;
            streaming << "";
            for( int j = 0; j < HEADERSIZE - dig; j++) streaming << " ";
            streaming << msg_size << msg ;
            string schar = streaming.str();            

            char buff[10];
            do{
                if (client.connected())
                    client.print(&schar[0]);
                client.read((uint8_t*)buff, 8);
            }while (strcmp(buff, "received")!=0 && client.available());
            
            client.stop();
        }
    }
    
    ESP.deepSleep(0, WAKE_RF_DEFAULT); //Put esp8266 to sleep for an undefined time
}
