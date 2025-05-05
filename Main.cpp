#include <iostream>     // do wypisywania na ekran
#include <vector>       // do dynamicznych tablic
#include <random>       // do losowania
#include <algorithm>    // do funkcji takich jak shuffle
#include <chrono>       // do liczenia czasu gry
#include <stack>        // do przechowywania historii ruchów

using namespace std;
using namespace std::chrono;

// Enum reprezentujący symbol koloru karty
enum SymbolKoloru { 
  Kier,   // ♥
  Karo,   // ♦
  Trefl,  // ♣
  Pik     // ♠
};

// Enum reprezentujący kolor wizualny (do logiki gry – czerwony lub czarny)
enum KolorWizualny { 
  Czerwony, // Kier i Karo
  Czarny    // Trefl i Pik
};

// Klasa reprezentująca pojedynczą kartę
class Karta {
  private:
    int wartosc;                // Wartość karty (1 = As, 11 = Walet, 12 = Dama, 13 = Król)
    SymbolKoloru kolor;         // Kolor karty (Kier, Karo, Trefl, Pik)
    bool odkryta;               // Czy karta jest odkryta (true) czy zakryta (false)

  public:
    explicit Karta(int wartosc, SymbolKoloru kolor) 
      : wartosc(wartosc), kolor(kolor), odkryta(false) {}

    Karta(int wartosc, SymbolKoloru kolor, bool odkryta)
      : wartosc(wartosc), kolor(kolor), odkryta(odkryta) {}

    int pobierzWartosc() const { return wartosc; }
    SymbolKoloru pobierzKolor() const { return kolor; }
    bool czyOdkryta() const { return odkryta; }
    void odkryj() { odkryta = true; }
    void zakryj() { odkryta = false; }

    KolorWizualny pobierzKolorWizualny() const {
      return (kolor == Kier || kolor == Karo) ? Czerwony : Czarny;
    }

    string jakoTekst() const {
      if (!odkryta) return "[?]";
      string wartosci[] = {"", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
      string symbole[] = {"♥", "♦", "♣", "♠"};
      return wartosci[wartosc] + symbole[static_cast<int>(kolor)];
    }
};

// Klasa reprezentująca stos kart
class Stos {
  private:
    vector<Karta> stos;

  public:
    Karta& zobaczWierzch() {
      if (czyPusty()) {
        static Karta fakeKarta(0, Kier);
        return fakeKarta;
      } else {
        return stos.back();
      }
    }

    Karta pobierzWierzch() {
      if (!stos.empty()) {
        Karta karta = stos.back();
        stos.pop_back();
        return karta;
      } else {
        return Karta(0, Kier); 
      }
    }

    int liczbaKart() const {
      return stos.size();
    }

    void dodajKarte(const Karta& karta) {
      stos.push_back(karta);
    }

    void usunWierzch() {
      stos.pop_back();
    }

    void odkryjWierzch() {
      if (!stos.empty()) {
        stos.back().odkryj();
      }
    }

    bool czyPusty() const {
      return stos.empty();
    }

    const vector<Karta>& pobierzKarty() const {
      return stos;
    }

    void wyczysc() {
      stos.clear();
    }

    bool moznaDodacKarte(const Karta& karta) const {
      if (czyPusty()) 
        return karta.pobierzWartosc() == 13;

      const Karta& wierzch = stos.back();
      return wierzch.pobierzWartosc() == karta.pobierzWartosc() + 1 && 
             wierzch.pobierzKolorWizualny() != karta.pobierzKolorWizualny();
    }

    vector<Karta> usunKartyOd(int indeks) {
      if (indeks < 0 || indeks >= (int)stos.size())
        return {};

      vector<Karta> usuniete(stos.begin() + indeks, stos.end());
      stos.erase(stos.begin() + indeks, stos.end());
      return usuniete;
    }

    void dodajKarty(const vector<Karta>& noweKarty) {
      stos.insert(stos.end(), noweKarty.begin(), noweKarty.end());
    }

    bool czyMoznaPolozycNaStosKoncowy(const Karta& nowa, const Stos& stos) {
      if (stos.czyPusty()) {
        return nowa.pobierzWartosc() == 1;
      }

      const Karta& wierzch = stos.pobierzKarty().back();
      return nowa.pobierzKolor() == wierzch.pobierzKolor() &&
             nowa.pobierzWartosc() == wierzch.pobierzWartosc() + 1;
    }
};

// Struktura przechowująca stan gry (do cofania ruchów)
struct StanGry {
  vector<Stos> kolumny;
  vector<Stos> stosyKoncowe;
  Stos stosRezerwowy;
  Stos stosOdrzuconych;
};

class Gra {
  private:     
    vector<Karta> talia;
    Stos stosRezerwowy;
    Stos stosOdrzuconych;

  public:
    vector<Stos> kolumny;
    vector<Stos> stosyKoncowe;
    time_point<high_resolution_clock> start_czas;
    stack<StanGry> historia;

    void zapiszStanGry() {
      StanGry stan = {kolumny, stosyKoncowe, stosRezerwowy, stosOdrzuconych};
      historia.push(stan);
    }

    void cofnijRuch() {
      if (historia.empty()) {
        cout << "Nie można cofnąć ruchu – brak historii.\n";
        return;
      }

      StanGry stan = historia.top();
      historia.pop();

      kolumny = stan.kolumny;
      stosyKoncowe = stan.stosyKoncowe;
      stosRezerwowy = stan.stosRezerwowy;
      stosOdrzuconych = stan.stosOdrzuconych;

      cout << "Cofnięto ostatni ruch.\n";
    }

    void stworzTalie() {
      for (int i = 0; i < 4; ++i) {
        SymbolKoloru kolor = static_cast<SymbolKoloru>(i);
        for (int j = 0; j < 13; ++j) {
          Karta karta(j + 1, kolor);
          talia.push_back(karta);
        }
      }
    }

    void rozpocznijGre() {
      start_czas = high_resolution_clock::now();
      kolumny = vector<Stos>(7);
      stosyKoncowe = vector<Stos>(4);

      stworzTalie();
      tasowanie_talii();

      for (int i = 0; i < 7; ++i) {
        for (int j = 0; j <= i; ++j) {
          kolumny.at(i).dodajKarte(talia.back());
          talia.pop_back();
        }
        kolumny[i].odkryjWierzch();
      }

      while (!talia.empty()) {
        stosRezerwowy.dodajKarte(talia.back());
        talia.pop_back();
      }
    }

    void tasowanie_talii() {
      random_device dev;
      mt19937 rng(dev());
      shuffle(talia.begin(), talia.end(), rng);
    }

    void przeniesKarte(Stos& zrodlo, Stos& cel, bool doStosuKoncowego = false) {
      if (zrodlo.czyPusty()) {
        cout << "Stos źródłowy jest pusty!" << endl;
        return;
      }

      Karta karta = zrodlo.zobaczWierzch();

      if (doStosuKoncowego) {
        if (cel.czyPusty()) {
          if (karta.pobierzWartosc() == 1) {
            zrodlo.usunWierzch();
            zrodlo.odkryjWierzch(); // Odkrywamy wierzchnią kartę w stosie źródłowym
            cel.dodajKarte(karta);
          } else {
            cout << "Na pusty stos końcowy można położyć tylko Asa!" << endl;
          }
        } else {
          Karta wierzchKoncowego = cel.zobaczWierzch();
          if (wierzchKoncowego.pobierzKolor() == karta.pobierzKolor() &&
              karta.pobierzWartosc() == wierzchKoncowego.pobierzWartosc() + 1) {
            zrodlo.usunWierzch();
            zrodlo.odkryjWierzch(); // Odkrywamy wierzchnią kartę w stosie źródłowym
            cel.dodajKarte(karta);
          } else {
            cout << "Karta nie pasuje do stosu końcowego!" << endl;
          }
        }
      } else {
        if (cel.czyPusty()) {
          if (karta.pobierzWartosc() == 13) {
            zrodlo.usunWierzch();
            cel.dodajKarte(karta);
          } else {
            cout << "Na pustą kolumnę można przenieść tylko Króla!" << endl;
          }
        } else {
          Karta celowa = cel.zobaczWierzch();
          if (celowa.pobierzWartosc() == karta.pobierzWartosc() + 1 &&
              celowa.pobierzKolorWizualny() != karta.pobierzKolorWizualny()) {
            zrodlo.usunWierzch();
            if (!zrodlo.czyPusty() && !zrodlo.zobaczWierzch().czyOdkryta()) {
              zrodlo.zobaczWierzch().odkryj();
            }
            cel.dodajKarte(karta);
          } else {
            cout << "Karta nie może zostać przeniesiona na ten stos!" << endl;
          }
        }
      }
    }
    
    // Dobiera kartę ze stosu rezerwowego
    void dobierzKarte(){
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu

      
      if (!(stosRezerwowy.czyPusty())){
        stosRezerwowy.odkryjWierzch();
        stosOdrzuconych.dodajKarte(stosRezerwowy.zobaczWierzch());
        stosRezerwowy.usunWierzch();
      } else {
        cout << "Stos jest pusty!" << endl;
      }
    }


    // Sprawdza, czy wszystkie stosy końcowe są pełne
    bool sprawdzWygrana(){
      int pelneStosy = 0;
      for (const auto& stos : stosyKoncowe) {
        if (stos.liczbaKart() == 13)
          ++pelneStosy;
      }


      if (pelneStosy == 4){
        cout << "Gratulacje! Wygrałeś!" << endl;
        return false;
      }

      return true;
    }


    // Wyświetla stan gry w konsoli
    void wyswietlStanGry(){
      int maxWysokosc = 0;
      for (const auto& kolumna : kolumny) {
        if (kolumna.liczbaKart() > maxWysokosc)
          maxWysokosc = kolumny[0].liczbaKart();
      }


      cout << endl;
      cout << "Kier(♥) i Karo(♦) - czerwony" << endl;
      cout << "Pik(♠) oraz Trefl(♣) - czarny\n" << endl;


      cout << "Stos rezerwowy: ";
      if (stosRezerwowy.czyPusty()) cout << "[ ]";
      else cout << "[?]";


      cout << "   Stos odrzuconych: ";
      if (stosOdrzuconych.czyPusty()) cout << "[ ]";
      else cout << stosOdrzuconych.zobaczWierzch().jakoTekst();


      cout << "   Stosy końcowe: ";
      for (int i = 0; i < 4; ++i) {
        if (stosyKoncowe[i].czyPusty()) cout << "[ ] ";
        else cout << stosyKoncowe[i].zobaczWierzch().jakoTekst() << " ";
      }
      cout << "\n";


      cout << "\nKolumny:\n";
      for (int i = 0; i < 7; ++i) {
        cout << "Kolumna " << i + 1 << ": ";
        for (const auto& karta : kolumny[i].pobierzKarty()) {
          cout << karta.jakoTekst() << " ";
        }
        cout << "\n";
      }
    }


    // Restartuje grę – czyści wszystkie dane i uruchamia od nowa
    void zrestartujGre() {
      talia.clear();
      stosRezerwowy.wyczysc();
      stosOdrzuconych.wyczysc();
      for (auto& kol : kolumny) kol.wyczysc();
      for (auto& koncowy : stosyKoncowe) koncowy.wyczysc();
      

      // Czyszczenie historii ruchów
      while (!historia.empty()) {
        historia.pop();
      }


      rozpocznijGre();
      cout << "Gra została zrestartowana.\n";
    }


    // Umożliwia przeniesienie karty między kolumnami po indeksach
    void przeniesKarteZKolumnyDoKolumny(int z, int do_) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu
      if (z < 0 || z > 6 || do_ < 0 || do_ > 7) {
        cout << "Nieprawidłowe kolumny.\n";
        return;
      }


     // Użycie przeniesKarte do przeniesienia karty
      przeniesKarte(kolumny[z], kolumny[do_]);


      if (!kolumny[z].czyPusty()) {
        kolumny[z].odkryjWierzch();
      }      
    }


    // Przenosi kartę do stosu końcowego, jeśli ruch jest poprawny
    void przeniesDoStosuKoncowego(Karta& karta, Stos& kolumna, Stos& cel) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu
      przeniesKarte(kolumna, cel, true); // Użycie flagi doStosuKoncowego
    }


    // Dobiera kartę ze stosu rezerwowego (alternatywna metoda)
    void dobierzZeStosuRezerwowego() {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu


      if (stosRezerwowy.czyPusty()) {
        cout << "Stos rezerwowy jest pusty. Przetasuj odrzucone, aby kontynuować.\n";
        return;
      }

      Karta karta = stosRezerwowy.zobaczWierzch();
      karta.odkryj();
      stosOdrzuconych.dodajKarte(karta);
      stosRezerwowy.usunWierzch();
    }


    // Przenosi wszystkie karty ze stosu odrzuconego z powrotem do rezerwowego po przetasowaniu
    void przetasujOdrzuconeDoRezerwowego() {
      if (stosOdrzuconych.czyPusty()) {
        cout << "Brak kart do przetasowania.\n";
        return;
      }


      vector<Karta> temp;
      while (!stosOdrzuconych.czyPusty()) {
        Karta karta = stosOdrzuconych.zobaczWierzch();
        karta.odkryj();
        temp.push_back(karta);
        stosOdrzuconych.usunWierzch();
      }

      random_device dev;
      mt19937 rng(dev());
      shuffle(temp.begin(), temp.end(), rng);

      for (Karta& k : temp) {
        stosRezerwowy.dodajKarte(k);
      }

      cout << "Karty zostały przetasowane i wróciły do stosu rezerwowego.\n";
    }


    // Przenosi kartę ze stosu odrzuconego do wskazanej kolumny (jeśli to możliwe)
    void przeniesZeStosuOdrzuconegoDoKolumny(int indeksKolumny) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu


      if (stosOdrzuconych.czyPusty()) {
        cout << "Stos odrzucony jest pusty.\n";
        return;
      }


      przeniesKarte(stosOdrzuconych, kolumny[indeksKolumny]);
    }


    // Przenosi kartę ze stosu odrzuconego do stosu końcowego (jeśli to możliwe)
    void przeniesZeStosuOdrzuconegoDoStosuKoncowego(int indeksStosu) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu


      if (stosOdrzuconych.czyPusty()) {
        cout << "Stos odrzucony jest pusty.\n";
        return;
      }


      if (indeksStosu < 0 || indeksStosu >= 4) {
        cout << "Nieprawidłowy indeks stosu końcowego.\n";
        return;
      }


      przeniesKarte(stosOdrzuconych, stosyKoncowe[indeksStosu], true);
      
    }
   


    bool czyMoznaPolozycNa(const Karta& gorna, const Karta& dolna) {
      return gorna.pobierzKolorWizualny() != dolna.pobierzKolorWizualny() &&
        gorna.pobierzWartosc() == dolna.pobierzWartosc() + 1;
    }


    void przeniesKolumny(int indeksZ, int indeksDo, int indeksKarty) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu


      // Sprawdzamy, czy indeksy kolumn są w zakresie
      if (indeksZ < 0 || indeksDo < 0 || indeksZ >= kolumny.size() || indeksDo >= kolumny.size()) {
        cout << "Nieprawidłowy numer kolumny. Wybierz poprawne kolumny.\n";
        return;
      }

      // Pobieramy referencje do kolumn źródłowej i docelowej
      Stos& zrodlowa = kolumny[indeksZ];
      Stos& docelowa = kolumny[indeksDo];
      const vector<Karta>& kartyZRodzla = zrodlowa.pobierzKarty();

      // Sprawdzamy, czy indeks karty jest poprawny
      if (indeksKarty < 0 || indeksKarty >= kartyZRodzla.size()) {
        cout << "Nieprawidłowy indeks karty. Wybierz poprawny indeks.\n";
        return;
      }

      // Pobieramy kartę startową (pierwszą kartę do przeniesienia)
      const Karta& kartaStartowa = kartyZRodzla[indeksKarty];

      // Sprawdzamy, czy karta startowa jest odkryta
      if (!kartaStartowa.czyOdkryta()) {
        cout << "Nie można przenieść zakrytej karty. Odkryj kartę przed przeniesieniem.\n";
        return;
      }

      // Wydzielamy karty do przeniesienia z kolumny źródłowej
      vector<Karta> doPrzeniesienia = zrodlowa.usunKartyOd(indeksKarty);

      // Sprawdzamy, czy można położyć karty na kolumnie docelowej
      if (!docelowa.czyPusty()) {
        const Karta& kartaDocelowa = docelowa.zobaczWierzch();
        if (!czyMoznaPolozycNa(kartaDocelowa, doPrzeniesienia.front())) {
          // Jeśli ruch jest nieprawidłowy, cofamy operację
          cout << "Nie można przenieść kart. Sprawdź zasady przenoszenia.\n";
          zrodlowa.dodajKarty(doPrzeniesienia);
          return;
        }
      } else {
        // Jeśli kolumna docelowa jest pusta, tylko Król może być przeniesiony
        if (doPrzeniesienia.front().pobierzWartosc() != 13) {
          cout << "Na pustą kolumnę można przenieść tylko Króla.\n";
          zrodlowa.dodajKarty(doPrzeniesienia);
          return;
        }
      }

      // Przenosimy karty do kolumny docelowej
      docelowa.dodajKarty(doPrzeniesienia);

      // Odkrywamy wierzchnią kartę w kolumnie źródłowej, jeśli są jeszcze karty
      if (!zrodlowa.czyPusty()) {
        zrodlowa.odkryjWierzch();
      }

      // Informujemy użytkownika o wykonanym ruchu
      cout << "Przeniesiono karty z kolumny " << indeksZ + 1 << " do kolumny " << indeksDo + 1 << ".\n";
    }
    
    
    void automatyczniePrzeniesDoStosowKoncowych() {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu
      bool przeniesionoWTejIteracji;
      bool przeniesionoWCalosci = false; // Śledzi, czy jakakolwiek karta została przeniesiona
    
      do {
        przeniesionoWTejIteracji = false;

    
        for (int i = 0; i < 7; ++i) {
          if (!kolumny[i].czyPusty()) {
            Karta karta = kolumny[i].zobaczWierzch();

    
            for (int j = 0; j < 4; ++j) {
              if (stosyKoncowe[j].czyPusty()) {
                if (karta.pobierzWartosc() == 1) { // As
                  przeniesDoStosuKoncowego(karta, kolumny[i], stosyKoncowe[j]);
                  przeniesionoWTejIteracji = true;
                  przeniesionoWCalosci = true;
                  break; 
                }
              } else {
                Karta& naWierzchu = stosyKoncowe[j].zobaczWierzch();
                if (naWierzchu.pobierzKolor() == karta.pobierzKolor() &&
                    karta.pobierzWartosc() == naWierzchu.pobierzWartosc() + 1) {
                  przeniesDoStosuKoncowego(karta, kolumny[i], stosyKoncowe[j]);
                  przeniesionoWTejIteracji = true;
                  przeniesionoWCalosci = true;
                  break;
                }
              }
            }
          }
        }
      } while (przeniesionoWTejIteracji);
    

      if (przeniesionoWCalosci) {
        cout << "Przeniesiono karty do stosów końcowych.\n";
      } else {
        cout << "Nie przeniesiono żadnej karty.\n";
      }
    }


    void przeniesZeStosuKoncowegoDoKolumny(int indeksStosu, int indeksKolumny) {
      zapiszStanGry(); // Zapisujemy stan gry przed wykonaniem ruchu
    
      // Sprawdzamy, czy indeksy są poprawne
      if (indeksStosu < 0 || indeksStosu >= 4) {
        cout << "Nieprawidłowy indeks stosu końcowego.\n";
        return;
      }
      if (indeksKolumny < 0 || indeksKolumny >= 7) {
        cout << "Nieprawidłowy indeks kolumny.\n";
        return;
      }
    
      // Użycie funkcji przeniesKarte
      przeniesKarte(stosyKoncowe[indeksStosu], kolumny[indeksKolumny]);
    }


    bool czyMoznaAutomatyczniePrzenosic() {
      // Sprawdzenie, czy stos odrzuconych i stos rezerwowy są puste
      if (!stosOdrzuconych.czyPusty() || !stosRezerwowy.czyPusty()) {
        return false;
      }
    
      // Sprawdzenie, czy wszystkie karty w kolumnach są odkryte
      for (const auto& kolumna : kolumny) {
        for (const auto& karta : kolumna.pobierzKarty()) {
          if (!karta.czyOdkryta()) {
            return false;
          }
        }
      }

    
      return true;
    }
};


int main() {
  Gra gra; // Tworzymy obiekt gry
  gra.rozpocznijGre(); // Rozpoczynamy nową grę

  while (gra.sprawdzWygrana()) {
    gra.wyswietlStanGry(); // Wyświetlamy aktualny stan gry


    // Wyświetlenie menu dla użytkownika
    cout << "\nWybierz akcję:\n";
    cout << "1. Dobierz kartę ze stosu rezerwowego\n";
    cout << "2. Przenieś kartę między kolumnami\n";
    cout << "3. Przenieś wiele kart między kolumnami\n";
    cout << "4. Przenieś kartę do stosu końcowego\n";
    cout << "5. Przenieś kartę ze stosu odrzuconych do kolumny\n";
    cout << "6. Przenieś kartę ze stosu odrzuconego do stosu końcowego\n";
    cout << "7. Przetasuj odrzucone karty do spowrotem do rezerwowego stosu\n";
    cout << "8. Przenieś kartę ze stosu końcowego do kolumny\n";
    cout << "9. Cofnij ostatni ruch\n";
    cout << "10. Zrestartuj grę\n";
    cout << "0. Zakończ grę\n";
    cout << "Twój wybór:\n";


    int wybor;


    // Pobieranie wyboru użytkownika z walidacją
    while (!(cin >> wybor)) {  
      cout << "Błąd! Podaj numer operacji, którą chciałbyś wykonać:\n";
      cin.clear();  
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }


    // Obsługa wyboru użytkownika
    switch (wybor) {
      case 1:
        gra.dobierzKarte(); // Dobranie karty ze stosu rezerwowego
        break;

      case 2: {
        int z, do_;
        // Pobranie numeru kolumny źródłowej
        cout << "Podaj numer kolumny źródłowej (1-7): ";
        while (!(cin >> z) || z < 1 || z > 7) {  
          cout << "Błąd! Podaj numer kolumny źródłowej (1-7): ";
          cin.clear();  
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Pobranie numeru kolumny docelowej
        cout << "Podaj numer kolumny docelowej (1-7): ";
        while (!(cin >> do_) || do_ < 1 || do_ > 7) {  
          cout << "Błąd! Podaj numer kolumny docelowej (1-7): ";
          cin.clear();  
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        gra.przeniesKarteZKolumnyDoKolumny(z - 1, do_ - 1); // Przeniesienie karty
        break;
      }

      case 3: { // Przeniesienie wielu kart między kolumnami
        int zrodlo, cel, indeks;
        // Pobranie numeru kolumny źródłowej
        cout << "Podaj numer kolumny źródłowej (1-7): ";
        while (!(cin >> zrodlo) || zrodlo < 1 || zrodlo > 7) {
          cout << "Błąd! Podaj numer kolumny źródłowej (1-7): ";
          cin.clear();
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Pobranie numeru kolumny docelowej
        cout << "Podaj numer kolumny docelowej (1-7): ";
        while (!(cin >> cel) || cel < 1 || cel > 7) {
          cout << "Błąd! Podaj numer kolumny docelowej (1-7): ";
          cin.clear();
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Pobranie numeru karty w kolumnie źródłowej
        cout << "Podaj numer karty w kolumnie źródłowej (1 = Pierwsza karta od lewej strony kolumny): ";
        while (!(cin >> indeks) || indeks < 1) {
          cout << "Błąd! Wprowadź poprawny numer (minimum 1): ";
          cin.clear();
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        gra.przeniesKolumny(zrodlo - 1, cel - 1, indeks - 1); // Przeniesienie kart
        break;
      }

      case 4: {
        int kolumna, indeksStosu;
        // Pobranie numeru kolumny źródłowej
        cout << "Podaj numer kolumny źródłowej (1-7): ";
        while (!(cin >> kolumna) || kolumna < 1 || kolumna > 7) {  
          cout << "Błąd! Podaj numer kolumny źródłowej (1-7):";
          cin.clear();  
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Pobranie numeru stosu końcowego
        cout << "Podaj numer stosu końcowego (1-4): ";
        while (!(cin >> indeksStosu )|| indeksStosu < 1 || indeksStosu > 4) {  
          cout << "Błąd! Podaj numer stosu końcowego (1-4): ";
          cin.clear();  
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Przeniesienie karty do stosu końcowego
        if (!gra.kolumny[kolumna - 1].czyPusty()) {
          Karta karta = gra.kolumny[kolumna - 1].zobaczWierzch();
          gra.przeniesDoStosuKoncowego(karta, gra.kolumny[kolumna - 1], gra.stosyKoncowe[indeksStosu - 1]);
        } else {
          cout << "Kolumna jest pusta, nie można przenieść karty.\n";
        }
        break;
      }

      case 5: {
        int indeksKolumny;
        // Pobranie numeru kolumny docelowej
        cout << "Podaj numer kolumny, do której chcesz przenieść kartę z odrzuconych: ";
        while (!(cin >> indeksKolumny )|| indeksKolumny< 1 || indeksKolumny > 7) {  
          cout << "Błąd! Podaj numer kolumny, do której chcesz przenieść kartę z odrzuconych: ";
          cin.clear();  
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        gra.przeniesZeStosuOdrzuconegoDoKolumny(indeksKolumny - 1); // Przeniesienie karty
        break;
      }

      case 6: {
        int indeksStosu;
        // Pobranie numeru stosu końcowego i walidacja wejścia
        cout << "Podaj numer stosu końcowego, do którego chcesz przenieść kartę z odrzuconych: ";
        while (!(cin >> indeksStosu )|| indeksStosu < 1 || indeksStosu > 4) {  
            cout << "Błąd! Wprowadź poprawną liczbę: ";
            cin.clear();  
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        // Przeniesienie karty ze stosu odrzuconego do stosu końcowego
        gra.przeniesZeStosuOdrzuconegoDoStosuKoncowego(indeksStosu - 1); 
        break;
      }

      case 7:
        // Przetasowanie kart ze stosu odrzuconego do stosu rezerwowego
        gra.przetasujOdrzuconeDoRezerwowego(); 
        break;

      case 8:
        int indeksStosu, indeksKolumny;
        // Pobranie numeru stosu końcowego
        cout << "Podaj numer stosu końcowego (1-4): ";
        while (!(cin >> indeksStosu) || indeksStosu < 1 || indeksStosu > 4) {
          cout << "Błąd! Podaj numer stosu końcowego (1-4): ";
          cin.clear();
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    
        // Pobranie numeru kolumny docelowej
        cout << "Podaj numer kolumny docelowej (1-7): ";
        while (!(cin >> indeksKolumny) || indeksKolumny < 1 || indeksKolumny > 7) {
          cout << "Błąd! Podaj numer kolumny docelowej (1-7): ";
          cin.clear();
          cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    
        // Wywołanie funkcji przenoszenia
        gra.przeniesZeStosuKoncowegoDoKolumny(indeksStosu - 1, indeksKolumny - 1);
        break;

      case 9:
        // Cofnięcie ostatniego ruchu
        gra.cofnijRuch();
        break;

      case 10:
        // Restart gry – resetuje wszystkie dane i rozpoczyna nową grę
        gra.zrestartujGre(); 
        break;

      case 0:
        // Wyjście z gry
        cout << "Do widzenia!\n";
        return 0; 

      default:
        // Obsługa nieprawidłowego wyboru
        cout << "Nieprawidłowy wybór, spróbuj ponownie.\n";
    }

    // Automatyczne przenoszenie kart do stosów końcowych
    if (gra.czyMoznaAutomatyczniePrzenosic()) {
      gra.automatyczniePrzeniesDoStosowKoncowych();
    }
  }

  
  auto koniec_czas = high_resolution_clock::now();
  auto czas_trwania = duration_cast<seconds>(koniec_czas - gra.start_czas);
  

  int minuty = czas_trwania.count() / 60;
  int sekundy = czas_trwania.count() % 60;


  std::cout << "Wygrałeś w " << minuty << " minut i " << sekundy << " sekund!\n";

  return 0;  
}
