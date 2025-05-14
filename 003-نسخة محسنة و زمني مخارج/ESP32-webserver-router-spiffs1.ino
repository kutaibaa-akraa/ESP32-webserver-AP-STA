// كود مطور و يعمل بكفاءة و الاتصال عبر wifi STA  أو عبر AP  مع واجهة حفظ إعادات الشبكة الجديدة و الاستعادة و الحذف
// تم إضافة زمنيلكل المخارج اليدوية نقلاً عن ملف المشروع  -- ESP32-webserver-new-5.ino --- 002-نسخة
// =================== 📚 المكتبات ===================
#include <WiFi.h>       // مكتبة الاتصال بالواي فاي
#include <WebServer.h>  // لإنشاء خادم ويب
#include <SPIFFS.h>     // نظام ملفات داخل الشريحة لتخزين الصفحات
// مكتبة رفع الملفات -SPIFFS.h- قديمة و تعمل على اردوينو 1.8.19

// =================== 🌐 إعدادات الشبكة ===================
const char* AP_SSID = "ESP32-Control";  // اسم نقطة الوصول Access Point
const char* AP_PASSWORD = "12345678";   // كلمة المرور

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
  unsigned long activationTime;   // وقت التفعيل الأخير
  unsigned long autoOffDuration;  // مدة التشغيل التلقائي قبل الإيقاف
  const char* name;               // اسم العرض
  const char* onCmd;              // رابط التشغيل
  const char* offCmd;             // رابط الإيقاف
  bool allowManualControl;        // هل يمكن التحكم يدوياً
};

//  ---- حفظ إعدادات الأزمنة ------- يجمع كل الإعدادات القابلة للحفظ في مكان واحد مع تحسين استخدام الذاكرة.----
struct SystemSettings {
  unsigned long toggleInterval;     // زمن التبادل (مللي ثانية)
  unsigned long totalDuration;      // الزمن الكلي (مللي ثانية)
  unsigned long manualDurations[10]; // مدة التشغيل لكل مخرج يدوي (1-10)
};

// =================== 🌍 متغيرات عامة ===================
WebServer server(80);       // خادم ويب على المنفذ 80
WiFiSettings wifiSettings;  // إعدادات الشبكة


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
  "فتح بخار", "مكب تصريف", "المخرج اليدوي 6",
  "المخرج اليدوي 7", "المخرج اليدوي 8",
  "المخرج اليدوي 9", "المخرج اليدوي 10"
};

// ---- يقلل حجم الملف ويضمن قراءة/كتابة أسرع ------
const char* SYSTEM_SETTINGS_FILE = "/system_settings.bin"; // استخدام تنسيق ثنائي للكفاءة

// =================== 🌐 إعدادات mDNS ===================
// const char* MDNS_NAME = "esp32-control"; // اسم الجهاز الثابت

//  ------ تحسين واجهة إعدادات الشبكة
const char* configPageHTML = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>إعدادات الشبكة</title>
      <link rel="stylesheet" href="/css/config.css">
	    <script src="/js/config.js"></script>
</head>
	<body>
	  <div class="card">
		<h1>⚙️ إعدادات اتصال Wi-Fi</h1>
		<div id="alert" class="alert"></div>
		<form id="wifiForm" onsubmit="return validateForm(event)">
		  <div class="form-group">
			<label for="ssid">اسم الشبكة (SSID)</label>
			<input type="text" id="ssid" name="ssid" required>
		  </div>
		  <div class="form-group">
			<label for="password">كلمة المرور</label>
			<input type="password" id="password" name="password" required>
		  </div>
		  <button type="submit">حفظ الإعدادات</button>
		</form>
	  </div>
	  <div class="danger-zone">
		<h3>⚠️ منطقة التهيئة - استخدمها لإعادة ضبط الشبكة للحالة الافتراضية:</h3>		
		<button 
		  onclick="resetConfig('default')" 
		  class="button button-danger restore"
		  title="استعادة الإعدادات الأولية للشبكة"
		>
		  <i class="fas fa-undo"></i> استعادة الإعدادات الافتراضية
		</button>
		
		<button 
		  onclick="resetConfig('delete')" 
		  class="button button-danger delete"
		  title="حذف جميع الإعدادات الحالية"
		>
		  <i class="fas fa-trash-alt"></i> حذف الإعدادات
		</button>
	  </div>
    </body>
 </html>
  )rawliteral";

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

// روابط الخطوط والأيقونات
const char* fullHtmlPage = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>نظام التحكم التبادلي</title>  
  <!-- link href="https://fonts.googleapis.com/css2?family=Tajawal:wght@400;700&family=Cairo:wght@600&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" -->  
  <link rel="stylesheet" href="/css/all.min.css?v=%CACHE_BUSTER%">  <!-- fonts for Awesome --> 
  <link rel="stylesheet" href="/css/cairo.css?v=%CACHE_BUSTER%">
  <link rel="stylesheet" href="/css/tajawal.css?v=%CACHE_BUSTER%">
  <link rel="stylesheet" href="/css/main.css?v=%CACHE_BUSTER%">
  <script src="/js/main.js"></script> 
</head>
<body>
    <!-- إشعار الحفظ -->
    <div id="saveNotification" class="save-notification">تم الحفظ!</div>
    
  <div class="card">
    <h2><i class="fas fa-cogs"></i> %SYSTEM_TITLE%</h2>
    <div class="status-box">
      <h2><i class="fas fa-info-circle"></i> %SYSTEM_STATUS_LABEL%: <span id="systemStatus">جار التحميل...</span></h2>
      
      <div class="progress-container">
        <div id="progressBar" class="progress-bar"></div>
      </div>
      
      <!-- الوقت المتبقي -->
      <div class="time-remaining">
        <i class="fas fa-clock"></i> الوقت المتبقي: <span id="remainingTime">0</span>
      </div>

       <!-- الوقت المنقضي -->
      <div class="time-elapsed">
        <i class="fas fa-hourglass-start"></i> الزمن المنقضي: <span id="elapsedTime">0</span>(<span id="elapsedPercent">0</span>)
      </div>
      
      <div class="control-settings">
        <button id="toggleBtn" class="button button-on"><i class="fas fa-power-off"></i> %toggleBtnStart%</button>
        <button id="pauseBtn" class="button button-pause"><i class="fas fa-pause"></i> إيقاف مؤقت</button>
      </div> 

     <div class="collapsible-section">
        <button class="collapse-btn" onclick="toggleSettings()">
        <i class="fas fa-cog"></i> <span class="btn-text">إظهار الإعدادات ▼</span>
        </button>
  
        <!-- المحتوى القابل للطي -->
   <div class="collapsible-content" id="advancedSettings">
      <div class="control-settings">
        <div>
          <table>
        <tr>
          <td>
            <label for="toggleInterval">زمني(ثواني)</label>
            <input type="number" id="toggleInterval" min="5" max="300" value="30">
          </td>
          <td>
            <label for="duration">تشغيل(دقائق)</label>
            <input type="number" id="duration" min="1" max="120" value="10">
          </td>
          <td>
            <button onclick="saveSettings()" class="button button-on">حفظ الإعدادات</button>
          </td>
        </tr>
      </table>
      </div>      
      <div class="preset-buttons">
        <button onclick="resetSettings()" class="button button-off">30*10</button>
        <button onclick="resetSettings3020()" class="button button-off">30*20</button>
        <button onclick="resetSettings6015()" class="button button-off">60*15</button>
        <button onclick="resetSettings6030()" class="button button-off">60*30</button>
        <button onclick="resetSettings6045()" class="button button-off">60*45</button>
        <button onclick="resetSettings6060()" class="button button-off">60*60</button>
        <button onclick="resetSettings6090()" class="button button-off">60*90</button>
        <button onclick="resetSettings60120()" class="button button-off">60*120</button>
        </div>
        <div>
        <button onclick="checkFiles()" class="button button-info"><i class="fas fa-search"></i> فحص الملفات</button>
        <button onclick="forceReload()" class="cache-btn"> ⟳ تحديث (مسح التخزين)</button>
      </div>
            <div class="preset-buttons">
      <button onclick="saveSystemSettings()" class="button button-on">💾 حفظ الإعدادات</button>
      <button onclick="loadSystemSettings()" class="button button-info">🔄 استعادة الإعدادات</button>
            </div>
    </div>
   </div>
   </div>
    </div>
  </div>  
  <div class="card">
    <h3><i class="fas fa-exchange-alt"></i> المخارج التبادلية</h3>
     <!-- جدول المخارج التبادلية -->
    <div class="toggle-outputs">
      <table>
        <tbody>
          <tr>
          <td>%TOGGLE_OUTPUT_1%</td>
           <!-- <td>المخرج التبادلي 1</td> -->
            <td id="out1State">جار التحميل...</td>
            <td><button id="out1Btn" class="button">جار التحميل...</button></td>
      <td rowspan="2"><button id="resetBtn" class="button button-off">%resetBtn%</button></td>
          </tr>
          <tr>
          <td>%TOGGLE_OUTPUT_2%</td>
            <!--<td>المخرج التبادلي 2</td> -->
            <td id="out2State">جار التحميل...</td>
            <td><button id="out2Btn" class="button">جار التحميل...</button></td>
      <td></td>
          </tr>
      
        </tbody>
      </table>
    </div>
  </div>  
  <div class="card">
    <h3><i class="fas fa-toggle-on"></i> المخارج اليدوية</h3>
    <table>
        <!-- صفوف المخارج اليدوية (مكررة بنمط منتظم) -->
      <tr>
            <td>%MANUAL_OUTPUT_1%</td>
          <td id="manual1State">جار التحميل...</td>
          <td>
            <button id="manual1Btn" class="button">تشغيل</button>
            <span id="manual1Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual1Duration" min="0" max="300" value="4">
            <button onclick="setDuration(1)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_2%</td>
          <td id="manual2State">جار التحميل...</td>
          <td>
            <button id="manual2Btn" class="button">تشغيل</button>
            <span id="manual2Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual2Duration" min="0" max="300" value="4">
            <button onclick="setDuration(2)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <!-- تكرار نفس الهيكل للمخارج من 3 إلى 10 -->
        <tr>
          <td>%MANUAL_OUTPUT_3%</td>
          <td id="manual3State">جار التحميل...</td>
          <td>
            <button id="manual3Btn" class="button">تشغيل</button>
            <span id="manual3Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual3Duration" min="0" max="300" value="0">
            <button onclick="setDuration(3)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_4%</td>
          <td id="manual4State">جار التحميل...</td>
          <td>
            <button id="manual4Btn" class="button">تشغيل</button>
            <span id="manual4Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual4Duration" min="0" max="300" value="0">
            <button onclick="setDuration(4)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_5%</td>
          <td id="manual5State">جار التحميل...</td>
          <td>
            <button id="manual5Btn" class="button">تشغيل</button>
            <span id="manual5Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual5Duration" min="0" max="300" value="0">
            <button onclick="setDuration(5)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_6%</td>
          <td id="manual6State">جار التحميل...</td>
          <td>
            <button id="manual6Btn" class="button">تشغيل</button>
            <span id="manual6Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual6Duration" min="0" max="300" value="0">
            <button onclick="setDuration(6)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_7%</td>
          <td id="manual7State">جار التحميل...</td>
          <td>
            <button id="manual7Btn" class="button">تشغيل</button>
            <span id="manual7Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual7Duration" min="0" max="300" value="0">
            <button onclick="setDuration(7)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_8%</td>
          <td id="manual8State">جار التحميل...</td>
          <td>
            <button id="manual8Btn" class="button">تشغيل</button>
            <span id="manual8Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual8Duration" min="0" max="300" value="0">
            <button onclick="setDuration(8)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_9%</td>
          <td id="manual9State">جار التحميل...</td>
          <td>
            <button id="manual9Btn" class="button">تشغيل</button>
            <span id="manual9Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual9Duration" min="0" max="300" value="0">
            <button onclick="setDuration(9)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        <tr>
          <td>%MANUAL_OUTPUT_10%</td>
          <td id="manual10State">جار التحميل...</td>
          <td>
            <button id="manual10Btn" class="button">تشغيل</button>
            <span id="manual10Remaining" class="timer"></span>
          </td>
          <td class="duration-control">
            <input type="number" id="manual10Duration" min="0" max="300" value="0">
            <button onclick="setDuration(10)" class="button button-settings">تعيين</button>
          </td>
        </tr>
        </tbody>
    </table>
  </div>

</body>
</html>
   )rawliteral";

// //--------------------دالة تجميع أجزاء صفحة الويب الواجهة ------------------------
// const String fullHtmlPage = String(htmlHeader) + cssStyles + htmlBody + javascriptCod;

// ============ Forward Declarations ============
// ============ التصريح المسبق عن الدوال قيل دالة الإعداد setup()  ============
void setupServer();
void STAsetup();
void connectToWiFi();
void startAPMode();
void handleConfigPage();
void handleSaveConfig();
void resetWiFiConfig(bool restoreDefaults);
String getSystemStatusJSON();
void toggleOutput(int pinIndex);
void toggleOutputs();
void startToggleSystem();
void stopToggleSystem();
void pauseToggleSystem();
void resumeToggleSystem();
unsigned long calculateRemainingTime();
int calculateProgress();
void checkFileSystem();
void handleLoadSystemSettings();
void handleSaveSystemSettings();
bool loadSystemSettings();
void saveSystemSettings();

void setup() {
  Serial.begin(115200);
  STAsetup();  // ------- دوال تشغيل الشبكة ---------
  // -------- تهيئة المنافذ -----------
  for (auto& pin : pins) {
    pinMode(pin.number, OUTPUT);
    digitalWrite(pin.number, pin.state);
    Serial.print("تم تهيئة المخرج: ");
    Serial.println(pin.name);
  }
  // -------- تهيئة SPIFFS ---------------
  if (!SPIFFS.begin(true)) {
    Serial.println("فشل في تهيئة SPIFFS!");
    return;
  }
  checkFileSystem();

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
    server.send(200, "application/json", getSystemStatusJSON());
    server.on("/saveConfig", HTTP_POST, handleSaveConfig);

  // // ----- بدء نقطة الوصول AP --------
  // WiFi.softAP(AP_SSID, AP_PASSWORD);
  // Serial.println("\nنقطة الوصول جاهزة");
  // Serial.print("SSID: ");
  // Serial.println(AP_SSID);
  // Serial.print("PASSWORD: ");
  // Serial.println(AP_PASSWORD);
  // Serial.print("عنوان IP: ");
  // Serial.println(WiFi.softAPIP());

  setupServer();

  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    String json = getSystemStatusJSON();
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
    server.send(200, "application/json", getSystemStatusJSON());
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

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");  // 7 أيام
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

// حفظ الإعدادات
void saveSystemSettings() {
  SystemSettings settings;
  settings.toggleInterval = toggleInterval;
  settings.totalDuration = totalDuration;
  for (int i=0; i<10; i++) {
    settings.manualDurations[i] = pins[i+2].autoOffDuration; // الفهرس 2-11 للمخارج اليدوية
  }

  File file = SPIFFS.open(SYSTEM_SETTINGS_FILE, "w");
  file.write((uint8_t*)&settings, sizeof(settings));
  file.close();
}

// تحميل الإعدادات
bool loadSystemSettings() {
  File file = SPIFFS.open(SYSTEM_SETTINGS_FILE, "r");
  if (!file) return false;

  SystemSettings settings;
  file.readBytes((char*)&settings, sizeof(settings));
  
  toggleInterval = settings.toggleInterval;
  totalDuration = settings.totalDuration;
  for (int i=0; i<10; i++) {
    pins[i+2].autoOffDuration = settings.manualDurations[i];
  }
  
  file.close();
  return true;
}

void handleSaveSystemSettings() {
  saveSystemSettings();
  server.send(200, "text/plain", "تم حفظ الإعدادات!");
}

void handleLoadSystemSettings() {
  if (loadSystemSettings()) {
    server.send(200, "application/json", getSystemStatusJSON());
  } else {
    server.send(404, "text/plain", "الملف غير موجود!");
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

  // مسار حذف الإعدادات ---- حذف ملف ال  wifi_config.txt -------
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
    if (isConnected) {
      String html = fullHtmlPage;
      html.replace("%CACHE_BUSTER%", String(random(10000)));

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

      // اطبع HTML كاملة في السيريال مونيتور
      // Serial.println("---------------- HTML النهائية ----------------"); // تستخدم للفحص فقط 
      // Serial.println(html); // تستخدم للفحص فقط و التأكد من تحميل الصفحة في السيريال
      // Serial.println("-----------------------------------------------");  

      // -------- إضافة رؤوس (Headers) HTTP لمنع التخزين الكاش ---------------------
      server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");  //  لتوجيه المتصفح بعدم تخزين الصفحة أو الملفات
      server.sendHeader("Pragma", "no-cache");                                    // لتوجيه المتصفح بعدم تخزين الصفحة أو الملفات
      server.sendHeader("Expires", "-1");  
      server.send(200, "text/html", html);                                        // إرسال الصفحة بعد الاستبدال
      } else {
      handleConfigPage();
    }
  });

  // ------- مسارات الحفظ و الاستعادة للأزمنة -----------------------
server.on("/saveSystemSettings", HTTP_POST, handleSaveSystemSettings);
server.on("/loadSystemSettings", HTTP_GET, handleLoadSystemSettings);

  // ----- لتكوين الستايل الخارجي لتفعيل الفونت ------------------
  server.on("/css/all.min.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/css/all.min.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

    server.on("/js/main.js", HTTP_GET, []() {  //  أكواد الجافاسكريبت
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/js/main.js", "r");
    server.streamFile(file, "text/javascript");
    file.close();
  });

  server.on("/css/main.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/css/main.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/config.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/css/config.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/js/config.js", HTTP_GET, []() {  //  أكواد الجافاسكريبت
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/js/config.js", "r");
    server.streamFile(file, "text/javascript");
    file.close();
  });

  server.on("/css/cairo.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/css/cairo.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/css/tajawal.css", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/css/tajawal.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/fonts/Cairo-SemiBold.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff2", "r");
    server.streamFile(file, "application/font-woff2");  // ✅
    file.close();
  });

  server.on("/fonts/Tajawal-Regular.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/fonts/Tajawal-Regular.woff2", "r");
    server.streamFile(file, "application/font-woff2");  // ✅
    file.close();
  });

  server.on("/webfonts/fa-solid-900.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/webfonts/fa-solid-900.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-v4compatibility.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/webfonts/fa-v4compatibility.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-regular-400.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/webfonts/fa-regular-400.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
  });

  server.on("/webfonts/fa-brands-400.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/webfonts/fa-brands-400.woff2", "r");
    server.streamFile(file, "application/font-woff2");
    file.close();
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

  // ----- مسار للتحقق من وجود الملفات في spiffs بشكل يدوي بوجود زر في الواجهة
  server.on("/checkFiles", HTTP_GET, []() {
    checkFileSystem();
    server.send(200, "text/plain", "تم فحص الملفات - راجع السيريال مونيتور");
  });
}

void resetWiFiConfig(bool restoreDefaults) {  //  --- دالة لحذف أو استعادة الإعدادات الافتراضية للشبكة ----
  if (restoreDefaults) {
    // استعادة الإعدادات الافتراضية
    WiFiSettings defaultSettings;
    strncpy(defaultSettings.ssid, "الاسم_الافتراضي", sizeof(defaultSettings.ssid));
    strncpy(defaultSettings.password, "كلمة_المرور_الافتراضية", sizeof(defaultSettings.password));
    File file = SPIFFS.open(WIFI_CONFIG_FILE, "w");
    file.write((uint8_t*)&defaultSettings, sizeof(defaultSettings));
    file.close();
    Serial.println("تم استعادة الإعدادات الافتراضية!");
  } else {
    // حذف الملف
    if (SPIFFS.exists(WIFI_CONFIG_FILE)) {
      SPIFFS.remove(WIFI_CONFIG_FILE);
      Serial.println("تم حذف ملف الإعدادات!");
    } else {
      Serial.println("الملف غير موجود!");
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
  for (int i = 0; i < 20; i++) {  // 20 محاولة (10 ثوانٍ)
    if (WiFi.status() == WL_CONNECTED) {
      isConnected = true;
      Serial.println("\nتم الاتصال!");
      Serial.print("اسم الشبكة: ");
      Serial.println(wifiSettings.ssid);
      Serial.print("عنوان IP   : ");
      Serial.println(WiFi.localIP());
      return;  // الخروج من الدالة بعد الاتصال
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nفشل الاتصال، التبديل إلى وضع AP");
  startAPMode();  // التبديل إلى نقطة الوصول
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
}

// ------ واجهة تكوين إعدادات الشبكة ------
void handleConfigPage() {
  server.send(200, "text/html", configPageHTML);
}

void handleSaveConfig() {  // ---------- حفظ إعدادات الشبكة الجديدة و إعادة التشغيل بدون تجميد الخادم -------
  if (server.hasArg("ssid") && server.hasArg("password")) {
    // تحديث إعدادات Wi-Fi
    strncpy(wifiSettings.ssid, server.arg("ssid").c_str(), sizeof(wifiSettings.ssid));
    strncpy(wifiSettings.password, server.arg("password").c_str(), sizeof(wifiSettings.password));
    saveWiFiConfig();  // حفظ الإعدادات في SPIFFS
  }

  // إرسال رد مع إعادة توجيه عبر JavaScript
  server.send(200, "text/html",
              "<script>"
                        "setTimeout(() => { window.location.href = '/?nocache=' + Math.random(); }, 2000);"
              "</script>"
              "✓ تم حفظ الإعدادات، جارِ إعادة التشغيل...");

  // إعادة التشغيل بعد تأخير قصير (بدون تجميد الخادم)
  delay(100);
  ESP.restart();
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

// -----  التحقق من وجود الملفات في SPIFFS وطباعتها -----
void checkFileSystem() {
  Serial.println("\n[فحص الملفات في SPIFFS]");

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file) {
    Serial.print("الملف: ");
    Serial.print(file.name());
    Serial.print(" - الحجم: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
  Serial.println("----------------------------");
}

String getSystemStatusJSON() {
  String json = "{";
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
  // json += "\"toggleInterval\":" + String(toggleInterval / 1000);
  json += "\"toggleInterval\":" + String(toggleInterval / 1000) + ",";
  json += "\"elapsedTime\":" + String(calculateElapsedTime()) + ",";    // زمن منقضي
  json += "\"elapsedProgress\":" + String(calculateElapsedProgress());  // نسبة تقدم الشريط
  json += "}";
  return json;
}
