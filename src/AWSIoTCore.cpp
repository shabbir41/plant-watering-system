/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/mshehab/Desktop/particle_apps/MQTT/AWSIoTCore/src/AWSIoTCore.ino"
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

void setup();
void loop();
#line 13 "/Users/mshehab/Desktop/particle_apps/MQTT/AWSIoTCore/src/AWSIoTCore.ino"
SerialLogHandler logHandler;

void callback(char *topic, byte *payload, unsigned int length);

const char amazonIoTRootCaPem[] = AMAZON_IOT_ROOT_CA_PEM;
const char clientKeyCrtPem[] = CELINT_KEY_CRT_PEM;
const char clientKeyPem[] = CELINT_KEY_PEM;

/**
 * if want to use IP address,
 * byte server[] = { XXX,XXX,XXX,XXX };
 * MQTT client(server, 1883, callback);
 * want to use domain name,
 * MQTT client("www.sample.com", 1883, callback);
 **/
MQTT client("a1myvp68229tdy-ats.iot.us-east-1.amazonaws.com", 8883, callback);

// recieve message
void callback(char *topic, byte *payload, unsigned int length)
{
  Log.info("Received Data on topic %s", topic);
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);

  if (message.equals("RED"))
    RGB.color(255, 0, 0);
  else if (message.equals("GREEN"))
    RGB.color(0, 255, 0);
  else if (message.equals("BLUE"))
    RGB.color(0, 0, 255);
  else
    RGB.color(255, 255, 255);
  delay(1000);
}

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();
int counter = 0;
void setup()
{
  if (millis() - lastSync > ONE_DAY_MILLIS)
  {
    Particle.syncTime();
    lastSync = millis();
  }

  RGB.control(true);

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
    Log.info("client connected");
    client.publish("sensor", "hello world");
    client.subscribe("led");
  }
}

void loop()
{
  if (client.isConnected())
  {
    client.loop();
  }
  delay(200);
}