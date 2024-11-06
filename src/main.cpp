#include <Arduino.h>

#define VERDE_RGB_PIN 5
#define ALBASTRU_RGB_PIN 4
#define ROSU_RGB_PIN 6
#define START_PIN 2
#define DIFICULTATE_PIN 3

String cuvinte[] = {
    "apa", "soare", "luna", "stea", "carte",
    "muzica", "computer", "floare", "joc", "casa"
};

int frecventa[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int timp_utilizator = 0;
int interval_verificare = 0;
bool inRunda = false;
bool inNumaratoareInversa = false;
int dificultate = 0;
unsigned long ultimulTimpDebounceStart = 0;
unsigned long ultimulTimpDebounceDificultate = 0;
unsigned long intarziereDebounce = 50;
bool ultimaStareButonStart = HIGH;
bool ultimaStareButonDificultate = HIGH;
const int durataRundei = 30000;
int numarCuvinteCorecte = 0;
const unsigned int intervalUsor = 6000;
const unsigned int intervalMediu = 3000;
const unsigned int intervalDificil = 1000;
unsigned int intervalCurent = 0;
const char* dificultati[] = {"Usor", "Mediu", "Dificil"};
bool butonDificultateApasat = false;
unsigned long timpInceputNumaratoareInversa = 0;
unsigned long timpSchimbareLED = 0;
bool numaratoareInversaActiva = false;
int numarPuncte = 0;

void seteazaCuloareLED(int rosu, int verde, int albastru);
void gestioneazaButonDificultate();
void opresteRunda();
void executaRunda();
void numaratoareInversa();
int genereazaCuvant();

void seteazaCuloareLED(int rosu, int verde, int albastru) {
  analogWrite(ROSU_RGB_PIN, rosu);
  analogWrite(VERDE_RGB_PIN, verde);
  analogWrite(ALBASTRU_RGB_PIN, albastru);
}

void setup() {
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(DIFICULTATE_PIN, INPUT_PULLUP);
  pinMode(ROSU_RGB_PIN, OUTPUT);
  pinMode(VERDE_RGB_PIN, OUTPUT);
  pinMode(ALBASTRU_RGB_PIN, OUTPUT);
  seteazaCuloareLED(255, 255, 255);
  Serial.begin(9600);
}

void loop() {
  bool citire = digitalRead(DIFICULTATE_PIN);
  if (citire != ultimaStareButonDificultate) {
    ultimulTimpDebounceDificultate = millis();
  }
  if ((millis() - ultimulTimpDebounceDificultate) > intarziereDebounce) {
    if (citire == LOW && !inRunda && !butonDificultateApasat) {
      gestioneazaButonDificultate();
      butonDificultateApasat = true;
    }
  }
  if (citire == HIGH) {
    butonDificultateApasat = false;
  }
  ultimaStareButonDificultate = citire;

  bool startCitire = digitalRead(START_PIN);
  if (startCitire != ultimaStareButonStart) {
    ultimulTimpDebounceStart = millis();
  }
  if ((millis() - ultimulTimpDebounceStart) > intarziereDebounce) {
    if (startCitire == LOW) {
      if (!inRunda && !inNumaratoareInversa) {
        Serial.println("Buton start apasat");
        numaratoareInversa();
        executaRunda();
      }
    }
  }
  ultimaStareButonStart = startCitire;
}

int genereazaCuvant() {
  int indexAleator;
  do {
    indexAleator = random(0, 10);
  } while (frecventa[indexAleator] == 1);
  frecventa[indexAleator] = 1;
  return indexAleator;
}

void numaratoareInversa() {
  numaratoareInversaActiva = true;
  unsigned long currentMillis = millis();
  for (int i = 3; i > 0; i--) {
    seteazaCuloareLED(255, 255, 255);
    Serial.print(i);
    Serial.print("\n");
    while (millis() - currentMillis < 500) {}
    seteazaCuloareLED(0, 0, 0);
    currentMillis = millis();
    while (millis() - currentMillis < 500) {}
    currentMillis = millis();
    seteazaCuloareLED(255, 255, 255);
  }
  inNumaratoareInversa = false;
}

void executaRunda() {
  Serial.println("Runda a inceput!");
  seteazaCuloareLED(0, 255, 0);
  inRunda = true;
  timp_utilizator = 0;
  numarPuncte = 0;
  unsigned long timpInceputRunda = millis();

  while (timp_utilizator < 30000) {
    int indexCuvant = genereazaCuvant();
    String cuvantCurent = cuvinte[indexCuvant];
    unsigned long timpInceputCuvant = millis();
    String inputUtilizator = "";

    Serial.print("Cuvant : ");
    Serial.println(cuvantCurent);

    while (millis() - timpInceputCuvant < intervalCurent) {
      if (Serial.available() > 0) {
        char caracterInput = Serial.read();
        if (caracterInput == '\b') {
          if (inputUtilizator.length() > 0) {
            inputUtilizator.remove(inputUtilizator.length() - 1);
          }
        } else {
          inputUtilizator += caracterInput;
        }

        // Afisam progresul textului pe aceeasi linie
        Serial.print("\rText introdus: ");
        Serial.print(inputUtilizator);
        
        // Asiguram ca restul liniei este gol dacă stergem caractere
        Serial.print("                 ");

        if (cuvantCurent.startsWith(inputUtilizator)) {
          seteazaCuloareLED(0, 255, 0);
        } else {
          seteazaCuloareLED(255, 0, 0);
        }

        if (inputUtilizator == cuvantCurent) {
          Serial.println("\nCuvant corect!");  // Trecem pe o linie noua pentru confirmare
          numarPuncte++;
          break;
        }
      }
    }

    timp_utilizator += intervalCurent;
  }
  opresteRunda();
}


void opresteRunda() {
  Serial.println("Runda oprita!");
  Serial.println("Ai tastat");
  Serial.print(numarPuncte);
  Serial.println(" cuvinte corect!");
  inRunda = false;
  seteazaCuloareLED(255, 255, 255);
  for (int i = 0; i < 10; i++) {
    frecventa[i] = 0;
  }
}

void gestioneazaButonDificultate() {
  if (!inRunda && !inNumaratoareInversa) {
    dificultate = (dificultate + 1) % 3;
    switch (dificultate) {
      case 0: intervalCurent = intervalUsor; break;
      case 1: intervalCurent = intervalMediu; break;
      case 2: intervalCurent = intervalDificil; break;
    }
    Serial.print(dificultati[dificultate]);
    Serial.println(" mod activat!");
  }
}
