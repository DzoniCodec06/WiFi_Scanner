// This is program that scanns free ip adresses and takes on of them
// Board is ESP8266
// Made by Nikola Bojinovic
// Last edited: 05.01.2026.

#include <LittleFS.h>
#include <ESP8266WiFi.h>
//#include <ESP8266Ping.h>

String ssid;
String password;

// Set your desired static IP address
//IPAddress local_IP(192, 168, 1, 150);
// Set your Gateway IP (usually your router's IP)
//IPAddress gateway(192, 168, 1, 1);
// Set your Subnet Mask
//IPAddress subnet(255, 255, 255, 0);

bool net_ip = false;
IPAddress local_IP;

bool alreadyHasData;

String wifi_data;

const String filename = "/config.txt";

int connection_cycles = 0;

void checkingConnection() {
  while (WiFi.status() != WL_CONNECTED) {
    if (connection_cycles < 30) {
      delay(250);
      Serial.println(".");
      delay(250);
      connection_cycles++;
    } else {
      Serial.println("Can't connect!");
      Serial.println("Tried to connect with ssid: " + ssid + " password: " + password + " unsuccessfuly!");
      return;
    }
  }
}

IPAddress convertToIp(String ip) {
  String f_o;
  String s_o;
  String t_o;
  String fo_o;

  int n = 0;

  for (int i = 0; i < 12; i++) {
    if (n == 0) {
      if (ip[i] != '.') {
        f_o += ip[i];
      } else n++;
    } else if (n == 1) {
      if (ip[i] != '.') {
        s_o += ip[i];
      } else n++;
    } else if (n == 2) {
      if (ip[i] != '.') {
        t_o += ip[i];
      } else n++;
    } else if (n == 3) {
      if (ip[i] != '.') {
        fo_o += ip[i];
      } else n++;
    }
  }

  //local_IP(f_o.toInt(), s_o.toInt(), t_o.toInt(), fo_o.toInt());
  Serial.println("Converted IP: " + f_o + " " + s_o + " " + t_o + " " + fo_o);

  int t_fo_o;

  bool found_target_ip = false;

  if ((fo_o.toInt() + 200) < 255 && found_target_ip == false) {
    t_fo_o = (fo_o.toInt() + 200);
    found_target_ip = true;
  } else if ((fo_o.toInt() + 100) < 255 && found_target_ip == false) {
    t_fo_o = (fo_o.toInt() + 100);
    found_target_ip = true;
  } else if ((fo_o.toInt() + 50) < 255 && found_target_ip == false) {
    t_fo_o = (fo_o.toInt() + 50);
    found_target_ip = true;
  } else if ((fo_o.toInt() + 25) < 255 && found_target_ip == false) {
    t_fo_o = (fo_o.toInt() + 25);
    found_target_ip = true;
  }

  IPAddress convertedIP(f_o.toInt(), s_o.toInt(), t_o.toInt(), t_fo_o);

  net_ip = true;

  return convertedIP;
}

void tryToConnect(String net_ssid, String net_pass) {
  if (net_ip) {
    Serial.println("Reconfiguring WiFi object -> IP: " + local_IP.toString());

    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();

    WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(net_ssid, net_pass);
    checkingConnection();
  } else {
    if (net_ssid == "" || net_pass == "") return;
    WiFi.begin(net_ssid, net_pass);
    Serial.println("Trying to connect with ssid: " + net_ssid + " password: " + net_pass);
    checkingConnection();
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.println("ip: " + WiFi.localIP().toString());
    //net_ip = true;

    //alreadyHasData = true;
    if (alreadyHasData) return;
    else {
      Serial.println("Trying to save data...");
      File writeFile = LittleFS.open(filename, "w");
      if (writeFile) {
        writeFile.println(net_ssid);
        writeFile.println(net_pass);
        writeFile.println(WiFi.localIP().toString());
        writeFile.close();
        Serial.println("Value saved successfully!");
      } else {
        Serial.println("Failed to open file for writing");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  connection_cycles = 0;

  LittleFS.begin();

  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  File file = LittleFS.open(filename, "r");

  if (file) {
    String ssid_value = file.readStringUntil('\n');
    String pass_value = file.readStringUntil('\n');
    String ip_value = file.readStringUntil('\n');

    if (ip_value != "") {
      Serial.println("Has best match free ip: " + ip_value);
      local_IP = convertToIp(ip_value);
    }

    Serial.println("SSID FROM FILE: " + ssid_value);
    Serial.println("PASSWORD FROM FILE: " + pass_value);

    ssid_value.trim();
    pass_value.trim();

    ssid = ssid_value;
    password = pass_value;

    file.close();

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    if (ssid != "" || password != "" && ip_value != "") {
      alreadyHasData = true;
      tryToConnect(ssid, password);
    } else {
      alreadyHasData = false;
    }

  } else {
    Serial.println("File not found. Starting from 0.");
  }
}

void loop() {
  if (Serial.available()) {
    wifi_data = Serial.readStringUntil('\n');
    if (wifi_data.startsWith("w")) {
      int delimiter = wifi_data.indexOf(',');
      if (delimiter != -1) {
        ssid = wifi_data.substring(1, delimiter);
        password = wifi_data.substring(delimiter + 1);
        connection_cycles = 0;
        if (ssid != "" && password != "") tryToConnect(ssid, password);
        else return;
      }
    } else if (wifi_data == "r") {
      Serial.println("Trying to disconnect device...");
      WiFi.disconnect();

      delay(2500);

      File writeFile = LittleFS.open(filename, "w");
      if (writeFile) {
        writeFile.close();
        Serial.println("Value erased successfully!");
        ssid = "";
        password = "";
      } else {
        return;
        Serial.println("Failed to open file for writing");
      }
    }
  }
}
