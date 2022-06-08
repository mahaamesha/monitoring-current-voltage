#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

Adafruit_INA219 ina219;

int LED_RED = 14;   // D5 in esp8266
int LED_YLW = 12;   // D6
int LED_GRN = 13;   // D7

// Update these with values suitable for your network.
const char* ssid = "Wi-Fu";
const char* password = "gorill4a";
// const char* ssid = "Sembilan Satu";
// const char* password = "sembilan";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


// struct to save ina219 measurements
struct param {
	float shuntvoltage;
	float busvoltage;
	float current_mA;
	float loadvoltage;
	float power_mW;
};

// read measurements using ina219
void read_ina219(param *p) {  // struct param
	(*p).shuntvoltage = ina219.getShuntVoltage_mV();
	(*p).busvoltage = ina219.getBusVoltage_V();
	(*p).current_mA = ina219.getCurrent_mA();
	(*p).power_mW = ina219.getPower_mW();
	(*p).loadvoltage = (*p).busvoltage + ((*p).shuntvoltage / 1000);
}

void print_ina219(param *p) {
	Serial.println();
	Serial.println("=== Measurement ===");
	Serial.print("Vsh\t: ");
	Serial.print((*p).shuntvoltage);
	Serial.println(" V");

	Serial.print("Vbs\t: ");
	Serial.print((*p).busvoltage);
	Serial.println(" V");

	Serial.print("I\t: ");
	Serial.print((*p).current_mA);
	Serial.println(" mA");

	Serial.print("P\t: ");
	Serial.print((*p).power_mW);
	Serial.println(" mW");

	Serial.print("Vl\t: ");
	Serial.print((*p).loadvoltage);
	Serial.println(" V");
	Serial.println();
}

// check flag, then turn on/off led
void control_led(String flag) {
	if (flag == "0") {
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_YLW, LOW);
		digitalWrite(LED_GRN, HIGH);
	}
	else if (flag == "1") {
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_YLW, HIGH);
		digitalWrite(LED_GRN, LOW);
	}
	else if (flag == "2") {
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_YLW, LOW);
		digitalWrite(LED_GRN, LOW);
	}
}


// Function MQTT
void setup_wifi() {
	delay(10);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	randomSeed(micros());

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}

	// Check if I should turn on/off the actuator
	if (String(topic) == String("ourSensorIn")) {
		String flag = String( (char)payload[0] );	// It can be 0,1,2
		control_led(flag);	//0:green, 1:yellow, 2:red
	}
	else {
		Serial.println();
	}
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Create a random client ID
		String clientId = "ESP8266Client-";
		clientId += String(random(0xffff), HEX);
		// Attempt to connect
		if (client.connect(clientId.c_str())) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			client.publish("ourSensorOut", "Hello World !!!");
			// ... and resubscribe
			client.subscribe("ourSensorOut"); //only for checking that the message has been received
			client.subscribe("ourSensorIn");
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
// (END) Function MQTT


void setup() {
	Serial.begin(9600);
	
	// For mqtt
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	
	// Initialize pin
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_YLW, OUTPUT);
	pinMode(LED_GRN, OUTPUT);

	// Initialize the INA219.
	ina219.begin();

	Serial.println("System on");
}



void loop() {
	// MQTT
	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	unsigned long now = millis();
	if (now - lastMsg > 2000) {
		lastMsg = now;
	

	param p;	// struct param to save measurements
	read_ina219(&p);
	print_ina219(&p);

	String message = String(p.current_mA) + "," + String(p.shuntvoltage);
		client.publish("ourSensorOut", message.c_str());
	}
	// (END) MQTT
}