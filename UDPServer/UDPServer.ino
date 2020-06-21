#include <ObirDhcp.h>           //dla pobierania IP z DHCP - proforma dla ebsim'a 
#include <ObirEthernet.h>       //niezbedne dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h>    //sama klasa 'ObirEthernetUDP'
//#include "coap-simple.h"        //Biblioteka CoAP

#define UDP_SERVER_PORT 5683 /*najczesciej uzywany port*/

#define HEADER_SIZE 4 /*Rozmiar naglowka w bajtach - oznacza pierwszy bit tokenu*/
#define ETAG_MAX_SIZE 2 /*Rozmiar obslugiwanego ETag w bajtach - patrz: dokumentacja*/
#define URIPATH_MAX_SIZE 4 /*Rozmiar URI-Path w bajtach - wskazanie zasobu*/

//#define

enum ETagStatus {NO_ETAG, VALID, INVALID};

byte MAC[]={0x28, 0x16, 0xAD, 0x71, 0xB4, 0xA7}; //Adres MAC wykorzystanej karty sieciowej

//dlugosc pakietu z danymi dla/z UDP
#define PACKET_BUFFER_LENGTH    50 
uint8_t packetBuffer[PACKET_BUFFER_LENGTH];

//numer portu na jakim nasluchujemy 
unsigned int localPort=UDP_SERVER_PORT;    

/*dla podejscia z biblioteka ObirEthernetUdp, nie powinno sie kreowac 
wiecej niz jednego obiektu klasy 'ObirEthernetUDP' */
ObirEthernetUDP Udp;

//Moze byc tylko jeden obiekt zbioru:
#include "Numbers.h"
Numbers Numbers;
/*    !!!UWAGA!!!    
           Instrukcja obslugi klasy Numbers w pliku naglowkowym (Numbers.h)
           Liczba przechowywanych liczb jest okreslana w powyzszym pliku.
*/   

#include "Conversions.h"
#include "coap-interpreter.h"

void setup() {
    //Zwyczajowe przywitanie z userem (niech wie ze system sie uruchomil poprawnie)
    Serial.begin(115200);
    Serial.println(F("OBIR eth UDP server init..."));
    Serial.print(F("Compiled on "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.println(F(__TIME__));

    //inicjaja karty sieciowej - proforma dla ebsim'a    
    ObirEthernet.begin(MAC);

    //potwierdzenie na jakim IP dzialamy - proforma dla ebsim'a
    Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; ++thisByte) {
        Serial.print(ObirEthernet.localIP()[thisByte], DEC);Serial.print(F("."));
    } Serial.println("");

    //Uruchomienie nasluchiwania na datagaramy UDP
    Udp.begin(localPort);
}

void loop() {
    //czekamy na pakiet - sprawdzajac jaka jest trego dlugosc (<=0 oznacza ze nic nie otrzymalismy)
    int packetSize=Udp.parsePacket(); 
    if(packetSize>0){
        //czytamy pakiet - maksymalnie do 'PACKET_BUFFER_LENGTH' bajtow
        int packetLen=Udp.read(packetBuffer, PACKET_BUFFER_LENGTH); //dlugosc pakietu
        
        //if(len<=0) Udp.flush();return;     //nie ma danych - wywolujemy funkcje wymiecenia bufora
            
        //prezentujemy otrzymany pakiet (zakladajac ze zawiera znaki ASCII)
        Serial.println("\n\n+---Received a message---+");
        packetBuffer[packetLen]='\0';

        /*---1.S Interpretacja odebranego pakietu---*/

        /*---1.1.S Interpretacja naglowka---*/

        uint8_t _version = (0xC0 & packetBuffer[0])>>6; 
        Serial.print(F("CoAP version: "));Serial.println(_version, DEC); //Tego tak naprawde nie potrzebujemy
        
        uint8_t _type = (0x30 & packetBuffer[0])>>4;              //1=NON, 0=CON
        if(_type==0)Serial.println(F("Type: CON"));
        
        if(_type == 1) //Nie obslugujemy CON; szkoda zasobow
        {
          Serial.println(F("Type: NON"));
          
          uint8_t _token_len = (0x0F & packetBuffer[0])>>0;
          uint8_t _class = ((packetBuffer[1]>>5)&0x07);
          uint8_t _code = ((packetBuffer[1]>>0)&0x1F);
          uint16_t _mid = (packetBuffer[2]<<8)|(packetBuffer[3]); //Message ID - ma 2 bajty
          uint8_t _token[_token_len]; //latwiej niz tworzyc 8 opcji roznych dlugosci int

          Serial.print(F("Token length: ")); Serial.println(_token_len, DEC);
          //Zczytanie tokena:
          if(_token_len > 0) 
          {
            Serial.println(F("Token: "));
            for(int i = 0; i < _token_len; ++i)
            {
              _token[i] = packetBuffer[i + HEADER_SIZE];
              /*przepisujemy bajty*/
              Serial.print(_token[i], HEX);Serial.print(F(" "));
            }
            Serial.println();
          }
          
          Serial.print(F("Code: ")); Serial.print(_class, DEC); Serial.print(F(".0"));Serial.println(_code, DEC);
          Serial.print(F("Message ID: ")); Serial.println(_mid, DEC);

          /*---1.1.E Koniec naglowka---*/


          /*---1.2.S Opcje---*/

          bool payloadFound = false;
          int marker = HEADER_SIZE + _token_len; //Znacznik polozenia w pakiecie (w bajtach)
          int payloadMarker = -1; //Znacznik polozenia payloadu (w bajtach)
          /*Ustawiony na -1, zeby wykryc blad*/

          uint32_t delta; //Moze miec 4 bit + 2 bajty = 20 bit, ale uint24 nie istnieje
          uint8_t optionLength;
          uint8_t optionNumber = 0;

          /*W celu oddzielenia odczytywania wiadomosci od wysylania odpowiedzi, 
          kazda obslugiwana opcja musi tez miec status, lub swoja wartoscia
          poczatkowa status ten odwzorowac*/
          uint8_t eTag[ETAG_MAX_SIZE]; //patrz: dokumentacja
          enum ETagStatus _eTagStatus = NO_ETAG;
          
          uint16_t contentFormat;
          uint8_t _uriPath[URIPATH_MAX_SIZE]; //Tablica na znaki w postaci liczb
          String uriPath = "NULL"; //wl. URI; zaczyna od "NULL" zeby mozna bylo sprawdzic, czy URI w ogole byl obecny
          
          while(!payloadFound && packetBuffer[marker]!='\0' ) //Dopoki nie znajdzie sie payload albo nie skonczy ramka
          {
            delta = (packetBuffer[marker] & 0xF0) >> 4; //Maska na pierwsze 4 bity
            optionLength = (packetBuffer[marker] & 0x0F); //Maska na kolejne 4 bity
            ++marker; //przesuniecie markera na nastepny bajt

            /*kiedy delta albo optionLength < 12, to jest brak rozszerzen; 
              Bajt markera oznacza wtedy wartosc opcji */
            
            if(delta == 13){
              //Jest 1 bajt rozszerzenia:
              delta+= packetBuffer[marker];
              ++marker;
            }
            else if(delta == 14){
              //Sa 2 bajty rozszerzenia
              //TODO: to moze nie byc konieczne, zalezy czy musimy obslugiwac opcje z duzym numerem
              delta = 269 + 256*packetBuffer[marker]; //pierwszy bajt ma wieksza wage
              ++marker;
              delta += packetBuffer[marker]; //dodajemy wartosc kolejnego bajtu
              ++marker;
            }
            else if(delta == 15) //Trafiono na marker payloadu
            {
              payloadFound = true;
              payloadMarker = marker+1; //payload zaczyna sie po markerze, ktory ma 1 bajt
              Serial.println(F("Payload reached"));
            }
            
            if(delta != 15) //Jezeli nie bylo markera payloadu, mozna obsluzyc opcje bez przejmowania sie bledami
            {

              //Dlugosc opcji w analogiczny sposob do delty.
              if(optionLength == 13){
                optionLength += packetBuffer[marker]; ++marker; 
              }
              else if(optionLength == 14){
                optionLength = 269 + 256*packetBuffer[marker]; ++marker;
                optionLength += packetBuffer[marker]; ++marker;
              }
            
              optionNumber+=delta; //Numer opcji to nr poprzedniej+delta
              
              /*Marker powinien w tym momencie wskazywac
                na zawartosc opcji, tzn optionValue.
                Pole to jest wielkosci optionLength. */

              //Opcje do obsluzenia podczas odbierania:

              Serial.print(F("Delta: "));Serial.print(delta, DEC);
              Serial.print(F(", Option number: "));Serial.print(optionNumber, DEC);
              Serial.print(F(", Option Length: "));Serial.println(optionLength, DEC);
              
 /*             if(optionNumber == 11)
              //URI-path
              {
                Serial.println(F("Opcja URI-Path"));
                if(uriPath == "NULL")
                {
                  for(int i=0; i<optionLength; i++)
                    _uriPath[i] = packetBuffer[marker]; ++marker;
                  ArrayToString(uriPath,_uriPath, URIPATH_MAX_SIZE);
                }
                Serial.print(F("URI-Path: ")); Serial.println(uriPath);
              }
*/
/*
              if(optionNumber == 17)
              //Accept - czyli jaka reprezentacje woli klient
              {
                Serial.println(F("Opcja Accept"));
                contentFormat = packetBuffer[marker]; ++marker;
                if(optionLength==2) ++marker;
                
                Serial.println(F("Chosen content format: ")); 
                if(contentFormat==0)
                  Serial.println(F("plain text"));
                else if(contentFormat==40)
                  Serial.println(F("application/link-format"));
                else
                  Serial.println(F("Requested format not supported"));

                //0 - plain text
                //40 - application/link-format
                //reszta raczej nas nie obchodzi
                //rozmiar moze byc do 2B, ale tak naprawde wystarczy sprawdzic 1
              }

              if(optionNumber == 12)//content-format: indicates the representation format of the message payload
              {
                Serial.println(F("Opcja Content-Format"));
              }
*/
              if(optionNumber == 4)//Etag
              {
                Serial.println(F("Opcja ETag"));
                //Obslugiwane sa tylko 2 bajty etag - patrz: dokumentacja
                if(optionLength == 2)
                {
                  eTag[1] = packetBuffer[marker]; ++marker;
                  eTag[2] = packetBuffer[marker]; ++marker;
                  _eTagStatus = VALID;
                  Serial.print(F("ETag: 0x"));Serial.print(eTag[1], HEX); Serial.println(eTag[2], HEX);
                }
                else{
                  _eTagStatus = INVALID;
                  Serial.println(F("Invalid ETag")); 
                }
                /* Jezeli dl. nierowna 2, od razu wiemy ze zasob nie bedzie mial zgodnego ETag.
                   Informacje na temat ETag juz mamy; ich obsluga bedzie oddzielnie, dalej   */
              }
              else //Jezeli jest nieobslugiwana opcja, i tak trzeba przesunac marker
              {
                marker += optionLength;
              }
              
            }
          }

        /*---1.2.E Koniec odczytu opcji---*/
        
        /*---1.E Koniec obieranego pakietu---*/
        
        /*---2.S Odpowiadanie---*/

          //Obsluga payloadu:
          //payload zaczyna sie na pozycji payloadMarker w pakiecie (packetBuffer)
        
          if(_class == 0)
          {
            //1+1; jako wypelniacz, do usuniecia
            if(_code == 0) 1+1; //empty; pewnie niepotrzebne
            
            if(_code == 1) //GET
            {
              1+1;
            }

            if(_code == 2) //PUT
            {
              1+1;
            }

          }
          
         /*---2.E koniec odpowiadania---*/
         
        }
    }
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
