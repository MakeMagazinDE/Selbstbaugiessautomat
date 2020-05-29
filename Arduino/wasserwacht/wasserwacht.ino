int sensorTopf1Pin = A1;
int sensorTopf2Pin = A2;
int sensorTopf3Pin = A3;

//Falls man einen Sensor für den Wasserstand braucht
int sensorReservePin = A0;

int pumpeTopf1Pin = 11;
int pumpeTopf2Pin = 12;
int pumpeTopf3Pin = 13;

//Unterhalb diser Sensorwerte wird nicht gegossen
int sollTopf1 = 300;
int sollTopf2 = 300;
int sollTopf3 = 300;
int sollReserve = 580;

int istTopf1 = 700;
int istTopf2 = 700;
int istTopf3 = 700;

//Wartezeit zwischen den Messungen
int Wartezeit = 20000;

//Solang wird bei zu trockenem Boden gepumpt
int Pumpzeit = 3000;

void setup() {
  
  // Alle Sensoren sind Eingänge
  pinMode(sensorTopf1Pin, INPUT);
  pinMode(sensorTopf2Pin, INPUT);
  pinMode(sensorTopf3Pin, INPUT);
  pinMode(sensorReservePin, INPUT);

  //Alle Pumpen sind Ausgänge
  pinMode(pumpeTopf1Pin, OUTPUT);
  pinMode(pumpeTopf2Pin, OUTPUT);
  pinMode(pumpeTopf3Pin, OUTPUT);

  //Pumpen zu Begin aus
  digitalWrite(pumpeTopf1Pin,LOW);
  digitalWrite(pumpeTopf2Pin,LOW);
  digitalWrite(pumpeTopf3Pin,LOW);
  
  Serial.begin(9600);
  
}
      
void loop() {
  //Wartezeit für die Senoren zu Beginn
  delay(Wartezeit);

  //Messen...
  istTopf1 = analogRead(sensorTopf1Pin);
  istTopf2 = analogRead(sensorTopf2Pin);
  istTopf3 = analogRead(sensorTopf3Pin);
  
  //... Werte seriell ausgeben...
  Serial.print("Wasser Topf 1: ");
  Serial.print(istTopf1);
  Serial.print("\t");   
  Serial.print("Wasser Topf 2: ");
  Serial.print(istTopf2);
  Serial.print("\t");   
  Serial.print("Wasser Topf 3: ");
  Serial.print(istTopf3);   
  Serial.println("\t");   

  //...und bei Bedarf pumpen
  if(istTopf1 > sollTopf1) 
    {
      digitalWrite(pumpeTopf1Pin, HIGH);
      Serial.print("Topf1 zu trocken!!!"   );
      Serial.print("\t");
      delay(Pumpzeit);
      digitalWrite(pumpeTopf1Pin, LOW);
    } 
  else
    {
      Serial.print("Topf1 ist ok!"); 
      Serial.print("\t");
      Serial.print("\t");
    }
    
  if(istTopf2 > sollTopf2) 
    {
      digitalWrite(pumpeTopf2Pin, HIGH);
      Serial.print("Topf2 zu trocken!!!"   );
      Serial.print("\t");
      delay(Pumpzeit);
      digitalWrite(pumpeTopf2Pin, LOW);
    } 
  else
    {
      Serial.print("Topf2 ist ok!"); 
      Serial.print("\t");
      Serial.print("\t");
    }

  if(istTopf3 > sollTopf3) 
    {
      digitalWrite(pumpeTopf3Pin, HIGH);
      Serial.println("Topf3 zu trocken!!!"   );
      delay(Pumpzeit);
      digitalWrite(pumpeTopf3Pin, LOW);
    } 
  else
    {
      Serial.println("Topf3 ist ok!"); 
    }
    
delay(Wartezeit);
}
