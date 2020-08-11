//Using LiquidCrystal library
#include <LiquidCrystal.h>

typedef enum _Display_EZustand
{ 
  AUSGABE_DIFFERENZ = 0,
  AUSGABE_WERT,
  AUSGABE_TEST,
} Display_EZustand;
/*==================================================================*/
/*  Display                                                         */
/*==================================================================*/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define btnANY    6


/*==================================================================*/
/*  const-definition                                                */
/*==================================================================*/

// Es sind maximal 4 Pflanzen moeglich.
// Bei 4 Pflanzen muss allerdings die Testfunktion geaendert werden.
// Oder man laesst die Testfunktion einfach weg.
#define ANZAHL_PFLANZEN 3

// Analoge Eingangspinne fuer die Sensoren festlegen
// A0 wird vom Display fuer die Taster verwendet
// A4 ist noch frei
#define SENSOR1 A1
#define SENSOR2 A2
#define SENSOR3 A3
//define SENSOR4 A4

// Digitale Ausgangspinne fuer die Pumpen festlegen
#define PUMPE1 11
#define PUMPE2 12
#define PUMPE3 13

#define KONTROLLZEIT 300000L  // 300 sec
#define WARTEZEIT     40000   //  40 sec
#define PUMPZEIT       4000   //   4 sec

/*==================================================================*/
/*  variable-definition                                             */
/*==================================================================*/

int16_t ai16Sensor[ANZAHL_PFLANZEN] = {SENSOR3,SENSOR2,SENSOR1};
int16_t ai16Pumpe[ANZAHL_PFLANZEN] = {PUMPE3,PUMPE2,PUMPE1};
int16_t ai16Messwert [ANZAHL_PFLANZEN] = {1000, 1000, 1000};
int16_t ai16Sollwert [ANZAHL_PFLANZEN] = {3550, 3550, 3550};
int16_t aui16PumpZaehler [ANZAHL_PFLANZEN] = {0, 0, 0};
uint32_t aui32Pumpzeit [ANZAHL_PFLANZEN] = {0, 0, 0};
uint32_t aui32LetztePumpzeit [ANZAHL_PFLANZEN] = {0, 0, 0};
boolean abPumpeAktiv [ANZAHL_PFLANZEN] = {false, false, false};
boolean abPumpeGesperrt [ANZAHL_PFLANZEN] = {false, false, false};
char acPosition [ANZAHL_PFLANZEN] = {'L', 'M', 'R'};

uint32_t ui32PumpenStart = 0;
uint32_t ui32Zeit_ZehntelSekunden = 0;
uint32_t ui32Zeit_Merker = 0;
int16_t i16count=0;
int16_t i16Taste=btnNONE;
Display_EZustand Display_Zustand = AUSGABE_DIFFERENZ;

/*==================================================================*/
/*  function-prototypes                                             */
/*==================================================================*/

int16_t read_LCD_buttons();
int16_t Tastenflanke();
void Zeit_ausgeben(const uint32_t ui32Millisekunden);

/*==================================================================*/
/*  function-implementation(s)                                      */
/*==================================================================*/

void setup()
{
  int16_t i;

  for (i=0; i< ANZAHL_PFLANZEN; i++ )
  {
    // Alle Sensoren sind Eingaenge
    pinMode(ai16Sensor[i], INPUT);
    //Alle Pumpen sind Ausgaenge
    pinMode(ai16Pumpe[i], OUTPUT);
    //Pumpen zu Begin aus
    digitalWrite(ai16Pumpe[i],LOW);
    //Pumpzeit auf aktuelle Zeit setzen
    aui32Pumpzeit[i] = millis();
  }

 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.print("Topf L: ");         // print a simple message
 
 ui32Zeit_Merker = millis();
}
 
void loop()
{
  int16_t i = 0;
  int16_t i16AusgabeIndex = 0;
  uint32_t ui32Zeitdifferenz;
  boolean bMessungaAktiv = false;

  ui32Zeit_ZehntelSekunden = millis()/100;

  // 100ms Takt erzeugen
  ui32Zeitdifferenz = millis() - ui32Zeit_Merker;
  if (ui32Zeitdifferenz >= 100UL)
  {
    if (ui32Zeitdifferenz >= 200UL)
    { // irgendwas hat sehr lange gedauert, Zeit neu aufsetzen
       ui32Zeit_Merker = millis();
    }
    else
    { // Merker um 100ms erhoehen
       ui32Zeit_Merker += 100UL;
    }
    bMessungaAktiv = true;
  }

  if (bMessungaAktiv)
  { // Alles was in dieser Klammer steht wir zyklisch alle 100ms aufgerufen

    for (i=0; i< ANZAHL_PFLANZEN; i++ )
    {
      ai16Messwert[i] += analogRead(ai16Sensor[i]) - (ai16Messwert[i]>>3); // Tiefpass
      if (millis() - aui32Pumpzeit[i] >= KONTROLLZEIT)
      { // Die Kontrollzeit seit dem letzten Punpen ist abgelaufen.
        // Pumpenzaehler wieder zuruecksetzen.
        aui16PumpZaehler[i]=0;
      }
    }

    if (abPumpeAktiv[0] || abPumpeAktiv[1] || abPumpeAktiv[2])
    { // Eine der Pumpen laueft
      if ( millis() - ui32PumpenStart >= PUMPZEIT)
      {
        for (i=0; i< ANZAHL_PFLANZEN; i++ )
        { // Alle Pumpen ausschalten
          abPumpeAktiv[i]=false;
          digitalWrite(ai16Pumpe[i],LOW);
        }

      }
    }
    else
    { // Zur Zeit ist keine  Pumpe an
      for (i=0; i< ANZAHL_PFLANZEN; i++ )
      {
        if ((ai16Messwert[i] > ai16Sollwert[i]) && (millis() - aui32Pumpzeit[i] >= WARTEZEIT))
        { // Erde trocken und Wartezeit abgelaufen
          if (aui16PumpZaehler[i] >=5)
          { // Fuenf mal hintereinander gepumpt und immer noch trocken
            // Da stimmt was nicht. Pumpe sperren
            abPumpeGesperrt[i] = true;
          }
          else
          {
            if (!abPumpeGesperrt[i])
            {
             //Pumpzeit setzen
              digitalWrite(ai16Pumpe[i], HIGH);
              abPumpeAktiv[i]=true;
              if (!aui16PumpZaehler[i])
              { // Keine Pumpwiederholung innerhalb des Kontrollzyklus => Pumpzeit merken
                aui32LetztePumpzeit[i] = millis() - aui32Pumpzeit[i];
              }
              ui32PumpenStart = millis();
              aui32Pumpzeit [i] = ui32PumpenStart;
              aui16PumpZaehler[i] += 1;
              break;  // nur eine Pumpe gleichzeitig laufen lassen
            }
          }
        }
      }
    }

// LCD-Ausgabe und Tasten einlesen
    i16Taste = Tastenflanke();
// Alle 10 s einen anderen Wert ausgeben.
    if (ui32Zeit_ZehntelSekunden%300 < 100)
    {
      i16AusgabeIndex=2;
    }
    else if (ui32Zeit_ZehntelSekunden%300 < 200)
    {
      i16AusgabeIndex=1;
    }
    else
    {
      i16AusgabeIndex=0;
    }

    switch (Display_Zustand)
    {
      case AUSGABE_DIFFERENZ:
      default:
      {
        lcd.setCursor(0,0);
        lcd.print("Topf ");
        lcd.print(acPosition[i16AusgabeIndex]);
        lcd.print(":         ");
        lcd.setCursor(8,0);            // move cursor to first line "0" position 8
        lcd.print(ai16Sollwert[i16AusgabeIndex] - ai16Messwert[i16AusgabeIndex]);
        lcd.setCursor(0,1);            // move cursor to second line "1" 
        if (abPumpeGesperrt[i16AusgabeIndex])
        {
          lcd.print("Pumpe gesperrt  ");
        }
        else
        {
          if (ui32Zeit_ZehntelSekunden%50 < 25)
          {
            lcd.print("Akt:            ");
            lcd.setCursor(6,1);            // move cursor to second line "1" position 6
            Zeit_ausgeben(millis() - aui32Pumpzeit[i16AusgabeIndex]);
          }
          else
          {
            lcd.print("Vorh:           ");
            lcd.setCursor(6,1);            // move cursor to second line "1" position 6
            Zeit_ausgeben(aui32LetztePumpzeit[i16AusgabeIndex]);
          }
        }
        if (i16Taste == btnSELECT)
        { // Wenn Select gedrueckt, alle Pumpen entsperren
          for (i=0; i< ANZAHL_PFLANZEN; i++ )
          {
            abPumpeGesperrt[i] = false;
            aui16PumpZaehler[i]=0;
          }
        }
        if (i16Taste == btnDOWN)
        {
          Display_Zustand = AUSGABE_WERT;
        }
        break;
      } // AUSGABE_DIFFERENZ

      case AUSGABE_WERT:
      {
        lcd.setCursor(0,0);
        lcd.print("Wert ");
        lcd.print(acPosition[i16AusgabeIndex]);
        lcd.print(":         ");
        lcd.setCursor(8,0);            // move cursor to first line "0" position 8
        lcd.print(ai16Messwert[i16AusgabeIndex]);
        lcd.setCursor(0,1);            // move cursor to second line "1" 
        if (abPumpeGesperrt[i16AusgabeIndex])
        {
          lcd.print("Pumpe gesperrt  ");
        }
        else
        {
          if (ui32Zeit_ZehntelSekunden%50 < 25)
          {
            lcd.print("Akt:            ");
            lcd.setCursor(6,1);            // move cursor to second line "1" position 6
            Zeit_ausgeben(millis() - aui32Pumpzeit[i16AusgabeIndex]);
          }
          else
          {
            lcd.print("Vorh:           ");
            lcd.setCursor(6,1);            // move cursor to second line "1" position 6
            Zeit_ausgeben(aui32LetztePumpzeit[i16AusgabeIndex]);
          }
        }
        if (i16Taste == btnSELECT)
        { // Wenn Select gedrueckt, alle Pumpen entsperren
          for (i=0; i< ANZAHL_PFLANZEN; i++ )
          {
            abPumpeGesperrt[i] = false;
            aui16PumpZaehler[i]=0;
          }
        }
        if (i16Taste == btnUP)
        {
          Display_Zustand = AUSGABE_DIFFERENZ;
        }
        if (i16Taste == btnDOWN)
        {
          Display_Zustand = AUSGABE_TEST;
        }
        break;
      } // AUSGABE_WERT

      case AUSGABE_TEST:
      {
        int16_t i16Pumpentest = read_LCD_buttons();

        lcd.setCursor(0,0);
        lcd.print("Test   L   M   R");
        lcd.setCursor(0,1);            // move cursor to second line "1" 
        lcd.print("Taste SEL  L   R");
        if (i16Taste == btnUP)
        {
          Display_Zustand = AUSGABE_WERT;
        }
        lcd.setCursor(0,1);            // move cursor to second line "1" 
        switch (i16Pumpentest)
        { // ueberschreibt den Pumpenausgang aus der Feuchtigkeitsmessung
          case btnRIGHT:
          {
               digitalWrite(ai16Pumpe[0],LOW);
               digitalWrite(ai16Pumpe[1],LOW);
               digitalWrite(ai16Pumpe[2],HIGH);
               lcd.print("Pumpe Rechts    ");
            break;
          }
          case btnLEFT:
          {
               digitalWrite(ai16Pumpe[0],LOW);
               digitalWrite(ai16Pumpe[1],HIGH);
               digitalWrite(ai16Pumpe[2],LOW);
               lcd.print("Pumpe Mitte     ");
            break;
          }
          case btnSELECT:
          {
               digitalWrite(ai16Pumpe[0],HIGH);
               digitalWrite(ai16Pumpe[1],LOW);
               digitalWrite(ai16Pumpe[2],LOW);
               lcd.print("Pumpe Links     ");
            break;
          }
          default:
          {
               digitalWrite(ai16Pumpe[0],LOW);
               digitalWrite(ai16Pumpe[1],LOW);
               digitalWrite(ai16Pumpe[2],LOW);
            break;
          }
        } // switch (i16Pumpentest)

        break;
      } // AUSGABE_TEST
    
    } //switch (Display_Zustand)

  } // Ende bMessungaAktiv (Zyklischer Aufruf alle 100ms)

} // Ende loop()

void Zeit_ausgeben(const uint32_t ui32Millisekunden)
{
  int32_t i32ZeitSekunden = ui32Millisekunden / 1000;
  int32_t i32ZeitStunden = i32ZeitSekunden / 3600;
  int32_t i32ZeitMinuten = (i32ZeitSekunden - i32ZeitStunden * 3600) / 60;

  i32ZeitSekunden -= i32ZeitStunden * 3600 + i32ZeitMinuten * 60;
  lcd.print(i32ZeitStunden);
  lcd.print("h");
  lcd.print(i32ZeitMinuten);
  lcd.print("m");
  lcd.print(i32ZeitSekunden);
  lcd.print("s");
}
// read the buttons
int16_t read_LCD_buttons()
{
  int16_t adc_key_in  =  analogRead(0);      // read the value from the sensor 
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close

  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result

  // For V1.1 us this threshold
  /* 
   if (adc_key_in < 50)   return btnRIGHT;  
   if (adc_key_in < 250)  return btnUP; 
   if (adc_key_in < 450)  return btnDOWN; 
   if (adc_key_in < 650)  return btnLEFT; 
   if (adc_key_in < 850)  return btnSELECT;   
  */

  // For V1.0 comment the other threshold and use the one below:

  if (adc_key_in < 50)   return btnRIGHT;  
  if (adc_key_in < 195)  return btnUP; 
  if (adc_key_in < 380)  return btnDOWN; 
  if (adc_key_in < 555)  return btnLEFT; 
  if (adc_key_in < 790)  return btnSELECT;   

  return btnNONE;  // when all others fail, return this...
}

//Tastenflanke erkennen
int16_t Tastenflanke()
{
  static int16_t i16_Taste_alt = btnNONE;
  int16_t i16Taste_intern = read_LCD_buttons();  // read the buttons
  int16_t i16_Rueckgabewert = btnNONE;


  switch (i16Taste_intern)               // depending on which button was pushed, we perform an action
  {
   case btnRIGHT:
     {
       if (i16_Taste_alt == btnRIGHT)
       {  // Taste fuer 100ms gedrueckt
          // Ausgebeb und Erkennung blockieren bis keine Taste gedrueckt
         i16_Taste_alt = btnANY;
         i16_Rueckgabewert = btnRIGHT;
       }
       else
       {
         if (i16_Taste_alt == btnNONE)
         {  // Taste erst beim naechsten Mal als gedrueckt ausgeben
           i16_Taste_alt = btnRIGHT;
         }
         else
         { // Erkennung blockieren bis keine Taste gedrueckt
           i16_Taste_alt = btnANY;
         }
         i16_Rueckgabewert = btnNONE;
       }
       break;
     }
   case btnLEFT:
     {
       if (i16_Taste_alt == btnLEFT)
       {  // Taste fuer 100ms gedrueckt
          // Ausgebeb und Erkennung blockieren bis keine Taste gedrueckt
         i16_Taste_alt = btnANY;
         i16_Rueckgabewert = btnLEFT;
       }
       else
       {
         if (i16_Taste_alt == btnNONE)
         {  // Taste erst beim naechsten Mal als gedrueckt ausgeben
           i16_Taste_alt = btnLEFT;
         }
         else
         { // Erkennung blockieren bis keine Taste gedrueckt
           i16_Taste_alt = btnANY;
         }
         i16_Rueckgabewert = btnNONE;
       }
       break;
     }
   case btnUP:
     {
       if (i16_Taste_alt == btnUP)
       {  // Taste fuer 100ms gedrueckt
          // Ausgeben und Erkennung blockieren bis keine Taste gedrueckt
         i16_Taste_alt = btnANY;
         i16_Rueckgabewert = btnUP;
       }
       else
       {
         if (i16_Taste_alt == btnNONE)
         {  // Taste erst beim naechsten Mal als gedrueckt ausgeben
           i16_Taste_alt = btnUP;
         }
         else
         { // Erkennung blockieren bis keine Taste gedrueckt
           i16_Taste_alt = btnANY;
         }
         i16_Rueckgabewert = btnNONE;
       }
       break;
     }
   case btnDOWN:
     {
       if (i16_Taste_alt == btnDOWN)
       {  // Taste fuer 100ms gedrueckt
          // Ausgeben und Erkennung blockieren bis keine Taste gedrueckt
         i16_Taste_alt = btnANY;
         i16_Rueckgabewert = btnDOWN;
       }
       else
       {
         if (i16_Taste_alt == btnNONE)
         {  // Taste erst beim naechsten Mal als gedrueckt ausgeben
           i16_Taste_alt = btnDOWN;
         }
         else
         { // Erkennung blockieren bis keine Taste gedrueckt
           i16_Taste_alt = btnANY;
         }
         i16_Rueckgabewert = btnNONE;
       }
       break;
     }
   case btnSELECT:
     {
       if (i16_Taste_alt == btnSELECT)
       {  // Taste fuer 100ms gedrueckt
          // Ausgeben und Erkennung blockieren bis keine Taste gedrueckt
         i16_Taste_alt = btnANY;
         i16_Rueckgabewert = btnSELECT;
       }
       else
       {
         if (i16_Taste_alt == btnNONE)
         {  // Taste erst beim naechsten Mal als gedrueckt ausgeben
           i16_Taste_alt = btnSELECT;
         }
         else
         { // Erkennung blockieren bis keine Taste gedrueckt
           i16_Taste_alt = btnANY;
         }
         i16_Rueckgabewert = btnNONE;
       }
       break;
     }
   case btnNONE:
   default:
     {
       i16_Taste_alt = btnNONE;
       i16_Rueckgabewert = btnNONE;
       break;
     }
  } //switch (i16Taste_intern)
  return i16_Rueckgabewert;
}
