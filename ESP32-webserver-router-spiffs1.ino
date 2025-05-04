// كود يعمل 1
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

//  ------ تحسين واجهة إعدادات الشبكة  
const char* configPageHTML = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>إعدادات الشبكة</title>
  <style>
    body {
      font-family: 'Tajawal', Arial, sans-serif;
      background: #f0f4f8;
      padding: 20px;
      max-width: 600px;
      margin: 0 auto;
    }
    .card {
      background: white;
	  padding: 40px;
	  margin: 10px 0;
	  border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      text-align: center;
      margin-bottom: 30px;
    }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #34495e;
      font-weight: 600;
    }
    input {
      width: 100%;
      padding: 12px;
      border: 1px solid #bdc3c7;
      border-radius: 6px;
      font-size: 16px;
    }
    button {
      background: #3498db;
      color: white;
      border: none;
      padding: 15px 30px;
      border-radius: 6px;
      font-size: 16px;
      cursor: pointer;
      width: 100%;
      transition: background 0.3s;
    }
    button:hover {
      background: #2980b9;
    }
    .alert {
      padding: 15px;
      border-radius: 6px;
      margin-bottom: 20px;
      display: none;
    }
    .alert-success {
      background: #2ecc71;
      color: white;
    }

    .danger-zone {
  border: 2px solid #ff6b6b;
  padding: 15px;
  margin: 20px 0;
  border-radius: 8px;
}

.danger-zone h3 {
  color: #ff6b6b;
  margin-top: 0;
}

.button-danger.restore {
  background: linear-gradient(145deg, #f39c12, #e67e22);
  
}

.button-danger.delete {
  background: linear-gradient(145deg, #ff6b6b, #e55039);
}
  </style>
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
		<h3>⚠️ منطقة الخطر:</h3>
		
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

	  <script>
	  function resetConfig(action) { // --- دالة الحذف أو إعادة التعيين لإعدادات الشبكة ------
		const actionName = (action === 'default') ? "استعادة الإعدادات الافتراضية" : "حذف جميع الإعدادات";
		
		if (!confirm(`⚠️ هل أنت متأكد من ${actionName}؟ لا يمكن التراجع عن هذا الإجراء!`)) {
		  return;
		}

		const endpoint = (action === 'default') ? '/resetConfigDefault' : '/resetConfigDelete';
		
		fetch(endpoint, { method: 'POST' })
		  .then(response => {
			if (response.ok) {
			  alert("✅ تمت العملية بنجاح! جارِ إعادة التحميل...");
			  setTimeout(() => location.reload(), 3000); // تأخير لإظهار الرسالة
			} else {
			  alert("❌ فشلت العملية! الرجاء المحاولة لاحقًا.");
			}
		  })
		  .catch(error => {
			alert("❌ حدث خطأ في الاتصال بالسيرفر!");
		  });
	  }

		  function validateForm(e) {
			e.preventDefault();
			const ssid = document.getElementById('ssid').value;
			const password = document.getElementById('password').value;
			const alertDiv = document.getElementById('alert');

			if (ssid.length < 2 || password.length < 8) {
			  alertDiv.style.display = 'block';
			  alertDiv.className = 'alert alert-error';
			  alertDiv.textContent = '❗ الرجاء إدخال بيانات صحيحة (كلمة المرور 8 أحرف على الأقل)';
			  return false;
			}

			submitForm();
			return false;
		  }

		  function submitForm() {
			const formData = new FormData(document.getElementById('wifiForm'));
			fetch('/saveConfig', {
			  method: 'POST',
			  body: new URLSearchParams(formData)
			})
			.then(response => response.text())
			.then(data => {
			  const alertDiv = document.getElementById('alert');
			  alertDiv.style.display = 'block';
			  alertDiv.className = 'alert alert-success';
			  alertDiv.textContent = '✓ تم الحفظ بنجاح، جارِ إعادة التوجيه...';
			  setTimeout(() => { window.location.href = '/'; }, 3000);
			});
		  }
	  </script>

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
const char* htmlHeader = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>نظام التحكم التبادلي</title>
  
  <!-- link href="https://fonts.googleapis.com/css2?family=Tajawal:wght@400;700&family=Cairo:wght@600&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" -->
  
  <link rel="stylesheet" href="/css/all.min.css?version=1.1"> <!-- Font Awesome --> 
  <!-- link rel="stylesheet" href="../css/cairo.css" -->
  <!-- link rel="stylesheet" href="../css/tajawal.css" -->
  <style>

  )rawliteral";
  
const char* cssStyles = R"rawliteral(
    body {
      font-family: 'Tajawal', Arial, sans-serif;
      max-width: 800px;
      margin: 0 auto;
      padding: 20px;
      background: #f0f4f8;
    }

    h1, h2 ,h3 {
      font-family: 'Cairo', sans-serif;
      font-weight: 600;
    }
    
    .card {
      background: white;
      border-radius: 12px;
      padding: 20px;
      margin: 15px 0;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    
    .button {
      transition: all 0.3s ease;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
      border: none;
      color: white;
      padding: 10px 20px;
      border-radius: 5px;
      font-size: 16px;
      cursor: pointer;
    }
    
    .button-on { 
      background: linear-gradient(145deg, #2ecc71, #27ae60);
    }
    
    .button-off {
      background: linear-gradient(145deg, #e74c3c, #c0392b);
    }

    .button-pause { background-color: #f39c12; }
    .button-disabled {
      background-color: #95a5a6;
      cursor: not-allowed;
    }

     .progress-container {
      width: 100%;
      background-color: #ecf0f1;
      border-radius: 5px;
      margin: 15px 0;
    }
    
    .progress-bar {
      height: 25px;
      background: linear-gradient(90deg, #2ecc71, #3498db);
      transition: width 0.5s ease;
      position: relative;
    }
    
    .progress-bar::after {
      content: attr(data-progress);
      position: absolute;
      right: 10px;
      color: white;
      font-weight: bold;
    }

    .state-active { color: #2ecc71; font-weight: bold; }
    .state-inactive { color: #e74c3c; }
    
    @media (max-width: 600px) {
      table { font-size: 14px; }
      .button { padding: 8px 16px; }
      .card { padding: 10px; }
    }
  
    .collapsible-content {
      max-height: 0;
      overflow: hidden;
      transition: max-height 0.3s ease-out;
      background-color: #f8f9fa;
    }

   .collapsible-content.active {
     max-height: 500px; /* أو أي قيمة تناسب محتواك */
    }

  .collapse-btn {
    display: flex;
    align-items: center;
    gap: 8px;
   }
/* لتشغيل الفونت الجديد */
/* arabic */
@font-face {
  font-family: 'Tajawal';
  font-style: normal;
  font-weight: 400;
  src: url('/fonts/Tajawal-Regular.woff2') format('woff2');
  unicode-range: U+0600-06FF, U+0750-077F, U+0870-088E, U+0890-0891, U+0897-08E1, U+08E3-08FF, U+200C-200E, U+2010-2011, U+204F, U+2E41, U+FB50-FDFF, U+FE70-FE74, U+FE76-FEFC;
}

/* latin */
@font-face {
  font-family: 'Tajawal';
  font-style: normal;
  font-weight: 400;
  src: url('/fonts/Tajawal-Regular.woff2') format('woff2');
  unicode-range: U+0000-00FF, U+0131, U+0152-0153, U+02BB-02BC, U+02C6, U+02DA, U+02DC, U+0304, U+0308, U+0329, U+2000-206F, U+20AC, U+2122, U+2191, U+2193, U+2212, U+2215, U+FEFF, U+FFFD;
}
  
  @font-face {
    font-family: 'Cairo';
    src: url('/fonts/Cairo-SemiBold.woff2') format('woff2');
  } 

.button {
  position: relative; /* تأكد من أن الأزرار في المقدمة */
  z-index: 1;
  pointer-events: auto !important;
}

.preset-buttons {
  display: flex;
  flex-wrap: wrap;
  gap: 5px;
  padding: 10px;
}

    .save-notification {
      position: fixed;
      top: 20px;
      right: 20px;
      background: #2ecc71;
      color: white;
      padding: 10px;
      border-radius: 5px;
    }

      .time-remaining {
    background-color: #f8f9fa; /* لون الخلفية */
    border-radius: 8px; /* زوايا مدورة */
    padding: 12px 20px; /* مساحة داخلية */
    margin: 15px 0; /* هامش خارجي */
    box-shadow: 0 2px 4px rgba(0,0,0,0.1); /* ظل خفيف */
    font-family: Arial, sans-serif; /* نوع الخط */
    font-size: 18px; /* حجم الخط العام */
    color: #2c3e50; /* لون النص */
    display: inline-block; /* عرض مناسب للمحتوى */
    border: 1px solid #e0e0e0; /* حد خفيف */
  }

  .time-remaining #remainingTime {
    font-size: 24px; /* حجم الخط للرقم */
    font-weight: bold; /* نص غامق */
    color: #2ecc71; /* لون مميز للرقم */
    margin: 0 5px; /* مسافة بين الرقم والنص */
  }

    .time-elapsed {
    background-color: #f8f9fa; /* لون الخلفية */
    border-radius: 8px; /* زوايا مدورة */
    padding: 12px 20px; /* مساحة داخلية */
    margin: 15px 0; /* هامش خارجي */
    box-shadow: 0 2px 4px rgba(0,0,0,0.1); /* ظل خفيف */
    font-family: Arial, sans-serif; /* نوع الخط */
    font-size: 18px; /* حجم الخط العام */
    color: #2c3e50; /* لون النص */
    display: inline-block; /* عرض مناسب للمحتوى */
    border: 1px solid #e0e0e0; /* حد خفيف */
  }

  .time-elapsed #elapsedTime {
    font-size: 24px; /* حجم الخط للرقم */
    font-weight: bold; /* نص غامق */
    color: #2ecc71; /* لون مميز للرقم */
    margin: 0 5px; /* مسافة بين الرقم والنص */
  }

  .timer {/*محتوى زمني للأزرار التبادلية اليدوية*/
  font-size: 14px;
  color: #e74c3c;
  margin-right: 8px;
  font-weight: bold;
}
  
  .cache-btn {
  background: #e74c3c;
  color: white;
  border: none;
  padding: 10px 20px;
  border-radius: 5px;
  cursor: pointer;
  margin: 10px;
}
.cache-btn:hover {
  background: #c0392b;
}
  </style>
)rawliteral";
const char* htmlBody = R"rawliteral(
</head>
<body>
 <!-- إشعار الحفظ -->
      <div id="saveNotification" class="save-notification">تم الحفظ!</div>
    </div>
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
        <button id="toggleBtn" class="button"><i class="fas fa-power-off"></i> %toggleBtnStart%</button>
        <button id="pauseBtn" class="button button-pause"><i class="fas fa-pause"></i> إيقاف مؤقت</button>
      </div>
     <div class="collapsible-section">
  <button class="collapse-btn" onclick="toggleSettings()">
    <i class="fas fa-cog"></i> <span class="btn-text">إظهار الإعدادات ▼</span>
  </button>
  
  <!-- المحتوى القابل للطي -->
  <div class="collapsible-content" id="advancedSettings">
    <div class="control-settings">
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
    </div>
  </div>
  <button onclick="forceReload()" class="cache-btn">
  ⟳ تحديث الصفحة (مسح التخزين)
</button>
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
  <!-- <td>المخرج اليدوي 1</td> --><td>%MANUAL_OUTPUT_1%</td>
  <td id="manual1State">جار التحميل...</td>
  <td>
    <button id="manual1Btn" class="button">جار التحميل...</button>
    <span id="manual1Remaining" class="timer"></span>
  </td>
</tr>
<tr>
  <!-- <td>المخرج اليدوي 2</td> --><td>%MANUAL_OUTPUT_2%</td>
  <td id="manual2State">جار التحميل...</td>
  <td>
    <button id="manual2Btn" class="button">جار التحميل...</button>
    <span id="manual2Remaining" class="timer"></span>
  </td></tr> 
        <tr><!--<td>المخرج اليدوي 3</td> --><td>%MANUAL_OUTPUT_3%</td><td id="manual3State">جار التحميل...</td><td><button id="manual3Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 4</td> --><td>%MANUAL_OUTPUT_4%</td><td id="manual4State">جار التحميل...</td><td><button id="manual4Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 5</td> --><td>%MANUAL_OUTPUT_5%</td><td id="manual5State">جار التحميل...</td><td><button id="manual5Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 6</td> --><td>%MANUAL_OUTPUT_6%</td><td id="manual6State">جار التحميل...</td><td><button id="manual6Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 7</td> --><td>%MANUAL_OUTPUT_7%</td><td id="manual7State">جار التحميل...</td><td><button id="manual7Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 8</td> --><td>%MANUAL_OUTPUT_8%</td><td id="manual8State">جار التحميل...</td><td><button id="manual8Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 9</td> --><td>%MANUAL_OUTPUT_9%</td><td id="manual9State">جار التحميل...</td><td><button id="manual9Btn" class="button">جار التحميل...</button></td></tr>
        <tr><!--<td>المخرج اليدوي 10</td> --><td>%MANUAL_OUTPUT_10%</td><td id="manual10State">جار التحميل...</td><td><button id="manual10Btn" class="button">جار التحميل...</button></td></tr>
      </tbody>
    </table>
  </div>
)rawliteral";

const char* javascriptCode = R"rawliteral(
  <script>
    // جميع دوال الجافاسكربت المحدثة
    function updateAllControls() {
      // الكود المحدث مع دعم الأيقونات
      document.getElementById('out1Btn').innerHTML = 
        outputsState.out1 ? '<i class="fas fa-stop"></i> إيقاف' : '<i class="fas fa-play"></i> تشغيل';
    }
    

  // ------ الدوال العامة ------

  // حفظ الإعدادات
  function saveSettings() {
    const interval = document.getElementById('toggleInterval').value;
    const duration = document.getElementById('duration').value;
    fetch(`/saveSettings?interval=${interval}&duration=${duration}`, { 
      method: 'POST' 
    })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى الإعدادات الافتراضية
  function resetSettings() {
    fetch('/resetSettings', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // ------ الدوال المخصصة لإعدادات محددة ------
  
  // إعادة التعيين إلى 30 ثانية و20 دقيقة
  function resetSettings3020() {
    fetch('/resetSettings3020', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و15 دقيقة
  function resetSettings6015() {
    fetch('/resetSettings6015', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و30 دقيقة
  function resetSettings6030() {
    fetch('/resetSettings6030', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و45 دقيقة
  function resetSettings6045() {
    fetch('/resetSettings6045', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و60 دقيقة
  function resetSettings6060() {
    fetch('/resetSettings6060', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و90 دقيقة
  function resetSettings6090() {
    fetch('/resetSettings6090', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إعادة التعيين إلى 60 ثانية و120 دقيقة
  function resetSettings60120() {
    fetch('/resetSettings60120', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        document.getElementById('toggleInterval').value = data.toggleInterval;
        document.getElementById('duration').value = data.duration;
        showSaveNotification();
      });
  }

  // إظهار إشعار الحفظ
  function showSaveNotification() {
    const notification = document.getElementById('saveNotification');
    notification.style.display = 'block';
    setTimeout(() => {
      notification.style.display = 'none';
    }, 2000);
  }

  // ------ إدارة الإدخالات ------
  
  let durationTimer;
  let intervalTimer;

  // تأخير الإرسال لتجنب الطلبات المتكررة
  function debounce(func, timeout = 1500) {
    clearTimeout(durationTimer);
    durationTimer = setTimeout(func, timeout);
  }

  // تحديث الإعدادات مع تأخير
  function updateSettings(type, value) {
    fetch(`/updateSettings?${type}=${value}`, { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
        showSaveNotification();
      })
      .catch(error => console.error('Error:', error));
  }

  // ------ تهيئة الأحداث ------
  
  document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('toggleInterval').addEventListener('input', function(e) {
      debounce(() => updateSettings('interval', e.target.value));
    });

    document.getElementById('duration').addEventListener('input', function(e) {
      debounce(() => updateSettings('duration', e.target.value));
    });
  });

  // ------ حالة النظام ------
  let outputsState = {
    out1: false,
    out2: false,
    manual1: false,
    manual2: false,
    manual3: false,
    manual4: false,
    manual5: false,
    manual6: false,
    manual7: false,
    manual8: false,
    manual9: false,
    manual10: false,
    systemActive: false,
    systemPaused: false
  };
  
  // تحديث جميع عناصر التحكم
  function updateAllControls() {
    // تحديث أزرار التبادل
    const out1Btn = document.getElementById('out1Btn');
    out1Btn.textContent = outputsState.out1 ? 'إيقاف' : 'تشغيل';
    out1Btn.className = outputsState.out1 ? 'button button-off' : 'button button-on';
    document.getElementById('out1State').textContent = outputsState.out1 ? 'يعمل' : 'مغلق';
    document.getElementById('out1State').className = outputsState.out1 ? 'state-active' : 'state-inactive';
    
    const out2Btn = document.getElementById('out2Btn');
    out2Btn.textContent = outputsState.out2 ? 'إيقاف' : 'تشغيل';
    out2Btn.className = outputsState.out2 ? 'button button-off' : 'button button-on';
    document.getElementById('out2State').textContent = outputsState.out2 ? 'يعمل' : 'مغلق';
    document.getElementById('out2State').className = outputsState.out2 ? 'state-active' : 'state-inactive';

    // تعطيل الأزرار عند التشغيل المتبادل
    //هذا الشرط للزرين التبادليين الخاصين بالمخارج TOGGLE_OUT1 و TOGGLE_OUT2
    if (outputsState.out1) {
      out2Btn.className = 'button button-disabled';
    } else if (outputsState.out2) {
      out1Btn.className = 'button button-disabled';
    }
    
// تعطيل الزر الآخر عند التشغيل - خاص بالزين اليدويين التبادليين 1 و 2 manual2Btn + manual3Btn
//تم إضافة هذا الشرط لتفعيل الزرين اليدويين 1و2 بشكل تبادلي 
if (outputsState.manual1) {
  document.getElementById('manual2Btn').className = 'button button-disabled';
} else if (outputsState.manual2) {
  document.getElementById('manual1Btn').className = 'button button-disabled';
}

// //يمكن تفعيل هذا الشرط إن أردت أن يعمل أحدهما و الثاني معطل  تماماً 
// //(لا يعمل حتى ينتهي الأول )و هو خيار لتأكيد الحالة
// //يتبع هذا الخيار للأزرار اليدوية التبادلية
//     else {
//          document.getElementById('manual1Btn').disabled = false;
//          document.getElementById('manual2Btn').disabled = false;
//         }

    // تحديث الأزرار اليدوية (1-10)
    for (let i = 1; i <= 10; i++) {
      const btn = document.getElementById(`manual${i}Btn`);
      const state = document.getElementById(`manual${i}State`);
      btn.textContent = outputsState[`manual${i}`] ? 'إيقاف' : 'تشغيل';
      btn.className = outputsState[`manual${i}`] ? 'button button-off' : 'button button-on';
      state.textContent = outputsState[`manual${i}`] ? 'يعمل' : 'مغلق';
      state.className = outputsState[`manual${i}`] ? 'state-active' : 'state-inactive';
    }

    // تحديث حالة النظام
    const systemStatus = document.getElementById('systemStatus');
    systemStatus.textContent = outputsState.systemPaused ? 'معلق' : 
      (outputsState.systemActive ? 'يعمل' : 'متوقف');
    systemStatus.style.color = outputsState.systemActive ? 
      (outputsState.systemPaused ? '#f39c12' : '#2ecc71') : '#e74c3c';
    
    // تحديث زر التشغيل/الإيقاف
    const toggleBtn = document.getElementById('toggleBtn');
    toggleBtn.textContent = outputsState.systemActive ? '%toggleBtnStop%' : '%toggleBtnStart%';
    toggleBtn.className = outputsState.systemActive ? 'button button-off' : 'button button-on';
    
    // تحديث زر الإيقاف المؤقت
    const pauseBtn = document.getElementById('pauseBtn');
    pauseBtn.textContent = outputsState.systemPaused ? 'متابعة' : 'إيقاف مؤقت';
    pauseBtn.style.display = outputsState.systemActive ? 'inline-block' : 'none';
    
    // تحديث القيم المدخلة
    document.getElementById('toggleInterval').value = outputsState.toggleInterval;
    document.getElementById('duration').value = outputsState.duration;
  }

  // ------ الأحداث التفاعلية ------  
  window.onload = function() {
    fetchStatus();    

    // ربط الأزرار بالدوال
    document.getElementById('out1Btn').onclick = () => toggleOutput('out1');
    document.getElementById('out2Btn').onclick = () => toggleOutput('out2');
    document.getElementById('manual1Btn').onclick = () => toggleOutput('manual1');
    document.getElementById('manual2Btn').onclick = () => toggleOutput('manual2');
    document.getElementById('manual3Btn').onclick = () => toggleOutput('manual3');
    document.getElementById('manual4Btn').onclick = () => toggleOutput('manual4');
    document.getElementById('manual5Btn').onclick = () => toggleOutput('manual5');
    document.getElementById('manual6Btn').onclick = () => toggleOutput('manual6');
    document.getElementById('manual7Btn').onclick = () => toggleOutput('manual7');
    document.getElementById('manual8Btn').onclick = () => toggleOutput('manual8');
    document.getElementById('manual9Btn').onclick = () => toggleOutput('manual9');
    document.getElementById('manual10Btn').onclick = () => toggleOutput('manual10');

    // document.getElementById('elapsedTime').textContent = data.elapsedTime; //خاص بالزمن المنقضي
    // document.getElementById('elapsedPercent').textContent = data.elapsedProgress; // خاص بالنسبة المئوية لشريط التقدم
    
    document.getElementById('elapsedTime').textContent = outputsState.elapsedTime;
    document.getElementById('elapsedPercent').textContent = outputsState.elapsedProgress;
    
    document.getElementById('toggleBtn').onclick = toggleSystem; //خاص بالازرار التبادلية
    document.getElementById('pauseBtn').onclick = pauseSystem; //خاص بزر التوقف المؤقت
    document.getElementById('resetBtn').onclick = resetToggleOutputs; //خاص بزر إعادة التعيين للمخارج التبادلية
  };

  // ------ وظائف النظام ------
  // تبديل حالة المخرج
  function toggleOutput(outputId) {
    fetch(`/${outputId}/toggle`, { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
      });
  }

  // تشغيل/إيقاف النظام التبادلي
  function toggleSystem() {
    const duration = document.getElementById('duration').value;
    const interval = document.getElementById('toggleInterval').value;
    fetch(`/toggle?duration=${duration}&interval=${interval}`, { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
      });
  }

  // إيقاف مؤقت/متابعة
  function pauseSystem() {
    fetch('/pause', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
      });
  }

  // إعادة تعيين المخارج التبادلية -  resetToggleOutputs()
  function resetToggleOutputs() {
    fetch('/reset', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
        updateAllControls();
      });
  }

  // جلب حالة النظام
function fetchStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      outputsState = data;
      updateAllControls();
      document.getElementById('progressBar').style.width = data.progress + '%';
      document.getElementById('remainingTime').textContent = data.remainingTime + ' دقيقة';
      document.getElementById('elapsedTime').textContent = data.elapsedTime + ' دقيقة';
      document.getElementById('elapsedPercent').textContent = data.elapsedProgress + '%';
      
      // تحديث العد التنازلي للمخرجين اليدويين 1 و 2
      const updateTimer = (id, activationTime, autoOffDuration) => {
        const element = document.getElementById(`manual${id}Remaining`);
        if (activationTime > 0 && autoOffDuration > 0) {
          const currentTime = Date.now();
          const elapsed = currentTime - activationTime;
          const remainingSeconds = Math.max(0, Math.floor((autoOffDuration - elapsed) / 1000));
          element.textContent = remainingSeconds > 0 ? `${remainingSeconds} ثانية` : "مغلق";
        } else {
          element.textContent = "";
        }
      };
      
      updateTimer(1, data.manual1ActivationTime, 4000); // 4000 مللي ثانية (4 ثواني)
      updateTimer(2, data.manual2ActivationTime, 4000);
    });
}

  // التحديث التلقائي كل ثانية
       setInterval(fetchStatus, 1000);

  // // التحديث التلقائي كل ثانيتين
  //      setInterval(fetchStatus, 2000);

  // ---- قسم خاص بالمنسدلة القابلة للطي ---------
function toggleSettings() {
  const content = document.getElementById('advancedSettings');
  const btnText = document.querySelector('.btn-text');
  content.classList.toggle('active');
  
  // تحديث نص الزر ديناميكيًا
  if (content.classList.contains('active')) {
    btnText.innerHTML = 'إخفاء الإعدادات ▲';
  } else {
    btnText.innerHTML = 'إظهار الإعدادات ▼';
  }
}

function forceReload() { // ----- خاص بزر إعادة التحديث لمنع تخزين الكاش --------
  // إرسال طلب إلى السيرفر لطباعة الرسالة
  fetch(`/debug?msg=forceReloadCalled_${new Date().getTime()}`)
    .then(() => {
      // إعادة التوجيه بعد إرسال الطلب
      const randomParam = `?nocache=${Math.random().toString(36).substr(2, 9)}`;
      window.location.href = window.location.origin + randomParam;
    });
}

  </script>
</body>
</html>
   )rawliteral";

//--------------------دالة تجميع أجزاء صفحة الويب الواجهة ------------------------
String fullHtmlPage = htmlHeader + String(cssStyles) + String(htmlBody) + String(javascriptCode);

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
void resetWiFiConfig(bool restoreDefaults);

void setup() {
  Serial.begin(115200);

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

  server.on("/debug", HTTP_GET, []() {  // إضافة مسار جديد في الخادم ( /debug) يستقبل الطلبات ويطبع الرسالة على السيريال ------ إرسال طلب إلى السيرفر لطباعة الرسالة
  if (server.hasArg("msg")) {
    String message = server.arg("msg");
    Serial.print("رسالة من الواجهة: ");
    Serial.println(message);
  }
  server.send(200, "text/plain", "OK");
});
  
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
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        File file = SPIFFS.open("/css/all.min.css", "r");
    server.streamFile(file, "text/css");
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

  // الفونتات
  server.on("/fonts/Cairo-SemiBold.woff", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff", "r");
    server.streamFile(file, "application/font-woff"); // ✅
    file.close();
  });
  
 server.on("/fonts/Cairo-SemiBold.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/fonts/Cairo-SemiBold.woff2", "r");
    server.streamFile(file, "application/font-woff2"); // ✅
    file.close();
  });

   server.on("/fonts/Tajawal-Regular.woff", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    File file = SPIFFS.open("/fonts/Tajawal-Regular.woff", "r");
    server.streamFile(file, "application/font-woff"); // ✅
    file.close();
  });

   server.on("/fonts/Tajawal-Regular.woff2", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
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

  server.on("/", HTTP_GET, []() {
    if (isConnected) {
      String html = fullHtmlPage;
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
      //  إضافة رؤوس (Headers) HTTP لمنع التخزين الكاش
      server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); //  لتوجيه المتصفح بعدم تخزين الصفحة أو الملفات
      server.sendHeader("Pragma", "no-cache"); // لتوجيه المتصفح بعدم تخزين الصفحة أو الملفات
      server.sendHeader("Expires", "-1"); // لتوجيه المتصفح بعدم تخزين الصفحة أو الملفات
      server.send(200, "text/html", html); // إرسال الصفحة بعد الاستبدال
     } else {
      handleConfigPage();
    }
  });
  
  server.on("/saveConfig", HTTP_POST, handleSaveConfig);
  server.begin();

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
               server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); // 7 أيام
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

void resetWiFiConfig(bool restoreDefaults) { //  --- دالة لحذف أو استعادة الإعدادات الافتراضية للشبكة ----
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
  for (int i = 0; i < 20; i++) { // 20 محاولة (10 ثوانٍ)
    if (WiFi.status() == WL_CONNECTED) { 
      isConnected = true;
      Serial.println("\nتم الاتصال!");
      Serial.print("عنوان IP: ");
      Serial.println(WiFi.localIP());
      return; // الخروج من الدالة بعد الاتصال
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nفشل الاتصال، التبديل إلى وضع AP");
  startAPMode(); // التبديل إلى نقطة الوصول
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
  server.send(200, "text/html", configPageHTML);
}

void handleSaveConfig() { // ---------- حفظ إعدادات الشبكة الجديدة و إعادة التشغيل بدون تجميد الخادم -------
  if (server.hasArg("ssid") && server.hasArg("password")) {
    // تحديث إعدادات Wi-Fi
    strncpy(wifiSettings.ssid, server.arg("ssid").c_str(), sizeof(wifiSettings.ssid));
    strncpy(wifiSettings.password, server.arg("password").c_str(), sizeof(wifiSettings.password));
    saveWiFiConfig(); // حفظ الإعدادات في SPIFFS
  }

  // إرسال رد مع إعادة توجيه عبر JavaScript
  server.send(200, "text/html", 
    "<script>"
    "setTimeout(() => { window.location.href = '/'; }, 3000);" // إعادة التوجيه بعد 3 ثوانٍ
    "</script>"
    "✓ تم حفظ الإعدادات، جارِ إعادة التشغيل..."
  );

  // إعادة التشغيل بعد تأخير قصير (بدون تجميد الخادم)
  delay(100); 
  ESP.restart();
}

// ------ إعداد الخادم ------
void setupServer() {


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
