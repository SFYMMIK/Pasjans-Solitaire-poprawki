#include <bits/stdc++.h>
using namespace std;
enum SymbolKoloru { Kier, Karo, Trefl, Pik };
enum KolorWizualny { Czerwony, Czarny };

struct Karta {
  int wartosc;
  SymbolKoloru kolor;
  bool odkryta;
  Karta(int w, SymbolKoloru k, bool o = false) : wartosc(w), kolor(k), odkryta(o) {}
  KolorWizualny wizualny() const { return (kolor == Kier || kolor == Karo) ? Czerwony : Czarny; }
  string tekst() const {
    if (!odkryta) return "[?]";
    string w[] = {"", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    string s[] = {"♥", "♦", "♣", "♠"};
    return w[wartosc] + s[kolor];
  }
};

struct Stos {
  vector<Karta> k;
  bool pusty() const { return k.empty(); }
  int rozmiar() const { return k.size(); }
  Karta& wierzch() { static Karta fake(0, Kier); return pusty() ? fake : k.back(); }
  void dodaj(const Karta& a) { k.push_back(a); }
  void dodajWiele(const vector<Karta>& v) { k.insert(k.end(), v.begin(), v.end()); }
  void usun() { if (!pusty()) k.pop_back(); }
  void odkryj() { if (!pusty()) k.back().odkryta = true; }
  Karta pobierz() { auto a = wierzch(); usun(); return a; }
  vector<Karta> pobierzOd(int i) { vector<Karta> out(k.begin() + i, k.end()); k.erase(k.begin() + i, k.end()); return out; }
  void wyczysc() { k.clear(); }
  bool moznaNa(const Karta& a) const {
    if (pusty()) return a.wartosc == 13;
    const Karta& top = k.back();
    return top.wartosc == a.wartosc + 1 && top.wizualny() != a.wizualny();
  }
};

struct StanGry {
  vector<Stos> kolumny, koncowe;
  Stos rezerwowy, odrzucone;
};

struct Gra {
  vector<Karta> talia;
  vector<Stos> kolumny = vector<Stos>(7), koncowe = vector<Stos>(4);
  Stos rezerwowy, odrzucone;
  stack<StanGry> historia;
  chrono::high_resolution_clock::time_point start;

  void zapisz() { historia.push({kolumny, koncowe, rezerwowy, odrzucone}); }
  void cofnij() {
    if (historia.empty()) return void(cout << "Brak historii\n");
    auto s = historia.top(); historia.pop();
    kolumny = s.kolumny; koncowe = s.koncowe;
    rezerwowy = s.rezerwowy; odrzucone = s.odrzucone;
    cout << "Cofnięto ruch\n";
  }
  void stworzTalie() {
    for (int i = 0; i < 4; ++i) for (int j = 1; j <= 13; ++j) talia.emplace_back(j, (SymbolKoloru)i);
  }
  void tasuj() {
    mt19937 r(random_device{}());
    shuffle(talia.begin(), talia.end(), r);
  }
  void rozpocznij() {
    start = chrono::high_resolution_clock::now();
    stworzTalie(); tasuj();
    for (int i = 0; i < 7; ++i) {
      for (int j = 0; j <= i; ++j) kolumny[i].dodaj(talia.back()), talia.pop_back();
      kolumny[i].odkryj();
    }
    while (!talia.empty()) rezerwowy.dodaj(talia.back()), talia.pop_back();
  }
  void wyswietl() {
    cout << "\n♥/♦ czerwone, ♣/♠ czarne\n";
    cout << "Rezerwowy: " << (rezerwowy.pusty() ? "[ ]" : "[?]");
    cout << "   Odrzucone: " << (odrzucone.pusty() ? "[ ]" : odrzucone.wierzch().tekst());
    cout << "   Końcowe: ";
    for (auto& s : koncowe) cout << (s.pusty() ? "[ ] " : s.wierzch().tekst() + " ");
    cout << "\n\nKolumny:\n";
    for (int i = 0; i < 7; ++i) {
      cout << "Kolumna " << i + 1 << ": ";
      for (auto& k : kolumny[i].k) cout << k.tekst() << ' ';
      cout << '\n';
    }
  }
  void dobierz() {
    zapisz();
    if (rezerwowy.pusty()) return void(cout << "Pusty stos\n");
    rezerwowy.odkryj();
    odrzucone.dodaj(rezerwowy.pobierz());
  }
  bool wygrana() {
    return all_of(koncowe.begin(), koncowe.end(), [](Stos& s) { return s.rozmiar() == 13; });
  }
  bool przegrana() {
    if (!rezerwowy.pusty() || !odrzucone.pusty()) return false;
    for (auto& kol : kolumny) for (auto& k : kol.k) if (!k.odkryta) return false;
    return true;
  }
};

int main() {
  Gra g; g.rozpocznij();
  while (true) {
    g.wyswietl();
    if (g.wygrana()) {
      cout << "Gratulacje! Wygrałeś!\n";
      break;
    }
    if (g.przegrana()) {
      cout << "Brak możliwych ruchów. Przegrałeś.\n";
      break;
    }
    cout << "\n1. Dobierz | 9. Cofnij | 0. Wyjście\nWybór: ";
    int w; cin >> w;
    if (w == 1) g.dobierz();
    else if (w == 9) g.cofnij();
    else if (w == 0) break;
  }
  auto czas = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - g.start).count();
  cout << "Czas gry: " << czas / 60 << "m " << czas % 60 << "s\n";
}
