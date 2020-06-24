#include <ObirDhcp.h>           //dla pobierania IP z DHCP - proforma dla ebsim'a 
#include <ObirEthernet.h>       //niezbedne dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h>    //sama klasa 'ObirEthernetUDP'

#define UDP_SERVER_PORT 5683 /*najczesciej uzywany port*/

#define HEADER_SIZE 4 /*Rozmiar naglowka w bajtach - oznacza pierwszy bit tokenu*/
#define ETAG_MAX_SIZE 2 /*Rozmiar obslugiwanego ETag w bajtach - patrz: dokumentacja*/
#define URIPATH_MAX_SIZE 255 /*max. rozmiar URI-Path w bajtach*/

//Sciezki do zasobow
#define AVERAGE "/mets/avg"
#define MEAN "/mets/mean"
#define STD_DEV "/mets/dev"
#define DIVIDIBLE "/nums/divs"
#define NUMBERS "/nums/all"
#define WELLKNOWN "/.well-known/core"

#define REOURCE1 "</mets/avg>;rt=\"Average\";ct=0"
#define REOURCE2 "</mets/mean>;rt=\"Mean\";ct=0"
#define REOURCE3 "</mets/dev>;rt=\"Standard deviation\";ct=0"
#define REOURCE4 "</nums/divs>;rt=\"Numbers dividable by a given number\";ct=0"
#define REOURCE5 "</nums/all>;rt=\"All stored numbers\";ct=0"

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
      Liczba przechowywanych liczb jest okreslana w tym samym pliku.
*/   

#include "Conversions.h"
#include "CoAP-factory.h"

coapFactory coapFactory; //Obiekt do wysylania pakietow

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

        Serial.println("\n\n+----Received a message----+");
        packetBuffer[packetLen]='\0';

        /*---1.S Interpretacja odebranego pakietu---*/

        /*---1.1.S Interpretacja naglowka---*/

    /*    uint8_t _version = (0xC0 & packetBuffer[0])>>6; 
        Serial.print(F("CoAP version: "));Serial.println(_version, DEC); //Tego tak naprawde nie potrzebujemy

    */
        
        uint8_t _type = (0x30 & packetBuffer[0])>>4;              //1=NON, 0=CON
        
        if(_type == 0)Serial.println(F("Type: CON")); //Nie obslugujemy CON; szkoda zasobow
        if(_type == 1) //Obslugujemy tylko NON
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

          Serial.println("\n|--->Reading options:");

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
          
          uint16_t contentFormat = 0xFFFF;
          uint16_t acceptFormat = 0xFFFF;
          uint8_t _uriPath[URIPATH_MAX_SIZE]; //Tablica na znaki w postaci liczb
          String uriPath = ""; //wlasciwe URI
          
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
              payloadMarker = marker;
              //Czytanie payloadu potem (po petli while), dla ulatwienia
            
            if(delta != 15) //Jezeli nie bylo markera payloadu, mozna obsluzyc opcje bez przejmowania sie bledami
            {

              //Dlugosc opcji zdobywamy w analogiczny sposob do delty.
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

              Serial.print(F("\nDelta: "));Serial.print(delta, DEC);
              Serial.print(F(", Option number: "));Serial.print(optionNumber, DEC);
              Serial.print(F(", Option Length: "));Serial.println(optionLength, DEC);
              
              if(optionNumber == 11)
              //URI-path 
              {
                Serial.println(F("Opcja URI-Path"));
                for(int i=0; i<optionLength; ++i)
                {
                  _uriPath[i] = packetBuffer[marker]; 
                  ++marker;
                }
                String tmp = "";
                ArrayToString(tmp, _uriPath, optionLength); 
                uriPath += '/';
                uriPath += tmp;  //URI moze skladac sie z kilku segmentow
                Serial.print(F("URI-Path: ")); Serial.println(uriPath);
              }

              else if(optionNumber == 17 || optionNumber == 12)
              /*Accept (czyli jaka reprezentacje woli klient)
              lub:
              Content-format (indicates the representation format of the message payload) 
              Opcje te sa praktycznie identyczne*/
              {
                uint16_t contentType; //Tu bedzie przechowana zawartosc opcji

                if(optionNumber == 17) Serial.println(F("Opcja Accept"));
                if(optionNumber == 12) Serial.println(F("Opcja Content-format"));
                
                if(optionLength==1)
                {
                  contentType = packetBuffer[marker]; 
                  ++marker;
                }
                if(optionLength==2)
                {
                  contentType = packetBuffer[marker];
                  contentType << 8; //Eksperymentalne; nie mialem na czym tego przetestowac
                  ++marker;
                  contentType = contentType | packetBuffer[marker];
                  ++marker;
                }
                
                Serial.print(F("Chosen content type: ")); 
                if(contentType==0) Serial.println(F("plain text\n"));
                else if(contentType==40) Serial.println(F("application/link-format\n"));
                else if(optionLength == 0)
                {
                  Serial.println(F("Content-type option length is zero: assuming text/plain"));
                  //Copper robil tak zamiast wysylac zero. Stad takie zalozenie (nie do konca zgodne z RFC)
                  contentType = 0;
                }
                else Serial.println(F("Given format not supported"));
                //reszta formatow raczej nas nie obchodzi

                //Zaleznie od opcji, wartosc jest wpisana do odp. zmiennej
                if(optionNumber == 17) acceptFormat = contentType;
                if(optionNumber == 12) contentFormat = contentType;
              }
              
              else if(optionNumber == 4)//Etag
              {
                Serial.println(F("Opcja ETag"));
                //Obslugiwane sa tylko 2 bajty etag - patrz: dokumentacja
                if(optionLength == 2)
                {
                  eTag[0] = packetBuffer[marker]; ++marker;
                  eTag[1] = packetBuffer[marker]; ++marker;
                  _eTagStatus = VALID;
                  Serial.print(F("ETag: 0x"));Serial.print(eTag[0], HEX); Serial.println(eTag[1], HEX);
                }
                else{
                  _eTagStatus = INVALID;
                  Serial.println(F("Invalid ETag")); 
                  marker += optionLength;
                }
                /* Jezeli dl. nierowna 2, od razu wiemy ze zasob nie bedzie mial zgodnego ETag.
                   Informacje na temat ETag juz mamy; ich obsluga bedzie oddzielnie, dalej   */
              }
              else //Jezeli jest nieobslugiwana opcja, i tak trzeba przesunac marker
              {
                marker += optionLength;
                Serial.println(F("Unsupported option")); 
              }
            }
          }

        /*---1.2.E Koniec odczytu opcji---*/

          Serial.println("\n|--->Reading the payload:");
          //Czytanie payloadu:
          //payload zaczyna sie na pozycji payloadMarker w pakiecie (packetBuffer)
          uint8_t payloadLen = packetLen - payloadMarker;
          uint8_t payload[payloadLen];
          
          if(payloadMarker > 0) //wykrycie, czy w ogole znaleziono payload
          /*Uwaga: powyzsze sprawdzenie musi byc wykonane przy kazdym uzyciu payloadu.
          Wymog ten wynika z faktu, ze tablica payload zostala zdefiniowana poza tym sprawdzeniem
          (dla latwiejszej osiagalnosci); Tym samym moze zawierac wartosci smieciowe.*/
          {
            Serial.print(F("Payload: 0x"));
            for(int i=0; i < payloadLen; ++i)
            {
              payload[i] = packetBuffer[payloadMarker+i];
              Serial.print(payload[i], HEX);
            }
            Serial.print(F(" = "));
            for(int i=0; i < payloadLen; ++i)Serial.print((char)payload[i]);
            Serial.println();
          }
          else Serial.println("No payload found");
        
        /*---1.E Koniec odbierania pakietu---*/

        Serial.println("\n|--->Handling the message:");
        /*---2.S Odpowiadanie---*/

        coapFactory.SetTokenAndMID(_token, _token_len, _mid); //Sa takie same dla kazdej odpowiedzi
        
          if(_class == 0)
          {            
            if(_code == 1) //------------------------------------------------------------------> GET
            {
              if(_eTagStatus == VALID) //Sprawdzenie, czy ETag w ogole zostal nadany
              {
                uint8_t second_hex_fresh = 0x00; //Tu w fcji checkETag bedzie wpisany aktualny drugi bajt ETaga
                if( Numbers.checkETag(eTag[0], second_hex_fresh) ) //Sprawdza, czy da sie znalezc zasob po pierwszym bajcie
                {
                  Serial.print(F("Received ETag: 0x"));Serial.print(eTag[0], HEX); Serial.println(eTag[1], HEX);
                  Serial.print(F("Correct  ETag: 0x"));Serial.print(eTag[0], HEX); Serial.println(second_hex_fresh, HEX); //Debug
                  
                  Serial.println(F("ETag matches an existing resource."));
                  if( eTag[1] == second_hex_fresh ) //porownujemy otrzmany bajt 2 z naszym
                  {
                    Serial.println(F("ETag is fresh."));
                    coapFactory.SetHeader(2, 3); //2.03 "Valid"
                    //Dodac opcje z aktualnym ETagiem
                    coapFactory.SendPacketViaUDP(Udp);
                  }
                  else
                  {
                    Serial.println(F("ETag is outdated."));
                    //Wyslac 2.05 z aktualnym ETagiem i zawartoscia - tylko najpierw trzeba wiedziec jaka:
                    
                    String resourceByEtag = ""; //To bedzie sciezka, ktorej chcial klient
                    
                         if(eTag[0] = 0x11) resourceByEtag = AVERAGE; //Hexy z tabeli z Numbers.h
                    else if(eTag[0] = 0x12) resourceByEtag = MEAN;
                    else if(eTag[0] = 0x13) resourceByEtag = STD_DEV;
                    else if(eTag[0] = 0x21) resourceByEtag = DIVIDIBLE;
                    else if(eTag[0] = 0x22) resourceByEtag = NUMBERS;

                    coapFactory.SetHeader(2, 5); //2.05 "Content"
                    //Dodac opcje z aktualnym ETagiem
                    coapFactory.SendPacketViaUDP(Udp);
                  }
                   
                }
                else _eTagStatus = INVALID;
              }
              if(_eTagStatus == INVALID) //W ten sposob pomijamy sytuacje, kiedy nie ma ETaga
              {
                  //Wyslanie bledu - 4.04 "Not found" 
                  coapFactory.SetHeader(4, 4);
                  coapFactory.SetPayloadString(F("Can't map ETag to resource"));
                  coapFactory.SendPacketViaUDP(Udp);
              }
              
              if(_eTagStatus == NO_ETAG) //Jezeli byl ETag, nie sprawdzamy zasobu
              {
                coapFactory.SetHeader(2, 5); //Wspolne dla wszystkich
                
                if(uriPath == MEAN) //------------------------------>Obsluga zasobu z URI-path
                {
                  String tmp = "Mean: " + Numbers.getMedianInt();
                  coapFactory.SetPayloadString(tmp); 
                }
                else if(uriPath == AVERAGE)
                {
                   
                }
                else if(uriPath == STD_DEV)
                {
                   
                }
                else if(uriPath == NUMBERS)
                {
                   
                }
                else if(uriPath == DIVIDIBLE)
                {
                   
                }
                else if(uriPath == WELLKNOWN)
                {
                  String tmp = REOURCE1;
                  tmp += REOURCE2;
                  tmp += REOURCE3;
                  tmp += REOURCE4;
                  tmp += REOURCE5;

                  coapFactory.AddOptionSimple(12, 40); //12=opcja content-format, 40 = link format
                  coapFactory.SetPayloadString(tmp); 
                  coapFactory.SendPacketViaUDP(Udp);
                }
                else
                {
                  //4.08; Request entity (tutaj URI) incomplete
                  coapFactory.SetHeader(4, 8);
                  coapFactory.SetPayloadString(F("Bad or empty URI"));
                }

                coapFactory.SendPacketViaUDP(Udp); //Wspolne dla wszystkich
              }
            }

            if(_code == 3) //------------------------------------------------------------------> PUT
            {
              if(contentFormat != 0) //inny niz text/plain
              {
                //Wysylanie bledu 4.15; Unsupported or unknown payload content format
                coapFactory.SetHeader(4, 15);
                //Dodac opcje Accept z text/plain
                coapFactory.SendPacketViaUDP(Udp); 
              }
              else if(payloadMarker > 0) //Sprawdzenie, czy w ogole jest payload
              {
                unsigned int new_number = 0; //Do tej zmiennej ponizsza linijka wpisze wartosc
                if(AnyBaseAsciiToInt(10, payload, new_number)) //Przyjmujemy tylko zapis dziesietny
                {
                  Serial.print(F("Number from payload: "));Serial.println(new_number);
                  if(Numbers.AddNum(new_number)) 
                  {
                    //Numer dodano poprawnie; odpowiedziec nalezyta wiadomoscia. Chyba mozna dolaczyc od razu ETag
                    //2.04:
                    coapFactory.SetHeader(2, 4);
                    coapFactory.SendPacketViaUDP(Udp);
                  }
                  else
                  {
                    /*Zbior jest przepelniony: 
                    "W przypadku wyczerpania pamieci na liczby mozna w odpowiedzi umiescic opcję Size1" */
                    coapFactory.SetHeader(5, 0); //5.00; "Internal server error"
                    coapFactory.SetPayloadString(F("Number buffer full"));
                    coapFactory.SendPacketViaUDP(Udp);
                  }
                }
                else
                {
                  //Wyslano cos innego niz znaki 0-9
                  coapFactory.SetHeader(4, 0); //4.00; "Bad request"
                  coapFactory.SetPayloadString(F("Non-numeric values in payload"));
                  coapFactory.SendPacketViaUDP(Udp);
                }
              }
            }

          }
          
         /*---2.E koniec odpowiadania---*/
         
        }
    }
}
