#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DHT.h>
#include <ArduinoJson.h>
#define DHTTYPE DHT22
#define DHTPIN  5

#define ECHO 4
#define TRIGGER 13

#define GPIO 2
#define GPIO_RELAY 12

/////////////////////////////////////////////////////////////////////////////////////////
// Access point credential and port Servers
const char* ssid = "Livebox";
const char* password = "youfuckingpassword";
WiFiServer server(80);
WiFiClient client;
//////////////////////////////////////////////////////////////////////////////////////////

DHT dht(DHTPIN, DHTTYPE, 11);            // 11 works fine for ESP8266

float humidity, temp_f;                  // Values read from sensor
unsigned long previousMillis_DHT = 0;        // will store last temp was read
unsigned long previousMillis_SR04 = 0;        // will store last temp was read
const long interval = 3000;              // interval at which to read sensor
long distance ;
int stop = 0 ;                           // Stopp Web Client
boolean LED_state[4] = {0};
char req_index = 0; 
String HTTP_req;


JsonObject& prepareResponse(JsonBuffer& jsonBuffer) {
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& Level = root.createNestedArray("Level");
    Level.add(distance);
  JsonArray& tempValues = root.createNestedArray("temperature");
    tempValues.add(temp_f);
  JsonArray& humiValues = root.createNestedArray("humidity");
    humiValues.add(humidity);
  JsonArray& WaterPump = root.createNestedArray("WaterPump");
    WaterPump.add(LED_state[0]);
  JsonArray& EsPvValues = root.createNestedArray("Systemv");
    EsPvValues.add("ESP8266-12E");
  return root;
}

void setup() {

pinMode(GPIO, OUTPUT);									
pinMode(GPIO_RELAY, OUTPUT);									

pinMode(TRIGGER, OUTPUT);
pinMode(ECHO, INPUT);

digitalWrite(GPIO, LOW);
digitalWrite(GPIO_RELAY, LOW);
//////////////////////////////////////////////////////////////////////////////////////////
  											//
  Serial.begin(115200);									//
  Serial.println();									//

/* 											// 
/// WIFI////										//
  WiFi.begin(ssid, password);								//
  Serial.println("");									//
											//
// Wait for connection									//
  while (WiFi.status() != WL_CONNECTED) {						//
    delay(500);										//	
    Serial.print(".");									//	
  }
*/
Wificonnection() ;											//
// OTA Upgrade										//
   ArduinoOTA.onStart([]() {								//
    stopclient();  
    Serial.println("Start");								//
  });											//
  ArduinoOTA.onEnd([]() {								//
    Serial.println("\nEnd");								//
  });											//
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {			//
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));			//
  });											//
  ArduinoOTA.onError([](ota_error_t error) {						//
    Serial.printf("Error[%u]: ", error);						//
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");				//
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");			//
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");		//
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");		//
    else if (error == OTA_END_ERROR) Serial.println("End Failed");			//
    ESP.restart();
  });											//
  ArduinoOTA.begin();									//
//  											//
  Serial.println("");									//
  Serial.print("Connected to ");							//
  Serial.println(ssid);									//
  Serial.print("IP address: ");								//
  Serial.println(WiFi.localIP());							//
// Start the server									//
  server.begin();									//
  Serial.println("Server started");							//
//////////////////////////////////////////////////////////////////////////////////////////

  
}

void loop() {
if (WiFi.status() != WL_CONNECTED) Wificonnection() ;
  //////////////////////////////////////////////////////////////////////////////////////////
  ArduinoOTA.handle();
  //////////////////////////////////////////////////////////////////////////////////////////
  if ( stop !=1 ) { 
    WiFiClient client = server.available();
    if (client) { 
	boolean currentLineIsBlank = true;
        while (client.connected()) {
  //////////////////////////////////////////////////////////////////////////////////////////
	ArduinoOTA.handle();
	if (stop == 1) client.stop();
  //////////////////////////////////////////////////////////////////////////////////////////
	    if (client.available() && stop != 1 ) { 
                char c = client.read();
                HTTP_req += c;

                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
			if (HTTP_req.indexOf("gpio=1") > -1){
			Serial.println("Request GET GPIO ON ");		
	                digitalWrite(GPIO, HIGH);
	                digitalWrite(GPIO_RELAY, HIGH);
			LED_state[0] = 1;
			HTTP_req = "";
                	break;
        	        }
                	else if (HTTP_req.indexOf("gpio=0") > -1){
                	digitalWrite(GPIO, LOW);
                	digitalWrite(GPIO_RELAY, LOW);
			Serial.println("Request GET GPIO OFF");		
			LED_state[0] = 0;
			HTTP_req = "";
                    	break;
                	}

			else if (HTTP_req.indexOf("reset") > -1){
			ESP.restart();
			HTTP_req = "";
			}

			else if (HTTP_req.indexOf("getJSON") > -1) {
			//client.println(millis()/10000);
			gettemperature();
      			getdistance();
			StaticJsonBuffer<500> jsonBuffer;
      			JsonObject& json = prepareResponse(jsonBuffer);
      			writeResponse(client, json);
			Serial.println("request JSON") ;
			HTTP_req = "";
			break;
			}	

 		    if (HTTP_req.indexOf("ajax_switch") > -1) {
		     if (stop == 1) client.stop();
	             client.println("Content-Type: text/xml");
                     client.println("Connection: keep-alive");
		     client.println();
	             SetLEDs();
                     XML_response(client);
			} else {
			if (stop == 1) client.stop();
                        client.println("<!DOCTYPE html>");
                        client.println("Content-Type: text/html");
                    	client.println("Connection: keep-alive");
			client.println();
			client.println("<html>");
                        client.println("<head>");
                        client.println("<title>ESP 8266 web display</title>");
                        //begin script
			client.println("<script>");
                        
			client.println("strLED1 = \"\";");
                        //function main
			client.println("function getTemperature() {");
                        client.println("nocache = \"&nocache=\"+ Math.random() * 1000000;");
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");

                        client.println("if (this.responseXML != null) {");
                        client.println("if (this.responseXML.getElementsByTagName('LED')[0].childNodes[0].nodeValue === \"checked\") {");
                        client.println("document.LED_form.LED1.checked = true; }");
                        client.println("else {");
                        client.println("document.LED_form.LED1.checked = false ;}");
                        client.println("}");
                        //Text
                        client.println("var num_an = this.responseXML.getElementsByTagName('analog').length;");
			client.println("document.getElementsByClassName(\"analog\")[0].innerHTML = this.responseXML.getElementsByTagName('analog')[0].childNodes[0].nodeValue;");
			client.println("document.getElementsByClassName(\"temp\")[0].innerHTML = this.responseXML.getElementsByTagName('temp')[0].childNodes[0].nodeValue;");
			client.println("document.getElementsByClassName(\"humidity\")[0].innerHTML = this.responseXML.getElementsByTagName('hum')[0].childNodes[0].nodeValue;");
			
			client.println("document.getElementById(\"avancement\").value = this.responseXML.getElementsByTagName('analog')[0].childNodes[0].nodeValue;");
                        client.println("}}}");
			//End Fucntion Main
			client.println("request.open(\"GET\", \"ajax_switch\" + strLED1 + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('getTemperature()', 1000);");
                        client.println("strLED1 = \"\";");
                        client.println("}");
                        //function getCheck()
                        client.println("function GetCheck() {");
                        client.println("if (LED_form.LED1.checked) {");
                        client.println("strLED1 = \"&LED1=1\"; }");
                        client.println("else {");
                        client.println("strLED1 = \"&LED1=0\";");
                        client.println("}");
			//end Function
                        client.println("}");
			client.println("</script>");
                        client.println("</head>");
                        // END Script
			//HTML build
                        client.println("<body onload=\"getTemperature()\">");
                        client.println("<h1>ESP8266  web server</h1>");
                       	client.println("<font color=\"#6a5acd\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n</div>\n<div style=\"clear:both;\"></div><p>");                                               
                       //client.println("<p id=\"switch_txt\">BMPSTATUS/p>");
                       //client.println("<p>Example Display of BMP data</p>");
                        	
			client.println("<div class=\"IO_box\">");
                        client.println("<font size = \"5\">Temperature: <span class=\"temp\">...</span> &deg;C  </font>");
			client.println("<br>");
			client.println("<br>");
                        client.println("<font size = \"5\">humidity: <span class=\"humidity\">...</span> % </font>");
                        client.println("<p>Level: <span class=\"analog\">...</span></p>");
                	client.println("</div>");

 			client.println("<p>Remplissage:");
        		client.println("<progress id=\"avancement\" value=\"50\" max=\"100\"></progress>");

		//	client.println("<p><a href=\"http://www.esp8266.com\">ESP8266 Support Forum</a></p>");
                 	client.println("<h1>WATER PUMP</h1>");
                 	client.println("<p>Click to switch water pump On and Off.</p>");
                 	client.println("<div class=\"IO_box\">");
                 	client.println("<form id=\"check_LEDs\" name=\"LED_form\">");
                 	client.println("<input type=\"checkbox\" name=\"LED1\" value=\"0\" onclick=\"GetCheck()\" /> Water pump<br /><br />");
                 	client.println("</form>");
                 	client.println("</div>");
                        client.println("</body>");
                        client.println("</html>");
                        }	
		    if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                    } 
                    else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                    }
		    HTTP_req = "";
                    break;
                }
            }
        }
        delay(1);
        client.stop();
    }
 }
}

void SetLEDs(void)
{
      if (HTTP_req.indexOf("LED1=1") > -1 ){
	 LED_state[0] = 1;  // save LED state
        digitalWrite(GPIO_RELAY, HIGH);
        digitalWrite(GPIO, HIGH);
    }
        if (HTTP_req.indexOf("LED1=0") > -1 ){
	 LED_state[0] = 0;  // save LED state
        digitalWrite(GPIO, LOW);
        digitalWrite(GPIO_RELAY, LOW);
    }

}

void XML_response(WiFiClient cl)
{
 if ( stop != 1) {
    gettemperature();	
    getdistance();        
    cl.print("<?xml version = \"1.0\" ?>");
    cl.print("<inputs>");

    // checkbox LED states
    cl.print("<LED>");
    if (LED_state[0]) {
        cl.print("checked");
    }
    else {
        cl.print("unchecked");
    }
    cl.println("</LED>");
    cl.println("<analog>");
    cl.print(- (distance));	
    cl.println("</analog>");
    cl.print("<temp>");
	cl.print(temp_f);
    cl.println("</temp>");
    cl.println("<hum>");
	cl.print(humidity);
    cl.println("</hum>");
    cl.println("</inputs>");
 }
}

int remplissage(){
static uint32_t p ;

p++ ;

 if ( p > 100) {
 p = 0 ;
 }

return p ;

}    


void writeResponse(WiFiClient& client, JsonObject& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  json.prettyPrintTo(client);
}

void gettemperature() {
 if ( stop != 1) {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis_DHT >= interval) {
    // save the last time you read the sensor 
    previousMillis_DHT = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature();     // Read temperature as Fahrenheit
    // Serial.println(humidity);
    // Serial.println(temp_f);
	// Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  } 
 }
}

void getdistance() {
 if ( stop != 1) {
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis_SR04 >= interval) {
    // save the last time you read the sensor 
    previousMillis_SR04 = currentMillis;

  long duration ; 
  digitalWrite(TRIGGER, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  
//  Serial.print(distance);
//  Serial.println("Centimeter:");
  distance = (distance * 100/35) - 100 ; // For my case, a 35cm height tank 
  }
 }
}
void stopclient () {

stop = 1 ;
Serial.println("Stopping Web client...");
Serial.println("Wait...");
WiFiUDP::stopAll();
WiFiClient::stopAll();
}

void Wificonnection() {
 WiFi.begin(ssid, password);                                                           //
 Serial.println("");                                                                   //
// Wait for connection                                                                  //
  while (WiFi.status() != WL_CONNECTED) {                                               //
    delay(500);                                                                         //
    Serial.print(".");                                                                  //
  }
}
