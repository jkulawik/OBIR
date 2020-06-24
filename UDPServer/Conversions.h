
/*

Zbior funkcji konwertujacych.
Zawartosc:
* ArrayToString - zamiana tablicy znakow na obiekt String
* pow_int - potegowanie liczb calkowitych
* AnyBaseAsciiToInt - sumuje ASCII do int. Sluzy do konwersji z hex, oct, etc. na zmienne int

*/

//wpisuje zawartosc aszMessage do String asData
void ArrayToString(String &asData, uint8_t aszMessage[], int alSize)
{
  asData = ""; //zerujemy dane
  for(int j=0; j<alSize; j++) asData += (char) aszMessage[j];
}

/*Zrodlo algorytmu potegowania:
https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
Funkcja potrzebna do zamiany ASCII na int 
*/
unsigned long pow_int(unsigned long base, int exp)
{
    unsigned long powresult = 1;
    for (;;)
    {
        if (exp & 1)
            powresult *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return powresult;
}

/*Funkcja zamienia ciag znakow typu string na int o dowolnej podstawie (hex, oct...)
oraz wpisuje go do zmiennej result. 
Fcja zwraca, czy tablica zawiera znaki nie-alfanumeryczne lub wieksze niz podana podstawa*/
unsigned int AnyBaseAsciiToInt(int alBase, uint8_t charArray[], unsigned int &result)
{
  unsigned int len = sizeof(charArray)/sizeof(charArray[0]); //wielkosc tablicy = rozmiar tablicy / rozmiar elementu:
  char c;
  unsigned int localresult = 0;
  unsigned int multiplier = 0;
  
  for(int i = 0; i < len; i++) //sprawdzamy wszystkie znaki
  {
    c = charArray[len-i-1]; //pobiera kolejny znak od tylu
    if(isAlphaNumeric(c)) 
    {
      //od tego momentu traktujemy c jako int
      if(c >= 48 && c <= 57) c -= 48; //znaki liczb na liczby
      
      if(c >= 97 && c <= 122) c -= 32; //litery male na wielkie
      if(c >= 65 && c <= 90) c -= 55; //litery wielkie na liczby
      
      multiplier = pow_int(alBase, i);
      
      if(c < alBase) localresult += ((unsigned int) c)*multiplier;
      else
      {
        Serial.println(F("Ignoring a character bigger than number base"));
        return false;
      }
    }
    else
    {
      Serial.println(F("Non-alphanumeric character"));
      return false;
    }
  }  
  result = localresult;
  return true;
}
