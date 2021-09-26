#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

//api
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

SoftwareSerial s(D6, D5);
int data;

const char *ssid =  "Kamar eja";
const char *pass =  "##1234##";

WiFiClient client;

HTTPClient http;

float ph_value=-1;
int ldr_value=-1;
int intensity_value=-1;
float temperature_value=-1;
int tds_value=-1;
int water_value=-1;

float temp_temperature=-1;
int temp_tds=-1;
int temp_water=-1;

void setup() {
  s.begin(9600);
  Serial.begin(9600);
  delay(5000);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // GET
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  
}

bool getDone = false;

void loop() {
  while(!getDone){
    getArduino();
  }

  ph_value=ph_value/10;
  Serial.print("ph_value= ");
  Serial.println(ph_value);
  
  Serial.print("ldr_value= ");
  Serial.println(ldr_value);
  
  Serial.print("intensity_value= ");
  Serial.println(intensity_value);

  temperature_value=temperature_value+ (temp_temperature/10);
  Serial.print("temperature_value= ");
  Serial.println(temperature_value);

  tds_value=(tds_value*10)+temp_tds;
  Serial.print("tds_value= ");
  Serial.println(tds_value);

  water_value=(water_value*10)+temp_water;
  Serial.print("water_value= ");
  Serial.println(water_value);

// send api

  delay(1000);
  //POST
  while(WiFi.status() != WL_CONNECTED){}
  if(WiFi.status()== WL_CONNECTED){
     // GET
    String serverName = "http://23.97.53.0:5000/data-ml/last";
    http.begin(serverName);
    int httpCode = http.GET();
    String payload;
    DynamicJsonDocument res(1024);

    if(httpCode >0 ){
      payload = http.getString();
      deserializeJson(res, payload);
    }else{
      Serial.println("error gais");
      Serial.println(httpCode);
    }
    
    int result = res["data"]["for_mcu"];
    Serial.println(payload); 
    Serial.print("Result: ");
    Serial.println(result);
    // delay
    Serial.println("");
    sendArduino(result);
    
    // POST
    String dataString = createJsonData(ph_value, ldr_value, intensity_value, temperature_value, tds_value, water_value);
    Serial.println(dataString);
    
    http.addHeader("Content-Type", "application/json");
    
    serverName = "http://23.97.53.0:5000/data";
    http.begin(serverName);
    
    httpCode = http.POST(dataString);
    
    if(httpCode >0 ){
      payload = http.getString();
    }else{
      Serial.println("error gais");
      Serial.println(httpCode);
    }
    Serial.println(payload);      
  }
  
  
  getDone = false;
  
  ph_value=-1;
  ldr_value=-1;
  intensity_value=-1;
  temperature_value=-1;
  tds_value=-1;
  water_value=-1;

  temp_temperature=-1;
  temp_tds=-1;
  temp_water=-1;
  delay(30000);
}

String createJsonData(float ph_value, int ldr_value, int intensity_value, float temperature_value, int tds_value, int water_value){
  DynamicJsonDocument jsonData(1024);

  jsonData["tds"] = tds_value;
  jsonData["ph_meter"] = ph_value;
  jsonData["temp"] = temperature_value;
  jsonData["water"] = water_value;
  jsonData["water_flow"] = intensity_value;
  jsonData["light"] = ldr_value;
  String stringData;
  serializeJson(jsonData, stringData);
  return stringData;
}

void getArduino(){
  if (s.available()>0) {
      int valueGet = s.read();
    if(ph_value<0){
      ph_value = valueGet;
    }else if(ph_value>=0 && ldr_value < 0){
      ldr_value = valueGet;
    }else if(ldr_value>=0 && intensity_value < 0){
      intensity_value = valueGet;
    }else if(intensity_value>=0 && temperature_value < 0){
      temperature_value = valueGet;
    }else if(temperature_value>=0 && temp_temperature < 0){
      temp_temperature = valueGet;
    }else if(temp_temperature>=0 && tds_value < 0){
      tds_value = valueGet;
    }else if(tds_value>=0 && temp_tds < 0){
      temp_tds = valueGet;
    }else if(temp_tds>=0 && water_value < 0){
      water_value = valueGet;
    }else if(water_value>=0 && temp_water < 0){
      temp_water = valueGet;
     
      getDone = true;
    }
  }
}

void sendArduino(int result){
  //Send to arduino
  s.write(result); 
}
