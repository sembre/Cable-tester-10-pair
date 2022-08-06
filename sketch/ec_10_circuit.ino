#include <LiquidCrystal.h>
// Sambungan pin pins LCD
// LCD 16 X 2 -----> ARDUINO
// 1 -----> GND          11 -----> 9(D4)
// 2 -----> +5V          12 -----> 10(D5)
// 3 -----> 6(CONTRAS)   13 -----> 11(D6)
// 4 -----> 7(RS)        14 -----> 12(D7)
// 5 -----> GND          15 -----> 5(BACKLIGHT)
// 6 -----> 8(E)         16 -----> GND
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //RS,E,D4,D5,D6,D7
//pins SIDE A , SIDE B
int endA[10] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; //pins end A 15CZ-6Y No 1,... ,
int endB[10] = {A9, A8, A7, A6, A5, A4, A3, A2, A1, A0}; //pins end B 15CZ-6H No 15,... ,

//Membuat variabel untuk LED
  int pinLed1 = 42; //KELUARAN BISA DI SAMBUNG DENGAN LED No.1
  int pinLed2 = 43;
  int pinLed3 = 44;
  int pinLed4 = 45;
  int pinLed5 = 46;
  int pinLed6 = 47;
  int pinLed7 = 48;
  int pinLed8 = 49;
  int pinLed9 = 50;
  int pinLed10 = 51;//KELUARAN BISA DI SAMBUNG DENGAN LED No.10

// Membuat variabel untuk Button atau baris S
  int pinBtn1 = 32; // STOP NO 1
  int pinBtn2 = 33;
  int pinBtn3 = 34;
  int pinBtn4 = 35;
  int pinBtn5 = 36;
  int pinBtn6 = 37;
  int pinBtn7 = 38;
  int pinBtn8 = 39;
  int pinBtn9 = 40;
  int pinBtn10 = 41;


#define NUMBER (Nu) // hasil hitungan untuk S dari sambungan s ke GND
int Nu = 10; // tanpa di sambung ke GND hitungan menjadi 10
int result[10] = {-1,-1, -1,-1,-1,-1,-1,-1,-1,-1};
int test[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int counter[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool fail =false;
int Contrast = 60;
#define LCD_Backlight 5
const int speakerPin = 4;
int frequency ;
int POT = A10;
int potVal = analogRead(POT);
const int BUZZER_PIN = 13; // Arduino pin connected to Buzzer's pin


void setup() {
  frequency=map(potVal,0,1023,0,255);//scalarizing the value of the potentiometer into PWM values
  pinMode(speakerPin, OUTPUT); //OUT Speaker headphone
  analogWrite(6, Contrast); //Pin for LCD_Backlight 
  pinMode(LCD_Backlight, OUTPUT); 
  analogWrite(LCD_Backlight, 400);//Adjust for LCD_Backlight

 // variabel pinLed menjadi output digunakan untuk hitungan S
  pinMode(pinLed1, OUTPUT); //KELUARAN LED 1-10
  pinMode(pinLed2, OUTPUT);
  pinMode(pinLed3, OUTPUT);
  pinMode(pinLed4, OUTPUT);
  pinMode(pinLed5, OUTPUT);
  pinMode(pinLed6, OUTPUT);
  pinMode(pinLed7, OUTPUT);
  pinMode(pinLed8, OUTPUT);
  pinMode(pinLed9, OUTPUT);
  pinMode(pinLed10, OUTPUT);
 
  // variabel pinBtn menjadi input
  pinMode(pinBtn1, INPUT_PULLUP); // BARIS S 1-10 satu satu DISAMBUNG DENGAN GND untuk hasilkan angka
  pinMode(pinBtn2, INPUT_PULLUP);
  pinMode(pinBtn3, INPUT_PULLUP);
  pinMode(pinBtn4, INPUT_PULLUP);
  pinMode(pinBtn5, INPUT_PULLUP);
  pinMode(pinBtn6, INPUT_PULLUP);
  pinMode(pinBtn7, INPUT_PULLUP);
  pinMode(pinBtn8, INPUT_PULLUP);
  pinMode(pinBtn9, INPUT_PULLUP);
  pinMode(pinBtn10, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT); // DISAMBUNG DENGAN TRANSISTOR SWITCH KEMUDIAN KE RELAY
  Serial.begin(115200); //serial used for debugging only / hasil lihat di omputer
  lcd.begin(16, 2); 
  //setup pins  
  for(int i=0; i< NUMBER; i++){
      pinMode(endA[i], OUTPUT);//set output pins (end A)
      pinMode(endB[i], INPUT_PULLUP);//set input pins (end B)
  }
}
void loop() {

  int statusBtn1 = digitalRead(pinBtn1); // kondisi S di sambung ke GND
  int statusBtn2 = digitalRead(pinBtn2);
  int statusBtn3 = digitalRead(pinBtn3);
  int statusBtn4 = digitalRead(pinBtn4);
  int statusBtn5 = digitalRead(pinBtn5);
  int statusBtn6 = digitalRead(pinBtn6);
  int statusBtn7 = digitalRead(pinBtn7);
  int statusBtn8 = digitalRead(pinBtn8);
  int statusBtn9 = digitalRead(pinBtn9);
  int statusBtn10 = digitalRead(pinBtn10);

  // Ketika push button tidak ditekan nilainya HIGH/1
   if(statusBtn1 == HIGH)
    {
      digitalWrite(pinLed1, LOW);
    } else {
      digitalWrite(pinLed1, HIGH);
      1;
      Nu = 1;
    }
    if(statusBtn2 == HIGH) {
      digitalWrite(pinLed2, LOW);
    } else{
      digitalWrite(pinLed2, HIGH);
      2;
      Nu = 2;
    }
    if(statusBtn3 == HIGH) {
      digitalWrite(pinLed3, LOW);
    } else {
      digitalWrite(pinLed3, HIGH);
      3;
      Nu = 3;
    }
    if(statusBtn4 == HIGH) {
      digitalWrite(pinLed4, LOW);
    } else {
      digitalWrite(pinLed4, HIGH);
      4;
      Nu = 4;
    }
    if(statusBtn5 == HIGH)
    {
      digitalWrite(pinLed5, LOW);
    } else {
      digitalWrite(pinLed5, HIGH);
      5;
      Nu = 5;
    }
    if(statusBtn6 == HIGH) {
      digitalWrite(pinLed6, LOW);
    } else{
      digitalWrite(pinLed6, HIGH);
      6;
      Nu = 6;
    }
    if(statusBtn7 == HIGH) {
      digitalWrite(pinLed7, LOW);
    } else {
      digitalWrite(pinLed7, HIGH);
      7;
      Nu = 7;
    }
    if(statusBtn8 == HIGH) {
      digitalWrite(pinLed8, LOW);
    } else {
      digitalWrite(pinLed8, HIGH);
      8;
      Nu = 8;
    }
    if(statusBtn9 == HIGH)
    {
      digitalWrite(pinLed9, LOW);
    } else {
      digitalWrite(pinLed9, HIGH);
      9;
      Nu = 9;
    }
    if(statusBtn10 == HIGH) {
      digitalWrite(pinLed10, LOW);
    } else{
      digitalWrite(pinLed10, HIGH);
      10;
      Nu = 10;
    }
      
  String resultS="";
  //user interface
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cek:");
  lcd.setCursor(6,0);
  lcd.print (NUMBER);
  lcd.print(" Circuit");
Serial.print("  Check :  ");
Serial.print (NUMBER); 
Serial.println (" Circuit");  
  lcd.setCursor(0,1);
  for(int i=0; i<NUMBER; i++){
     counter[i]=0;
    for(int j=0; j<NUMBER; j++){
        digitalWrite(endA[i], LOW); //set all outputs to LOW
    }
    for(int j=0; j<NUMBER; j++){  //check for crossed / open circuits vs closed, good, circuits
        digitalWrite(endA[j], HIGH); //cek input satu persatu
        test[i] = digitalRead(endB[i]); //read the output
        if(test[i] == 1 && j!=i){ //crossed or open circuit
          counter[i]++;
          result[i] = 2*NUMBER+j; 
        }
        else if(test[i] == 1 && j==i && result[i] <2*NUMBER ){ //Good, closed circuit
          result[i] = 0; 
        }
        digitalWrite(endA[j],LOW);
        //debugging
        /*
          Serial.print("test1 input core  ");
          Serial.print(i);
          Serial.print(" with output core  ");
          Serial.print(j);
          Serial.print(" test =");
          Serial.print(test[i]);
          Serial.print(" counter =");
          Serial.println(counter[i]);*/
     
   } 
    Serial.print("Core ");
    Serial.print(i);
    Serial.print (" Ke ");
    Serial.print (i);
    Serial.print(" result = ");    
    if(result[i] == 0){
       Serial.println(" Good");
       resultS+="1";
    }
    else if(counter[i] == NUMBER-1){        
       Serial.println(" Open");
       resultS+="O";
       lcd.setCursor(10,1);
       lcd.print("=OPEN ");
       fail=true;

    }
    else{
       Serial.println(" Cross");
       resultS+="X";
       lcd.setCursor(10,1);
       lcd.print("=CROSS");
       fail=true;
    }
  }  
 if(fail){
  digitalWrite(13, LOW);//Pin 13 MATI BUZZER MATI
   Serial.println("FAILED"); 
    lcd.setCursor(0,1);
    lcd.print(resultS);
    noTone (speakerPin); 
 }
 else{
    digitalWrite(13, HIGH);//Pin 13 Hidup , BUZZER NYALA
    Serial.println("PASSED");
    lcd.print("     PASSED"); 
    tone(speakerPin,frequency);//using tone function to generate the tone of the frequency given by POT

 }
 void(* resetFunc) (void) = 0; //declare reset function @ address 0
  Serial.println("resetting");
delay(600);
 resetFunc();  //call reset
  } 
