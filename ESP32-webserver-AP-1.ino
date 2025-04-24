
// =================== 📚 المكتبات ===================
#include <WiFi.h>          // مكتبة الاتصال بالواي فاي
#include <WebServer.h>     // لإنشاء خادم ويب
#include <SPIFFS.h>        // نظام ملفات داخل الشريحة لتخزين الصفحات 
// مكتبة رفع الملفات -SPIFFS.h- قديمة و تعمل على اردوينو 1.8.19

// =================== 🌐 إعدادات الشبكة ===================
const char* AP_SSID = "ESP32-Control";       // اسم نقطة الوصول Access Point
const char* AP_PASSWORD = "12345678";        // كلمة المرور

// تعريف المنافذ الآمنة
// =================== ⚙️ تعريف المنافذ ===================
#define TOGGLE_OUT1 17   // المخرج التبادلي 1
#define TOGGLE_OUT2 16   // المخرج التبادلي 2
#define MANUAL_OUT1 18   // المخرج اليدوي 1
#define MANUAL_OUT2 19   // المخرج اليدوي 2
#define MANUAL_OUT3 21   // المخرج اليدوي 3
#define MANUAL_OUT4 22   // المخرج اليدوي 4
#define MANUAL_OUT5 23   // المخرج اليدوي 5
#define MANUAL_OUT6 25   // المخرج اليدوي 6
#define MANUAL_OUT7 26   // المخرج اليدوي 7
#define MANUAL_OUT8 27   // المخرج اليدوي 8
#define MANUAL_OUT9 32   // المخرج اليدوي 9
#define MANUAL_OUT10 33  // المخرج اليدوي 10

// =================== 📦 هياكل البيانات ===================
// هيكل لتخزين إعدادات Wi-Fi
struct WiFiSettings {
  char ssid[32];
  char password[64];
};

// هيكل بيانات لإدارة المنافذ
struct GpioPin {
  int number;                     // رقم البن
  int state;                      // حالته (1 أو 0)
  unsigned long activationTime;  // وقت التفعيل الأخير
  unsigned long autoOffDuration; // مدة التشغيل التلقائي قبل الإيقاف
  const char* name;              // اسم العرض
  const char* onCmd;             // رابط التشغيل
  const char* offCmd;            // رابط الإيقاف
  bool allowManualControl;       // هل يمكن التحكم يدوياً
};

// =================== 🌍 متغيرات عامة ===================
WebServer server(80);      // خادم ويب على المنفذ 80
WiFiSettings wifiSettings; // إعدادات الشبكة 


// =================== 🔌 مصفوفة تعريف المخارج ===================
GpioPin pins[] = {
  { TOGGLE_OUT1, 0, 0, 0, "المخرج التبادلي 1", "/out1/on", "/out1/off", false },
  { TOGGLE_OUT2, 0, 0, 0, "المخرج التبادلي 2", "/out2/on", "/out2/off", false },
  { MANUAL_OUT1, 0, 0, 4000, "المخرج اليدوي 1", "/manual1/on", "/manual1/off", true },
  { MANUAL_OUT2, 0, 0, 4000, "المخرج اليدوي 2", "/manual2/on", "/manual2/off", true },
  { MANUAL_OUT3, 0, 0, 0, "المخرج اليدوي 3", "/manual3/on", "/manual3/off", true },
  { MANUAL_OUT4, 0, 0, 0, "المخرج اليدوي 4", "/manual4/on", "/manual4/off", true },
  { MANUAL_OUT5, 0, 0, 0, "المخرج اليدوي 5", "/manual5/on", "/manual5/off", true },
  { MANUAL_OUT6, 0, 0, 0, "المخرج اليدوي 6", "/manual6/on", "/manual6/off", true },
  { MANUAL_OUT7, 0, 0, 0, "المخرج اليدوي 7", "/manual7/on", "/manual7/off", true },
  { MANUAL_OUT8, 0, 0, 0, "المخرج اليدوي 8", "/manual8/on", "/manual8/off", true },
  { MANUAL_OUT9, 0, 0, 0, "المخرج اليدوي 9", "/manual9/on", "/manual9/off", true },
  { MANUAL_OUT10, 0, 0, 0, "المخرج اليدوي 10", "/manual10/on", "/manual10/off", true }
};

// إعدادات النظام
const char* systemTitle = "نظام التحكم لغسالة صناعية";
const char* systemStatusLabel = "حالة النظام";
const char* resetBtn = "إيقاف دوران";
const char* toggleBtnStart ="تشغيل البرنامج";
const char* toggleBtnStop = "إيقاف البرنامج";

const char* toggleOutputNames[] = { "دوران يمين", "دوران يسار" };
const char* manualOutputs[] = {
  "فتح الباب", "إغلاق الباب", "تعبئة ماء", 
  "فتح بخار", "مكب تصريف", "المخرج اليدوي 6",
  "المخرج اليدوي 7", "المخرج اليدوي 8",
  "المخرج اليدوي 9", "المخرج اليدوي 10"
};

// متغيرات النظام
bool toggleSystemActive = false;
bool toggleSystemPaused = false;
unsigned long toggleStartTime = 0;
unsigned long totalDuration = 7200000;  // 120 دقيقة
unsigned long lastToggleTime = 0;
unsigned long pausedTime = 0;
unsigned long toggleInterval = 60000;
unsigned long savedToggleInterval = 60000;

// حالة الاتصال الحالية
bool isConnected = false;

// مسارات الملفات في SPIFFS
const char* WIFI_CONFIG_FILE = "/wifi_config.txt";

// ------ إدارة اتصال Wi-Fi ------
bool loadWiFiConfig() {
  File file = SPIFFS.open(WIFI_CONFIG_FILE, "r");
  if (!file) return false;
  
  size_t len = file.readBytes((char*)&wifiSettings, sizeof(wifiSettings));
  file.close();
  return (len == sizeof(wifiSettings));
}

//--------------------دالة تجميع أجزاء صفحة الويب الواجهة ---deleted---------------------
// String fullHtmlPage = htmlHeader + String(cssStyles) + String(htmlBody) + String(javascriptCode);

// ============ Forward Declarations ============
// ============ التصريح المسبق عن الدوال قيل دالة الإعداد setup()  ============
String getSystemStatusJSON();
void toggleOutput(int pinIndex);
void toggleOutputs();
void startToggleSystem();
void stopToggleSystem();
void pauseToggleSystem();
void resumeToggleSystem();
unsigned long calculateRemainingTime();
int calculateProgress();
void connectToWiFi();
void startAPMode();
void setupServer();
void handleConfigPage();
void handleSaveConfig();

void setup() {
  Serial.begin(115200);
  
  // تهيئة المنافذ
  for (auto& pin : pins) {
    pinMode(pin.number, OUTPUT);
    digitalWrite(pin.number, pin.state);
  }
  
  // تكوين نقطة الوصول
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
// تهيئة SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("فشل في تهيئة SPIFFS!");
    return;
  }
// تحقق من وجود ملفات ضرورية
  if (!SPIFFS.exists("/css/all.min.css")) {
    Serial.println("ملف الستايل مفقود!");
  }

 // تحميل إعدادات Wi-Fi
  if (loadWiFiConfig()) {
    connectToWiFi();
  } else {
    startAPMode();
  }

  setupServer();
  // لتكوين الستايل الخارجي لتفعيل الفونت
// CSS
  server.on("/css/all.min.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/css/all.min.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/cairo.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/css/cairo.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/tajawal.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/css/tajawal.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  // الفونتات
  server.on("/fonts/Cairo-SemiBold.woff", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff", "r");
    server.streamFile(file, "application/font-woff"); // ✅
    file.close();
  });
  
 server.on("/fonts/Cairo-SemiBold.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff2", "r");
    server.streamFile(file, "application/font-woff2"); // ✅
    file.close();
  });

   server.on("/fonts/Tajawal-Regular.woff", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/fonts/Tajawal-Regular.woff", "r");
    server.streamFile(file, "application/font-woff"); // ✅
    file.close();
  });

   server.on("/fonts/Tajawal-Regular.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    File file = SPIFFS.open("/fonts/Tajawal-Regular.woff2", "r");
    server.streamFile(file, "application/font-woff2"); // ✅
    file.close();
  });

  server.on("/webfonts/fa-solid-900.woff2", HTTP_GET, []() {
  File file = SPIFFS.open("/webfonts/fa-solid-900.woff2", "r");
  server.streamFile(file, "application/font-woff2");
  file.close();
});

server.on("/webfonts/fa-v4compatibility.woff2", HTTP_GET, []() {
  File file = SPIFFS.open("/webfonts/fa-v4compatibility.woff2", "r");
  server.streamFile(file, "application/font-woff2");
  file.close();
});

server.on("/webfonts/fa-regular-400.woff2", HTTP_GET, []() {
  File file = SPIFFS.open("/webfonts/fa-regular-400.woff2", "r");
  server.streamFile(file, "application/font-woff2");
  file.close();
});

server.on("/webfonts/fa-brands-400.woff2", HTTP_GET, []() {
  File file = SPIFFS.open("/webfonts/fa-brands-400.woff2", "r");
  server.streamFile(file, "application/font-woff2");
  file.close();
});


  
  // في قسم setup() تحت server.on()
  server.on("/updateSettings", HTTP_POST, []() {
    if (server.hasArg("interval")) {
      toggleInterval = server.arg("interval").toInt() * 1000;
      Serial.print("[إعداد] الفاصل الزمني الجديد: ");
      Serial.println(toggleInterval / 1000);
    }

    if (server.hasArg("duration")) {
      totalDuration = server.arg("duration").toInt() * 60000;
      Serial.print("[إعداد] المدة الكلية الجديدة: ");
      Serial.println(totalDuration / 60000);
    }

    server.send(200, "application/json", getSystemStatusJSON());
  });
    
  server.on("/saveSettings", HTTP_POST, []() {
    if (server.hasArg("interval")) {
      toggleInterval = server.arg("interval").toInt() * 1000;
      Serial.print("[حفظ] الفاصل الزمني: ");
      Serial.println(toggleInterval / 1000);
    }
    if (server.hasArg("duration")) {
      totalDuration = server.arg("duration").toInt() * 60000;
      Serial.print("[حفظ] المدة الكلية: ");
      Serial.println(totalDuration / 60000);
    }
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 10دقيقة * 30 ثانية
  server.on("/resetSettings", HTTP_POST, []() {
    toggleInterval = 30 * 1000;      // 30 ثانية
    totalDuration = 10 * 60 * 1000;  // 10 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 30ثانية*10دقائق");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 20دقيقة * 30 ثانية
  server.on("/resetSettings3020", HTTP_POST, []() {
    toggleInterval = 30 * 1000;      // 30 ثانية
    totalDuration = 20 * 60 * 1000;  // 20 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 30ثانية*20دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 15دقيقة * 60 ثانية
  server.on("/resetSettings6015", HTTP_POST, []() {
    toggleInterval = 60 * 1000;      // 60 ثانية
    totalDuration = 15 * 60 * 1000;  // 15 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*15دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 30دقيقة * 60 ثانية
  server.on("/resetSettings6030", HTTP_POST, []() {
    toggleInterval = 60 * 1000;      // 60 ثانية
    totalDuration = 30 * 60 * 1000;  // 30 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*30دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 45دقيقة * 60 ثانية
  server.on("/resetSettings6045", HTTP_POST, []() {
    toggleInterval = 60 * 1000;      // 60 ثانية
    totalDuration = 45 * 60 * 1000;  // 45 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*45دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 60دقيقة * 60 ثانية
  server.on("/resetSettings6060", HTTP_POST, []() {
    toggleInterval = 60 * 1000;      // 60 ثانية
    totalDuration = 60 * 60 * 1000;  // 60 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*60دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 90دقيقة * 60 ثانية
  server.on("/resetSettings6090", HTTP_POST, []() {
    toggleInterval = 60 * 1000;      // 60 ثانية
    totalDuration = 90 * 60 * 1000;  // 90 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*90دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  //أعدادات السيرفر 120دقيقة * 60 ثانية
  server.on("/resetSettings60120", HTTP_POST, []() {
    toggleInterval = 60 * 1000;       // 60 ثانية
    totalDuration = 120 * 60 * 1000;  // 120 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 60ثانية*120دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/updateSettings", HTTP_POST, []() {
    if (server.hasArg("interval")) {
      int newInterval = server.arg("interval").toInt();
      if (newInterval >= 5 && newInterval <= 300) {  // تحقق من القيم المسموحة
        toggleInterval = newInterval * 1000;
        Serial.print("[إعداد] الفاصل الزمني الجديد: ");
        Serial.println(toggleInterval / 1000);
      }
    }

    if (server.hasArg("duration")) {
      int newDuration = server.arg("duration").toInt();
      if (newDuration >= 1 && newDuration <= 120) {  // تحقق من القيم المسموحة
        totalDuration = newDuration * 60000;
        Serial.print("[إعداد] المدة الكلية الجديدة: ");
        Serial.println(totalDuration / 60000);
      }
    }

    server.send(200, "application/json", getSystemStatusJSON());
  });

  // تهيئة المنافذ
  for (auto& pin : pins) {
    pinMode(pin.number, OUTPUT);
    digitalWrite(pin.number, pin.state);
    Serial.print("تم تهيئة المخرج: ");
    Serial.println(pin.name);
  }

  // بدء نقطة الوصول
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("\nنقطة الوصول جاهزة");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("عنوان IP: ");
  Serial.println(WiFi.softAPIP());

  /* / تكوين مسارات الخادم
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
  }); */

 // تكوين مسارات الخادم التعديل الجديد
server.on("/", HTTP_GET, []() {
    String html = fullHtmlPage; // <-- تم تصحيح المتغير هنا
    
    // استبدال العناوين
    html.replace("%SYSTEM_TITLE%", systemTitle);
    html.replace("%SYSTEM_STATUS_LABEL%", systemStatusLabel);
    html.replace("%resetBtn%", resetBtn);
    html.replace("%toggleBtnStart%", toggleBtnStart);
    html.replace("%toggleBtnStop%", toggleBtnStop);

    // استبدال أسماء المخارج التبادلية
    html.replace("%TOGGLE_OUTPUT_1%", toggleOutputNames[0]);
    html.replace("%TOGGLE_OUTPUT_2%", toggleOutputNames[1]);

    // استبدال أسماء المخارج اليدوية
    for (int i = 0; i < 10; i++) {
        html.replace("%MANUAL_OUTPUT_" + String(i + 1) + "%", manualOutputs[i]);
    }

    server.send(200, "text/html", html);
});

  server.on("/status", HTTP_GET, []() {
    String json = getSystemStatusJSON();
    server.send(200, "application/json", json);
  });

  // مسارات تبديل المخارج
  server.on("/out1/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج التبادلي 1");
    toggleOutput(0);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/out2/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج التبادلي 2");
    toggleOutput(1);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual1/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 1");
    toggleOutput(2);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual2/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 2");
    toggleOutput(3);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual3/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 3");
    toggleOutput(4);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual4/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 4");
    toggleOutput(5);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual5/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 5");
    toggleOutput(6);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual6/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 6");
    toggleOutput(7);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual7/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 7");
    toggleOutput(8);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual8/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 8");
    toggleOutput(9);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual9/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 9");
    toggleOutput(10);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/manual10/toggle", HTTP_POST, []() {
    Serial.println("[حدث] تبديل المخرج اليدوي 10");
    toggleOutput(11);
    server.send(200, "application/json", getSystemStatusJSON());
  });

  // مسارات النظام التبادلي
  server.on("/toggle", HTTP_POST, []() {
    if (server.hasArg("duration")) {
      totalDuration = server.arg("duration").toInt() * 60000;
      Serial.print("[إعداد] المدة الكلية: ");
      Serial.print(totalDuration / 60000);
      Serial.println(" دقيقة");
    }
    if (server.hasArg("interval")) {
      toggleInterval = server.arg("interval").toInt() * 1000;
      Serial.print("[إعداد] الفاصل الزمني: ");
      Serial.print(toggleInterval / 1000);
      Serial.println(" ثانية");
    }

    if (toggleSystemActive) {
      stopToggleSystem();
    } else {
      startToggleSystem();
    }
    server.send(200, "application/json", getSystemStatusJSON());
  });

  server.on("/pause", HTTP_POST, []() {
    if (toggleSystemPaused) {
      resumeToggleSystem();
    } else {
      pauseToggleSystem();
    }
    server.send(200, "application/json", getSystemStatusJSON());
  });

  // مسار إعادة التعيين
  server.on("/reset", HTTP_POST, []() {
    Serial.println("[حدث] إعادة تعيين المخارج التبادلية");
    digitalWrite(TOGGLE_OUT1, LOW);
    digitalWrite(TOGGLE_OUT2, LOW);
    pins[0].state = 0;
    pins[1].state = 0;
    server.send(200, "application/json", getSystemStatusJSON());
  });
               server.sendHeader("Cache-Control", "max-age=604800"); // 7 أيام
  server.begin();
}

// =================== 🔁 حلقة التنفيذ  الحلقة الرئيسية ===================
void loop() {
  server.handleClient();

 // إعادة الاتصال التلقائي إذا فُقد الاتصال
  if (isConnected && WiFi.status() != WL_CONNECTED) {
    Serial.println("فقدان الاتصال، إعادة المحاولة...");
    isConnected = false;
    connectToWiFi();
  }

  // التحقق من الإغلاق التلقائي كل ثانية
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck >= 1000) {
    lastCheck = millis();

    for (int i = 2; i <= 11; i++) {
      GpioPin& pin = pins[i];
      if (pin.autoOffDuration > 0 && pin.state == HIGH) {
        if (millis() - pin.activationTime >= pin.autoOffDuration) {
          pin.state = LOW;
          digitalWrite(pin.number, LOW);
          pin.activationTime = 0;
          Serial.print("إغلاق تلقائي: ");
          Serial.println(pin.name);
        }
      }
    }
  }


  if (toggleSystemActive && !toggleSystemPaused) {
    unsigned long currentTime = millis();

    if (currentTime - toggleStartTime >= totalDuration) {
      stopToggleSystem();
    } else if (currentTime - lastToggleTime >= toggleInterval) {
      lastToggleTime = currentTime;
      toggleOutputs();
    }
  }
}

void saveWiFiConfig() {
  File file = SPIFFS.open(WIFI_CONFIG_FILE, "w");
  if (!file) return;
  
  file.write((uint8_t*)&wifiSettings, sizeof(wifiSettings));
  file.close();
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSettings.ssid, wifiSettings.password);
  
  Serial.print("جاري الاتصال بشبكة WiFi...");
  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      isConnected = true;
      Serial.println("\nتم الاتصال!");
      Serial.print("عنوان IP: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nفشل الاتصال، التبديل إلى وضع AP");
  startAPMode();
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("وضع AP نشط");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("عنوان IP: ");
  Serial.println(WiFi.softAPIP());
}

// ------ واجهة تكوين الشبكة ------
void handleConfigPage() {
  String html = R"(
    <html dir="rtl">
    <head><title>تكوين الشبكة</title>
    <meta charset="UTF-8">
    </head>
    <body>
      <h1>إعدادات الشبكة</h1>
      <form action="/saveConfig" method="POST">
        SSID: <input type="text" name="ssid"><br>
        كلمة المرور: <input type="password" name="password"><br>
        <input type="submit" value="حفظ">
      </form>
    </body>
    </html>
  )";
  server.send(200, "text/html", 
    "<script>"
    "setTimeout(() => { window.location.href = '/'; }, 3000);" // إعادة التوجيه بعد 3 ثواني
    "</script>"
    "<h1>تم الحفظ! جارِ إعادة التشغيل...</h1>"
  );
}

void handleSaveConfig() {
  strncpy(wifiSettings.ssid, server.arg("ssid").c_str(), sizeof(wifiSettings.ssid));
  strncpy(wifiSettings.password, server.arg("password").c_str(), sizeof(wifiSettings.password));
  saveWiFiConfig();
  server.send(200, "text/html", "<h1>تم الحفظ! سيتم إعادة التشغيل...</h1>");
  delay(3000);
  ESP.restart();
}

// ------ إعداد الخادم ------
void setupServer() {
  server.on("/", []() {
    if (isConnected) {
      // تحميل صفحة التحكم الكاملة
      String html = fullHtmlPage;
      html.replace("%SYSTEM_TITLE%", systemTitle);
      html.replace("%SYSTEM_STATUS_LABEL%", systemStatusLabel);
      html.replace("%resetBtn%", resetBtn);
      html.replace("%toggleBtnStart%", toggleBtnStart);
      html.replace("%toggleBtnStop%", toggleBtnStop);
      
      // استبدال أسماء المخارج
      html.replace("%TOGGLE_OUTPUT_1%", toggleOutputNames[0]);
      html.replace("%TOGGLE_OUTPUT_2%", toggleOutputNames[1]);
      for (int i = 0; i < 10; i++) {
        html.replace("%MANUAL_OUTPUT_" + String(i + 1) + "%", manualOutputs[i]);
      }
      
      server.send(200, "text/html", html);
    } else {
      handleConfigPage(); // عرض صفحة التكوين إذا لم يكن متصلاً
    }
  });
  
  server.on("/saveConfig", HTTP_POST, handleSaveConfig);
  server.begin();
}

// ============ دوال النظام التبادلي ============
void toggleOutputs() {
  pins[0].state = !pins[0].state;
  pins[1].state = !pins[1].state;
  digitalWrite(TOGGLE_OUT1, pins[0].state);
  digitalWrite(TOGGLE_OUT2, pins[1].state);
  Serial.println("[عملية] تبديل المخارج التبادلية");
}

void startToggleSystem() {
  toggleSystemActive = true;
  toggleSystemPaused = false;
  toggleStartTime = millis();
  lastToggleTime = millis();
  digitalWrite(TOGGLE_OUT1, HIGH);
  digitalWrite(TOGGLE_OUT2, LOW);
  pins[0].state = 1;
  pins[1].state = 0;
  Serial.println("[نظام] بدء التشغيل التبادلي");
}

void stopToggleSystem() {
  toggleSystemActive = false;
  toggleSystemPaused = false;
  digitalWrite(TOGGLE_OUT1, LOW);
  digitalWrite(TOGGLE_OUT2, LOW);
  pins[0].state = 0;
  pins[1].state = 0;
  Serial.println("[نظام] إيقاف التشغيل التبادلي");
}

void pauseToggleSystem() {
  if (toggleSystemActive && !toggleSystemPaused) {
    toggleSystemPaused = true;
    pausedTime = millis();
    savedToggleInterval = toggleInterval;
    digitalWrite(TOGGLE_OUT1, LOW);
    digitalWrite(TOGGLE_OUT2, LOW);
  pins[0].state = 0;
    pins[1].state = 0;
    Serial.println("[نظام] إيقاف مؤقت");
  }
}

void resumeToggleSystem() {
  if (toggleSystemActive && toggleSystemPaused) {
    toggleSystemPaused = false;
    unsigned long pauseDuration = millis() - pausedTime;
    toggleStartTime += pauseDuration;
    lastToggleTime += pauseDuration;
    toggleInterval = savedToggleInterval;
  digitalWrite(TOGGLE_OUT1, HIGH);
    digitalWrite(TOGGLE_OUT2, LOW);
    pins[0].state = 1;
    pins[1].state = 0;
    Serial.println("[نظام] استئناف التشغيل");
  }
}

/*/ ============ دوال مساعدة ============
void toggleOutput(int pinIndex) {
  if (pinIndex < 2) {
    pins[pinIndex].state = !pins[pinIndex].state;
    digitalWrite(pins[pinIndex].number, pins[pinIndex].state);
    
    int otherPinIndex = (pinIndex == 0) ? 1 : 0;
    pins[otherPinIndex].state = !pins[pinIndex].state;
    digitalWrite(pins[otherPinIndex].number, pins[otherPinIndex].state);
    Serial.print("[تبديل] المخرج التبادلي ");
    Serial.println(pinIndex + 1);
  } else {
    pins[pinIndex].state = !pins[pinIndex].state;
    digitalWrite(pins[pinIndex].number, pins[pinIndex].state);
    Serial.print("[تبديل] المخرج اليدوي ");
    Serial.println(pinIndex - 1);
  }
}*/


void toggleOutput(int pinIndex) {
  // الجزء الخاص بالمخارج التبادلية (OUT1 و OUT2)
  if (pinIndex < 2) {
    // إيقاف النظام التبادلي التلقائي إذا كان نشطًا
    if (toggleSystemActive) {
      stopToggleSystem();
      Serial.println("[نظام] تم إيقاف التشغيل التلقائي للتحكم اليدوي");
    }

    // إذا كان المخرج الحالي مُنشَّطًا، قم بإطفائه وتنشيط الآخر
    if (pins[pinIndex].state == HIGH) {
      pins[pinIndex].state = LOW;
      digitalWrite(pins[pinIndex].number, LOW);

      int otherPinIndex = (pinIndex == 0) ? 1 : 0;
      pins[otherPinIndex].state = HIGH;
      digitalWrite(pins[otherPinIndex].number, HIGH);
    } else {
      // إذا كان المخرج الحالي مُعطَّلًا، قم بتنشيطه وإطفاء الآخر
      pins[pinIndex].state = HIGH;
      digitalWrite(pins[pinIndex].number, HIGH);

      int otherPinIndex = (pinIndex == 0) ? 1 : 0;
      pins[otherPinIndex].state = LOW;
      digitalWrite(pins[otherPinIndex].number, LOW);
    }

    Serial.print("[تحكم يدوي] المخرج التبادلي ");
    Serial.println(pinIndex + 1);
  }

  // الجزء الخاص بالمخارج اليدوية (الفهرس 2 فما فوق)
  //هذا الكود صالح لتشغيل المخارج العشرة الباقية اليدوية بشكل طبيعي
  //لم يتم حذفه للاستفادة منه
 /* else {
    GpioPin& pin = pins[pinIndex];

    if (pin.state == LOW) {
      pin.state = HIGH;
      digitalWrite(pin.number, HIGH);
      pin.activationTime = millis();  // ⚡ تعيين وقت التفعيل
      Serial.print("تم تشغيل: ");
      Serial.println(pin.name);
    } else {
      pin.state = LOW;
      digitalWrite(pin.number, LOW);
      pin.activationTime = 0;  // ⚡ إعادة تعيين الوقت عند الإغلاق
      Serial.print("تم إغلاق: ");
      Serial.println(pin.name);
    }
  } */

 //----------- الجزء الخاص بالمخارج اليدوية (الفهرس 2 فما فوق)---------

 //خاص بالمخرجين اليدويين الأول و الثاني و رقمها 2 و 3 
 //حيث تم تحويلهما إلى مخرجين تبادليين متعاكسين
else {
    GpioPin& pin = pins[pinIndex];

    // معالجة خاصة للمخرجين اليدويين 1 و2 (الفهرس 2 و3)
        // تطبيق التعاكس فقط على المخرجين 1 و 2 (index 2 و 3)
    if (pinIndex == 2 || pinIndex == 3) {
        int otherPinIndex = (pinIndex == 2) ? 3 : 2; // تحديد المخرج الآخر
        GpioPin& otherPin = pins[otherPinIndex];

        if (pin.state == LOW) {
            // إيقاف المخرج الآخر إذا كان يعمل
                    // إيقاف المخرج الآخر وإعادة ضبط توقيته
            if (otherPin.state == HIGH) {
                otherPin.state = LOW;
                digitalWrite(otherPin.number, LOW);
                otherPin.activationTime = 0; // إلغاء العد التنازلي
                Serial.print("تم إغلاق: ");
                Serial.println(otherPin.name);
            }
            
            // تشغيل المخرج الحالي
                    // تشغيل المخرج الحالي مع الاحتفاظ بالمدة التلقائية
            pin.state = HIGH;
            digitalWrite(pin.number, HIGH);
            pin.activationTime = millis();
            Serial.print("تم تشغيل: ");
            Serial.println(pin.name);
        } else {
            // إيقاف المخرج الحالي
            pin.state = LOW;
            digitalWrite(pin.number, LOW);
            pin.activationTime = 0;
            Serial.print("تم إغلاق: ");
            Serial.println(pin.name);
        }
    }
    // معالجة باقي المخارج اليدوية (3-10) بشكل عادي 
    //بالضغط على هذه الأزرار تفتح و بالضغط مرة أخرى تغلق
    else {
        if (pin.state == LOW) {
            pin.state = HIGH;
            digitalWrite(pin.number, HIGH);
            pin.activationTime = millis();
            Serial.print("تم تشغيل: ");
            Serial.println(pin.name);
        } else {
            pin.state = LOW;
            digitalWrite(pin.number, LOW);
            pin.activationTime = 0;
            Serial.print("تم إغلاق: ");
            Serial.println(pin.name);
        }
    }
}
}

// قسم الدوال المساعدة
unsigned long calculateRemainingTime() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
  unsigned long remaining = totalDuration - elapsed;
  return remaining / 60000;
}

int calculateProgress() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
  int progress = (elapsed * 100) / totalDuration;
  return progress > 100 ? 100 : progress;
}

unsigned long calculateElapsedTime() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
  return elapsed / 60000;
}

int calculateElapsedProgress() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
  int progress = (elapsed * 100) / totalDuration;
  return progress > 100 ? 100 : progress;
}

String getSystemStatusJSON() {
  String json = "{";
  json += "\"manual1ActivationTime\":" + String(pins[2].activationTime) + ",";  // ⚡ إرسال الوقت بالميلي ثانية
  json += "\"manual2ActivationTime\":" + String(pins[3].activationTime) + ",";
  json += "\"out1\":" + String(pins[0].state ? "true" : "false") + ",";
  json += "\"out2\":" + String(pins[1].state ? "true" : "false") + ",";
  json += "\"manual1\":" + String(pins[2].state ? "true" : "false") + ",";
  json += "\"manual2\":" + String(pins[3].state ? "true" : "false") + ",";
  json += "\"manual3\":" + String(pins[4].state ? "true" : "false") + ",";
  json += "\"manual4\":" + String(pins[5].state ? "true" : "false") + ",";
  json += "\"manual5\":" + String(pins[6].state ? "true" : "false") + ",";
  json += "\"manual6\":" + String(pins[7].state ? "true" : "false") + ",";
  json += "\"manual7\":" + String(pins[8].state ? "true" : "false") + ",";
  json += "\"manual8\":" + String(pins[9].state ? "true" : "false") + ",";
  json += "\"manual9\":" + String(pins[10].state ? "true" : "false") + ",";
  json += "\"manual10\":" + String(pins[11].state ? "true" : "false") + ",";
  json += "\"systemActive\":" + String(toggleSystemActive ? "true" : "false") + ",";
  json += "\"systemPaused\":" + String(toggleSystemPaused ? "true" : "false") + ",";
  json += "\"remainingTime\":" + String(calculateRemainingTime()) + ",";
  json += "\"progress\":" + String(calculateProgress()) + ","; // تقدم الشريط
  json += "\"duration\":" + String(totalDuration / 60000) + ",";
  // json += "\"toggleInterval\":" + String(toggleInterval / 1000);
  json += "\"toggleInterval\":" + String(toggleInterval / 1000) + ",";
  json += "\"elapsedTime\":" + String(calculateElapsedTime()) + ","; // زمن منقضي
  json += "\"elapsedProgress\":" + String(calculateElapsedProgress());  // نسبة تقدم الشريط
  json += "}";
  return json;
}
