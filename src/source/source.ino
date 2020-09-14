#include <Wire.h>
#include <EEPROM.h>
#include <RtcDS3231.h>
#include <Adafruit_HTU21DF.h>

#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
SdFat SD;

#define SD_CS_PIN D8
File dataFile;
File logFile;

Adafruit_HTU21DF htu; //temperature and humidity sensor object
RtcDS3231<TwoWire> Rtc(Wire); //RTC object

#define sec 1000

//Macros for date
#define year 2020
#define month 8
#define dayOfMonth 3
#define hour 20
#define minute 0
#define second 0
    
void setup () 
{   
    
    htu.begin(); // Inicia sensor de temperatura e umidade                                  
    Serial.begin(115200); // Inicia porta serial
    Rtc.Begin(); // Inicia RTC
    EEPROM.begin(4);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).
    
    // verifica se o cartão SD está presente e se pode ser inicializado
    if (!SD.begin(SD_CS_PIN)) return;
    
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) Rtc.SetDateTime(compiled);
    if (!Rtc.GetIsRunning())  Rtc.SetIsRunning(true);

    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmOne);
    
    int ison = EEPROM.read(0);
    if (ison)
    {
        RtcDateTime now = Rtc.GetDateTime();
        RtcDateTime alarmTime = now+30;//3600; // it adds 1h
        DS3231AlarmOne alarm1(
                alarmTime.Day(),
                alarmTime.Hour(),
                alarmTime.Minute(), 
                alarmTime.Second(),
                DS3231AlarmOneControl_MinutesSecondsMatch);
        Rtc.SetAlarmOne(alarm1);

        logFile = SD.open("log.txt", FILE_WRITE);
        // se o arquivo foi aberto corretamente, escreve os dados nele
        if (logFile) 
        {
            logFile.seek(logFile.size() - 10);
            logFile.print(now.Year(), DEC);
            logFile.print('-');
            if (now.Month() < 10) logFile.print("0");
            logFile.print(now.Month(), DEC);
            logFile.print('-');
            if (now.Day() < 10) logFile.print("0");
            logFile.print(now.Day(), DEC);

            
            //fecha o arquivo após usá-lo
            logFile.close();
        }
        
    }
    else
    {
        delay(5*sec);

        logFile = SD.open("log.txt", FILE_WRITE);
        // se o arquivo foi aberto corretamente, escreve os dados nele
        if (logFile) 
        {
            
            logFile.println("ID da Estacao: 1");
            logFile.println("Nome da Estacao: APTO Vila Prost Souza");
            logFile.println("Coordenadas Geograficas (Lat, Lng): -22.899874, -47.093644");
            logFile.println("Elevacao (em metros): 695");
            logFile.print("Data de Inicio: ");

            logFile.print(year, DEC);
            logFile.print('-');
            if (month < 10) logFile.print("0");
            logFile.print(month, DEC);
            logFile.print('-');
            if (dayOfMonth < 10) logFile.print("0");
            logFile.println(dayOfMonth, DEC);

            logFile.print("Data de Encerramento: ");
            logFile.print(year, DEC);
            logFile.print('-');
            if (month < 10) logFile.print("0");
            logFile.print(month, DEC);
            logFile.print('-');
            if (dayOfMonth < 10) logFile.print("0");
            logFile.print(dayOfMonth, DEC);

            
            //fecha o arquivo após usá-lo
            logFile.close();
        }
        
        EEPROM.write(0, 1);//Esp8266 on
        EEPROM.commit();//Salva o dado na EEPROM.

        const RtcDateTime* now = new RtcDateTime(year, month, dayOfMonth, hour, minute, second);
        Rtc.SetDateTime(*now);
    
        RtcDateTime alarmTime = *now+30;//3600; // it adds 1h
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
}

void loop () 
{
    float temp = htu.readTemperature(), hum = htu.readHumidity();
    RtcDateTime now = Rtc.GetDateTime();

    dataFile = SD.open("weather_station.csv", FILE_WRITE);
    // se o arquivo foi aberto corretamente, escreve os dados nele
    if (dataFile) 
    {
        if (dataFile.size() == 0)
            dataFile.println("Date,Time,Temperature (C),Humidity (%)");
        
        //formatação no arquivo
        dataFile.print(now.Year(), DEC);
        dataFile.print('-');
        if (now.Month() < 10) dataFile.print("0");
        dataFile.print(now.Month(), DEC);
        dataFile.print('-');
        if (now.Day() < 10) dataFile.print("0");
        dataFile.print(now.Day(), DEC);
        dataFile.print(",");
        
        if (now.Hour() < 10) dataFile.print("0");
        dataFile.print(now.Hour(), DEC);
        dataFile.print(':');
        if (now.Minute() < 10) dataFile.print("0");
        dataFile.print(now.Minute(), DEC);
        dataFile.print(':');
        if (now.Second() < 10) dataFile.print("0");
        dataFile.print(now.Second(), DEC);
        
        dataFile.print(',');
        dataFile.print(temp);
        
        dataFile.print(",");
        dataFile.println(hum);
        
        //fecha o arquivo após usá-lo
        dataFile.close();
    }

    delay(500);
    //ESP.deepSleep(0, WAKE_RF_DEFAULT); //Put esp8266 to sleep for an undefined time
}
