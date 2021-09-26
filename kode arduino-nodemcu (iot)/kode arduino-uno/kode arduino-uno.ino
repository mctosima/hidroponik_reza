#include <DHT.h>
#include <EEPROM.h>

#include <SoftwareSerial.h>
SoftwareSerial s(5, 6);

#define TDS A1
#define WATER A2
#define PH A3

#define LDR 12
#define DHTPIN 7
#define DHTTYPE DHT11

int count=0;
int temp_value=0,temp_temperature=0,temp_tds=0,temp_water=0;
int ml=-5;

//***************RELAY***************
#define RELAY_ON 0
#define RELAY_OFF 1
#define RELAY_1  3   
#define RELAY_2  4   

//***************PH SENSOR***************
float calibrationPh = 21.34;
int valPh = 0, buffer_arr[10],temp;
unsigned long int avgvalPh; 

//***************cahaya***************
int ldr;

//***************intensitas air***************
int waterFlow;

byte sensorInterrupt = 0;
byte sensorPin       = 2;
float flowRate;
float calibrationFactor = 4.5;
volatile byte pulseCount;  
unsigned long oldTime;

//***************suhu***************
DHT dht(DHTPIN,DHTTYPE);
float temperatures;

//***************ppm***************
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 28;

//***************tinggi air***************
int water;


void setup(){
  s.begin(9600);
  Serial.begin(9600);

//***************Relay***************
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
 
  // Initialize relay one as off so that on reset it would be off by default
  digitalWrite(RELAY_1, RELAY_OFF);
  digitalWrite(RELAY_2, RELAY_OFF);
  
//***************ph***************



//***************cahaya***************
  pinMode(LDR, INPUT);

//***************intensitas air***************
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  oldTime           = 0;
  
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

//***************suhu***************
  dht.begin();

//***************ppm***************
  pinMode(TDS,INPUT);

//***************tinggi air***************


//delay(5000);
}

void loop(){
  float ph_value=codePH();
  int ldr_value=codeLDR();
  int intensity_value=codeIntensity();
  float temperature_value=codeTemperature();
  int tds_value=codeTDS();
  int water_value=codeWater();

//  delay(5000);
  if(count>100){
    sendMcu(7, ldr_value, 1, 27, tds_value, water_value);
    ml = getMcu();
    Serial.println(ml);
    
//    count=0;
    aktuator();
    delay(300000);
    
  }count++;

}
void aktuator(){
  if(ml==1){
    digitalWrite(RELAY_1, RELAY_ON);
    digitalWrite(RELAY_2, RELAY_OFF);
  }else if (ml==2){
    digitalWrite(RELAY_2, RELAY_ON);
    digitalWrite(RELAY_1, RELAY_OFF);
  }else if (ml==3){
    digitalWrite(RELAY_1, RELAY_ON);
    digitalWrite(RELAY_2, RELAY_ON);
  }else{
    digitalWrite(RELAY_1, RELAY_OFF);
    digitalWrite(RELAY_2, RELAY_OFF);
  }
}

void sendMcu(float ph_value, int ldr_value, int intensity_value, float temperature_value, int tds_value, int water_value){
//Send to mcu

  delay(5000);
  ph_value=ph_value*10;
  s.write(ph_value);
  Serial.print("ph_value= ");
  Serial.println(ph_value);
  delay(1000);

  s.write(ldr_value);  
  Serial.print("ldr_value= ");
  Serial.println(ldr_value);
  delay(1000);
  
  s.write(intensity_value);
  Serial.print("intensity_value= ");
  Serial.println(intensity_value);
  delay(1000);

  temp_value=temperature_value*10;
  temp_temperature=temp_value%10;
  s.write(temperature_value);
  s.write(temp_temperature);
  Serial.print("temperature_value= ");
  Serial.println(temperature_value);
  Serial.println(temp_temperature);
  delay(1000);
  
  temp_tds=tds_value%10;
  tds_value=tds_value/10;
  s.write(tds_value);
  s.write(temp_tds);
  Serial.print("tds_value= ");
  Serial.println(tds_value);
  Serial.println(temp_tds);
  delay(1000);

  temp_water=water_value%10;
  water_value=water_value/10;
  s.write(water_value);
  s.write(temp_water);
  Serial.print("water_value= ");
  Serial.println(water_value);
  Serial.println(temp_water);
  delay(1000);
  Serial.println("-----------");
}

int getMcu(){
  //Get from mcu
  while(s.available()<=0){}
  int hasil_ml;
  if (s.available()>0) {
      hasil_ml=s.read();
  }
  Serial.println(hasil_ml);
  return hasil_ml;
}

float codePH(){
  for(int i=0;i<10;i++) { 
    buffer_arr[i]=analogRead(PH);
    delay(30);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buffer_arr[i]>buffer_arr[j]){
        temp=buffer_arr[i];
        buffer_arr[i]=buffer_arr[j];
        buffer_arr[j]=temp;
      }
    }
  }
  avgvalPh=0;
  
  for(int i=2;i<8;i++){
    avgvalPh+=buffer_arr[i];
  }
  float voltPh=(float)avgvalPh*5.0/1024/6;
  float pH = -5.70 * voltPh + calibrationPh;
  
  return pH;
}

int codeIntensity(){
  int x=0;
  if((millis() - oldTime) > 1000){ 
    detachInterrupt(sensorInterrupt);
        
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    oldTime = millis();
    
    x = (int(flowRate)); 
  
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
  return x;
}

int codeLDR(){
  ldr = digitalRead(LDR);
  return ldr;
//  Serial.println(ldr);
}

float codeTemperature(){
  temperatures = dht.readTemperature();
  return temperatures;
}

int codeTDS(){
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){         //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TDS);    //read the analog value and store into the buffer
    analogBufferIndex++;
    
    if(analogBufferIndex == SCOUNT){
      analogBufferIndex = 0;
    }
  }   
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    
    for(copyIndex=0;copyIndex<SCOUNT;copyIndex++){
      analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
    }
    
    averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient=1.0+0.02*(temperature-28.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
    tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
    
    return tdsValue;
   }
}

int codeWater(){
  water = analogRead(WATER);
  return water;
}

void pulseCounter(){
  pulseCount++;
}
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++){
    bTab[i] = bArray[i];
  }
  int i,j,bTemp;
  
  for (j = 0; j < iFilterLen - 1; j++){
    for (i = 0; i < iFilterLen - j - 1; i++){
      if (bTab[i] > bTab[i + 1]){
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else{
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  
  return bTemp;
}
