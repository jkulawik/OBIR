
//Maksymalna liczba opcji w pakiecie CoAP
#ifndef COAP_MAX_OPTION_NUM
#define COAP_MAX_OPTION_NUM 10
#endif

//Definicja klasy opcji wraz z jej atrybutami
class CoapOption {
    public:
    uint8_t number;
    uint8_t length;
    uint8_t *buffer; //wskaznik na payload opcji
};

//Klasa sluzaca do obslugi problematycznych fragmentow pakietu
class receivedPacket
{

   CoapOption options[COAP_MAX_OPTION_NUM];


  
};
