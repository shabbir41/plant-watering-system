/*
 * Project AWSIoTCore
 * Description:
 * Author:
 * Date:
 * Ref: https://github.com/hirotakaster/MQTT-TLS/blob/master/examples/a2-example/a2-example.ino
 */

#include "application.h"
#include "MQTT-TLS.h"
#include "keys.h"

SerialLogHandler logHandler;

void callback(char *topic, byte *payload, unsigned int length);

const char amazonIoTRootCaPem[] = AMAZON_IOT_ROOT_CA_PEM;
const char clientKeyCrtPem[] = CELINT_KEY_CRT_PEM;
const char clientKeyPem[] = CELINT_KEY_PEM;

MQTT client("a1ajexr64dg511-ats.iot.us-east-1.amazonaws.com", 8883, callback);


//Sensor and Board Data
int moisturePin = A1;
int moistureValue = 0;
int outp = A5;
int DRYVALUE = 3500;
int WETVALUE = 1600;
int DELAY_CONST = 10000;
float moisturePercentage = 0;
bool externalCommand = false;
bool motorStatus = false;
bool startPublish = false;

Timer timer_sense(DELAY_CONST, senseData);
// Timer timer_publish(DELAY_CONST, publishData);
Timer timer_blinker(500, blinker);

void blinker(){
  if(digitalRead(D7) == LOW){
    digitalWrite(D7, HIGH);
  } else {
    digitalWrite(D7, LOW);
  }
}

void senseData(){

    moistureValue = analogRead(moisturePin);

    if(moistureValue < WETVALUE){
      Log.info("Sensor is not connected Properly..!!");

    } else {
      moisturePercentage = map(moistureValue, WETVALUE, DRYVALUE, 100, 0);

      if (moisturePercentage < 35){
        digitalWrite(outp, HIGH);
        motorStatus = true;
      } else {
        digitalWrite(outp, LOW);
        motorStatus = false;
      }
    
      startPublish = true; 
    }
}

// recieve message
void callback(char *topic, byte *payload, unsigned int length)
{
  Log.info("Received Data on topic %s", topic);
  String topic_rec = String(topic);
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  if(message.indexOf("toggle") != -1){
    int status = switchMotor("toggle");
  }
}

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();
int counter = 0;
String command = "on";

void setup()
{

  pinMode(moisturePin, INPUT);
  pinMode(outp, OUTPUT);
  pinMode(D7, OUTPUT);

  Particle.function("motorSwitch", switchMotor);

  if (millis() - lastSync > ONE_DAY_MILLIS)
  {
    Particle.syncTime();
    lastSync = millis();
  }

  // enable tls. set Root CA pem, private key file.
  client.enableTls(amazonIoTRootCaPem, sizeof(amazonIoTRootCaPem),
                   clientKeyCrtPem, sizeof(clientKeyCrtPem),
                   clientKeyPem, sizeof(clientKeyPem));
  Log.info("tls enable");

  // connect to the server
  // client.connect("sparkclient");

  bool status = client.connect("aws_" + String(Time.now()));

  if (status)
  {
    Log.info("Connection is completed");
  }
  else
  {
    Log.info("Connection is not completed");
  }

  // publish/subscribe
  if (client.isConnected())
  {
    Log.info("Client connected to AWS IoT Core..");
    // client.publish("moisture/data", "-1");
    client.subscribe("moisture/data");
  }

  timer_sense.start();
}

int switchMotor(String cmd){
  externalCommand = true;
  return 1;
}


void publishData(){
    if (startPublish == true){
    Log.info("Sensed Data: %d", moistureValue);
    String dataString = "{\"moisture\": \"" + String(moisturePercentage) + "\", "; 
    dataString = dataString + "\"motorStatus\": \"" + (motorStatus? "On":"Off") + "\" }";
    
    if(client.isConnected()){
    
      Log.info("Published Data: %s", dataString);
      client.publish("moisture/data", dataString);
    } else {
      Log.info("Reconnection Required..!!");
      timer_blinker.start();
      timer_sense.stop();
      if (client.connect("aws_" + String(Time.now()))){
        Log.info("Connected.. Again");
        timer_blinker.stop();
        timer_sense.start();
      } else {
        Log.info("REconnection Failed");
      }
    }
  } else {
    return;
  }
}

void loop()
{
  client.loop();
  delay(1000);
    if(externalCommand == false){
      if (startPublish == false){
        timer_sense.start();
      }
      publishData();
      delay(DELAY_CONST);
    } else {
      timer_sense.stop();
      if(motorStatus == true){
        digitalWrite(outp, LOW);
        motorStatus = false;
      } else {
        digitalWrite(outp, HIGH);
        motorStatus = true;
      }
      externalCommand = false;
      delay(5000);
      timer_sense.start();
    }

}