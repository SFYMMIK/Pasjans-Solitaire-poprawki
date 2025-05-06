#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

enum Kolor { Kier, Karo, Trefl, Pik };
string symbolKoloru[] = {"♥", "♦", "♣", "♠"};

struct Karta {
    int wartosc;
    Kolor kolor;
    bool odkryta;

    Karta(int w, Kolor k, bool o = false) : wartosc(w), kolor(k), odkryta(o) {}

    string tekst() const {
        if (!odkryta) return "[?]";
        string w[] = {"", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
        return w[wartosc] + symbolKoloru[kolor];
    }
};

using Stos = vector<Karta>;

vector<Karta> stworzTalie() {
    vector<Karta> talia;
    for (int k = 0; k < 4; ++k)
        for (int w = 1; w <= 13; ++w)
            talia.emplace_back(w, static_cast<Kolor>(k));
    shuffle(talia.begin(), talia.end(), mt19937(random_device{}()));
    return talia;
}

void pokazKolumny(const vector<Stos>& kolumny) {
    cout << "\nKolumny:\n";
    for (size_t i = 0; i < kolumny.size(); ++i) {
        cout << i+1 << ": ";
        for (const auto& k : kolumny[i])
            cout << k.tekst() << " ";
        cout << "\n";
    }
}

int main() {
    auto talia = stworzTalie();
    vector<Stos> kolumny(7);

    // Rozdaj karty do kolumn
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j <= i; ++j) {
            kolumny[i].push_back(talia.back());
            talia.pop_back();
        }
    for (auto& k : kolumny)
        k.back().odkryta = true;

    // Główna pętla
    while (true) {
        pokazKolumny(kolumny);
        cout << "\nPodaj numer kolumny źródłowej i docelowej (0 aby zakończyć): ";
        int z, d;
        cin >> z >> d;
        if (z == 0 || d == 0) break;
        if (--z < 0 || z > 6 || --d < 0 || d > 6 || kolumny[z].empty()) continue;

        Karta karta = kolumny[z].back();
        if (kolumny[d].empty() && karta.wartosc == 13) {
            kolumny[d].push_back(karta);
            kolumny[z].pop_back();
        } else if (!kolumny[d].empty()) {
            Karta cel = kolumny[d].back();
            if ((karta.kolor == Kier || karta.kolor == Karo) != (cel.kolor == Kier || cel.kolor == Karo)
                && karta.wartosc + 1 == cel.wartosc) {
                kolumny[d].push_back(karta);
                kolumny[z].pop_back();
            }
        }

        if (!kolumny[z].empty())
            kolumny[z].back().odkryta = true;
    }

    cout << "Koniec gry.\n";
    return 0;
}