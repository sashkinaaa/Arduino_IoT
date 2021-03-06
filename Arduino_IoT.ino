#include <SPI.h>
#include <Ethernet.h>

byte mac[] =
{
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(10, 10, 10,14);

EthernetClient client;

char server[] = "10.10.10.17";

const int Relay1=2,Relay2=3;
const int NICDelay=500;
int state=0,time=0,newTime=0;
unsigned long waitingTime=0,startTime=0,lastConnectionTime=0;

void setup()
{
    pinMode(Relay1,OUTPUT);
    pinMode(Relay2,OUTPUT);
    digitalWrite(Relay1,1);
    digitalWrite(Relay2,1);
    Serial.begin(9600);
    while (!Serial)
    {
        ;
    }

    delay(1000);
    Ethernet.begin(mac, ip);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
}

int reader()
{
    int value=0;
    while (client.available())
    {
        value=value*10;
        char c = client.read();
        value=value+(c-'0');
    }
    return value;
}

void HTTPRequest()
{
    client.stop();

    if (client.connect(server, 80))
    {
        client.println("GET /fileArduino3.html");
        client.println("Host: 10.10.10.17");
        client.println("Connection: close");
        client.println();
    }
    else
    {
        Serial.println("connection failed");
    }
}

void loop()
{
    switch(state)
    {
    case 0:
        Serial.println("State:");
        Serial.println(state);
        digitalWrite(Relay1,1);
        digitalWrite(Relay2,1);
        delay(2*NICDelay);
        HTTPRequest();
        delay(NICDelay);
        while((newTime=reader())==0)
        {
            HTTPRequest();
            delay(NICDelay);
            Serial.println("newTime:");
            Serial.println(newTime);
        }
        time=newTime;
        if(newTime%10==2)
            state=1;
        break;
    case 1:
        Serial.println("State:");
        Serial.println(state);
        digitalWrite(Relay1,0);
        delay(5000);
        startTime=millis();
        state=2;
        break;
    case 2:
        Serial.println("State:");
        Serial.println(state);
        waitingTime=(newTime/10)*60000L;
        digitalWrite(Relay1,1);
        delay(NICDelay);
        HTTPRequest();
        delay(NICDelay);
        lastConnectionTime=millis();
        while(((newTime=reader())==0)&&(millis() - lastConnectionTime < 10*NICDelay))
        {
            HTTPRequest();
            delay(NICDelay);
            Serial.println("newTime:");
            Serial.println(newTime);
        }
        if(newTime==0)
            newTime=time;
        if(newTime!=time)
            if(newTime%10==1)
            {
                digitalWrite(Relay2,0);
                delay(5000);
                state=0;
                digitalWrite(Relay2,1);
            }    
            else
            {
                waitingTime=(newTime/10)*60000;
                Serial.println("newWaitingTime:");
                Serial.println(waitingTime);
            }
        if(((millis())-startTime)>waitingTime)
        {
            state=6;
        }
        break;
    case 6:
        Serial.println("State: \n3");
        digitalWrite(Relay2,0);
        delay(5000);
        state=7;
        break;
    case 7:
        Serial.println("State: \n4");
        digitalWrite(Relay2,1);
        delay(NICDelay);
        HTTPRequest();
        while((newTime=reader())==0)
        {
            HTTPRequest();
            delay(NICDelay);
            Serial.println("newTime:");
            Serial.println(newTime);
        }
        if(newTime%10==1)
            state=0;
        break;
    }
}

