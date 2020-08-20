#include <Wire.h>
#include <RtcDS3231.h>
#include <Adafruit_HTU21DF.h>
#include <SD.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

Adafruit_HTU21DF htu; //temperature and humidity sensor object
RtcDS3231<TwoWire> Rtc(Wire); //RTC object

//pino ligado ao CS do módulo SD Card
#define CS_PIN  D8
#define sec 1000
#define VCC_PIN D3

void setup () 
{   
    pinMode(VCC_PIN, OUTPUT);
    digitalWrite(VCC_PIN, HIGH); //turn off power suplly for sensors, rtc and sd module.
    
    htu.begin(); // Inicia sensor de temperatura e umidade
    //Serial.begin(115200); // Inicia porta serial
    Rtc.Begin(); // Inicia RTC
    EEPROM.begin(4);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    
    // verifica se o cartão SD está presente e se pode ser inicializado
    if (!SD.begin(CS_PIN)) 
    {
        return;
    }
    
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) 
    {
        Rtc.SetDateTime(compiled);
    }
    
    if (!Rtc.GetIsRunning())
    {
        Rtc.SetIsRunning(true);
    }

    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmOne);
    
    int on = EEPROM.read(0);
    if (on)
    {
        RtcDateTime now = Rtc.GetDateTime();
        RtcDateTime alarmTime = now+1800; // it adds 30 min
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
        EEPROM.write(0, 1);//Esp8266 on
        EEPROM.commit();//Salva o dado na EEPROM.

        int year = 2020;
        int month = 8;
        int dayOfMonth = 3;
        int hour = 20;
        int minute = 0;
        int second = 0;
        const RtcDateTime* now = new RtcDateTime(year, month, dayOfMonth, hour, minute, second);
        Rtc.SetDateTime(*now);
    
        RtcDateTime alarmTime = *now+1800; // it adds 30 min
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

    File dataFile = SD.open("weather_station.csv", FILE_WRITE);
    // se o arquivo foi aberto corretamente, escreve os dados nele
    if (dataFile) 
    {
        if (dataFile.size() == 0)
            dataFile.println("Date, Time, Temperature (C), Humidity (%)");
        
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
    
    digitalWrite(VCC_PIN, LOW); // turn off power suplly for sensors, rtc and sd module. RTC will still work on battery.
    ESP.deepSleep(0, WAKE_RF_DISABLED); //Put esp8266 to sleep for an undefined time
}
