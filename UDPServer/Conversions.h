
/*

Zbior funkcji konwertujacych.
Zawartosc:
* ArrayToString - zamiana tablicy znakow na obiekt String
* AnyBaseAsciiToInt - sumuje ASCII do int. Sluzy do konwersji z hex, oct, etc. na zmienne int
* pow_int - potegowanie liczb calkowitych

*/

//wpisuje zawartosc aszMessage do String asData
void ArrayToString(String asData,char aszMessage[], int alSize)
{
  asData = ""; //zerujemy dane
  for(int j=0; j<alSize; j++) asData += (char) aszMessage[j];
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
