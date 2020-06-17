
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

class Numbers
{
  public:
  int nums[MAX_NUMBERS];
  int current_len = 0;

  /*Konstruktor domyslny; jedyne co robi to ust. */
  Numbers() {} 
  
  /*Dodaje numer do zbioru.
  Zwraca false jezeli zbior jest pelny.
  Sortuje zbior jezeli dodano element.*/
  bool AddNum(int num) 
  {
    if(current_len < MAX_NUMBERS)
    {
      nums[current_len] = num;
      ++current_len;
      void BubbleSort();
      return true;
    }
    else return false;
  }

  int getMedian() //Metryka 1/3
  {
    
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
