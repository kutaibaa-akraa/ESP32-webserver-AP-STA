// كود مطور و يعمل بكفاءة و الاتصال عبر wifi STA  أو عبر AP  مع واجهة حفظ إعادات الشبكة الجديدة و الاستعادة و الحذف
// تم إضافة زمني لكل المخارج اليدوية نقلاً عن ملف المشروع  -- ESP32-webserver-new-5.ino --- 002-نسخة
// =================== 📚 المكتبات ===================
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <WiFi.h>       // مكتبة الاتصال بالواي فاي
#include <WebServer.h>  // لإنشاء خادم ويب
#include <SPIFFS.h>     // نظام ملفات داخل الشريحة لتخزين الصفحات
// مكتبة رفع الملفات -SPIFFS.h- قديمة و تعمل على اردوينو 1.8.19
#include <Update.h> // مطلوبة لمعالجة التحديثات
#include <ESPmDNS.h>
#include <ArduinoJson.h>
//#include <esp32-hal.h> // --- معالج استثناءات ----
//#include <WiFiSettings.h> // --- https://github.com/Juerd/ESP-WiFiSettings ----

// =================== 🌐 إعدادات الشبكة ===================
const char* AP_SSID = "ESP32-Control";  // اسم نقطة الوصول Access Point
const char* AP_PASSWORD = "12345678";   // كلمة المرور

// ===== إضافة قسم إعدادات المصادقة =====
const char* AUTH_USERNAME = "admin";
const char* AUTH_PASSWORD = "admin";

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
#define MAX_SERIAL_MESSAGES 50
String serialMessages[MAX_SERIAL_MESSAGES];
int serialIndex = 0;

// =================== 📦 هياكل البيانات ===================
// هيكل لتخزين إعدادات Wi-Fi
// struct WiFiSettings {
//   char ssid[32];
//   char password[64];
//   bool useStaticIP;      // هل يتم استخدام IP ثابت
//   IPAddress localIP;     // العنوان المحلي
//   IPAddress gateway;     // البوابة
//   IPAddress subnet;      // القناع
// };

// استخدام هذا بدلاً منه:
struct WiFiNetwork {
  char ssid[32] = "";
  char password[64] = "";
  bool useStaticIP = false;
   IPAddress localIP = INADDR_NONE;  // بدلاً من 0.0.0.0
   IPAddress gateway = INADDR_NONE;
//  IPAddress localIP = IPAddress(192,168,1,100); // القيمة الافتراضية
//  IPAddress gateway = IPAddress(192,168,1,1);
  IPAddress subnet = IPAddress(255, 255, 255, 0);
  int priority = 0;
};

const int MAX_NETWORKS = 5;
WiFiNetwork wifiNetworks[MAX_NETWORKS];
SemaphoreHandle_t xMutex; //  لمنع تضارب الموارد المشتركة يضاف إلى المتغيرات العامة

// هيكل بيانات لإدارة المنافذ
struct GpioPin {
  int number;                     // رقم البن
  int state;                      // حالته (1 أو 0)
  unsigned long activationTime;   // وقت التفعيل الأخير
  unsigned long autoOffDuration;  // مدة التشغيل التلقائي قبل الإيقاف
  const char* name;               // اسم العرض
  const char* onCmd;              // رابط التشغيل
  const char* offCmd;             // رابط الإيقاف
  bool allowManualControl;        // هل يمكن التحكم يدوياً
};

//  ---- حفظ إعدادات الأزمنة ------- يجمع كل الإعدادات القابلة للحفظ في مكان واحد مع تحسين استخدام الذاكرة.----
// struct SystemSettings {
//   unsigned long toggleInterval;     // زمن التبادل (مللي ثانية)
//   unsigned long totalDuration;      // الزمن الكلي (مللي ثانية)
//   unsigned long togglePauseDuration; // زمن الاستراحة بين التبديلات (مللي ثانية)
//   unsigned long manualDurations[10]; // مدة التشغيل لكل مخرج يدوي (1-10)
// };
// ---- الدالة المعدلة بقيم افتراضية مسبقة منعاً لمشكلة تحميل قيم غير منطقية عند استعادة القيم ----
struct SystemSettings {
  unsigned long toggleInterval = 30000;     // قيمة افتراضية: 30 ثانية
  unsigned long totalDuration = 600000;    // قيمة افتراضية: 10 دقائق
  unsigned long togglePauseDuration = 0;   // قيمة افتراضية: لا استراحة
  unsigned long manualDurations[10] = {4000, 4000, 0, 0, 0, 0, 0, 0, 0, 0}; // 4 ثواني للأولين
};

// =================== 🌍 متغيرات عامة ===================
WebServer server(80);       // خادم ويب على المنفذ 80
// WiFiSettings wifiSettings;  // إعدادات الشبكة

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
const char* toggleBtnStart = "تشغيل البرنامج";
const char* toggleBtnStop = "إيقاف البرنامج";
const char* toggleOutputNames[] = { "دوران يمين", "دوران يسار" };
const char* manualOutputs[] = {
  "فتح الباب", "إغلاق الباب", "تعبئة ماء",
  "فتح بخار", "مكب تصريف", "إغلاق تصريف",
  "المخرج اليدوي 7", "المخرج اليدوي 8",
  "المخرج اليدوي 9", "المخرج اليدوي 10"
};
 
const char* SYSTEM_SETTINGS_FILE = "/system_settings.bin"; // استخدام تنسيق ثنائي للكفاءة ---- يقلل حجم الملف ويضمن قراءة/كتابة أسرع ------
const char* WIFI_CONFIG_FILE = "/wifi_config.bin"; // مسارات الملفات في SPIFFS
bool isConnected = false; // حالة الاتصال الحالية

// =================== 🌐 إعدادات mDNS ===================
 const char* MDNS_NAME = "esp32-control"; // اسم الجهاز الثابت

// متغيرات النظام
bool toggleSystemActive = false;
bool toggleSystemPaused = false;
unsigned long toggleStartTime = 0;
unsigned long totalDuration = 7200000;  // 120 دقيقة
unsigned long lastToggleTime = 0;
unsigned long pausedTime = 0;
unsigned long toggleInterval = 60000;
unsigned long savedToggleInterval = 60000;
unsigned long togglePauseDuration = 0; // زمن الاستراحة بين التبادل (مللي ثانية)
bool isInPauseBetweenToggle = false; // حالة الاستراحة بين التبادل
int currentToggleState = 0; // 0: غير نشط، 1: OUT1 نشط، 2: OUT2 نشط
unsigned long pauseStartTime = 0; // وقت بدء الاستراحة
unsigned long expectedToggleTime = 0; // 

// دالة التحقق من المصادقة
bool authenticate() {
  // تمكين المصادقة في كلا الوضعين (STA و AP)
  if (!server.authenticate(AUTH_USERNAME, AUTH_PASSWORD)) {
    server.sendHeader("WWW-Authenticate", "Basic realm=\"ESP32 Control Panel\"");
    server.send(401, "text/plain", "يتطلب المصادقة");
    return false;
  }
  return true;
}

// ============ Forward Declarations ============
// ============ التصريح المسبق عن الدوال قيل دالة الإعداد setup()  ============
bool isValidIP(const IPAddress& ip);
void createWiFiConfig();
void setupServer();
void STAsetup();
void connectToWiFi();
void startAPMode();
void saveWiFiConfig();
void handleConfigPage();
void handleSaveConfig();
void resetWiFiConfig(bool restoreDefaults);
String getSystemStatusJSON();
void toggleOutput(int pinIndex);
void toggleOutputs();
void setOutputStates(int state); // الآن نستطيع اختيار ثلاث حالات للمخارج التبادلية للتناسب مع زمن الاستراحة
void printPauseStateSmart(bool currentState);
void startToggleSystem();
void stopToggleSystem();
void pauseToggleSystem();
void resumeToggleSystem();
unsigned long calculateRemainingTime();
int calculateProgress();
void checkFileSystem();
void handleLoadSystemSettings();
void saveSystemSettings();
void setDefaultSystemSettings();
bool loadSystemSettings();
void handleSaveSystemSettings();
void handleFileUpload();
String getContentType(String filename);
String processTemplate(String html);
String readTemplateFile(String path);


// دالة مساعدة لتخزين رسائل السيريال
void logSerialMessage(String message) {
  serialMessages[serialIndex] = message;
  serialIndex = (serialIndex + 1) % MAX_SERIAL_MESSAGES;
}

void debugPrint(String message) {
  Serial.println(message);  // الطباعة العادية للسيريال
  logSerialMessage(message); // تخزين الرسالة للمراقبة
}

void printWiFiConfig() {
  if (SPIFFS.exists(WIFI_CONFIG_FILE)) {
    File file = SPIFFS.open(WIFI_CONFIG_FILE, "r");
    
    Serial.println("\n[فحص ملف wifi_config.bin]");
    
    // قراءة جميع الشبكات
    for (int i=0; i<MAX_NETWORKS; i++) {
      WiFiNetwork network;
      if (file.available() >= sizeof(WiFiNetwork)) {
        file.readBytes((char*)&network, sizeof(WiFiNetwork));
        
        Serial.printf("شبكة %d:\n", i+1);
        Serial.printf("  SSID: %s\n", network.ssid);
        Serial.printf("  Password: %s\n", network.password);
        Serial.printf("  useStaticIP: %s\n", network.useStaticIP ? "نعم" : "لا");
        Serial.printf("  Local IP: %s\n", network.localIP.toString().c_str());
        Serial.printf("  Gateway: %s\n", network.gateway.toString().c_str());
        Serial.printf("  Subnet: %s\n", network.subnet.toString().c_str());
        Serial.printf("  Priority: %d\n", network.priority);
        Serial.println("----------------------------");
      }
    }
    
    file.close();
  } else {
    Serial.println("ملف wifi_config.bin غير موجود!");
  }
}

bool initTemplates() {
  // تحقق من وجود ملفات القوالب
  if (!SPIFFS.exists("/index.html") || 
      !SPIFFS.exists("/config.html") ||
      !SPIFFS.exists("/upload.html") ||
      !SPIFFS.exists("/network.html")) {
    Serial.println("خطأ: ملفات القوالب مفقودة في SPIFFS");
    return false;
  }
  return true;
}

bool loadWiFiConfig() {
 if (!SPIFFS.exists(WIFI_CONFIG_FILE)) {
    Serial.println("❌ ملف الإعدادات غير موجود!");
    return false;
  }

  File file = SPIFFS.open(WIFI_CONFIG_FILE, "r");
  if (!file) {
    Serial.println("❌ فشل فتح ملف الإعدادات");
    return false;
  }

  // تعريف الهياكل القديمة داخل الدالة
  struct OldWiFiSettings {
    char ssid[32];
    char password[64];
    bool useStaticIP;
    IPAddress localIP;
    IPAddress gateway;
    IPAddress subnet;
  };
  
  struct OldWiFiSettingsWithoutIP {
    char ssid[32];
    char password[64];
  };
  
  size_t fileSize = file.size();
  Serial.printf("حجم ملف الإعدادات: %d بايت\n", fileSize);

  if (fileSize == sizeof(WiFiNetwork) * MAX_NETWORKS) {
    if (file.available() >= sizeof(wifiNetworks)) {
      file.readBytes((char*)&wifiNetworks, sizeof(wifiNetworks));
      file.close();
      
      // التحقق من صحة البيانات
      bool configValid = true;
      for (int i = 0; i < MAX_NETWORKS; i++) {
        if (strlen(wifiNetworks[i].ssid) > 32) {
          Serial.println("❌ SSID طويل جداً");
          configValid = false;
        }
        
        // إعادة بناء عناوين IP بشكل صحيح
        wifiNetworks[i].localIP = IPAddress(wifiNetworks[i].localIP);
        wifiNetworks[i].gateway = IPAddress(wifiNetworks[i].gateway);
        wifiNetworks[i].subnet = IPAddress(wifiNetworks[i].subnet);
      }
      
      if (configValid) {
        Serial.println("تم تحميل إعدادات Wi-Fi بنجاح");
        return true;
      }
    }
  }
  // الحالة 2: حجم ملف الإصدار القديم (نسخة واحدة مع IP)
  else if (fileSize == sizeof(OldWiFiSettings)) {
    Serial.println("⚙️ تم اكتشاف إعدادات قديمة (نسخة واحدة مع IP)");
    
    OldWiFiSettings oldSettings;
    size_t len = file.readBytes((char*)&oldSettings, sizeof(oldSettings));
    file.close();
    
    // التحقق من صحة القراءة
    if (len != sizeof(oldSettings)) {
      Serial.printf("❌ خطأ في قراءة الإعدادات (%d/%d بايت)\n", len, sizeof(oldSettings));
      return false;
    }
    
    // نسخ الإعدادات القديمة إلى أول شبكة في المصفوفة
    memcpy(wifiNetworks[0].ssid, oldSettings.ssid, sizeof(wifiNetworks[0].ssid)); wifiNetworks[0].ssid[sizeof(wifiNetworks[0].ssid) - 1] = '\0'; // التأكد من نهاية السلسلة
    strncpy(wifiNetworks[0].password, oldSettings.password, sizeof(wifiNetworks[0].password));
    wifiNetworks[0].useStaticIP = oldSettings.useStaticIP;
    wifiNetworks[0].localIP = oldSettings.localIP;
    wifiNetworks[0].gateway = oldSettings.gateway;
    wifiNetworks[0].subnet = oldSettings.subnet;
    wifiNetworks[0].priority = 1; // الأولوية القصوى
    
    Serial.printf("تم تحويل الإعدادات القديمة: %s\n", oldSettings.ssid);
    
    // حفظ التنسيق الجديد تلقائياً
    saveWiFiConfig();
    Serial.println("✓ تم تحويل الإعدادات بنجاح وحفظها بالتنسيق الجديد");
    return true;
  }
  
  // الحالة 3: ملفات الإصدار القديم جداً (بدون إعدادات IP)
  else if (fileSize == sizeof(OldWiFiSettingsWithoutIP)) {
    Serial.println("⚙️ تم اكتشاف إعدادات قديمة جداً (بدون IP)");
    
    OldWiFiSettingsWithoutIP oldSettings;
    size_t len = file.readBytes((char*)&oldSettings, sizeof(oldSettings));
    file.close();

    // التحقق من صحة القراءة
    if (len != sizeof(oldSettings)) {
      Serial.printf("❌ خطأ في قراءة الإعدادات (%d/%d بايت)\n", len, sizeof(oldSettings));
      return false;
    }
    
    // نسخ الإعدادات القديمة إلى أول شبكة في المصفوفة
    memcpy(wifiNetworks[0].ssid, oldSettings.ssid, sizeof(wifiNetworks[0].ssid)); wifiNetworks[0].ssid[sizeof(wifiNetworks[0].ssid) - 1] = '\0'; // التأكد من نهاية السلسلة
    strncpy(wifiNetworks[0].password, oldSettings.password, sizeof(wifiNetworks[0].password));
    wifiNetworks[0].priority = 1; // الأولوية القصوى
    wifiNetworks[0].useStaticIP = false;
    wifiNetworks[0].localIP = IPAddress(192,168,1,100);
    wifiNetworks[0].gateway = IPAddress(192,168,1,1);
    wifiNetworks[0].subnet = IPAddress(255,255,255,0);
    
    Serial.printf("تم تحويل الإعدادات القديمة: %s\n", oldSettings.ssid);
    
    // حفظ التنسيق الجديد تلقائياً
    saveWiFiConfig();
    Serial.println("✓ تم تحويل الإعدادات بنجاح وحفظها بالتنسيق الجديد");
    return true;
  }
  
  // حالة الفشل: حجم ملف غير معروف
  Serial.printf("❌ حجم ملف غير متوقع: %d بايت\n", fileSize);
  Serial.printf("  المتوقع للإصدار الجديد: %d بايت\n", sizeof(WiFiNetwork) * MAX_NETWORKS);
  Serial.printf("  المتوقع للإصدار القديم (مع IP): %d بايت\n", sizeof(OldWiFiSettings));
  Serial.printf("  المتوقع للإصدار القديم (بدون IP): %d بايت\n", sizeof(OldWiFiSettingsWithoutIP));
    
  file.close();
  return false;
}

//--- إضافة دالة لعرض حالة الشبكة الحالية ---
String getNetworkStatusHTML() {
  String html = "";
  html += "<div class='network-settings'>";
  html += "<table>";
  
  if (isConnected) {
    // البحث عن الشبكة المتصلة
    for (int i=0; i<MAX_NETWORKS; i++) {
      if (strlen(wifiNetworks[i].ssid) > 0 && 
          WiFi.SSID() == String(wifiNetworks[i].ssid)) {
        html += "<tr><td>وضع الاتصال:</td><td>STA (عميل)</td></tr>";
        html += "<tr><td>اسم الشبكة:</td><td>" + String(wifiNetworks[i].ssid) + "</td></tr>";
        html += "<tr><td>عنوان IP:</td><td>" + WiFi.localIP().toString() + "</td></tr>";
        html += "<tr><td>البوابة:</td><td>" + WiFi.gatewayIP().toString() + "</td></tr>";
        html += "<tr><td>القناع:</td><td>" + WiFi.subnetMask().toString() + "</td></tr>";
        break;
      }
    }
  } else {
    html += "<tr><td>وضع الاتصال:</td><td>AP (نقطة وصول)</td></tr>";
    html += "<tr><td>اسم نقطة الوصول:</td><td>" + String(AP_SSID) + "</td></tr>";
    html += "<tr><td>عنوان IP:</td><td>" + WiFi.softAPIP().toString() + "</td></tr>";
  }
  
  html += "</table></div>";
  return html;
}

// --- هذه الدالة الجديدة للتحكم في المخارج (النواة 0)
// مهمة النواة 0 (التحكم في المخارج)
void core0_task(void *parameter) {
  static unsigned long lastCheck = 0; // نقلنا المتغير هنا
  
  for(;;) {
    // التحكم في التبديل التبادلي
    if (toggleSystemActive && !toggleSystemPaused) {
      unsigned long currentTime = millis();
      if (currentTime - toggleStartTime >= totalDuration) {
        stopToggleSystem();
      } else {
        toggleOutputs();
      }
    }
    
    // التحقق من الإغلاق التلقائي (كل ثانية)
    if (millis() - lastCheck >= 1000) {
      lastCheck = millis();
      for (int i = 2; i <= 11; i++) {
        GpioPin& pin = pins[i];
        if (pin.autoOffDuration > 0 && 
            pin.state == HIGH && 
            millis() - pin.activationTime >= pin.autoOffDuration) 
        {
          // حماية الوصول للموارد المشتركة
          if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
            pin.state = LOW;
            digitalWrite(pin.number, LOW);
            pin.activationTime = 0;
            xSemaphoreGive(xMutex);
          }
        }
      }
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS); // تأخير 10ms
  }
}

void setup() {
  Serial.begin(115200);

  // إنشاء مهمة منفصلة للنواة 0 (التحكم في المخارج)
  xTaskCreatePinnedToCore(
    core0_task,    // اسم الدالة
    "Core0_Task",  // اسم المهمة
    10000,         // حجم المكدس (بايت)
    NULL,          // المعاملات
    1,             // الأولوية
    NULL,          // مقبض المهمة
    0              // النواة (0)
  );

  STAsetup();  // ------- دوال تشغيل الشبكة ------
  
  // -------- تهيئة المنافذ -----------
  for (auto& pin : pins) {
    pinMode(pin.number, OUTPUT);
    digitalWrite(pin.number, pin.state);
    Serial.print("تم تهيئة المخرج: ");
    Serial.println(pin.name);
  }
  // -------- تهيئة SPIFFS ---------
  if (!SPIFFS.begin(true)) {
    Serial.println("فشل في تهيئة SPIFFS!");
    return;
  }
  checkFileSystem();

  xMutex = xSemaphoreCreateMutex(); // بعد تهيئة SPIFFS لمنع تضارب الموارد

if (!initTemplates()) {
  Serial.println("خطأ في تحميل القوالب!");
}

  // --------مثال تحقق من وجود ملفات ضرورية --------
  if (!SPIFFS.exists("/css/all.min.css")) {
    Serial.println("ملف الستايل مفقود!");
  }
  // --------- تحميل إعدادات Wi-Fi ----------
  if (loadWiFiConfig()) {
    connectToWiFi();
  } else {
    startAPMode();
  }
  
  printWiFiConfig();
    server.send(200, "application/json", getSystemStatusJSON());
    
  server.on("/saveConfig", HTTP_POST, []() {
    if (!authenticate()) return; // --- مصادقة قبل الوصول للصفحة ---
    handleSaveConfig();
  });

    // --- تثبيت اسم بديل ثابت للعنوان مهما تغير -------     استخدم هذا الرابط بدلاً من IP:  http://esp32-control.local
    if (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP) {
  if (!MDNS.begin(MDNS_NAME)) {
    Serial.println("فشل في بدء خدمة mDNS!");
  } else {
    Serial.println("تم تثبيت العنوان الثابت: http://" + String(MDNS_NAME) + ".local");
    MDNS.addService("http", "tcp", 80); // إضافة خدمة الويب
  }
}

  if (!loadSystemSettings()) {
    Serial.println("تم تهيئة الإعدادات الزمنية بالقيم الافتراضية");
  }
   
   if (!SPIFFS.exists("/wifi_config.bin")) {
    Serial.println("ملف الإعدادات غير موجود، جاري الإنشاء...");
    createWiFiConfig(); // --- إنشاء ملف إعدادات إن كان غير موجود ----
  }

  // // ----- بدء نقطة الوصول AP --------
  // // --- يمكن تفعيل هذه الحالة و الاتصال باستخدام نقطة الوصول APحتى لو كان متصلاً بحالة STA
  // WiFi.softAP(AP_SSID, AP_PASSWORD);
  // Serial.println("\nنقطة الوصول جاهزة");
  // Serial.print("SSID: ");
  // Serial.println(AP_SSID);
  // Serial.print("PASSWORD: ");
  // Serial.println(AP_PASSWORD);
  // Serial.print("عنوان IP: ");
  // Serial.println(WiFi.softAPIP());

    // التحقق من صحة الشبكات
  for (int i = 0; i < MAX_NETWORKS; i++) {
    if (wifiNetworks[i].useStaticIP && !isValidIP(wifiNetworks[i].localIP)) {
      Serial.printf("تعطيل IP ثابت للشبكة %d بسبب عنوان IP غير صالح\n", i);
      wifiNetworks[i].useStaticIP = false;
    }
  }

  server.on("/createConfig", HTTP_GET, []() { // -- اختياري لإنشاء مسار و استدعؤه لإنشاء ملف إعدادات شبكة ------ الاستدعاء http://esp32-ip/createConfig
  createWiFiConfig();
  server.send(200, "text/plain", "تم إنشاء الإعدادات!");
});

  // أضف المسار الجديد (قبل setupServer())
server.on("/serial", HTTP_GET, []() {
  String serialContent = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl">
<head>
  <meta charset="UTF-8">
  <title>مراقبة السيريال</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      background-color: #f5f5f5;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
      background: white;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      text-align: center;
    }
    #serialOutput {
      width: 100%;
      height: 400px;
      font-family: monospace;
      padding: 10px;
      border: 1px solid #ddd;
      border-radius: 4px;
      background-color: #f9f9f9;
      resize: none;
      white-space: pre;
    }
    .controls {
      margin-top: 10px;
      text-align: center;
    }
    button {
      padding: 8px 16px;
      background-color: #3498db;
      color: white;
      border: none;
      border-radius: 4px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>مراقبة السيريال</h1>
    <textarea readonly id="serialOutput"></textarea>
    <div class="controls">
      <button onclick="refreshSerial()">تحديث</button>
      <button onclick="clearSerial()">مسح</button>
    </div>
  </div>

  <script>
    function loadSerialData() {
      fetch('/serialData')
        .then(response => response.json())
        .then(data => {
          document.getElementById('serialOutput').value = data.serialData;
          document.getElementById('serialOutput').scrollTop = 
            document.getElementById('serialOutput').scrollHeight;
        });
    }

    function refreshSerial() {
      loadSerialData();
    }

    function clearSerial() {
      fetch('/clearSerial')
        .then(() => loadSerialData());
    }

    // التحديث التلقائي كل 2 ثانية
    setInterval(loadSerialData, 2000);
    // تحميل البيانات عند فتح الصفحة
    window.onload = loadSerialData;
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", serialContent);
});

// أضف مسارًا جديدًا لبيانات السيريال
server.on("/serialData", HTTP_GET, []() {
  String allMessages = "";
  for (int i = 0; i < MAX_SERIAL_MESSAGES; i++) {
    int idx = (serialIndex + i) % MAX_SERIAL_MESSAGES;
    if (serialMessages[idx] != "") {
      allMessages += serialMessages[idx] + "\n";
    }
  }
  String json = "{\"serialData\":\"" + allMessages + "\"}";
  server.send(200, "application/json", json);
});

// أضف مسارًا لمسح السيريال
server.on("/clearSerial", HTTP_POST, []() {
  for (int i = 0; i < MAX_SERIAL_MESSAGES; i++) {
    serialMessages[i] = "";
  }
  serialIndex = 0;
  server.send(200, "text/plain", "تم مسح السيريال");
});

  setupServer();

  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    String json = getSystemStatusJSON();
    server.send(200, "application/json", json);
  });

server.on("/getNetworks", HTTP_GET, []() {
JsonDocument doc;
  JsonArray networks = doc.to<JsonArray>();
  
  for (int i = 0; i < MAX_NETWORKS; i++) {
    if (strlen(wifiNetworks[i].ssid) > 0) {
      JsonObject net = networks.createNestedObject();
      net["ssid"] = wifiNetworks[i].ssid;
      net["priority"] = wifiNetworks[i].priority;
      net["password"] = wifiNetworks[i].password;
      net["useStaticIP"] = wifiNetworks[i].useStaticIP;
      net["localIP"] = wifiNetworks[i].localIP.toString();
      net["gateway"] = wifiNetworks[i].gateway.toString();
      net["subnet"] = wifiNetworks[i].subnet.toString();
    }
  }
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
});

  server.on("/setDuration", HTTP_POST, []() {  // ----- لتعديل مدة تشغيل المخرج بالثواني يدوياً من الواجهة -----
  if (server.hasArg("pin") && server.hasArg("duration")) {
    int pinNumber = server.arg("pin").toInt(); // رقم المخرج (1-10)
    unsigned long duration = server.arg("duration").toInt(); // المدة بالمللي ثانية

    // التحقق من أن رقم المخرج صالح (1-10)
    if (pinNumber >= 1 && pinNumber <= 10) {
      int pinIndex = pinNumber + 1; // لأن الفهرس 2 هو manual1، 3 هو manual2، إلخ
      pins[pinIndex].autoOffDuration = duration;
      
      Serial.print("تم تعيين مدة المخرج ");
      Serial.print(pins[pinIndex].name);
      Serial.print(" إلى ");
      Serial.println(duration / 1000);
    }
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
    if (server.hasArg("pauseDuration")) { // إضافة حفظ زمن الاستراحة
      togglePauseDuration = server.arg("pauseDuration").toInt() * 1000;
      Serial.print("[حفظ] زمن الاستراحة: ");
      Serial.println(togglePauseDuration / 1000);
    }
    server.send(200, "application/json", getSystemStatusJSON());
  });

  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");  // 7 أيام
  server.begin();
}

// =================== 🔁 حلقة التنفيذ  الحلقة الرئيسية ===================
void loop() {
  server.handleClient();

  // إعادة الاتصال بالشبكة (النواة 1)
  if (isConnected && WiFi.status() != WL_CONNECTED) {
    if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      isConnected = false;
      xSemaphoreGive(xMutex);
    }
    connectToWiFi();
  }
    if (Serial.available() > 0) { // --- إنشاء الملف عند الضغط على زر c في السيريال ---
    char input = Serial.read();
    if (input == 'C' || input == 'c') {
      createWiFiConfig();
    }
  }

  // ---- تم حذف هذا القسم لضرورة التغييرات باستخدام دالة جديدة void core0_task(void *parameter) -----
//  // التحكم في التبديل التبادلي
//  if (toggleSystemActive && !toggleSystemPaused) {
//    unsigned long currentTime = millis();
//    
//    // إيقاف النظام عند انتهاء المدة الكلية
//    if (currentTime - toggleStartTime >= totalDuration) {
//      stopToggleSystem();
//    } 
//    // التحكم في التبديل والاستراحة
//    else {
//      toggleOutputs(); // يتم كل المنطق هنا
//    }
//  }
//
//  // التحقق من الإغلاق التلقائي للمخارج اليدوية (كل ثانية)
//  static unsigned long lastCheck = 0;
//  if (millis() - lastCheck >= 1000) {
//    lastCheck = millis();
//    for (int i = 2; i <= 11; i++) {
//      GpioPin& pin = pins[i];
//      if (pin.autoOffDuration > 0 && pin.state == HIGH) {
//        if (millis() - pin.activationTime >= pin.autoOffDuration) {
//          pin.state = LOW;
//          digitalWrite(pin.number, LOW);
//          pin.activationTime = 0;
//          Serial.print("إغلاق تلقائي: ");
//          Serial.println(pin.name);
//        }
//      }
//    }
//  }
    vTaskDelay(100 / portTICK_PERIOD_MS); // تأخير 100ms
}

// ---- معالجة طلبات رفع الملفات الى الشريحة و تحديد ضوابط لذلك -----
void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  static File fsUploadFile;
  String filename;
  bool isAdminFile = false;

  if (upload.status == UPLOAD_FILE_START) {
    // ------ تحديد المسار بناءً على نوع الملف ------
    String targetDir = "/";
    if (upload.filename.endsWith(".css")) {
      targetDir = "/css/";
    } else if (upload.filename.endsWith(".js")) {
      targetDir = "/js/";
    } else if (upload.filename.endsWith(".woff2")) {
      targetDir = "/fonts/";
    } else if (upload.filename.endsWith(".bin") || upload.filename.endsWith(".txt")) {
      targetDir = "/"; // ملفات النظام في المسار الجذر
      isAdminFile = true;
    }

    // ------ إنشاء المسار الكامل ------
    filename = targetDir + upload.filename;

    // ------ التحقق من الصلاحيات للملفات الحساسة ------
    if (isAdminFile) {
      if (!server.authenticate("admin", "admin")) {
        server.send(403, "text/plain", "يتطلب صلاحية مدير");
        return;
      }
    }

    // ------ التحقق من الهيكل المسموح ------
    const bool validPath = (
      filename.startsWith("/css/") ||
      filename.startsWith("/js/") ||
      filename.startsWith("/fonts/") ||
      filename.startsWith("/webfonts/") || 
      filename.endsWith(".html") ||        // السماح بجميع ملفات HTML
      filename.equals("/wifi_config.bin") ||
      filename.equals("/system_settings.bin")
    );

    if (!validPath) {
      server.send(403, "text/plain", "مسار غير مصرح به");
      return;
    }

    // ------ منع التعديل على ملفات النظام دون صلاحية ------
    if (filename.startsWith("/system/")) {
      server.send(403, "text/plain", "ملف نظام - ممنوع التعديل");
      return;
    }

    // ------ التحقق من مساحة التخزين ------
    if (SPIFFS.totalBytes() - SPIFFS.usedBytes() < upload.totalSize) {
      server.send(507, "text/plain", "مساحة التخزين غير كافية");
      return;
    }

    // ------ تحديد الحد الأقصى لحجم الملف ------
if (
    (filename.endsWith(".css") && upload.totalSize > 20480) ||
    (filename.endsWith(".bin") && upload.totalSize > 5120) ||
    (filename.endsWith(".html") && upload.totalSize > 30720) // 30KB لملفات HTML
  ) {
    server.send(413, "text/plain", "حجم الملف يتجاوز الحد المسموح");
    return;
  }

    // ------ إنشاء المجلدات إذا لزم الأمر ------
    if (!SPIFFS.exists(targetDir)) {
      SPIFFS.mkdir(targetDir);
    }

    // ------ فتح الملف للكتابة ------
    fsUploadFile = SPIFFS.open(filename, "w");
    if (!fsUploadFile) {
      server.send(500, "text/plain", "فشل في إنشاء الملف");
      return;
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // ------ كتابة البيانات الجزئية ------
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    // ------ إغلاق الملف بعد الرفع الكامل ------
    if (fsUploadFile) {
      fsUploadFile.close();
      Serial.printf(
        "تم رفع الملف: %s (%u بايت)\n",
        filename.c_str(),
        upload.totalSize
      );
    }
  }
}

// حفظ الإعدادات الزمنية
void saveSystemSettings() {
  SystemSettings settings;

 settings.toggleInterval = toggleInterval;
 settings.totalDuration = totalDuration;
settings.togglePauseDuration = togglePauseDuration; // حفظ زمن الاستراحة
  for (int i=0; i<10; i++) {
    settings.manualDurations[i] = pins[i+2].autoOffDuration; // الفهرس 2-11 للمخارج اليدوية
  }

  File file = SPIFFS.open(SYSTEM_SETTINGS_FILE, "w");
  file.write((uint8_t*)&settings, sizeof(settings));
  file.close();
}

// تحميل الإعدادات  الزمنية المحفوظة

void setDefaultSystemSettings() {
  // القيم الافتراضية للإعدادات الزمنية
  toggleInterval = 30000;      // 30 ثانية
  totalDuration = 600000;      // 10 دقائق
  togglePauseDuration = 0;     // لا يوجد استراحة
  
  // القيم الافتراضية للمخارج اليدوية
  for (int i=2; i<12; i++) {
    pins[i].autoOffDuration = (i == 2 || i == 3) ? 4000 : 0; // 4 ثواني للمخرجين 1 و2 فقط
  }
}

bool loadSystemSettings() {
  // التحقق من وجود الملف أولاً
  if (!SPIFFS.exists(SYSTEM_SETTINGS_FILE)) {
    Serial.println("ملف الإعدادات غير موجود، استخدام القيم الافتراضية");
    setDefaultSystemSettings(); // استدعاء الدالة التي تعين القيم الافتراضية
    return false;
  }

  File file = SPIFFS.open(SYSTEM_SETTINGS_FILE, "r");
  if (!file || file.size() != sizeof(SystemSettings)) {
    Serial.println("ملف الإعدادات تالف، استخدام القيم الافتراضية");
    if (file) file.close();
    setDefaultSystemSettings();
    return false;
  }

  SystemSettings settings;
  size_t bytesRead = file.readBytes((char*)&settings, sizeof(settings));
  file.close();

  if (bytesRead != sizeof(SystemSettings)) {
    Serial.println("خطأ في قراءة الملف، استخدام القيم الافتراضية");
    setDefaultSystemSettings();
    return false;
  }

  // تطبيق الإعدادات فقط إذا كانت القيم منطقية
  if (settings.toggleInterval >= 5000 && settings.toggleInterval <= 3600000 &&
      settings.totalDuration >= 60000 && settings.totalDuration <= 7200000) {
    toggleInterval = settings.toggleInterval;
    totalDuration = settings.totalDuration;
    togglePauseDuration = settings.togglePauseDuration;
    
    for (int i=0; i<10; i++) {
      if (settings.manualDurations[i] <= 300000) { // 5 دقائق كحد أقصى لكل مخرج
        pins[i+2].autoOffDuration = settings.manualDurations[i];
      } else {
        pins[i+2].autoOffDuration = 0; // قيمة افتراضية إذا كانت غير منطقية
      }
    }
    return true;
  } else {
    Serial.println("إعدادات غير منطقية في الملف، استخدام القيم الافتراضية");
    setDefaultSystemSettings();
    return false;
  }
}

// --- إضافة دالة لإنشاء ملف إعدادات افتراضي  ---
void createDefaultSettingsFile() {
  SystemSettings defaults;
  File file = SPIFFS.open(SYSTEM_SETTINGS_FILE, "w");
  file.write((uint8_t*)&defaults, sizeof(defaults));
  file.close();
}

void handleSaveSystemSettings() {
  saveSystemSettings();
  server.send(200, "text/plain", "تم حفظ الإعدادات!");
}

void handleLoadSystemSettings() {
  if (loadSystemSettings()) {
    server.send(200, "application/json", getSystemStatusJSON());
    Serial.println("تم استعادة الإعدادات بنجاح");
  } else {
    // إرسال القيم الحالية (التي أصبحت افتراضية الآن) مع رسالة تحذير
    String json = getSystemStatusJSON();
    json.replace("\"}", "\",\"message\":\"تم استخدام القيم الافتراضية\"}");
    server.send(200, "application/json", json);
    Serial.println("تم استخدام القيم الافتراضية");
  }
}

void STAsetup() {
  // مسار استعادة الإعدادات الافتراضية للشبكة المفترضة
  server.on("/resetConfigDefault", HTTP_POST, []() {
    resetWiFiConfig(true);
    server.send(200, "text/plain", "تم استعادة الإعدادات الافتراضية!");
    delay(1000);
    ESP.restart();
  });

  // مسار حذف الإعدادات ---- حذف ملف ال  wifi_config.bin -------
  server.on("/resetConfigDelete", HTTP_POST, []() {
    resetWiFiConfig(false);
    server.send(200, "text/plain", "تم حذف الإعدادات!");
    delay(1000);
    ESP.restart();
  });
}

// ------ إعداد الخادم ------
void setupServer() {

  server.on("/", HTTP_GET, []() {
  
  // ---  تم إضافة هذا الجزء لفتحصفحة السيرفر الرئيسية في كلا الحالتين AP و STA ----
  // --- و هو التعديل الضروري لقدرة الوصول إلى صفحة التحكم في وضعية عدم وجود شبكات حول الشريحة لتتصل بها
  //--- يبقى الوصول إلى إعدادات الشبكة و إدخال إعدادات شبكة جديدة عبر الواجهة الرئيسية نفسها  مثل إضافة /config  للصفحة
  if (!isConnected) {
    // في وضع AP: عرض صفحة التحكم مباشرة
    String html = readTemplateFile("/index.html");
    if (html != "") {
      html = processTemplate(html);
      server.send(200, "text/html", html);
      return;
    }
  }
// --- نهاية الجزء المضاف ---

  // إضافة تحقق من المصادقة في وضع STA فقط
  if (isConnected && !authenticate()) {
    server.requestAuthentication();
    return;
  }

// --- تعيين صفحة السيرفر الرئيسية في وضع اتصال STA ---
      if (server.uri() == "/networkstatus") {
    File file = SPIFFS.open("/network.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Error loading network page");
      return;
    }

    String html = file.readString();
    file.close();
    html.replace("%NETWORK_STATUS%", getNetworkStatusHTML());
    
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    server.send(200, "text/html", html);
    return;
  }

  if (isConnected || server.uri() == "/config") {

    String html = readTemplateFile("/index.html");
    if (html == "") {
      server.send(500, "text/plain", "Error loading template");
      return;
    }

  
    html = processTemplate(html);
    
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(200, "text/html", html);
  } 
  
  else {
    handleConfigPage();
  }
});

// --- لتسجيل الخروج من جلسات تعديل الصفحات الحساسة ---
server.on("/logout", HTTP_POST, []() {
  server.sendHeader("WWW-Authenticate", "Basic realm=\"Restricted\"");
  server.send(401);
});

server.on("/checkAuth", HTTP_GET, []() {
  if (!server.authenticate(AUTH_USERNAME, AUTH_PASSWORD)) {
    server.send(401, "text/plain", "Unauthorized");
  } else {
    server.send(200, "text/plain", "Authenticated");
  }
});

  // خدمة الملفات الثابتة من SPIFFS
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/config", SPIFFS, "/config.html");
  server.serveStatic("/uploadfile", SPIFFS, "/upload.html");
  server.serveStatic("/webfonts/", SPIFFS, "/webfonts/");
  // server.serveStatic("/networkstatus", SPIFFS, "/network.html");
  
  // مسارات الملفات الثابتة الأخرى
  server.serveStatic("/css/", SPIFFS, "/css/");
  server.serveStatic("/js/", SPIFFS, "/js/");
  server.serveStatic("/fonts/", SPIFFS, "/fonts/");

    // أضف هذا المسار للوصول إلى صفحة الإعدادات في كلا الوضعين (STA وAP)
  server.on("/config", HTTP_GET, []() {
    if (!isConnected) {
      // إذا كان في وضع AP، عرض الإعدادات مباشرة
      handleConfigPage();
    } else {
      // إذا كان في وضع STA، تحقق من الصلاحيات أولاً
      if (!authenticate()) return;
       server.sendHeader("Cache-Control", "public, max-age=60"); // 60 ثانية
       handleConfigPage();
    }
  });

// إضافة مسار رفع الملفات
server.on("/uploadfile", HTTP_GET, []() {
     if (!authenticate()) return; // --- للمصادقة للوصول إلى الصفحة ---
  
  String html = readTemplateFile("/upload.html");
  if (html == "") {
    server.send(500, "text/plain", "Error loading upload page");
    return;
  }
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.send(200, "text/html", html);
});

// إضافة مسار حالة الشبكة
server.on("/networkstatus", HTTP_GET, []() {
    if (!authenticate()) return;
  // فتح ملف network.html مباشرة
  File file = SPIFFS.open("/network.html", "r");
  if (!file || file.isDirectory()) {
    server.send(500, "text/plain", "Error loading network page");
    return;
  }
  
  // قراءة محتوى الملف
  String html = file.readString();
  file.close();
  
  // استبدال العنصر النائب
  html.replace("%NETWORK_STATUS%", getNetworkStatusHTML());
  
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
  server.send(200, "text/html", html);
});

  // ------- مسارات الحفظ و الاستعادة للأزمنة -----------------------
//  server.on("/saveSystemSettings", HTTP_POST, handleSaveSystemSettings);
//  server.on("/loadSystemSettings", HTTP_GET, handleLoadSystemSettings);

  server.on("/loadSystemSettings", HTTP_GET, []() {
  if (!authenticate()) return;
  handleLoadSystemSettings();
});
  
  server.on("/saveSystemSettings", HTTP_POST, []() {
    if (!authenticate()) return;
    handleSaveSystemSettings();
  });

  

// ----- مسار تحميل الملفات و رفعها ------
  server.on("/upload", HTTP_POST, []() { 
      if (!authenticate()) return;
      server.send(200);
    },
    handleFileUpload
  );

server.on("/download", HTTP_GET, []() {
  String filename = server.arg("file");
  if (SPIFFS.exists(filename)) {
    File file = SPIFFS.open(filename, "r");
    
    // إضافة الرؤوس المطلوبة
    server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
          if (!authenticate()) return;
    server.streamFile(file, getContentType(filename));
    file.close();
  } else {
    server.send(404, "text/plain", "الملف غير موجود");
  }
});


// // ---مراقبة السيريال ----------
// server.on("/serial", HTTP_GET, []() {
//   String serialContent = "<html dir='rtl'><head><meta charset='UTF-8'>";
//   serialContent += "<title>مراقبة السيريال</title>";
//   serialContent += "<style>body{font-family: Arial; margin: 20px;}";
//   serialContent += "textarea{width:100%; height:300px;}</style>";
//   serialContent += "<meta http-equiv='refresh' content='2'></head><body>";
//   serialContent += "<h1>مراقبة السيريال</h1>";
//   serialContent += "<textarea readonly id='serialOutput'></textarea>";
//   serialContent += "<script>document.getElementById('serialOutput').value = '";
  
//   // هنا يمكنك إضافة آخر رسائل السيريال
//   // (هذا مثال بسيط، تحتاج لتنفيذ طريقة لتخزين الرسائل)
  
//   serialContent += "'; window.scrollTo(0,document.body.scrollHeight);</script>";
//   serialContent += "</body></html>";
  
//   server.send(200, "text/html", serialContent);
// });

  // ----- لتكوين الستايل الخارجي لتفعيل الفونت ------------------
  server.on("/css/all.min.css", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/css/all.min.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

    server.on("/js/main.js", HTTP_GET, []() {  //  أكواد الجافاسكريبت
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/js/main.js", "r");
    server.streamFile(file, "text/javascript");
    file.close();
  });

  server.on("/css/main.css", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/css/main.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/config.css", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/css/config.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/js/config.js", HTTP_GET, []() {  //  أكواد الجافاسكريبت
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/js/config.js", "r");
    server.streamFile(file, "text/javascript");
    file.close();
  });

  server.on("/css/cairo.css", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/css/cairo.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/tajawal.css", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/css/tajawal.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/fonts/Cairo-SemiBold.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff2", "r");
    server.streamFile(file, "application/font-woff2");  // ✅
    file.close();
  });

  server.on("/fonts/Tajawal-Regular.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/fonts/Tajawal-Regular.woff2", "r");
    server.streamFile(file, "application/font-woff2");  // ✅
    file.close();
  });

  server.on("/webfonts/fa-solid-900.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/webfonts/fa-solid-900.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-v4compatibility.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/webfonts/fa-v4compatibility.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-regular-400.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/webfonts/fa-regular-400.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-brands-400.woff2", HTTP_GET, []() {
  server.sendHeader("Cache-Control", "public, max-age=30"); // 30 ثانية
    File file = SPIFFS.open("/webfonts/fa-brands-400.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  // ------ مسارات تبديل المخارج -------
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

  // ----- مسارات النظام التبادلي -----
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

  // ------- إضافة مسار جديد في الخادم ( /debug) يستقبل الطلبات ويطبع الرسالة على السيريال ------ إرسال طلب إلى السيرفر لطباعة الرسالة
  server.on("/debug", HTTP_GET, []() {  
    if (server.hasArg("msg")) {
      String message = server.arg("msg");
      Serial.print("رسالة من الواجهة: ");
      Serial.println(message);
    }
    server.send(200, "text/plain", "OK");
  });

  // ---أعدادات السيرفر 10دقيقة * 30 ثانية-----------------------
  server.on("/resetSettings", HTTP_POST, []() {
    toggleInterval = 30 * 1000;      // 30 ثانية
    totalDuration = 10 * 60 * 1000;  // 10 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 30ثانية*10دقائق");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  // --- أعدادات السيرفر 20دقيقة * 30 ثانية ---------------------
  server.on("/resetSettings3020", HTTP_POST, []() {
    toggleInterval = 30 * 1000;      // 30 ثانية
    totalDuration = 20 * 60 * 1000;  // 20 دقائق
    Serial.println("[إعادة] الإعدادات إلى القيم 30ثانية*20دقيقة");
    server.send(200, "application/json", getSystemStatusJSON());
  });

  // --- أعدادات السيرفر 15دقيقة * 60 ثانية ---------------------
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

// دالة مرتبطة باستدعاء في الجافاسكريبت لحفظ إعدادات و تقيير القيم المدخلة
server.on("/updateSettings", HTTP_POST, []() {
  // معالجة الفاصل الزمني
  if (server.hasArg("interval")) {
    int newInterval = server.arg("interval").toInt();
    if (newInterval >= 5 && newInterval <= 300) { // تحقق من القيم المسموحة
      toggleInterval = newInterval * 1000;
      Serial.printf("[إعداد] الفاصل الزمني: %d ثانية\n", newInterval);
    }
  }

  // معالجة المدة الكلية
  if (server.hasArg("duration")) {
    int newDuration = server.arg("duration").toInt();
    if (newDuration >= 1 && newDuration <= 120) { // تحقق من القيم المسموحة
      totalDuration = newDuration * 60000;
      Serial.printf("[إعداد] المدة الكلية: %d دقيقة\n", newDuration);
    }
  }

  // معالجة مدة الاستراحة الجديدة
  if (server.hasArg("pauseDuration")) {
    int newPause = server.arg("pauseDuration").toInt();
    if (newPause >= 0 && newPause <= 10) { // 0-10 ثانية
      togglePauseDuration = newPause * 1000;
      Serial.printf("[إعداد] مدة الاستراحة: %d ثانية\n", newPause);
    }
  }

  // إرسال الاستجابة مع كافة القيم المحدثة
  server.send(200, "application/json", getSystemStatusJSON());
});

  // ----- مسار للتحقق من وجود الملفات في spiffs بشكل يدوي بوجود زر في الواجهة
  server.on("/checkFiles", HTTP_GET, []() {
    checkFileSystem();
    server.send(200, "text/plain", "تم فحص الملفات - راجع السيريال مونيتور");
  });
}

void resetWiFiConfig(bool restoreDefaults) {
  if (restoreDefaults) {
    // استعادة الإعدادات الافتراضية للشبكة الأولى
    memset(wifiNetworks, 0, sizeof(wifiNetworks));
    strncpy(wifiNetworks[0].ssid, "ESP32-Control", sizeof(wifiNetworks[0].ssid));
    strncpy(wifiNetworks[0].password, "12345678", sizeof(wifiNetworks[0].password));
    wifiNetworks[0].priority = 1;
    
    saveWiFiConfig();
    Serial.println("تم استعادة الإعدادات الافتراضية!");
  } else {
    SPIFFS.remove(WIFI_CONFIG_FILE);
    Serial.println("تم حذف ملف الإعدادات!");
  }
}

void saveWiFiConfig() {
  File file = SPIFFS.open(WIFI_CONFIG_FILE, "w");
  if (!file) return;
  
  for (int i=0; i<MAX_NETWORKS; i++) {
    file.write((uint8_t*)&wifiNetworks[i], sizeof(WiFiNetwork));
  }
  file.close();
}

//bool isValidIP(const IPAddress& ip) {
//  if (ip[0] == 0 || ip[0] == 255) return false;
// // return ip != INADDR_NONE && ip.toString() != "0.0.0.0";
//   return ip != INADDR_NONE;
//}

bool isValidIP(const IPAddress& ip) {
  // العناوين الصحيحة: ليست 0.0.0.0 ولا 255.255.255.255
  return (ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0) && 
         !(ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255);
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true); // إعادة تعيين كاملة
  
  // فرز الشبكات حسب الأولوية (الأعلى أولاً)
  std::sort(wifiNetworks, wifiNetworks + MAX_NETWORKS, [](const WiFiNetwork& a, const WiFiNetwork& b) {
    return a.priority > b.priority;
  });

  // مسح إعدادات الـ IP السابقة
  WiFi.config(IPAddress(), IPAddress(), IPAddress());

  // تجربة الاتصال بكل الشبكات حسب الأولوية
  for (int i = 0; i < MAX_NETWORKS; i++) {
    if (strlen(wifiNetworks[i].ssid) == 0) {
      continue; // تخطي الشبكات الفارغة
    }

    Serial.printf("\nمحاولة الاتصال بالشبكة %d (الأولوية: %d): %s\n", 
                  i+1, wifiNetworks[i].priority, wifiNetworks[i].ssid);

    // تطبيق إعدادات IP الثابتة إذا كانت مفعلة لهذه الشبكة
    if (wifiNetworks[i].useStaticIP) {
      Serial.println("⚙️ تطبيق إعدادات IP ثابتة...");
      
      // التحقق من صحة العناوين قبل التطبيق
      if (isValidIP(wifiNetworks[i].localIP) && 
          isValidIP(wifiNetworks[i].gateway) &&
          isValidIP(wifiNetworks[i].subnet)) 
      {
        // طباعة الإعدادات
        Serial.printf("  - IP: %s\n", wifiNetworks[i].localIP.toString().c_str());
        Serial.printf("  - Gateway: %s\n", wifiNetworks[i].gateway.toString().c_str());
        Serial.printf("  - Subnet: %s\n", wifiNetworks[i].subnet.toString().c_str());
        
        // التطبيق مع معالجة الأخطاء
        if (WiFi.config(wifiNetworks[i].localIP, 
                        wifiNetworks[i].gateway, 
                        wifiNetworks[i].subnet,
                        wifiNetworks[i].gateway)) // DNS = Gateway
        {
          Serial.println("✅ تم تطبيق الإعدادات بنجاح");
        } else {
          Serial.println("⚠️ فشل في تطبيق الإعدادات الثابتة!");
        }
      } else {
        Serial.println("⛔ عناوين IP غير صالحة، استخدام DHCP بدلاً من ذلك");
        WiFi.config(IPAddress(), IPAddress(), IPAddress()); // إعادة تعيين إلى DHCP
      }
    }

    // بدء الاتصال
    WiFi.begin(wifiNetworks[i].ssid, wifiNetworks[i].password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    // التحقق من نجاح الاتصال
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ تم الاتصال بنجاح!");
      Serial.print("  - عنوان IP: ");
      Serial.println(WiFi.localIP().toString());
      isConnected = true;
      
      // التحقق من تطبيق IP الثابت
      if (wifiNetworks[i].useStaticIP && WiFi.localIP() != wifiNetworks[i].localIP) {
        Serial.println("⚠️ تحذير: لم يتم تطبيق عنوان IP الثابت بشكل صحيح!");
        Serial.print("  - المتوقع: "); Serial.println(wifiNetworks[i].localIP.toString());
        Serial.print("  - الفعلي: "); Serial.println(WiFi.localIP().toString());
      }
      
      // طباعة معلومات الشبكة المتصلة
      Serial.print("  - اسم الشبكة: "); Serial.println(wifiNetworks[i].ssid);
      Serial.print("  - البوابة: "); Serial.println(WiFi.gatewayIP().toString());
      Serial.print("  - القناع: "); Serial.println(WiFi.subnetMask().toString());
      return; // الخروج عند الاتصال الناجح
    } else {
      Serial.println("\n❌ فشل الاتصال بهذه الشبكة.");
      
      // إعادة تعيين إعدادات الـ IP لتجنب تأثيرها على المحاولة التالية
      WiFi.config(IPAddress(), IPAddress(), IPAddress());
    }
  }

  // إذا وصلنا إلى هنا، فشلنا في الاتصال بجميع الشبكات
  Serial.println("\n❌ فشل الاتصال بجميع الشبكات المخزنة، التبديل إلى وضع AP");
  startAPMode();
}

bool tryConnect(const WiFiNetwork& net) {
  Serial.printf("محاولة الاتصال بـ %s...\n", net.ssid);
  
  if (net.useStaticIP) {
    WiFi.config(net.localIP, net.gateway, net.subnet);
  }
  
  WiFi.begin(net.ssid, net.password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  return WiFi.status() == WL_CONNECTED;
}

// ----- بدء تشغيل نقطة الوصول AP -----
void startAPMode() {
  WiFi.mode(WIFI_MODE_AP);            // ✅ وضع Access Point
  WiFi.softAP(AP_SSID, AP_PASSWORD);  // إنشاء نقطة الوصول

  Serial.println("\n[✔️] تم تفعيل وضع AP");
  Serial.print("اسم الشبكة: ");
  Serial.println(AP_SSID);
  Serial.print("عنوان IP: ");
  Serial.println(WiFi.softAPIP());  // مثال: 192.168.4.1

  // إضافة mDNS للعنوان الثابت
  if (!MDNS.begin(MDNS_NAME)) {
    Serial.println("❌ فشل بدء mDNS");
  } else {
    Serial.println("🌐 العنوان الثابت: http://" + String(MDNS_NAME) + ".local");
  }
}

// ------ واجهة تكوين إعدادات الشبكة ------
  void handleConfigPage() {

  // إرسال صفحة الإعدادات مع تعطيل الكاش
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
 
  // قراءة ملف config.html من SPIFFS وإرساله
  File file = SPIFFS.open("/config.html", "r");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
   
   } else {
    server.send(500, "text/plain", "Error loading config page");
   }
}

void handleSaveConfig() {
  // مسح الشبكات القديمة
  memset(wifiNetworks, 0, sizeof(wifiNetworks));
  int networkCount = 0;
  bool hasError = false;
  String errorMessage = "";

  // معالجة كل شبكة مرسلة من النموذج
  for (int i = 0; i < MAX_NETWORKS; i++) {
    String ssidArg = "ssid_" + String(i);
    String passwordArg = "password_" + String(i);
    String useStaticIPArg = "useStaticIP_" + String(i);
    String localIPArg = "localIP_" + String(i);
    String gatewayArg = "gateway_" + String(i);
    String subnetArg = "subnet_" + String(i);
    String priorityArg = "priority_" + String(i);

    // التحقق من وجود بيانات الشبكة
    if (server.hasArg(ssidArg) && server.arg(ssidArg).length() > 0) {
      // التحقق من الحد الأقصى للشبكات
      if (networkCount >= MAX_NETWORKS) {
        errorMessage = "تجاوز الحد الأقصى لعدد الشبكات (" + String(MAX_NETWORKS) + ")";
        hasError = true;
        break;
      }

      // نسخ SSID وكلمة المرور مع التحقق من الطول
      String ssidValue = server.arg(ssidArg);
      if (ssidValue.length() > sizeof(wifiNetworks[networkCount].ssid) - 1) {
        errorMessage = "اسم الشبكة طويل جداً للشبكة " + String(i+1);
        hasError = true;
        break;
      }
      strncpy(wifiNetworks[networkCount].ssid, ssidValue.c_str(), sizeof(wifiNetworks[networkCount].ssid));
      
      String passwordValue = server.arg(passwordArg);
      if (passwordValue.length() > sizeof(wifiNetworks[networkCount].password) - 1) {
        errorMessage = "كلمة المرور طويلة جداً للشبكة " + String(i+1);
        hasError = true;
        break;
      }
      strncpy(wifiNetworks[networkCount].password, passwordValue.c_str(), sizeof(wifiNetworks[networkCount].password));
      
      // معالجة IP الثابت
      wifiNetworks[networkCount].useStaticIP = server.hasArg(useStaticIPArg) && server.arg(useStaticIPArg) == "on";
      
      // معالجة الأولوية
      if (server.hasArg(priorityArg)) {
        int priorityValue = server.arg(priorityArg).toInt();
        // تأكد من أن الأولوية في النطاق الصحيح
        if (priorityValue >= 1 && priorityValue <= MAX_NETWORKS) {
          wifiNetworks[networkCount].priority = priorityValue;
        } else {
          // إذا كانت الأولوية غير صالحة، استخدم القيمة الافتراضية (الترتيب)
          wifiNetworks[networkCount].priority = networkCount + 1;
        }
      } else {
        // تعيين أولوية افتراضية إذا لم يتم توفيرها
        wifiNetworks[networkCount].priority = networkCount + 1;
      }

      // معالجة IP الثابت إذا كان مفعلاً
      if (wifiNetworks[networkCount].useStaticIP) {
        // التحقق من صحة عناوين IP
        IPAddress localIP, gateway, subnet;
        
        if (!localIP.fromString(server.arg(localIPArg))) {
          errorMessage = "عنوان IP غير صالح للشبكة " + String(i+1);
          hasError = true;
          break;
        }
        wifiNetworks[networkCount].localIP = localIP;
        
        if (!gateway.fromString(server.arg(gatewayArg))) {
          errorMessage = "بوابة غير صالحة للشبكة " + String(i+1);
          hasError = true;
          break;
        }
        wifiNetworks[networkCount].gateway = gateway;
        
        if (!subnet.fromString(server.arg(subnetArg))) {
          errorMessage = "قناع شبكة غير صالح للشبكة " + String(i+1);
          hasError = true;
          break;
        }
        wifiNetworks[networkCount].subnet = subnet;
      } else {
        // إعادة تعيين عناوين IP إذا لم يتم استخدام IP ثابت
        wifiNetworks[networkCount].localIP = INADDR_NONE;
        wifiNetworks[networkCount].gateway = INADDR_NONE;
        wifiNetworks[networkCount].subnet = IPAddress(255, 255, 255, 0);
      }

      networkCount++;
    }
  }

  // التحقق من وجود شبكة واحدة على الأقل
  if (!hasError && networkCount == 0) {
    errorMessage = "يجب إدخال شبكة واحدة على الأقل";
    hasError = true;
  }

  // التحقق من وجود أولويات مكررة
  if (!hasError) {
    bool priorities[MAX_NETWORKS + 1] = {false};
    for (int i = 0; i < networkCount; i++) {
      int prio = wifiNetworks[i].priority;
      if (prio < 1 || prio > MAX_NETWORKS) {
        errorMessage = "الأولوية " + String(prio) + " خارج النطاق المسموح (1-" + String(MAX_NETWORKS) + ")";
        hasError = true;
        break;
      }
      if (priorities[prio]) {
        errorMessage = "تم تكرار الأولوية " + String(prio);
        hasError = true;
        break;
      }
      priorities[prio] = true;
    }
  }

  // معالجة الأخطاء
  if (hasError) {
    server.send(400, "text/plain", "❌ " + errorMessage);
    return;
  }

  // حفظ الإعدادات في SPIFFS
  saveWiFiConfig(); 

  // طباعة الإعدادات للتأكد
  Serial.println("تم حفظ إعدادات الشبكات:");
  for (int i = 0; i < networkCount; i++) {
    Serial.printf("شبكة %d:\n", i+1);
    Serial.printf("  SSID: %s\n", wifiNetworks[i].ssid);
    Serial.printf("  Password: %s\n", wifiNetworks[i].password);
    Serial.printf("  Priority: %d\n", wifiNetworks[i].priority);
    Serial.printf("  useStaticIP: %s\n", wifiNetworks[i].useStaticIP ? "نعم" : "لا");
    
    if (wifiNetworks[i].useStaticIP) {
      Serial.printf("  Local IP: %s\n", wifiNetworks[i].localIP.toString().c_str());
      Serial.printf("  Gateway: %s\n", wifiNetworks[i].gateway.toString().c_str());
      Serial.printf("  Subnet: %s\n", wifiNetworks[i].subnet.toString().c_str());
    }
  }

  server.send(200, "text/html", 
    "<script>"
    "setTimeout(() => window.location.href='/', 3000);"
    "</script>"
    "<div style='text-align:center; padding:20px;'>"
    "✓ تم حفظ الإعدادات بنجاح، جارِ إعادة التوجيه..."
    "</div>");
  
  delay(100);
  ESP.restart();
}
// ============ دوال النظام التبادلي ============
void toggleOutputs() {
  static int currentState = 1; // 1: OUT1 نشط، 2: OUT2 نشط
  static unsigned long lastToggleTime = 0;

  if (isInPauseBetweenToggle) {
    // حالة الاستراحة
    if (millis() - pauseStartTime >= togglePauseDuration) {
      isInPauseBetweenToggle = false;
      currentState = (currentState == 1) ? 2 : 1; // تبديل الحالة
      setOutputStates(currentState);
      lastToggleTime = millis(); // تحديث وقت التبديل
      Serial.print("✅ انتهت الاستراحة - الحالة الجديدة: ");
      Serial.println(currentState == 1 ? "OUT1 تشغيل، OUT2 إيقاف" : "OUT1 إيقاف، OUT2 تشغيل");
    } else {
      // طباعة التقدم في الاستراحة كل ثانية
      static unsigned long lastPausePrint = 0;
      if (millis() - lastPausePrint >= 1000) {
        lastPausePrint = millis();
        Serial.print("⏳ زمن الاستراحة المتبقي: ");
        Serial.print((togglePauseDuration - (millis() - pauseStartTime)) / 1000);
        Serial.println(" ثانية");
      }
    }
  } else {
    // حالة التشغيل العادي
    if (millis() - lastToggleTime >= toggleInterval) {
      if (togglePauseDuration > 0) {
        // بدء الاستراحة
        isInPauseBetweenToggle = true;
        pauseStartTime = millis();
        setOutputStates(0); // إيقاف كليهما
        Serial.println("⏸️ بدء استراحة: تم إيقاف OUT1 و OUT2");
        Serial.print("⏱️ مدة الاستراحة: ");
        Serial.print(togglePauseDuration / 1000);
        Serial.println(" ثانية");
      } else {
        // تبديل مباشر بدون استراحة
        currentState = (currentState == 1) ? 2 : 1;
        setOutputStates(currentState);
        lastToggleTime = millis();
        Serial.print("🔄 تبديل - الحالة: ");
        Serial.println(currentState == 1 ? "OUT1 تشغيل، OUT2 إيقاف" : "OUT1 إيقاف، OUT2 تشغيل");
      }
    } else {
      // طباعة التقدم في التشغيل كل ثانية
      static unsigned long lastActivePrint = 0;
      if (millis() - lastActivePrint >= 1000) {
        lastActivePrint = millis();
        Serial.print("⏱️ زمن التشغيل المتبقي: ");
        Serial.print((toggleInterval - (millis() - lastToggleTime)) / 1000);
        Serial.println(" ثانية");
      }
    }
  }
}

// من أهم الدوال الخاصة بحالة تشغيل المخارج التبادلية بشكل متعاكس أو إطفائها وقت الاستراحة 
void setOutputStates(int state) {
  switch (state) {
    case 1: // تشغيل OUT1 وإيقاف OUT2
      digitalWrite(TOGGLE_OUT1, HIGH);
      digitalWrite(TOGGLE_OUT2, LOW);
      pins[0].state = true;
      pins[1].state = false;
      break;
    case 2: // تشغيل OUT2 وإيقاف OUT1
      digitalWrite(TOGGLE_OUT1, LOW);
      digitalWrite(TOGGLE_OUT2, HIGH);
      pins[0].state = false;
      pins[1].state = true;
      break;
    case 0: // إيقاف كليهما (فترة الاستراحة)
      digitalWrite(TOGGLE_OUT1, LOW);
      digitalWrite(TOGGLE_OUT2, LOW);
      pins[0].state = false;
      pins[1].state = false;
      break;
  }
}

// ---  دالة لإعادة تعيين جميع المتغيرات الزمنية عند بدء/إيقاف النظام ---
void resetToggleSystem() {
  lastToggleTime = millis();
  pauseStartTime = 0;
  isInPauseBetweenToggle = false;
}

// الجمع بين الطباعة عند التغيير + الطباعة كل فترة زمنية + تجاهل الحالة إذا كانت الاستراحة معطلة و ذلك لزمن الاستراحة
void printPauseStateSmart(bool currentState) {
  static bool lastState = false;
  static unsigned long lastPrintTime = 0;
  const unsigned long printInterval = 1000; // كل ثانية

  if (togglePauseDuration == 0) return; // لا طباعة إذا لم تُفعّل الاستراحة

  if (currentState != lastState || (millis() - lastPrintTime >= printInterval)) {
    Serial.print("isInPauseBetweenToggle: ");
    Serial.println(currentState ? "نعم" : "لا");
    lastState = currentState;
    lastPrintTime = millis();
  }
}

void startToggleSystem() {
  toggleSystemActive = true;
  toggleSystemPaused = false;
  toggleStartTime = millis();
  // lastToggleTime = millis();
  expectedToggleTime = millis() + toggleInterval; // بدلًا من lastToggleTime
  digitalWrite(TOGGLE_OUT1, HIGH);
  digitalWrite(TOGGLE_OUT2, LOW);
  pins[0].state = 1;
  pins[1].state = 0;
  Serial.println("[نظام] بدء التشغيل التبادلي");
}

void stopToggleSystem() {
  toggleSystemActive = false;
  toggleSystemPaused = false;
  expectedToggleTime = 0; // ، يجب إعادة تعيين expectedToggleTime لتجنب السلوك غير المتوقع.
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
    // toggleStartTime += pauseDuration;
    lastToggleTime += pauseDuration;
    toggleInterval = savedToggleInterval;
    digitalWrite(TOGGLE_OUT1, HIGH);
    digitalWrite(TOGGLE_OUT2, LOW);
    pins[0].state = 1;
    pins[1].state = 0;
    Serial.println("[نظام] استئناف التشغيل");
  }
}

void toggleOutput(int pinIndex) {

  if (pinIndex < 2) {
    // إيقاف النظام التبادلي وإعادة تعيين جميع المتغيرات الزمنية
    if (toggleSystemActive) {
      stopToggleSystem();
      lastToggleTime = 0;
      pauseStartTime = 0;
      isInPauseBetweenToggle = false;
      Serial.println("[نظام] تم إيقاف التشغيل التلقائي للتحكم اليدوي");
    }

 // إضافة هذا الجزء لتحديث وقت التفعيل
  if (pinIndex >= 2) {
    pins[pinIndex].activationTime = millis();
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

  //----------- الجزء الخاص بالمخارج اليدوية (الفهرس 2 فما فوق)---------
  //خاص بالمخرجين اليدويين الأول و الثاني و رقمها 2 و 3
  //حيث تم تحويلهما إلى مخرجين تبادليين متعاكسين
  else {
    GpioPin& pin = pins[pinIndex];

    // معالجة خاصة للمخرجين اليدويين 1 و2 (الفهرس 2 و3)
    // تطبيق التعاكس فقط على المخرجين 1 و 2 (index 2 و 3)
    if (pinIndex == 2 || pinIndex == 3) {
      int otherPinIndex = (pinIndex == 2) ? 3 : 2;  // تحديد المخرج الآخر
      GpioPin& otherPin = pins[otherPinIndex];

      if (pin.state == LOW) {
        // إيقاف المخرج الآخر إذا كان يعمل
        // إيقاف المخرج الآخر وإعادة ضبط توقيته
        if (otherPin.state == HIGH) {
          otherPin.state = LOW;
          digitalWrite(otherPin.number, LOW);
          otherPin.activationTime = 0;  // إلغاء العد التنازلي
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

// ============ دوال مساعدة ============
void createWiFiConfig() {
  memset(wifiNetworks, 0, sizeof(wifiNetworks));
  
  // إعدادات الشبكة الأولى
  strncpy(wifiNetworks[0].ssid, "D-uni", sizeof(wifiNetworks[0].ssid));
  strncpy(wifiNetworks[0].password, "syfert122333444455555", sizeof(wifiNetworks[0].password));
  wifiNetworks[0].useStaticIP = true;
  wifiNetworks[0].priority = 1;
  wifiNetworks[0].localIP = IPAddress(192, 168, 1, 100);
  wifiNetworks[0].gateway = IPAddress(192, 168, 1, 1);
  wifiNetworks[0].subnet = IPAddress(255, 255, 255, 0);

  saveWiFiConfig();
  Serial.println("✅ تم إنشاء ملف الإعدادات بنجاح!");
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html"; // إضافة لنوع HTML
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".txt")) return "text/plain";
  if (filename.endsWith(".bin")) return "application/octet-stream";
  if (filename.endsWith(".woff2")) return "font/woff2";
  return "text/plain";
}

// أضف هذه الدوال المساعدة
String readTemplateFile(String path) {
  File file = SPIFFS.open(path, "r");
  if (!file) return "";
  String content = file.readString();
  file.close();
  return content;
}

String processTemplate(String html) {
  html.replace("%SYSTEM_TITLE%", systemTitle);
  html.replace("%SYSTEM_STATUS_LABEL%", systemStatusLabel);
  html.replace("%resetBtn%", resetBtn);
  html.replace("%toggleBtnStart%", toggleBtnStart);
  html.replace("%toggleBtnStop%", toggleBtnStop);
  html.replace("%TOGGLE_OUTPUT_1%", toggleOutputNames[0]);
  html.replace("%TOGGLE_OUTPUT_2%", toggleOutputNames[1]);
  html.replace("%NETWORK_STATUS%", getNetworkStatusHTML());
  
  for (int i = 0; i < 10; i++) {
    html.replace("%MANUAL_OUTPUT_" + String(i + 1) + "%", manualOutputs[i]);
  }
  
  // إضافة نسخة عشوائية لمنع التخزين المؤقت
  html.replace("%CACHE_BUSTER%", String(random(10000)));
  
  return html;
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

// unsigned long calculateElapsedTime() {
//   if (!toggleSystemActive) return 0;
//   unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
//   return elapsed / 60000;
// }

unsigned long calculateElapsedTime() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = (toggleSystemPaused ? pausedTime : millis()) - toggleStartTime;
  return elapsed / 60000;
}

int calculateElapsedProgress() {
  if (!toggleSystemActive) return 0;
  unsigned long elapsed = toggleSystemPaused ? (pausedTime - toggleStartTime) : (millis() - toggleStartTime);
  int progress = (elapsed * 100) / totalDuration;
  return progress > 100 ? 100 : progress;
}

void checkFileSystem() { // --- إظهار مسارات و أسماء الملفات في قسم SPIFFS ---
  Serial.println("\n[فحص الملفات في SPIFFS]");

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file) {
    // طباعة المسار الكامل مع اسم الملف
    Serial.printf("الملف: %s - الحجم: %d بايت\n", file.path(), file.size());
    
    file = root.openNextFile();
  }
  Serial.println("----------------------------");
}

String getSystemStatusJSON() {
  String json = "{";
   // إضافة الحقول الجديدة للوقت المتبقي
      // إضافة الوقت الحالي للخادم
  json += "\"currentTime\":" + String(millis()) + ",";
  
  // تحديث حقول المخارج اليدوية
//  for (int i = 1; i <= 10; i++) {
//    int pinIndex = i + 1;
//    json += "\"manual" + String(i) + "ActivationTime\":" + String(pins[pinIndex].activationTime) + ",";
//    json += "\"manual" + String(i) + "Duration\":" + String(pins[pinIndex].autoOffDuration) + ",";
//  }
  
    // إرسال بيانات كل مخرج يدوي (1-10)
  for (int i = 2; i <= 11; i++) { // الفهرس 2 = manual1 في الواجهة
    json += "\"manual" + String(i-1) + "ActivationTime\":" + String(pins[i].activationTime) + ",";
    json += "\"manual" + String(i-1) + "Duration\":" + String(pins[i].autoOffDuration) + ",";
  }
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
  json += "\"progress\":" + String(calculateProgress()) + ",";  // تقدم الشريط
  json += "\"duration\":" + String(totalDuration / 60000) + ",";
  json += "\"toggleInterval\":" + String(toggleInterval / 1000) + ",";
  json += "\"pauseDuration\":" + String(togglePauseDuration / 1000) + ","; // إضافة قيمة الاستراحة للمخرجين التبادليين
  json += "\"elapsedTime\":" + String(calculateElapsedTime()) + ",";    // زمن منقضي
  json += "\"elapsedProgress\":" + String(calculateElapsedProgress());  // نسبة تقدم الشريط
  json += "}";
  return json;
}
