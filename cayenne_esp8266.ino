
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>    //CayenneMQTT library 
#include <Servo.h>
#include <DHT.h>                    //DHT library 
#define DHTTYPE DHT22
#define DHTPIN 4
DHT dht(DHTPIN,DHTTYPE);
int topl,topr,botl,botr;
int threshold_value=10;        
float hum;
float temp;
//variable of voltage sensor
const int voltageSensor = 15;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;


// Wifi Network
char ssid[]="Redmi_6E18";
char wifiPassword[]="walaoA4896";

//MQTT credentials   
char username[]="2b3c7d60-30d3-11ec-bbfc-979c23804144";
char password[]="be4c0cc6d2770a12143b45246cd240541eaa8bf0";
char clientID[]="a7797700-47b9-11ec-9f5b-45181495093e";

Servo servo_x;                   //up-down servomotor  
int servoh = 0;
int servohLimitHigh = 170;     
int servohLimitLow = 10;       

Servo servo_z;                   //left-right servomotor 
int servov = 0; 
int servovLimitHigh = 170;
int servovLimitLow = 10;

void setup()
{ Serial.begin(9600);
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  servo_x.attach(13);//d13
  servo_z.attach(12);//d8
  dht.begin();
  pinMode(2,OUTPUT);//d6
  digitalWrite(2,LOW); 
  Serial.print("Setup");
}

void loop()
{ topr= analogRead(33);//d0       
  topl= analogRead(32);//d1   
  botr= analogRead(35);//d2  
  botl= analogRead(34);//d4  
  value = analogRead(voltageSensor);
  vOUT = (value * 5.0) / 6250.0;
  vIN = vOUT / (R2/(R1+R2));
  delay(500);
  Cayenne.loop();

  if (digitalRead(2)==LOW){
  Serial.println("Manual-mode");
  Serial.println(topr);
  Serial.println(topl);
  Serial.println(botr);
  Serial.println(botl);
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  Serial.println(vOUT);
  Serial.print("Input = ");
  Serial.println(vIN);
  }
  else if (digitalRead(2)==HIGH){
  Serial.println(" Automatic-mode");
  Serial.println(topr);
  Serial.println(topl);
  Serial.println(botr);
  Serial.println(botl);
  Serial.println(botl);
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
    servoh = servo_x.read();
    servov = servo_z.read();
    int avgtop = (topr + topl) / 2;     
    int avgbot = (botr + botl) / 2;   
    int avgright = (topr + botr) / 2;   
    int avgleft = (topl + botl) / 2;    
    int diffhori= avgtop - avgbot;      
    int diffverti= avgleft - avgright;    
    
    /*tracking according to horizontal axis*/ 
    if (abs(diffhori) <= threshold_value)
    {
     servo_x.write(servoh);            //stop the servo up-down
    }else {
       if (diffhori > threshold_value)
          { Serial.println(" x - 2 ");
          servo_x.write(servoh -2);    //Clockwise rotation CW
          if (servoh > servohLimitHigh)
          {
           servoh = servohLimitHigh;
          }
          delay(10);
          }else {
           servo_x.write(servoh +2);   //CCW
           if (servoh < servohLimitLow)
           {
           servoh = servohLimitLow;
           }
           delay(10);
           }
      }      
    /*tracking according to vertical axis*/ 
    if (abs(diffverti) <= threshold_value)
    {     
     servo_z.write(servov);       //stop the servo left-right
    }else{
       if (diffverti > threshold_value)
       { 
       servo_z.write(servov -2);  //CW
       if (servov > servovLimitHigh) 
       { 
       servov = servovLimitHigh;
       }
       delay(10);
       }else{ 
        servo_z.write(servov +2);  //CCW
        if (servov < servovLimitLow) 
        {
        servov = servovLimitLow;
        }
        delay(10);
        }
     }
  }
}
// Cayenne Functions
CAYENNE_IN(8){
  int value = getValue.asInt();
  CAYENNE_LOG("Channel %d, pin %d, value %d", 8, 3, value);
  digitalWrite(2,value);
}
CAYENNE_IN(7){ //up-down servo motor
  if (digitalRead(2)==HIGH){ //Automatic_mode
  }
  else{ //Manual_mode
  servo_x.write(getValue.asDouble() * 180);
  }
}
CAYENNE_IN(6){ //left-right servo motor
  if (digitalRead(5)==HIGH){
  }  
  else{
  servo_z.write(getValue.asDouble() * 180);
  }
}

CAYENNE_OUT(0) { //Current
  float current = vIN/10;
  Cayenne.virtualWrite(0, current);
  Serial.print("Current: ");
  Serial.println(current);
}
CAYENNE_OUT(1) { //Voltage
  float voltage = vIN * 2;
  Cayenne.virtualWrite(1, voltage);
  Serial.print("Voltage: ");
  Serial.println(voltage);
}
CAYENNE_OUT(2){ //LDR Top-right
  Cayenne.virtualWrite(2, topr);
}
CAYENNE_OUT(3){ //LDR Top-left
  Cayenne.virtualWrite(3,topl);
}
CAYENNE_OUT(4){ //LDR Bot-left
  Cayenne.virtualWrite(4,botl);
}
CAYENNE_OUT(5){ //LDR Bot-right
  Cayenne.virtualWrite(5,botr);
}
CAYENNE_OUT(10) { //Power
  float power = (vIN * 2 * vIN)/10 ;
  Cayenne.virtualWrite(10, power);
  Serial.print("Power: ");
  Serial.println(power);
}
CAYENNE_OUT(11){ //Temperature
  float t = dht.readTemperature();
  //int chk = dht.read(DHT11PIN);
  Cayenne.virtualWrite(11, t, TYPE_TEMPERATURE, UNIT_CELSIUS);
  Serial.print("temperature: ");
  Serial.println(t);
}
CAYENNE_OUT(12){ //Huidity
  float h = dht.readHumidity();
  //int chk = dht.read(DHT11PIN);
  Cayenne.virtualWrite(12, h);
  Serial.print("  humidity: ");
  Serial.println(h);
}
