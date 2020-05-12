#include <ObirDhcp.h>           //dla pobierania IP z DHCP - proforma dla ebsim'a 
#include <ObirEthernet.h>       //niezbedne dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h>    //sama klasa 'ObirEthernetUDP'

#define UDP_SERVER_PORT 5683 //najczesciej uzywany port

byte MAC[]={0x28, 0x16, 0xAD, 0x71, 0xB4, 0xA7}; //Adres MAC wykorzystanej karty sieciowej

//dlugosc pakietu z danymi dla/z UDP
#define PACKET_BUFFER_LENGTH    20 
unsigned char packetBuffer[PACKET_BUFFER_LENGTH];

//numer portu na jakim nasluchujemy 
unsigned int localPort=UDP_SERVER_PORT;    

//dla podejscia z biblioteka ObirEthernetUdp, nie powinno sie kreowac 
//wiecej niz jednego obiektu klasy 'ObirEthernetUDP'
ObirEthernetUDP Udp;

void setup() {
    //Zwyczajowe przywitanie z userem (niech wie ze system sie uruchomil poprawnie)
    Serial.begin(115200);
    Serial.println(F("OBIR eth UDP server init..."));
    Serial.print(F("Compiled on "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.println(F(__TIME__));

    //inicjaja karty sieciowe - proforma dla ebsim'a    
    ObirEthernet.begin(MAC);

    //potwierdzenie na jakim IP dzialamy - proforma dla ebsim'a
    Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ObirEthernet.localIP()[thisByte], DEC);Serial.print(F(".\n"));
    }

    //Uruchomienie nasluchiwania na datagaramy UDP
    Udp.begin(localPort);
}

void loop() {
    //czekamy na pakiet - sprawdzajac jaka jest trego dlugosc (<=0 oznacza ze nic nie otrzymalismy)
    int packetSize=Udp.parsePacket(); 
    if(packetSize>0){
        //czytamy pakiet - maksymalnie do 'PACKET_BUFFER_LENGTH' bajtow
        int len=Udp.read(packetBuffer, PACKET_BUFFER_LENGTH); //dlugosc pakietu
        /*
        if(len<=0){                     //nie ma danych - wywolujemy funkcje wymiecenia bufora
            Udp.flush();return;
        }*/

        //prezentujemy otrzymany pakiet (zakladajac ze zawiera znaki ASCII)
        Serial.print("Received: ");
        packetBuffer[len]='\0';
        Serial.println((char*)packetBuffer);

        //interpretacja odebranego pakietu
    }
}

//wpisuje zawartosc aszMessage do String asData
void ArrayToString(String asData,char aszMessage[], int alSize)
{
  asData = ""; //zerujemy dane
  for(int j=0; j<alSize; j++) asData += (char) aszMessage[j];
}

void SendUDPString(char packetBuffer[])
{
        //odsylamy pakiet do nadawcy (wywolania: Udp.remoteIP(), Udp.remotePort() - pozwola nam sie 
        //   zorentowac jaki jest jego IP i numer portu UDP)
        //UWAGA!!!  wywolania Udp.remoteIP(), Udp.remotePort() zwracaja
        //          poprawny(sensorwny) wynik jezeli jakis datagram UDP faktycznie otrzymano!!!

        //wielkosc tablicy = rozmiar tablicy / rozmiar elementu:
        int len = sizeof(packetBuffer)/sizeof(packetBuffer[0]); 

        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(packetBuffer, len);
        Udp.endPacket();
}


//Funkcja zamienia ciag znakow typu string na int o dowolnej podstawie (hex, oct...)
unsigned long AnyBaseAsciiToInt(int alBase, String strData)
{
  int len = strData.length();
  char c;
  unsigned long localresult = 0;
  unsigned long multiplier = 0;
  
  for(int i = 0; i < len; i++) //sprawdzamy wszystkie znaki
  {
    c = strData.charAt(len-i-1); //pobiera kolejny znak od tylu
    if(isAlphaNumeric(c)) 
    {
      //od tego momentu traktujemy c jako int
      if(c >= 48 && c <= 57) c -= 48; //znaki liczb na liczby
      
      if(c >= 97 && c <= 122) c -= 32; //litery male na wielkie
      if(c >= 65 && c <= 90) c -= 55; //litery wielkie na liczby
      
      multiplier = pow_int(alBase, i);
      
      if(c < alBase) localresult += ((int) c)*multiplier;
      else Serial.println(F("Ignoring a character bigger than number base"));
    }
    else Serial.println(F("Skipping non-alphanumeric character"));
  }  
  return localresult;
}
