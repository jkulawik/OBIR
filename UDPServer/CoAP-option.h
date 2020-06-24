
//Klasa opcji protokolu CoAP.

#define OPTION_MAX_SIZE 2

class coapOption{

public:
unsigned int optionNumber = 0;
uint8_t optionValues[OPTION_MAX_SIZE]; 
unsigned int optionLen = 0;

coapOption(){} //Konstruktor domyslny

//Konstruktor wlasciwy:
coapOption(unsigned int _optionNumber, uint8_t _optionValues[], unsigned int _optionLen)
{
  optionNumber = _optionNumber;
  optionLen = _optionLen;
  
  for(int i=0; i < _optionLen; ++i) //Przepisanie tablicy
    optionValues[i] = _optionValues[i];
}

coapOption(const coapOption &obj) //Kopia gleboka
{
  optionNumber = obj.optionNumber;
  optionLen = obj.optionLen;

  for(int i=0; i < optionLen; ++i) //Przepisanie tablicy
    optionValues[i] = obj.optionValues[i];
}

};
