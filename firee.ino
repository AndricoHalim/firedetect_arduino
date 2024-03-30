#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

#define SSID "$"
#define SSID_PASSWORD "dicky1017"

// Konfigurasi MQTT
const char* mqtt_server = "103.167.112.188";
const int mqtt_port = 1883;
const char* mqtt_user = "/fik:fik";
const char* mqtt_password = "fik123";

const char* queue_name = "Firedect";

#define DHTTYPE DHT11
#define DHTPIN D2
#define relayPin D1
DHT dht(DHTPIN, DHTTYPE);
int pinDHT = D2;

String kondisi = "";
float hum;  //Stores humidity value
float temp; //Stores temperature value
int sensorApi = A0;
int sensorApiValue = 0;
const unsigned char buzzer = D5;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
 pinMode(relayPin, OUTPUT);

  // Menghubungkan ke jaringan WiFi
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Menghubungkan ke RabbitMQ server
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ArduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("Connected to RabbitMQ");
    } else {
      Serial.print("Failed to connect to RabbitMQ, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
  dht.begin();
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  sensorApiValue = analogRead(sensorApi);
  Serial.println(sensorApiValue);

  if (sensorApiValue < 560) {
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    Serial.println("Api terdeteksi");
    kondisi = "Api Terdeteksi";
    digitalWrite(buzzer, HIGH);
   delay(3000);
   digitalWrite(relayPin, LOW);
    delay(1000);
  } else {
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    Serial.println("Aman terkendali");
    kondisi = "Aman Terkendali";
    digitalWrite(buzzer, LOW);
   delay(3000);
   digitalWrite(relayPin, HIGH);
    delay(1000);
  }

  // Mengirim data ke RabbitMQ
  if (client.connected()) {
    char message[100];
    snprintf(message, sizeof(message), "{\"Temp\": %.2f, \"Hum\": %.2f, \"kondisi\": \"%s\"}", temp, hum, kondisi.c_str());
    client.publish(queue_name, message);
    Serial.println("Data published to RabbitMQ");
  } else {
    Serial.println("Failed to publish data. MQTT client disconnected");
  }

  delay(2000); // Delay selama 2 detik sebelum membaca data sensor lagi
}
