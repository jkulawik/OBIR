# Simple CoAP server
Projekt z przedmiotu OBIR.

Serwer CoAP obsługujący proste działania na zbiorze liczb.
Projekt został napisany do współdziałania z platformą EBSimUnoEth.

# Szybkie prototypowanie na EBSimUnoEth

0. W `run_sim.bat` zmienić: ścieżkę EBSimUnoEth na własną, IP na IP swojej karty sieciowej

Następnie na początku każdej sesji programowania:

1. Zweryfikować kod .ino w programie Arduino
2. Skopiować ścieżkę .ino.hex wypisaną w terminalu w Arduino do `run_sim.bat`

Ścieżka nie powinna zmieniać się dopóki nie zamkniecie Arduino, więc następnie dla każdej zmiany w kodzie wystarczy wykonać następujące kroki:

3. (Opcjonalnie) Zamknąć aktualnie uruchomioną instancję EBSimUnoEth
4. Uruchomić EBSimUnoEth za pomocą `run_sim.bat`

# Dokumentacja

## Rozdzielenie odbioru i nadawania

Kod głównej pętli został dla klarowności podzielony na dwie części.
Jedna z nich zajmuje się jedynie interpretacją odebranej wiadomości, a druga jej obsługą, tzn. wysłaniem odpowiedzi.

W modelu takim obsłużenie opcji nie jest proste; 
Ponieważ jednak w projekcie liczba opcji jest ograniczona, można było pozwolić sobie na przechowywanie wartości opcji w zmiennych.
Podejście takie wymagało jednak monitorowania "statusu" tych opcji;
Dlatego są one inicjowane z charakterystycznymi wartościami lub mają towarzyszące zmienne enum symbolizujące ich status.

Sekcje kodu zostały oznaczone komentarzami z kodem sekcji oraz jej nazwą.
Sekcja 1 to odczyt wiadomości, a sekcja 2 to odpowiedź.
Kody sekcji i podsekcji zawierają końcówki .S(tart) lub .E(nd), które podpowiadają, czy dany komentarz jest początkiem czy końcem sekcji kodu.

## Odnośnie opcji ETag

Dla uproszczenia programu przyjęto pewne założenia na temat opcji ETag.
Ponieważ klient musi zwrócić tag nadany przez serwer, program ma dowolność w formacie tej wartości.
Mówiąc ściślej, może nadawać i sprawdzać z góry ustaloną liczbę bajtów ETag;
Jednak podczas odbierania pakietu jest zmuszony do odnotowania długości opcji ETag, aby odczytać pozostałą część pakietu poprawnie.

W związku z tymi założeniami, program przyjmuje dwubajtowe tagi, w czym:

- Pierwszy bajt symbolizuje zasób, którego aktualność jest walidowana,
- Drugi bajt to losowa wartość, która jest właściwym wyznacznikiem aktualności zasobu.

Jak już wspomniano, założenia te pozwalają na uproszczenie programu poprzez statyczne deklaracje tablic.
Kolejnym uproszczeniem, które można wprowadzić, to odgórne uznanie taga o innej dlugości niż 2 za nieaktualnego.

