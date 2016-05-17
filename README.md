# ESP8266-Server-HTML-JSON
Remote control an irrigation water pump through javascript Webpage or GET requests. 
Get temperature / humidity, and level of the water tank.

### ESP8266-12E

Framework used: Arduino

IDE used:
[PlateformIO](http://platformio.org/)

Upgrade OTA is available on this code 

Build and upgrade command:

``` bash
platformio run --target upload --upload-port IP_ESP
```
## Example :
```
http://IP/gpio=0
```
-> set GPIO to OFF
```
http://IP/gpio=1
```
--> set GPIO to ON
```
http://IP/getJSON
```
--> Request JSON :
``` JSON
{
  "temperature": [
    "20"
  ],
  "humidity": [
    "70"
  ],
  "WaterPump": [
    0
  ],
  "Systemv": [
    "ESP8266-12E"
  ]
```
```
http://IP/reset
```

--> Reset ESP
```
http://IP/
```

--> Display the WebPage !

## TODO
Add Temp & humidity Sensor (like DHT22)
Add level water mesurement through sonar sensor
