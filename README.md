# Simple CoAP server
Projekt z przedmiotu OBIR.
# Szybkie prototypowanie na EBSimUnoEth

0. W `run_sim.bat` zmienić: ścieżkę EBSimUnoEth na własną, IP na IP swojej karty sieciowej

Następnie na początku każdej sesji programowania:

1. Zweryfikować kod .ino w programie Arduino
2. Skopiować ścieżkę .ino.hex wypisaną w terminalu w Arduino do `run_sim.bat`

Ścieżka nie powinna zmieniać się dopóki nie zamkniecie Arduino, więc następnie dla każdej zmiany w kodzie wystarczy wykonać następujące kroki:

3. (Opcjonalnie) Zamknąć aktualnie uruchomioną instancję EBSimUnoEth
4. Uruchomić EBSimUnoEth za pomocą `run_sim.bat`

# Dokumentacja

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

