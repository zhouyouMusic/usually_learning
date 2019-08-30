#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <stdlib.h>
#include <string.h>

volatile MQTTClient_deliveryToken deliveredtoken;
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}


int main(int argc, char* argv[])
{
    char* endpoint = "tcp://81c62603c7854e79b258b84dc4a15f24.mqtt.iot.gz.baidubce.com:1883"; // like "ssl://tut.mqtt.iot.gz.baidubce.com:1884";
    char* username = "81c62603c7854e79b258b84dc4a15f24/LIGHT_REAL";	// like "tut/demo";
    char* password = "w2qcdzCRfhb2FNSNVCS0DrBqUxYe/utAoZh6KN/eyHM=";
    char* topic = "$baidu/iot/shadow/LIGHT_REAL/update/documents";
    char* message = "Hello, Baidu MQTT service, from c program";

    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    ssl_opts.trustStore = "root_cert.pem";
    ssl_opts.enableServerCertAuth = 1;

    MQTTClient_connectOptions connect_options = MQTTClient_connectOptions_initializer;
    connect_options.keepAliveInterval = 20;  // Alive interval
    connect_options.cleansession = 1;
    connect_options.username = username;
    connect_options.password = password;
    connect_options.ssl = &ssl_opts;
		
	
    MQTTClient client;
    int rc = MQTTClient_create(&client, endpoint, "LIGHT_REAL", MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
		rc = MQTTClient_connect(client, &connect_options);		

printf("already connect \n");

		MQTTClient_subscribe(client, topic,1);
    char ch;
		do 
    {
        ch = getchar();
	printf("%c",ch);
    } while(ch!='Q' && ch != 'q');

    MQTTClient_disconnect(client, 0);
    MQTTClient_destroy(&client);
    return 0;
}
