// Stepperspezifizierung
#include <Stepper.h> 							//Einbindung der Stepperbibliothek
#define STEPS 200 							//Stepper mit 200 Schritten pro Umdrehung werden verwendet
Stepper Stepper1(STEPS, 2, 3, 4, 5); 		//Stepper1 wird mit Arduino-Pins D2,D3,D4,D5 verbunden
Stepper Stepper2(STEPS, 6, 7, 8, 9);		 //Stepper2 wird mit Arduino-Pins D6,D7,D8,D9 verbunden

//    Verknuepfung Anschluesse peripherer Bauelemete mit Arduino-Pins
int   TempSensorPin = 0;      			//Temperatursensor an A0
int   LichtPin = 0;           				//Anzeigelicht bzw. Blinklicht für "Ausserhalb-Messbereich-Anzeige"  an D0     
int   NaeherungsPin = 1;      			//Naeherungsschalter an D1
int   EnableHBridge1Pin =10;  		//Enable HBridge1 an D10 
int   EnableHBridge2Pin = 11;		//Enable HBridge2 an D11 
int   NullPosGabel1Pin = 12;  		//Ausgang Gabel1 für die NullPosEinstellung an D12
int   NullPosGabel2Pin = 13;  		//Ausgang Gabel2 für die NullPosEinstellung an D13

// Weitere Variablendeklarationen
float   GradCelsius = 0;        			//Der gemessene Celsiuswert
float   GradCelsiusRund = 0;   		//Der gerundete Celsiuswert
int     ZahlScheibe1 = 0;      		     //Die auf Scheibe1 darzstellende Zahl
float   ZahlScheibe2 = 0;       			//Die auf Scheibe2 darzustellnde Zahlenfolge
int     Stepper1Offset = 11;   			//Schritte von der Null- zur Startposition von Stepper1 (danach weiter je nach ZahlScheibe1) 
int     Stepper2Offset = 5;     			 //Schritte von der Null- zur Startposition von Stepper2 (danach weiter je nach ZahlScheibe2) 
int     Stepper1Delta = 20;    			 //Schritte zur Überwindung eines Segments von Stepper1
int     Stepper2Delta = 9;      			 //Schritte zur Überwindung eines Segments von Stepper1
int     Stepper1Schritte = 0;   			 //Gesamtschrittzahl von Stepper1 von der Nullposition zur Anzeigeposition
int     Stepper2Schritte = 0;  			 //Gesamtschrittzahl von Stepper2 von der Nullposition zur Anzeigeposition
boolean Naeherung;                  


void setup() 
{
  Stepper1.setSpeed(15); //15 Umdrehungen pro Minute
  Stepper2.setSpeed(10); //10 Umdrehungen pro Minute 
  analogReference(DEFAULT);
  pinMode(LichtPin, OUTPUT);
  pinMode(NaeherungsPin, INPUT);
  pinMode(EnableHBridge1Pin, OUTPUT);
  pinMode(EnableHBridge2Pin, OUTPUT);
  pinMode(NullPosGabel1Pin, INPUT);
  pinMode(NullPosGabel2Pin, INPUT);
  digitalWrite(EnableHBridge1Pin,LOW);
  digitalWrite(EnableHBridge2Pin,LOW);
  digitalWrite(LichtPin,LOW);
}


 
void loop() 
{
  digitalWrite(LichtPin,LOW); 
  Temperaturmessen();
  if ((GradCelsius >= 10.0) and (GradCelsius < 39.6)) 
   {
      Aufrunden(); 
      ZahlenfolgeZerlegen();
      StepperNullPosAnfahren(); //Stepper1, Stepper 2 in die Nullposition fahren     
      StepperSchritteBerechnen();
      StepperSchritteAusfuehren();
      digitalWrite(LichtPin,HIGH);
      do
        Naeherung = digitalRead(NaeherungsPin);
      while
       (Naeherung == false);
   }
  else
   {
     digitalWrite(LichtPin,HIGH);
     delay(1000);
     digitalWrite(LichtPin,LOW);
     delay(1000);
   }
}

void Temperaturmessen()
{
  GradCelsius = (500.0/1024.0)*analogRead(TempSensorPin);
}



void Aufrunden()
{
  for (int Schwelle = 10; Schwelle < 40; Schwelle++)
   {
    if (GradCelsius == Schwelle) GradCelsiusRund = Schwelle; 
    if ((GradCelsius > Schwelle) and (GradCelsius <=(Schwelle+0.5))) GradCelsiusRund = Schwelle+0.5; 
    if ((GradCelsius > (Schwelle+0.5)) and (GradCelsius <=(Schwelle+1))) GradCelsiusRund = Schwelle+1;
   }
}



void ZahlenfolgeZerlegen()
{
  if (((GradCelsiusRund-10) >= 0) and (GradCelsiusRund-10) <= 9.5)
    {
      ZahlScheibe1 = 1; 
      ZahlScheibe2 = GradCelsiusRund - 10;
    }

  if (((GradCelsiusRund-20) >= 0) and (GradCelsiusRund-20) <= 9.5)
    {
      ZahlScheibe1 = 2;
      ZahlScheibe2 = GradCelsiusRund - 20;
    }

  if (((GradCelsiusRund-30) >= 0) and (GradCelsiusRund-30) <= 9.5)
    {
      ZahlScheibe1 = 3;
      ZahlScheibe2 = GradCelsiusRund - 30;
    }  
}



void StepperNullPosAnfahren()
{
  //Stepper1
  digitalWrite(EnableHBridge1Pin,HIGH);
  bool Gabel1 = digitalRead(NullPosGabel1Pin);
  while (Gabel1 != HIGH) //Schleifendurchlauf solange Gabel1 ungleich HIGH
    {
      Stepper1.step(-1);
      Gabel1 = digitalRead(NullPosGabel1Pin); 
    }   
  //Stepper2
  digitalWrite(EnableHBridge2Pin,HIGH);
  bool Gabel2 = digitalRead(NullPosGabel2Pin);
  while (Gabel2 != HIGH) //Schleifendurchlauf solange Gabel2 ungleich HIGH
    {
      Stepper2.step(-1);
      Gabel2 = digitalRead(NullPosGabel2Pin); 
    }
}



void StepperSchritteBerechnen()
{
  Stepper1Schritte = Stepper1Offset + ZahlScheibe1 * Stepper1Delta;
  Stepper2Schritte = Stepper2Offset + Stepper2Delta + (ZahlScheibe2*2)*Stepper2Delta;
}







void StepperSchritteAusfuehren()
{
  digitalWrite(EnableHBridge1Pin,HIGH);
  Stepper1.step(Stepper1Schritte);
  digitalWrite(EnableHBridge1Pin,LOW);
  delay(100);
  digitalWrite(EnableHBridge2Pin,HIGH);
  Stepper2.step(Stepper2Schritte);
  digitalWrite(EnableHBridge2Pin,LOW);
  delay(100);
}
