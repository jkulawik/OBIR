
#define COAP_VERSION 1
#define MAX_PAYLOAD_SIZE 50
#define MAX_TOKEN_LEN 8
#define TOKEN_LOCATION 4 

class coapFactory{

  uint8_t workPacket[MAX_PAYLOAD_SIZE];
  unsigned int packetLen = 0;

  uint8_t workToken[MAX_TOKEN_LEN];
  uint8_t tokenLen = 0;

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

    CleanTheFactory();
  }

  void CleanTheFactory()
  {
    packetLen = 0;
    tokenLen = 0;
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
    
      for(int i=0; i < tokenLen; ++i) workPacket[TOKEN_LOCATION+i] = _tokenArr[i]; //Przepisujemy tokenLen bajtow
      packetLen += tokenLen;

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
    workPacket[packetLen] = 0xFF;
    ++packetLen;

    for(int i=0; i < payloadLen; ++i) workPacket[packetLen+i] = payload[i]; //Przepisujemy payload
    packetLen += payloadLen;
  }

  
};
