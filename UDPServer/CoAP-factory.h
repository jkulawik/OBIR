
/*Builder* class for making and sending CoAP packets.
Make sure to call the functions in proper order:
1. Header or Token
2. Token or header
3. Options in option number order (if any) //Na razie nie ma sortowania
4. Payload (if any) or** PrepareOptions if there is no payload
5. Sending the packet


Using a different order will overwrite octets and/or
construct the packet in an incorrect order, which will corrupt the message.

*using "factory" in the class name was a misnomer

**PrepareOptions is called within SetPayload, so it must be done manually if none was set.

*/

#include "CoAP-option.h"

#define COAP_VERSION 1

#define MAX_PACKET_SIZE 220
#define MAX_TOKEN_LEN 8
#define TOKEN_LOCATION 4
#define MAX_OPTIONS 5 

class coapFactory{

  uint8_t workPacket[MAX_PACKET_SIZE];
  unsigned int packetLen = 0;

  uint8_t workToken[MAX_TOKEN_LEN];
  uint8_t tokenLen = 0;

  coapOption options[MAX_OPTIONS];
  uint8_t optionCount = 0;
  unsigned int lastOptionNum = 0;

  public:
  //Konstruktor domyslny
  coapFactory()
  {

  }

  void SendPacketViaUDP(ObirEthernetUDP &Udp)
  {
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(workPacket, packetLen);
    Udp.endPacket();

    //PrepareOptions();

    CleanTheFactory();
  }

  void CleanTheFactory()
  {
    packetLen = 0;
    tokenLen = 0;
    
    //wyczyscic opcje
    optionCount = 0;
    lastOptionNum = 0;
    
    //Reszta i tak bedzie nadpisana
  }

   bool SetTokenAndMID(uint8_t _tokenArr[], uint8_t tokenLen, uint16_t _messageID)
   {
      //Pierwszy bajt:
      workPacket[0] = (workPacket[0] & 0xF0) | tokenLen; //maska w razie odwrotnej kolejnosci ustawienia naglowka.
      
      //Trzeci i czwarty bajt - MID:
      workPacket[2] = (0xFF00 & _messageID) >> 8; //bierzemy pierwsze pol MID i przesuwamy je do konca w prawo
      workPacket[3] = (0x00FF & _messageID); //bierzemy drugie pol; zera powinny byc uciete przy konwersji

      if(packetLen == 0) packetLen = 4;

    if(tokenLen>0)
    {
      for(int i=0; i < tokenLen; ++i) workPacket[TOKEN_LOCATION+i] = _tokenArr[i]; //Przepisujemy tokenLen bajtow
      packetLen += tokenLen;
    }

      return true;
   }
   
   bool SetHeader(uint8_t _class, uint8_t _code)
   {
      //always NON type
      return SetHeaderFull(1, _class, _code);
   }

  //Ustawia naglowek, zwraca czy zostal poprawnie stworzony
  bool SetHeaderFull(uint8_t _type, uint8_t _class, uint8_t _code)
  {
    if(_type > 3) return false; //Istnieja tylko 4 typy, od 0 do 3
    
    uint8_t _version = COAP_VERSION;
    //Pierwszy bajt:
    workPacket[0] = (_version << 6) | (_type << 4) | workPacket[0];
    /*         pierwsze 2 bity^       kolejne 2^    ^TKL powinno juz byc ustawione i miec pierwsze 4 bity puste      */
    
    //Drugi bajt:
    workPacket[1] = (_class << 5) | _code;
    /*         pierwsze 3 bity^     ^kolejne 5, kod nie powinien miec nic na pierwszych 3 bitach */

    if(packetLen == 0) packetLen = 4;

    return true;
  }

  void SetPayload(uint8_t payload[], uint8_t payloadLen)
  {
    PrepareOptions();
    
    workPacket[packetLen] = 0xFF;
    ++packetLen;

    for(int i=0; i < payloadLen; ++i) workPacket[packetLen+i] = payload[i]; //Przepisujemy payload
    packetLen += payloadLen;
  }

  void SetPayloadString(String str)
  {
    unsigned int maxPayloadLen = MAX_PACKET_SIZE-packetLen;
    if(str.length() <= maxPayloadLen )
    {
      uint8_t TxPayload[maxPayloadLen+1];
      str.toCharArray(TxPayload, str.length()+1); //+1 to miejsce na \0
      SetPayload(TxPayload, str.length()); //-1 zeby nie wysylac \0
    }
    else Serial.println(F("Payload too large!"));
  }

  

  void AddOptionSimple(unsigned int optionNumber, uint8_t optionValue) //Dla opcji z jednym bajtem w wartosci
  {
    uint8_t optionValues[1] = {optionValue};
    AddOptionFull(optionNumber, optionValues, 1);
  }
  
  void AddOptionFull(unsigned int optionNumber, uint8_t optionValues[], uint8_t optionLen)
  {
    options[optionCount] = coapOption(optionNumber, optionValues, optionLen);
    ++optionCount;
  }


  void PrepareOptions()
  {
    for(int j=0; j<optionCount; ++j) //Dla kazdej opcji
    {      
      uint8_t delta = options[j].optionNumber - lastOptionNum;
      
      uint8_t optionHeaderMark = packetLen; //Znacznik pierwszego bajtu opcji

      unsigned int oLen = options[j].optionLen; //Zeby nie wywolywac niepotrzebnie

      //Pierwszy bajt - delta
      if(delta <= 12){
        workPacket[packetLen] = (delta & 0x0F) << 4;
        ++packetLen;
      }

      //TODO: zaimplementowac to poprawnie
      /*
      if(delta < 268 && delta > 12) //umieszczanie kolejnych bajtow //<---------- rozszerzenia; do implementacji potem...
      {
        uint8_t delta2 = delta - 13;
        delta = 13;
        workPacket[packetLen] = (delta & 0x0F) << 4;
        ++packetLen;
        workPacket[packetLen] = delta2;
        ++packetLen;
      } */
      //if(delta >= 269) ...

      //Pierwszy bajt - optionLen
      if(oLen <= 12)
      {
        uint8_t oLen_8b = oLen; //Rzutujemy na mniejsza zmienna
        workPacket[optionHeaderMark] = workPacket[optionHeaderMark] | oLen_8b; //sumujemy z istniejacymi bitami
      }
      //if(oLen <= 268 && oLen > 12) //umieszczanie kolejnych bajtow
      //if(oLen >= 269) ...

      //Option value:
      for(int i=0; i < oLen; ++i)
        workPacket[packetLen+i] = options[j].optionValues[i]; //Przepisujemy bajty wartosci
      
      packetLen += oLen; //Poprawiamy glowny marker
      lastOptionNum = options[j].optionNumber;
    }
  }

 
};
