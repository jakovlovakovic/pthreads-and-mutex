#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <pthread.h>
#include <vector>

using namespace std;

#define BRCITACA 4
#define BRPISACA 10
#define BRBRISACA 2

pthread_mutex_t monitor;
pthread_cond_t citac_cond;
pthread_cond_t pisac_cond;
pthread_cond_t brisac_cond;
int br_citaca = 0, br_pisaca = 0, br_brisaca = 0;
int br_citaca_ceka = 0, br_citaca_cita = 0;
int br_pisaca_ceka = 0, br_pisaca_pise = 0;
int br_brisaca_ceka = 0, br_brisaca_brise = 0;

// funkcionalnost liste
template<typename T>
class ListElement {
    public:
        T value;
        ListElement<T>* next;

        ListElement(T value, ListElement<T>* next) { this->value = value; this->next = next; };
        ~ListElement() {};
};

template<typename T>
class List {
    private:
        ListElement<T>* head;
        int size;

    public:
        List() {
            this->head = nullptr; 
        };
        ~List() {
            ListElement<T>* temp = head;
            if(temp == nullptr) return;
            while(temp != nullptr) {
                head = temp;
                temp = temp->next;
                delete head;
            }
        };

        void add(T value) {
            ListElement<T>* temp_value = new ListElement<T>(value, nullptr);
            if(this->head == nullptr) {
                this->head = temp_value;
                size = size + 1;
                return;
            }
            ListElement<T>* temp;
            for(temp = this->head; temp->next != nullptr; temp = temp->next) { }
            temp->next = temp_value;
            size = size + 1;
        }

        int get_size() {
            return this->size;
        }

        int get_at_index(int index) {
            ListElement<T>* temp = this->head;
            T return_value;
            if(this->head != nullptr) {
                return_value = this->head->value;
            }
            for(int i = 0; i < index; i++) {
                temp = temp->next;
                return_value = temp->value;
            }
            return return_value;
        }

        void delete_at_index(int index) {
            ListElement<T>* temp = this->head;
            T return_value;
            for(int i = 0; i < index; i++) {
                if(i + 1 == index) {
                    ListElement<T>* delete_this = temp->next;
                    temp->next = temp->next->next;
                    this->size = this->size - 1;
                    delete delete_this;
                    break;
                }
                temp = temp->next;
            }
        }

        void iterate() { 
            ListElement<T>* temp = head;
            if(temp == nullptr) return;
            while(temp != nullptr) {
                cout << temp->value << " ";
                temp = temp->next;
            }
        }
};

// ova lista se koristi za dretve
List<int>* list;

// funkcija za generiranje random broja iz intervala
int generiraj_rand_br(int a, int b) {
    //srand(time(0));
    return rand() % (b - a + 1) + a;
}

// funkcija za generiranje random znakova
char generiraj_rand_character() {
    int br = generiraj_rand_br(0, 51);

    if(br < 26) {
        return 'a' + br;
    }
    else {
        return 'A' + (br - 26);
    }
}

// dretva citac
void* dretva_citac(void*) {
    int br_dretve = br_citaca;
    br_citaca = br_citaca + 1;

    do {
        // dobi index
        int index = generiraj_rand_br(0, list->get_size() - 1);
    
        // zakljucaj monitor, kad zavrsis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        cout << "Citac[" << br_dretve << "] zeli citati element " << index << " liste" << endl;
        br_citaca_ceka = br_citaca_ceka + 1;
        while(br_brisaca_brise + br_brisaca_ceka >= 1) {
            pthread_cond_wait(&citac_cond, &monitor);
        }
        br_citaca_cita = br_citaca_cita + 1;
        br_citaca_ceka = br_citaca_ceka - 1;
        if(index < list->get_size()) {
            int element = list->get_at_index(index);
            cout << "Citac[" << br_dretve << "] cita element " << index << " liste (vrijednost: " << element << ")" << endl;
        }
        else {
            cout << "CITAC[" << br_dretve << "]: TAJ INDEX VISE NE POSTOJI U LISTI (drugi brisac ga je obrisao)!" << endl;
        }
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(1, 6));

        // zakljucaj monitor kad zavrisis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        br_citaca_cita = br_citaca_cita - 1;
        if(br_citaca_cita == 0 && br_brisaca_ceka >= 1) {
            pthread_cond_signal(&brisac_cond);
        }
        cout << "Citac[" << br_dretve << "] vise ne koristi listu" << endl;
        cout << "Lista: "; list->iterate(); cout << endl;
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(1, 6));
    } while(true);
}

// dretva pisac
void* dretva_pisac(void*) {
    int br_dretve = br_pisaca;
    br_pisaca = br_pisaca + 1;

    do {
        // dobi vrijednost
        int value = generiraj_rand_br(0, 100);
    
        // zakljucaj monitor, kad zavrsis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        cout << "Pisac[" << br_dretve << "] zeli pisati vrijednost " << value << " u listu" << endl;
        br_pisaca_ceka = br_pisaca_ceka + 1;
        while(br_brisaca_brise >= 1 || br_pisaca_pise >= 1) {
            pthread_cond_wait(&pisac_cond, &monitor);
        }
        br_pisaca_pise = br_pisaca_pise + 1;
        br_pisaca_ceka = br_pisaca_ceka - 1;
        cout << "Pisac[" << br_dretve << "] zapocinje pisanje elementa " << value << " na kraj liste" << endl;
        pthread_cond_broadcast(&citac_cond);
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(2, 4));

        // zakljucaj monitor kad zavrisis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        br_pisaca_pise = br_pisaca_pise - 1;
        list->add(value);
        cout << "Pisac[" << br_dretve << "] dodao element " << value << " na kraj liste" << endl;
        cout << "Lista: "; list->iterate(); cout << endl;
        if(br_pisaca_pise == 0 && br_citaca_cita == 0 && br_brisaca_ceka >= 1) {
            pthread_cond_signal(&brisac_cond);
        }
        if(br_pisaca_pise == 0 && br_brisaca_ceka == 0) {
            pthread_cond_signal(&pisac_cond);
        }
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(2, 4));
    } while(true);
}

// dretva brisac
void* dretva_brisac(void*) {
    int br_dretve = br_brisaca;
    br_brisaca = br_brisaca + 1;

    do {
        // dobi index
        int index = generiraj_rand_br(0, list->get_size() - 1);
    
        // zakljucaj monitor, kad zavrsis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        cout << "Brisac[" << br_dretve << "] zeli brisati element " << index << " iz liste" << endl;
        br_brisaca_ceka = br_brisaca_ceka + 1;
        while(br_citaca_cita >= 1 || br_pisaca_pise >= 1 || list->get_size() < 1 || br_brisaca_brise >= 1) {
            pthread_cond_wait(&brisac_cond, &monitor);
        }
        br_brisaca_brise = br_brisaca_brise + 1;
        br_brisaca_ceka = br_brisaca_ceka - 1;
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(5, 10));

        // zakljucaj monitor kad zavrisis sa svime izadji iz monitora
        pthread_mutex_lock(&monitor);
        br_brisaca_brise = br_brisaca_brise - 1;
        if(index < list->get_size()) {
            int element = list->get_at_index(index);
            cout << "Brisac[" << br_dretve << "] zapocinje brisanje elementa " << index << " liste (vrijednost: " << element << ")" << endl;
            list->delete_at_index(index);
            cout << "Brisac[" << br_dretve << "] izbrisao element " << index << " iz liste (vrijednost: " << element << ")" << endl;
            cout << "Lista: "; list->iterate(); cout << endl;
        }
        else {
            cout << "Brisac[" << br_dretve << "]: TAJ INDEX VISE NE POSTOJI U LISTI (drugi brisac ga je obrisao)!" << endl;
        }
        if(br_brisaca_brise == 0 && br_citaca_ceka >= 1) {
            pthread_cond_signal(&citac_cond);
        }
        if(br_brisaca_brise == 0 && br_pisaca_ceka >= 1) {
            pthread_cond_signal(&pisac_cond);
        }
        pthread_mutex_unlock(&monitor);

        // odspavaj nasumicno vrijeme
        sleep(generiraj_rand_br(10, 20));
    } while(true);
}

int main(void) {
    srand(time(0));
    // lista koja ce se koristiti
    list = new List<int>();
    
    // inicijalizacija monitora i uvjeta
    pthread_mutex_init(&monitor, NULL);
    pthread_cond_init(&citac_cond, NULL);
    pthread_cond_init(&pisac_cond, NULL);
    pthread_cond_init(&brisac_cond, NULL);

    vector<pthread_t> pisaci(BRPISACA);
    for(int i = 0; i < BRPISACA; i++) {
        sleep(1);
        pthread_create(&pisaci[i], NULL, dretva_pisac, NULL);
    }
    // DODANO RADI SIMULACIJE
    sleep(5);
    vector<pthread_t> citaci(BRCITACA);
    for(int i = 0; i < BRCITACA; i++) {
        sleep(1);
        pthread_create(&citaci[i], NULL, dretva_citac, NULL);
    }
    sleep(5);
    vector<pthread_t> brisaci(BRBRISACA);
    for(int i = 0; i < BRBRISACA; i++) {
        sleep(1);
        pthread_create(&brisaci[i], NULL, dretva_brisac, NULL);
    } 
    for(int i = 0; i < BRPISACA; i++) {
        pthread_join(pisaci[i], NULL);
    }
    for(int i = 0; i < BRCITACA; i++) {
        pthread_join(citaci[i], NULL);
    }
    
    for(int i = 0; i < BRBRISACA; i++) {
        pthread_join(brisaci[i], NULL);
    }

    delete list;
    pthread_mutex_destroy(&monitor);
    
    return 0;
}