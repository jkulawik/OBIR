
/*Klasa obslugujaca zbior liczb naturalnych

UWAGA: Poniewaz zwracanie danych przez klase jest zalezne od implementacji CoAP,
nie implementowano tych funkcji w tym pliku. 

Zamiast tego, proponowany kod zwracajacy dane zostal umieszczony na koncu tego pliku. 
Nalezy umiescic go w odpowiednim miejscu glownego programu.

Kod ten to m.in:
- Zwracanie zbioru
- Zwracanie liczb podzielnych przez dostarczony argument
*/

#define MAX_NUMBERS 5 //Rozmiar bufora przechowywanych liczb
#define ETAG_MAX_SIZE 2 //Patrz: dokumentacja
#include <math.h>

//Makro do generacji losowego bajta:
#define hexRAND random(0xFF)

class Numbers
{
  public:
  int nums[MAX_NUMBERS];
  int current_len = 0;
  float median = 0, average = 0, deviation = 0; //Metryki

  /*Konstruktor domyslny*/
  Numbers() {} 

//Tablica do ETagow. Objasnienie w tabeli ponizej
  uint8_t eTags [2][5] = {
/*           rzad^  ^kolumna  */ 
    {0x11, 0x12, 0x13, 0x21, 0x22},
    {hexRAND,hexRAND,hexRAND,hexRAND,hexRAND}
    };

/*
+--------------+---------+------+---------+-----------+---------+
|     Name:    | AVERAGE | MEAN | STD_DEV | DIVIDIBLE | NUMBERS |
+--------------+---------+------+---------+-----------+---------+
| Resource ID: | 0x11    | 0x12 | 0x13    | 0x21      | 0x22    |
| Tag:         | rand    | rand | rand    | rand      | rand    |
+--------------+---------+------+---------+-----------+---------+ 
 */

  bool checkETag(uint8_t id, uint8_t &tag) 
  /*id - pierwszy bajt ETaga, tag - drugi 
  Fcja sprawdza, czy pierwszy bajt sie zgadza;
  Ponadto wpisuje aktualny tag do zmiennej tag */
  {
    for(int i = 0; i < 5; ++i)
    {
      if(id == eTags[1][i])
      {
        tag = eTags[2][i];
        return true;
      }
    }
    return false;
  }
  
  /*AddNum - Dodaje numer do zbioru.
  Zwraca false jezeli zbior jest pelny.
  Sortuje zbior jezeli dodano element oraz przelicza metryki.*/
  bool AddNum(int num) 
  {
    if(current_len < MAX_NUMBERS)
    {
      nums[current_len] = num;
      ++current_len;
      BubbleSort();

      //Liczymy metryki przy dodaniu liczby, aby nie powtarzac obliczen przy GET
      countMedian();      
      countAverage();
      countDeviation();

      //updateDivTag
      //updateNumsTag

      return true;
    }
    else return false;
  }

  //Metryka 1/3 - mediana
  float getMedian() {return median;}
  void countMedian()
  {
    if(current_len%2==0)
      median = float((nums[current_len/2]+nums[current_len/2 + 1])/2);
    else median = nums[current_len-1/2];
  }
  
  
  //Metryka 2/3 - srednia
  float getAverage() {return average;}
  void countAverage()
  {
    int sum = 0;
    
    for(int i=0; i<current_len; i++)
        sum += nums[i];
    average = float(sum/current_len);
  }

  //Metryka 3/3 - odchylenie standardowe
  float getStandardDeviation() {return deviation;}
  void countDeviation()
  {
    if(average!=0)
    {
      float sum=0;
      for(int i=0; i<current_len; i++)
        sum += (nums[i]-average)*(nums[i]-average);
      sum /= (current_len-1);
      deviation = sqrt(sum);
      /*Bardziej wyszukane algorytmy pierwiastkowania if we fancy:
       https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
      */
    }
  }

  void BubbleSort() //mozna zmodyfikowac do Comb sort jezeli bedzie za wolny
  {
    int i,j;
    bool swapped;
    for(i=0; i < current_len-1; ++i)
    {
      /*ostatnie i liczb jest juz na miejscu*/
      for(j=0; j < current_len-i-1; ++j)
      {
        swapped = false;
        if(nums[j] > nums[j+1])
        {
          swap(&nums[j], &nums[j+1]); 
          swapped = true;
        }
      }
      /*jezeli po przejsciu calego podzbioru nic nie zmieniono,
      oznacza to ze calosc jest juz posortowana*/
      if(swapped == false) 
      break; 
    }
  }

  /*Funkcja pomocnicza do sortowania;
  Zamienia dwie liczby pod danymi adresami.*/
  void swap(int *xp, int *yp)  
  {
    int temp = *xp;  
    *xp = *yp;  
    *yp = temp;  
  }

/* Pobieranie zbioru; przepisujemy te wartosci z tablicy, ktore nie sa 'smieciowe':
if(Numbers.current_len == 0) //wyslac 4.08 (Request Entity Incomplete)
else
{
  int result[Numbers.current_len];
  for(int i=0; i < Numbers.current_len; ++i)
  result[i] = Numbers.nums[i];
  //wyslac zawartosc result w wiadomosci 2.05 (content)
}
*/

/* Pobieranie liczb podzielnych przez liczbe z zapytania (arg)
 * Kod przechodzi przez liste dwa razy; raz liczy rozmiar wyniku, raz wpisuje wyniki
 * Dla oszczedzania zasobow korzysta z tego samego licznika (cnt)
int arg = //pobrac z zapytania
int cnt, leng = 0; 
for(; cnt < Numbers.current_len; ++cnt)
if(Numbers.nums[cnt] % arg == 0) ++len;
//Wiadomo juz ile liczb jest podzielnych; mozna stworzyc tablice na wyniki
int result[leng];
for(cnt=0; cnt < Numbers.current_len; ++cnt)
if(Numbers.nums[cnt] % arg == 0) result[cnt] = Numbers.nums[cnt];
 */
  
};
