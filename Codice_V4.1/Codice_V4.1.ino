/*
   Project Morsy
   Author hackAbility@PoliTo
   Modified 11/05/2019 by Andrea
   Arduino Nano
*/

/*Bug
  Problemi selettore modalità causa modulo bluetooth
*/

// -----DICHIARAZIONI ED INIZIALIZZAZIONI----- //
/*Librerie*/
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

/*Costanti*/
#define D_MAXINPUTTIME 800
#define COLUMNLCD 20 //colonne dello schermo LCD, minimo 20
#define ROWLCD 4 //righe dello schermo LCD, minimo 4
#define PROWLCD 3 //righe dello schermo LCD che possono contenere il testo
#define toneDuration 100 //durata dei toni eseguiti alla pressione dei tasti [ms]
#define R 37 //righe matrice
#define C 7 //colonne matrice

/*Strutture*/
enum State { // Stato del bottone
  UP = 0,
  DOWN = 1
} state;

/*Pinout*/
int buttonDot = 9;
int buttonLine = 8;
int buttonSpace = 7;
int buttonEndChar = 6;
int buttonCanc = 10;
int buzzerPin = 13;
int timePin = A7; //se non si utilizza il selettore per il modo, collegare a GND

/*Inizializzazone delle librerie*/
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address, Arduino pin A4->SDA  A5->SCL
SoftwareSerial BTserial(2, 3); // RX | TX // Connect the HC-06 TX to the Arduino RX on pin 2. Connect the HC-06 RX to the Arduino TX on pin 3 through a voltage divider

/*Prototipi funzioni*/
void saluto(); /*Funzione di avvio, saluta l'utente e fornisce istruzioni circa la modalità impiegata tramite la chiamata a timeTrigger*/
void readDashDot(State LineState, State DotState); /*Legge e stampa su lcd il punto o la linea*/
void printReadChar(); /* Stampa su schermo lcd la lettera inserita*/
char readCharacter(); /*Trasforma punti e linee in lettera, restituisce 0 se non trova la corrispondenza*/
void clearCharacter(); /*Pulisce la stringa usata per memorizzare la sequenza di punti e linee che compongono una lettera*/
void clearlcdline(int line); /*Cancella la linea di schermo passata come parametro*/
void clearLCD(); /*Cancella lo schermo e resetta i contatori di righe e colonne*/
void timeTrigger(); /*Regola la velocità (tempo minimo di attesa tra la pressione dei tasti) tramite input da trimmer*/
void clearBtString(); /*Pulisco la stringaBT da inviare*/
void sendBtString(); /*Invio la stringaBT*/
void endBtString(); /*Fine frase e invio stringa BT*/
void initgame(); /*Inizializzazione modalità gioco*/
void checkword(char g); /*Verifica che il carattere inserito corrisponda a quello richiesto dal gioco*/

/*Dichirazione matrice contenente la lettera ed il corrispettivo in morse*/
const char CLEAR = 0;
const char DOT = 1;
const char DASH = 2;
const char HEART = 3;
const char HAPPY = 4;
const char SAD = 5;

const char alphabet[R][C] {
  { 'A', DOT, DASH, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'B', DASH, DOT, DOT, DOT, CLEAR, CLEAR},
  { 'C', DASH, DOT, DASH, DOT, CLEAR, CLEAR},
  { 'D', DASH, DOT, DOT, CLEAR, CLEAR, CLEAR},
  { 'E', DOT, CLEAR, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'F', DOT, DOT, DASH, DOT, CLEAR, CLEAR},
  { 'G', DASH, DASH, DOT, CLEAR, CLEAR, CLEAR},
  { 'H', DOT, DOT, DOT, DOT, CLEAR, CLEAR},
  { 'I', DOT, DOT, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'J', DOT, DASH, DASH, DASH, CLEAR, CLEAR},
  { 'K', DASH, DOT, DASH, CLEAR, CLEAR, CLEAR},
  { 'L', DOT, DASH, DOT, DOT, CLEAR, CLEAR},
  { 'M', DASH, DASH, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'N', DASH, DOT, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'O', DASH, DASH, DASH, CLEAR, CLEAR, CLEAR},
  { 'P', DOT, DASH, DASH, DOT, CLEAR, CLEAR},
  { 'Q', DASH, DASH, DOT, DASH, CLEAR, CLEAR},
  { 'R', DOT, DASH, DOT, CLEAR, CLEAR, CLEAR},
  { 'S', DOT, DOT, DOT, CLEAR, CLEAR, CLEAR},
  { 'T', DASH, CLEAR, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'U', DOT, DOT, DASH, CLEAR, CLEAR, CLEAR},
  { 'V', DOT, DOT, DOT, DASH, CLEAR, CLEAR},
  { 'W', DOT, DASH, DASH, CLEAR, CLEAR, CLEAR},
  { 'X', DASH, DOT, DOT, DASH, CLEAR, CLEAR},
  { 'Y', DASH, DOT, DASH, DASH, CLEAR, CLEAR},
  { 'Z', DASH, DASH, DOT, DOT, CLEAR, CLEAR},
  { '0', DASH, DASH, DASH, DASH, DASH, CLEAR},
  { '1', DOT, DASH, DASH, DASH, DASH, CLEAR},
  { '2', DOT, DOT, DASH, DASH, DASH, CLEAR},
  { '3', DOT, DOT, DOT, DASH, DASH, CLEAR},
  { '4', DOT, DOT, DOT, DOT, DASH, CLEAR},
  { '5', DOT, DOT, DOT, DOT, DOT, CLEAR},
  { '6', DASH, DOT, DOT, DOT, DOT, CLEAR},
  { '7', DASH, DASH, DOT, DOT, DOT, CLEAR},
  { '8', DASH, DASH, DASH, DOT, DOT, CLEAR},
  { '9', DASH, DASH, DASH, DASH, DOT, CLEAR},
  { '?', DOT, DOT, DASH, DASH, DOT, DOT}
};

/*Definizione bit disegni*/
byte clear[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte dot[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B01110,
  B00100,
  B00000,
  B00000
};

byte dash[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte heart[8] = {
  B00000,
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte happy[8] = {
  B00000,
  B01010,
  B01010,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000
};

byte sad[8] = {
  B00000,
  B01010,
  B01010,
  B00000,
  B00000,
  B01110,
  B10001,
  B00000
};

/*Variabili*/
//variabili attinenti alla posizione su schermo LCD
int counterC = 0; // contatore utilizzato per la posizione della lettera (colonna) nello schermo LCD
int counterR = 0; // contatore utilizzato per la posizione della lettera (riga) nello schermo LCD
int cnt = 0; // sets the LCD dot/line sign column position

//click multiplo del tasto fine carattere/cancella
int count = 0;
unsigned long duration = 0, lastPress = 0;

//controllo pressione tasti
bool isReadingChar = false;
bool nextRead = true;

//memorizzano la sequenz di punti e linee di una lettera
char character[C - 1]; // dash-dot-sequence of the current character
int characterIndex; // index of the next dot/dash in the current character

//varabili timeTrigger
int maxinputtime = D_MAXINPUTTIME;
int attesa = D_MAXINPUTTIME * 3.5; //non cambiare

//variabili sezione gioco
bool gamemode = false;
int r; //riga della matrice dove si trova il carattere gamechar
char gamechar;
int life = 0;

//variabili bluetooth
char btString[30];
int scount = 0;
int baudRate = 9600;

// --------SETUP----- //
void setup() {
  Serial.begin(9600);
  BTserial.begin(baudRate);

  lcd.begin(COLUMNLCD, ROWLCD);
  lcd.createChar(HEART, heart);
  lcd.createChar(DASH, dash);
  lcd.createChar(DOT, dot);
  lcd.createChar(CLEAR, clear);
  lcd.createChar(HAPPY, happy);
  lcd.createChar(SAD, sad);

  pinMode(buttonDot, INPUT_PULLUP);
  pinMode(buttonLine, INPUT_PULLUP);
  pinMode(buttonSpace, INPUT_PULLUP);
  pinMode(buttonCanc, INPUT_PULLUP);
  pinMode(buttonEndChar, INPUT_PULLUP);

  analogReference(INTERNAL);
  randomSeed(analogRead(A0));
  lcd.backlight();
  delay(100);

  saluto();
  Serial.print("SR Baudrate: ");
  Serial.println(9600);
  Serial.print("BT Baudrate: ");
  Serial.println(baudRate);
  BTserial.print("Ready, Steady, GO!\r\n");
}

// --------LOOP----- //
void loop() {

  State DotState = digitalRead(buttonDot) ? UP : DOWN;
  State LineState = digitalRead(buttonLine) ? UP : DOWN;
  State GameState = digitalRead(buttonCanc) ? UP : DOWN;
  State SpaceState = digitalRead(buttonSpace) ? UP : DOWN;
  State EndCharState = digitalRead(buttonEndChar) ? UP : DOWN;

  if (DotState == DOWN || LineState == DOWN) { // leggo i punti e linee
    lastPress = millis();
    if (nextRead) {
      nextRead = false;
      readDashDot(LineState, DotState);
    }
  }

  else if (EndCharState == DOWN) {
    lastPress = millis();
    if (nextRead) {
      nextRead = false;
      clearlcdline(ROWLCD - 1);
      lcd.setCursor(0, ROWLCD - 1);
      duration = millis() + attesa;
      count++;
      if (count == 1) {
        lcd.print("Un click");
        tone(buzzerPin, 532, toneDuration);
      } else if (count == 2) {
        lcd.print("Due click");
        tone(buzzerPin, 587, toneDuration);
      } else if (count == 3) {
        lcd.print("Tre click");
        tone(buzzerPin, 659, toneDuration);
      }
    }
  }
  else if (millis() > duration && count) {
    if (count == 1) { //fine carattere
      printReadChar();
    } else if (count == 2) {
      if (isReadingChar) { //cancella buffer
        clearCharacter();
        clearlcdline(ROWLCD - 1);
        lcd.setCursor(0, ROWLCD - 1);
        lcd.print("Reinserire carattere");
      } else if (!gamemode) { //cancella carattere
        counterC--;
        if (counterC < 0) {
          if (counterR == PROWLCD - 1 && counterR > 0) {
            counterR--;
            if (counterR < 0) counterR = 0;
            counterC = COLUMNLCD - 1;
          } else counterC = 0;
        }
        lcd.setCursor(counterC, counterR);
        lcd.print(' ');
        clearlcdline(ROWLCD - 1);
        lcd.setCursor(0, ROWLCD - 1);
        //////////////
        btString[--scount] = '\000';
        /////////////
        lcd.print("Carattere cancellato");
        
      }
    } else if (count == 3) {
      if (!gamemode) {
        clearLCD();
        endBtString();
        lcd.setCursor(0, ROWLCD - 1);
        lcd.print("Schermo pulito");
      } else initgame();
    } else lcd.print("Annulla");
    count = 0;
  }
  else if (SpaceState == DOWN && !gamemode) { // stampo a video lo spazio
    lastPress = millis();
    if (nextRead) {
      nextRead = false;
      if (isReadingChar) printReadChar();
      else  clearCharacter();
      clearlcdline(ROWLCD - 1);
      lcd.setCursor(0, ROWLCD - 1);
      lcd.print("Spazio");
      lcd.setCursor(counterC++, counterR);
      if (counterC > COLUMNLCD) { // Riparte dall'inizio della riga sucessiva se finisce lo spazio sullo schermo
        if (counterR == PROWLCD - 1) {
          counterR = 0;
          for (int i = 0; i < PROWLCD; i++) clearlcdline(i);
        } else counterR++;
        lcd.setCursor(0, counterR);
        counterC = 1;
      }
      lcd.print(' ');
      ///////////
      btString[scount++] = ' ';
      sendBtString();
      ///////////////
    }
  }
  else if (GameState == DOWN) {
    lastPress = millis();
    if (nextRead) {
      nextRead = false;
      if (gamemode) {
        //esco da game mode
        gamemode = false;
        clearLCD();
        lcd.setCursor(0, 0);
        lcd.print("MODALITA' SCRITTURA");
        delay(1000);
        clearlcdline(0);
      } else { //entro in gamemode
        gamemode = true;
        clearLCD();
        lcd.setCursor(0, 0);
        lcd.print("MODALITA' GIOCO");
        delay(1000);
        initgame();
      }
    }
  }

  if (DotState == UP && LineState == UP && SpaceState == UP && GameState == UP && EndCharState == UP && !nextRead && ((millis() - lastPress) > maxinputtime )) nextRead = true;

  delay(10);
}


// -----FUNZIONI----- //
/*Funzione di avvio, saluta l'utente e fornisce istruzioni circa la modalità impiegata tramite la chiamata a timeTrigger*/
void saluto() {
  lcd.setCursor(0, 0);
  lcd.print("Ciao, sono MORSY!");
  lcd.setCursor(0, 1);
  lcd.print("V4.1");
  delay(1250);
  lcd.clear();
  timeTrigger();
  lcd.setCursor(0, 0);
  lcd.print("Ora sono pronto per");
  lcd.setCursor(0, 1);
  lcd.print("essere utilizzato!");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MODALITA' SCRITTURA");
  delay(1500);
  lcd.clear();
}

/*Legge e stampa su lcd il punto o la linea*/
void readDashDot(State LineState, State DotState) {
  if (characterIndex < C - 1) {
    if (cnt == 0) clearlcdline(ROWLCD - 1);
    isReadingChar = true;
    if (LineState == DOWN) {
      tone(buzzerPin, 880, toneDuration);
      character[characterIndex] = DASH;
      lcd.setCursor(0, ROWLCD - 1);
      lcd.print("Linea");
    } else if (DotState == DOWN) {
      tone(buzzerPin, 988, toneDuration);
      character[characterIndex] = DOT;
      lcd.setCursor(0, ROWLCD - 1);
      lcd.print("Punto");
    }
    lcd.setCursor(cnt + 6, ROWLCD - 1);
    lcd.write(character[characterIndex++]);
    cnt++;
  }
}

/* Stampa su schermo lcd la lettera inserita*/
void printReadChar() {
  char c = readCharacter();
  clearCharacter();
  clearlcdline(ROWLCD - 1);
  lcd.setCursor(0, ROWLCD - 1);
  if (c != 0) {
    lcd.print("Lettera inserita");
    if (!gamemode) {
      lcd.setCursor(counterC++, counterR);
      if (counterC > COLUMNLCD) { // Riparte dall'inizio della riga sucessiva se finisce lo spazio sullo schermo
        if (counterR == PROWLCD - 1) {
          counterR = 0;
          for (int i = 0; i < PROWLCD; i++) clearlcdline(i);
        } else counterR++;
        lcd.setCursor(0, counterR);
        counterC = 1;
      }
      lcd.print(c);
      ///////////
      btString[scount++] = c;
      ///////////////
    } else checkword(c);
  } else {
    lcd.print("Carattere non valido");
  }
}

/*Trasforma punti e linee in lettera, restituisce 0 se non trova la corrispondenza*/
char readCharacter() {
  bool found;
  for (int i = 0; i < R; ++i) {
    found = true;
    for (int j = 0; found && j < C - 1; ++j) {
      if (character[j] != alphabet[i][j + 1]) {
        found = false;
      }
    }
    if (found) return alphabet[i][0];
  }
  return 0;
}

/*Pulisce la stringa usata per memorizzare la sequenza di punti e linee che compongono una lettera*/
void clearCharacter() {
  isReadingChar = false;
  characterIndex = 0;
  cnt = 0;
  for (int i = 0; i < C - 1; ++i) {
    character[i] = CLEAR;
  }
}

/*Cancella la linea di schermo passata come parametro*/
void clearlcdline(int line) {
  for (int i = 0; i < COLUMNLCD; i++) {
    lcd.setCursor(i, line);
    lcd.print(" ");
  }
}

/*Cancella lo schermo e resetta i contatori di righe e colonne*/
void clearLCD() {
  clearCharacter();
  counterC = 0;
  counterR = 0;
  for (int j = 0; j < ROWLCD; j++)
    for (int i = 0; i < COLUMNLCD; i++) {
      lcd.setCursor(i, j);
      lcd.print(" ");
    }
}

/*Regola la velocità (tempo minimo di attesa tra la pressione dei tasti) tramite input da trimmer*/
void timeTrigger() {
  int reg = analogRead(timePin) + 20;

  lcd.setCursor(0, 0);
  lcd.print("Velocita'");
  lcd.setCursor(10, 0);
  lcd.print("Lento");
  lcd.setCursor(10, 1);
  lcd.print("Normale");
  lcd.setCursor(10, 2);
  lcd.print("Esperto");
  lcd.setCursor(10, 3);
  lcd.print("Pro");

  if (reg > 19) { //seleziono la velocità, dall'alto: Lento, Normale, Esperto, Pro
    if (reg <= 270) {
      maxinputtime = D_MAXINPUTTIME * 2;
      lcd.setCursor(18, 0);
      lcd.print("<-");
    } else if (reg <= 520) {
      maxinputtime = D_MAXINPUTTIME;
      lcd.setCursor(18, 1);
      lcd.print("<-");
    } else if (reg <= 770) {
      maxinputtime = D_MAXINPUTTIME / 2;
      lcd.setCursor(18, 2);
      lcd.print("<-");
    } else if (reg > 770) {
      maxinputtime = D_MAXINPUTTIME / 6;
      lcd.setCursor(18, 3);
      lcd.print("<-");
    }
    attesa = maxinputtime * 3.5;
  }
  delay(2500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Per modificare la");
  lcd.setCursor(0, 1);
  lcd.print("velocita' ruota la");
  lcd.setCursor(0, 2);
  lcd.print("manopola, se esiste,");
  lcd.setCursor(0, 3);
  lcd.print("e riavviami.");
  delay(4500);
  lcd.clear();
}


// -----FUNZIONI BLUETOOTH----- //
/*Pulisco la stringaBT da inviare*/
void clearBtString(){
  for (int i = 0; i < scount; i++) {
    btString[i] = '\000';
  }
  scount = 0;
}

/*Invio la stringaBT*/
void sendBtString(){
  for (int i = 0; i < scount; i++) {
    BTserial.print(btString[i]);
    btString[i] = '\000';
  }
  clearBtString();
  delay(3);
}

/*Fine frase e invio stringa BT*/
void endBtString(){
  sendBtString();
  BTserial.print(".\r\n");
}


// -----FUNZIONI MODALITA' GIOCO----- //
/*Inizializzazione modalità gioco*/
void initgame() {
  r = random(R);
  clearLCD();
  gamechar = alphabet[r][0];
  lcd.setCursor(0, 0);
  lcd.print("Traduci in Morse: ");
  lcd.print(gamechar);
  lcd.setCursor(0, 1);
  lcd.print("Vite: ");
  for (int i = 0; i < (life = 3); i++) lcd.write(HEART);
}

/*Verifica che il carattere inserito corrisponda a quello richiesto dal gioco*/
void checkword(char g) {
  int thisNote, noteDuration, pauseBetweenNotes, i;
  /*Array toni e musichette*/
  int melodyWin[] = {462, 396, 396, 420, 396, 0, 447, 462};
  int melodyFail[] = {494, 0, 480, 0, 461, 0, 600, 0};
  int noteDurationsWin[] = {4, 8, 8, 4, 4, 4, 4, 4 }; // note duration: 4 = quarter note, 8 = eighth note, etc.
  int noteDurationsFail[] = {4, 16, 4, 16, 4, 16, 2, 4}; // note duration: 4 = quarter note, 8 = eighth note, etc.

  Serial.println(life);
  if (g == gamechar || life == 1) {
    for (i = 1; i < ROWLCD; i++) clearlcdline(i);
    lcd.setCursor(0, 1);
    if (g == gamechar) {
      lcd.print("HAI VINTO ");
      lcd.print(HAPPY);
      for (thisNote = 0; thisNote < 8; thisNote++) {
        noteDuration = 1000 / noteDurationsWin[thisNote];
        tone(buzzerPin, melodyWin[thisNote], noteDuration);
        pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
    }
    else {
      for (i = 0; i < 2; i++) {
        lcd.setCursor(0, 1);
        lcd.print("Errore");
        delay(500);
        clearlcdline(1);
        delay(500);
      }
      lcd.setCursor(0, 1);
      lcd.print("Hai inserito: ");
      lcd.print(g);
      delay(1500);
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("HAI PERSO ");
      lcd.print(SAD);
      for (thisNote = 0; thisNote < 8; thisNote++) {
        noteDuration = 1000 / noteDurationsFail[thisNote];
        tone(buzzerPin, melodyFail[thisNote], noteDuration);
        pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
      lcd.setCursor(0, 2);
      lcd.print("Corretta: ");
      for (int i = 1; i < C; i++) lcd.write(alphabet[r][i]);
      delay(4000);
    }  
    initgame();

  } else {
    life--;
    for (i = 1; i < ROWLCD; i++) clearlcdline(i);
    for (i = 0; i < 2; i++) {
      lcd.setCursor(0, 1);
      lcd.print("Errore");
      delay(500);
      clearlcdline(1);
      delay(500);
    }
    lcd.setCursor(0, 1);
    lcd.print("Hai inserito: ");
    lcd.print(g);
    delay(1500);
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Vite: ");
    for (i = 0; i < life; i++) lcd.write(HEART);
  }
}
