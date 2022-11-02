#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"

#define PART_BOUNDARY "123456789000000000000987654321"


#define CAMERA_MODEL_WROVER_KIT

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>Mars Rover live feed</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      //camera resolution
      img {  width: 200 ;
        max-width: 100% ;
        height: 200 ; 
      }
    </style>
  </head>
  <body>
    <h1>Group 6 live feed</h1>
    <img src="" id="photo" >
                      
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/action?go=" + x, true);
     xhr.send();
   }
   window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
  </script>
  </body>
</html>
)rawliteral";
//Camera setup

#include "camera_pins.h"
#include <DFRobot_SGP40.h>
#include <Wire.h>
#define I2C_SDA 32
#define I2C_SCL 33
DFRobot_SGP40 mySgp40;
//Setup for airquality sensor

char dustMsg[30];
int dust25 = 0;
//Dust sensor setup

#include <PubSubClient.h>
const char* ssid = "ITEK 2nd";
const char* password = "Four_Sprints_F21v";
const char* mqtt_server = "10.120.0.94";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
char command;
int parameter = 0;
String missionCmd[100];
int value = 0;
int missionCmdIndex = 0;
//MQTT setup

#define RF 12
#define RB 13
#define LF 15
#define LB 14
//Motor setup

void startCameraServer();

void setup() {
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  // Camera setup

  Serial.begin(9600);
  Serial2.begin(9600,SERIAL_8N1, 2,-1);
  Serial.end();
  //Setup Dust sensor

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  while(mySgp40.begin(/*duration = */10000) !=true){
    Serial.println("failed to init chip, please check if the chip connection is fine");
    delay(1000);
  }
  Serial.println("----------------------------------------------");
  Serial.println("sgp40 initialized successfully!");
  Serial.println("----------------------------------------------");
  //Setup for airquality sensor

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //MQTT setup

  pinMode(RF, OUTPUT);
  pinMode(RB, OUTPUT);
  pinMode(LF, OUTPUT);
  pinMode(LB, OUTPUT);
  //Motor setup
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
  //Camera setup  
}

void loop(){
  MQTT_handler();
}

void runMission(){
  for(int i = 0; i <= missionCmdIndex; i++){
    String message = missionCmd[i];
    command = getCommand(message);
    parameter = getParameter(message);

    switch (command){
      case 'f':
        forward(parameter);
        break;
      case 'b':
        backward(parameter);
        break;
      case 'r':
        pivotR(parameter);
        break;
      case 'l':
        pivotL(parameter);
        break;
      case 'w':
        Delay(parameter);
        break;
      case 'a':
        for (int i = 0; i < parameter; i++) {
          Airquality_Sensor();
          Dust_Sensor();
          delay(500);
          }
        break;
    }
  }
}

void stop(){
  analogWrite(LB, 0);
  analogWrite(RB, 0);
  analogWrite(LF, 0);
  analogWrite(RF, 0);

}

void forward(int t) {
  stop();

  analogWrite(RF, 200);
  analogWrite(LF, 255);

  DelayD(t);
}

void backward(int t) {
  stop();

  analogWrite(LB, 255);
  analogWrite(RB, 200);

  DelayD(t);
}

void pivotR(int t){
  stop();
  analogWrite(LB, 255);
  analogWrite(RF, 255);

  DelayR(t);
}
void pivotL(int t){
  stop();
  analogWrite(RB, 255);
  analogWrite(LF, 255);

  DelayL(t);
}

void Delay(int t){
  if (t > 0) {
    delay(t * 1000);
    stop();
    delay(20);
  } else {
    delay(5);
    stop();
  }
}
void DelayD(int t){
  if (t > 0) {
    delay(t * 28);
    stop();
    delay(20);
  } else {
    delay(5);
    stop();
  }
}

void DelayL(int t){
  if (t > 0) {
    delay(t * 9);
    stop();
    delay(20);
  } else {
    delay(5);
    stop();
  }
}
void DelayR(int t){
  if (t > 0) {
    delay(t * 11);
    stop();
    delay(20);
  } else {
    delay(5);
    stop();
  }
}

void Airquality_Sensor(){
    uint16_t index = mySgp40.getVoclndex();
  
    Serial.print("vocIndex = ");
    String testString = String(index);
    testString.toCharArray(msg, MSG_BUFFER_SIZE);
    Serial.println(index);
    delay(1000);

    client.publish("RoboCar/Sensor/Air", msg);
}

void Dust_Sensor(){
  
    if(Serial2.available()){
    
    if((Serial2.read() == 0X42) && (Serial2.read() == 0x4D)) // if we got the headers
    {
      int msgIdx=0;
      for(int i = 0;i<30;i++){
        while(!Serial2.available()); //wait for byte
        dustMsg[msgIdx++] = Serial2.read();
        //if we get a byte put it in the array and incerement index

      }
      dust25 = (dustMsg[4]<<8) + dustMsg[5];
      //when we are done, we can take out the relevant data and calculate value
     
    }
   
  }
  Serial.println(dust25);
  String output = String(dust25);
  output.toCharArray(msg, MSG_BUFFER_SIZE);
  delay(1000);

  client.publish("RoboCar/Sensor/Dust", msg);
}

void MQTT_handler(){
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}

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

char getCommand(String message){
   return message[0];
} 

int getParameter(String message){
  String valueString = message.substring(1); //creates a new string starting from index 1 (after the command letter)
  int value = valueString.toInt(); //converts a string to an integer
  return value;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String temp = ""; // string to store command
  missionCmdIndex = 0; // variable to count number of commands
  for (int i = 0; i < length; i++) {
   
    if ((char)payload[i]=='s'){ //stop
      Serial.println("Mission data received");

      runMission();

  //** PRINT OUT MISSION DATA (HERE, INSTEAD, YOU WANT TO CALL A FUNCTION THATRUNS THE MISSION)
      for (int i = 0;i<missionCmdIndex;i++){

        Serial.print("Command: ");
        Serial.print(getCommand(missionCmd[i]));
        Serial.print(" ");
        Serial.print("Parameter: ");
        Serial.println(getParameter(missionCmd[i]));
        }
    }
  temp += (char)payload[i]; // add character to mission command

  if ((char)payload[i]==' ') // if we receive next mission command
  {
    missionCmd[missionCmdIndex++]=temp; // store command and increment qeue index
    temp = ""; // clear temp string
  }

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
      client.publish("RoboCar/connection", "hello world");
      // ... and resubscribe
      client.subscribe("RoboCar/Mission420/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

