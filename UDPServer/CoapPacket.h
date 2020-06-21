
/*Klasa tworząca pakiet w postaci tablicy do wysłania

	- dostarczone parametry zamieniane są na obiekt klasy CoapPacket, ktory nastepnie jest przekazywany dalej
	(docelowo do sendPacket w udpserver)

*/

//niekoniecznie potrzebne - jeśli tak, to jeszcze jest parsowanie w pliku coap-simple.cpp
class CoapOption {
    public:
    uint8_t number;
    uint8_t length;
    uint8_t *buffer; //wskaznik na payload opcji
};

class CoapPacket
{
  public:
	uint8_t type = 0; //u nas tylko NON?
	uint8_t tokenlen = 0;
	uint8_t code = 0;
	uint16_t messageid = 0;
	uint8_t *token = NULL;
	uint8_t *payload = NULL;
	size_t payloadlen = 0; //niewykorzystywane, więc niepotrzebne?
	uint8_t optionnum = 0; //licznik liczby opcji danego pakietu
	
	/*Konstruktor domyslny*/
	Packet() {} 
	
	//niekoniecznie potrzebne
	//tablica opcji
	CoapOption options[10]; //max opcji dla nas to nieco mniej
	//niekoniecznie potrzebne	
	void addOption(uint8_t number, uint8_t length, uint8_t *opt_payload);

  uint16_t sendPacket(CoapPacket &packet); //predefinicja funkcji wysylajacej pakiet
  
	uint16_t to_send(uint8_t type, uint8_t tokenlen, uint8_t code, uint16_t messageid, uint8_t *token, uint8_t *payload, size_t payloadlen, uint16_t contentformat) {
	
	//Tworzenie pakietu - ale my chcemy tablicę?
    CoapPacket packet;

    packet.type = type;
	packet.tokenlen = tokenlen;
    packet.code = code;
	packet.messageid = messageid;
    packet.token = token;
    packet.payload = payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;

	/*DODAWANIE OPCJI
    //Dodaje adres nadawcy jako opcje URI_HOST 
    char IPAddress[16] = "";
    sprintf(IPAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]); //przepisanie adresu z obiektu IPAddress do tablicy znakow
    packet.addOption(COAP_URI_HOST, strlen(IPAddress), (uint8_t *)IPAddress);

	//Dodawanie URL jako kolejnych opcji URI_PATH
    int idx = 0;
    for (int i = 0; i < strlen(url); i++) //parsowanie po tablicy znakow URL
	{
        if (url[i] == '/') 
		{
			packet.addOption(COAP_URI_PATH, i-idx, (uint8_t *)(url + idx));
			//Dodawanie opcji URI_PATH; dlugosc = pozycja iteratora - pozycja ostatniego znaku poprzedniego fragmentu URL
			//payload opcji = wskaznik na pozycje odp. danych w pamieci, tzn. adres URL przesuniety o polozenie konca poprzedniego fragmentu
            idx = i + 1;
        }
    }
	//To jest chyba po prostu oddzielne dodanie ostatniej opcji
    if (idx <= strlen(url)) {
		packet.addOption(COAP_URI_PATH, strlen(url)-idx, (uint8_t *)(url + idx));
    }
	

	//Dodawanie opcji content format, jezeli zostala podana
	uint8_t optionBuffer[2] {0}; //2 bajty zer, chyba? //mozna wrzucic pod if zeby zaosczedzic zasoby, ale to podczas testowania
	if (content_type != COAP_NONE) {
		optionBuffer[0] = ((uint16_t)content_type & 0xFF00) >> 8;
		optionBuffer[1] = ((uint16_t)content_type & 0x00FF) ;
		packet.addOption(COAP_CONTENT_FORMAT, 2, optionBuffer);
	}
	*/
	return this->sendPacket(packet);
	}
	
	/*DO UDPSERVER NA KONIEC? - pakowanie obiektu pakiet do tablicy i wysylka
	//Wysyla podany pakiet na podane IP poprzez wybrany port
	uint16_t sendPacket(CoapPacket &packet) {
	//po dołączeniu .h do udpserver można dać PACKET_BUFFER_LENGTH zamiast 50
    uint8_t buffer[50]; //Pakiet to de-facto tablica osmiobitowych l. calkowitych (a raczej bajtow danych)
    uint8_t *p = buffer; 	//"Uchwyt" pakietu, cos ala iterator chodzacy po pamieci bufora
	
	//Idea tej funkcji to kopiowanie danych na pozycje p w pamieci, a nastepnie przesuwanie p do przodu.
	
	//Przydaje sie znac funkcje memcpy: memcpy(dest, src, size); Funkcja ta kopiuje size bajtów z obiektu src do obiektu dest.
	
    uint16_t running_delta = 0;
    uint16_t packetSize = 0;

    // make coap packet base header
    *p = 0x01 << 6;
    *p |= (packet.type & 0x03) << 4;
    *p++ |= (packet.tokenlen & 0x0F);
    *p++ = packet.code;
    *p++ = (packet.messageid >> 8);
    *p++ = (packet.messageid & 0xFF);
    p = buffer + COAP_HEADER_SIZE;
    packetSize += 4;

    // make token
    if (packet.token != NULL && packet.tokenlen <= 0x0F) {
        memcpy(p, packet.token, packet.tokenlen);
        p += packet.tokenlen;
        packetSize += packet.tokenlen;
    }

    //---------------Dodawanie opcji----------------
    for (int i = 0; i < packet.optionnum; i++)  {
        uint32_t optdelta;
        uint8_t len, delta;

        if (packetSize + 5 + packet.options[i].length >= COAP_BUF_MAX_SIZE) {
            return 0;
        }
        optdelta = packet.options[i].number - running_delta;
        COAP_OPTION_DELTA(optdelta, &delta);
        COAP_OPTION_DELTA((uint32_t)packet.options[i].length, &len);

        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13) {
            *p++ = (optdelta - 13);
            packetSize++;
        } else if (delta == 14) {
            *p++ = ((optdelta - 269) >> 8);
            *p++ = (0xFF & (optdelta - 269));
            packetSize+=2;
        } if (len == 13) {
            *p++ = (packet.options[i].length - 13);
            packetSize++;
        } else if (len == 14) {
            *p++ = (packet.options[i].length >> 8);
            *p++ = (0xFF & (packet.options[i].length - 269));
            packetSize+=2;
        }

        memcpy(p, packet.options[i].buffer, packet.options[i].length); //Payload opcji (buffer) jest kopiowany na ostatnia pozycje 
        p += packet.options[i].length;
        packetSize += packet.options[i].length + 1;
        running_delta = packet.options[i].number;
    }
	//---------------Koniec dodawania opcji----------------

    // make payload
    if(packet.payloadlen > 0) {
        if ((packetSize + 1 + packet.payloadlen) >= COAP_BUF_MAX_SIZE) {
            return 0;
        }
        *p++ = 0xFF;
        memcpy(p, packet.payload, packet.payloadlen);
        packetSize += 1 + packet.payloadlen;
    }

	//"->" zamiast "."?
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(buffer, packetSize);
    Udp.endPacket();

    return packet.messageid;
	}
	*/
};
