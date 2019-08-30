#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[])
{
    char* endpoint = "tcp://81c62603c7854e79b258b84dc4a15f24.mqtt.iot.gz.baidubce.com:1883"; // like "ssl://tut.mqtt.iot.gz.baidubce.com:1884";
    char* username = "81c62603c7854e79b258b84dc4a15f24/LIGHT_REAL";	// like "tut/demo";
    char* password = "w2qcdzCRfhb2FNSNVCS0DrBqUxYe/utAoZh6KN/eyHM=";
    char* topic = "$baidu/iot/shadow/LIGHT_REAL/update";
    char* message = "Hello, Baidu MQTT service, from c program";

    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    ssl_opts.trustStore = "root_cert.pem";
    ssl_opts.enableServerCertAuth = 1;

    MQTTClient_connectOptions connect_options = MQTTClient_connectOptions_initializer;
    connect_options.keepAliveInterval = 20;  // Alive interval
    connect_options.cleansession = 1;
    connect_options.username = username;
    connect_options.password = password;
  //  connect_options.ssl = &ssl_opts;

    MQTTClient client;
    int rc = MQTTClient_create(&client, endpoint, "LIGHT_REAL", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    rc = MQTTClient_connect(client, &connect_options);

printf("already CONNECT\n");
	while(1);

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken delivery_token;
    pubmsg.payload = message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = 0;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, topic,  &pubmsg, &delivery_token);

while(1);

    MQTTClient_disconnect(client, 0);
    MQTTClient_destroy(&client);
    return 0;
}