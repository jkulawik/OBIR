# Simple CoAP server
Projekt z przedmiotu OBIR.
## Szybkie prototypowanie na EBSimUnoEth

0. W `run_sim.bat` zmienić: ścieżkę EBSimUnoEth na własną, IP na IP swojej karty sieciowej

Następnie na początku każdej sesji programowania:

1. Zweryfikować kod .ino w programie Arduino
2. Skopiować ścieżkę .ino.hex wypisaną w terminalu w Arduino do `run_sim.bat`

Ścieżka nie powinna zmieniać się dopóki nie zamkniecie Arduino, więc następnie dla każdej zmiany w kodzie wystarczy wykonać następujące kroki:

3. (Opcjonalnie) Zamknąć aktualnie uruchomioną instancję EBSimUnoEth
4. Uruchomić EBSimUnoEth za pomocą `run_sim.bat`
