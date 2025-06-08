 // ------ الدوال العامة ------
  //دالة حفظ الإعدادات الزمنية
function saveSettings() {
  const interval = parseInt(document.getElementById('toggleInterval').value) || 5; // القيمة الافتراضية 5 ثواني
  const duration = parseInt(document.getElementById('duration').value) || 10; // القيمة الافتراضية 10 دقائق
  const pauseDuration = parseInt(document.getElementById('pauseDuration').value) || 0; // القيمة الافتراضية 0 ثواني

  fetch(`/saveSettings?interval=${interval}&duration=${duration}&pauseDuration=${pauseDuration}`, {
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
		  document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
		document.getElementById('pauseDuration').value = data.pauseDuration;
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
  function debounce(func, timeout = 1000) {
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
  
  // استدعاء الدالة عند تحميل الصفحات المحمية
document.addEventListener('DOMContentLoaded', function() {
	
	  // التحقق من حالات الخطأ في المصادقة
  const urlParams = new URLSearchParams(window.location.search);
  if (urlParams.has('auth_error')) {
    alert('خطأ في المصادقة! الرجاء إدخال اسم المستخدم وكلمة المرور الصحيحة');
  }
	
  if (window.location.pathname.includes('config') || 
      window.location.pathname.includes('upload')) {
    checkAuth();
  }
});

	  // ------ تهيئة الأحداث ------ دالة ذات أهمية عالية لتسهيل تحديث قيم صنادي الإدخال لقيم الأزمنة
	document.addEventListener('DOMContentLoaded', function() {
	  // ------ دالة مساعدة لمعالجة الإدخالات مع التحقق ------
	  const handleInput = (elementId, settingType, minValue, maxValue) => {
		const element = document.getElementById(elementId);
		
		element.addEventListener('input', function(e) {
		  let value = parseInt(e.target.value) || 0; // افتراضي 0 إذا كانت القيمة غير صحيحة
		  
		  // التحقق من النطاق المسموح
		  if (value < minValue) value = minValue;
		  if (value > maxValue) value = maxValue;
		  
		  // تطبيق القيمة المصححة على الحقل
		  e.target.value = value;
  
    // إرسال الطلب مع Debounce
    debounce(() => {
      fetch(`/setDuration?pin=${settingType}&duration=${value * 1000}`, { 
        method: 'POST' 
      })
      .then(response => response.json())
      .then(data => {
        outputsState = data;
	   updateSettings(settingType, value);
        showSaveNotification();
      });
    }, 500);
  });
}; 

  // ------ تهيئة أحداث الإدخال ------
  handleInput('toggleInterval', 'interval', 5, 300);    // / الفاصل الزمني  5-300 ثانية
  handleInput('duration', 'duration', 1, 120);         // المدة الكلية 1-120 دقيقة
   
   // إضافة معالجة زمن الاستراحة
  handleInput('pauseDuration', 'pauseDuration', 0, 10); // 0-10 ثواني

	// ------ تهيئة المخرجات اليدوية (1-10) ------
	for (let i = 1; i <= 10; i++) {
	  handleInput(
		`manual${i}Duration`, // ID الحقل (مثال: manual1Duration)
		i, // ⚡ إرسال رقم المخرج (1-10) بدلًا من اسم
		0,  // القيمة الدنيا (0 ثانية)
		300 // القيمة القصوى (300 ثانية)
	  );
	}
	  // ------ (اختياري) إضافة أحداث إضافية هنا ------
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
  systemPaused: false,
  // toggleInterval: 30,
  // duration: 10,
  // pauseDuration: 0,
  // isInPauseBetweenToggle: false
}; 

  // تعريف دالة عامة لتحديث حالة المخرجات التبادلية
    const updateToggleOutputs = (outputs) => {
       outputs.forEach(({ btnId, stateId, outputKey }) => {
       const btn = document.getElementById(btnId);
       const stateElement = document.getElementById(stateId);
       const isActive = outputsState[outputKey];

    // تحديث الأيقونة والنص
    btn.innerHTML = isActive 
      ? '<i class="fas fa-stop"></i> إيقاف' 
      : '<i class="fas fa-play"></i> تشغيل';

    // إدارة الفئات بشكل آمن
    btn.classList.remove('button-on', 'button-off');
    btn.classList.add(isActive ? 'button-off' : 'button-on');
    

    // تحديث الحالة
    stateElement.textContent = isActive ? 'يعمل' : 'مغلق';
    stateElement.className = isActive ? 'state-active' : 'state-inactive';
  });

  // إدارة التعطيل المتبادل
  const out1Btn = document.getElementById('out1Btn');
  const out2Btn = document.getElementById('out2Btn');
  out1Btn.disabled = outputsState.out2;
  out2Btn.disabled = outputsState.out1;
  out1Btn.classList.toggle('button-disabled', outputsState.out2);
  out2Btn.classList.toggle('button-disabled', outputsState.out1);
};

  // 1. تعريف أزواج الأزرار التبادلية (يمكن تعديلها حسب الحاجة)
const manualPairs = [
  { primary: 'manual1', secondary: 'manual2' }  // تبادل المخرجين الأول و اثاني
   ,
  // { primary: 'manual3', secondary: 'manual4' },
   { primary: 'manual5', secondary: 'manual6' } //, // تبادل المخرجين الخامس و السادس
  // { primary: 'manual7', secondary: 'manual8' },
  // { primary: 'manual9', secondary: 'manual10' }
];

// 2. دالة تحديث الأزرار اليدوية المعدلة
const updateManualButtons = () => {
  // تحديث جميع الأزرار من 1 إلى 10
  for (let i = 1; i <= 10; i++) {
    const btn = document.getElementById(`manual${i}Btn`);
    const state = document.getElementById(`manual${i}State`);
    const isActive = outputsState[`manual${i}`];

    // تحديث النص والفئات الأساسية
    btn.innerHTML = isActive 
      ? '<i class="fas fa-stop"></i> إيقاف' 
      : '<i class="fas fa-play"></i> تشغيل';      

    btn.classList.remove('button-on', 'button-off', 'button-disabled');
    btn.classList.add(isActive ? 'button-off' : 'button-on');
    state.textContent = isActive ? 'يعمل' : 'مغلق';
    state.className = isActive ? 'state-active' : 'state-inactive';
  }

  // 3. تطبيق التعطيل التبادلي على الزوج المحدد فقط
  manualPairs.forEach(pair => {
    const primaryBtn = document.getElementById(`${pair.primary}Btn`);
    const secondaryBtn = document.getElementById(`${pair.secondary}Btn`);
    primaryBtn.classList.remove('button-disabled');
    secondaryBtn.classList.remove('button-disabled');
    primaryBtn.disabled = false;
    secondaryBtn.disabled = false;
    if (outputsState[pair.primary]) {
      secondaryBtn.classList.add('button-disabled');
      secondaryBtn.disabled = true;
    } else if (outputsState[pair.secondary]) {
      primaryBtn.classList.add('button-disabled');
      primaryBtn.disabled = true;
    }
  });
};

  // تحديث جميع عناصر التحكم
  function updateAllControls() {
                     // تحديث أزرار التبادل
                     // استدعاء الدالة مع تعريف المخرجات
                     // تحديث المخرجات التبادلية
     updateToggleOutputs([
    { btnId: 'out1Btn', stateId: 'out1State', outputKey: 'out1' },
    { btnId: 'out2Btn', stateId: 'out2State', outputKey: 'out2' }
  ]);

  // تحديث الأزرار اليدوية
  if (outputsState.manual1) {
    document.getElementById('manual2Btn').classList.add('button-disabled');
  } else if (outputsState.manual2) {
    document.getElementById('manual1Btn').classList.add('button-disabled');
  }

// //يمكن تفعيل هذا الشرط إن أردت أن يعمل أحدهما و الثاني معطل  تماماً 
// //(لا يعمل حتى ينتهي الأول )و هو خيار لتأكيد الحالة
// //يتبع هذا الخيار للأزرار اليدوية التبادلية
//     else {
//          document.getElementById('manual1Btn').disabled = false;
//          document.getElementById('manual2Btn').disabled = false;
//         }

    // تحديث الأزرار اليدوية (1-10)
	  updateManualButtons(); 
/*     for (let i = 1; i <= 10; i++) {
      const btn = document.getElementById(`manual${i}Btn`);
      const state = document.getElementById(`manual${i}State`);
      btn.textContent = outputsState[`manual${i}`] ? 'إيقاف' : 'تشغيل';
      btn.className = outputsState[`manual${i}`] ? 'button button-off' : 'button button-on';
      state.textContent = outputsState[`manual${i}`] ? 'يعمل' : 'مغلق';
      state.className = outputsState[`manual${i}`] ? 'state-active' : 'state-inactive';
    } */

    // تحديث حالة النظام
    const systemStatus = document.getElementById('systemStatus');
    systemStatus.textContent = outputsState.systemPaused ? 'معلق' : 
      (outputsState.systemActive ? 'يعمل' : 'متوقف');
    systemStatus.style.color = outputsState.systemActive ? 
      (outputsState.systemPaused ? '#f39c12' : '#2ecc71') : '#e74c3c';    

  // تحديث زر التشغيل/الإيقاف
  const toggleBtn = document.getElementById('toggleBtn');
  toggleBtn.innerHTML = outputsState.systemActive 
    ? '<i class="fas fa-power-off"></i> إيقاف البرنامج' 
    : '<i class="fas fa-play"></i> تشغيل البرنامج';
  toggleBtn.className = `button ${outputsState.systemActive ? 'button-off' : 'button-on'}`;

  // تحديث زر الإيقاف المؤقت
  const pauseBtn = document.getElementById('pauseBtn');
    pauseBtn.innerHTML = outputsState.systemPaused 
    ? '<i class="fas fa-play"></i> متابعة' 
    : '<i class="fas fa-pause"></i> إيقاف مؤقت';
    pauseBtn.className = 'button button-pause';
    pauseBtn.style.display = outputsState.systemActive ? 'inline-block' : 'none';  

  // تحديث القيم المدخلة (فقط إذا لم تكن قيد التحرير) ---- لصندوقي إدخال زمن التبادل و الزمن الكلي
  const toggleIntervalInput = document.getElementById('toggleInterval');
  const durationInput = document.getElementById('duration');
  const pauseDurationInput = document.getElementById('pauseDuration');

  if (!toggleIntervalInput.matches(':focus')) {
    toggleIntervalInput.value = outputsState.toggleInterval;
  }

  if (!durationInput.matches(':focus')) {
    durationInput.value = outputsState.duration;
  }

   if (!pauseDurationInput.matches(':focus')) {
     pauseDurationInput.value = outputsState.pauseDuration;
   }
  
    // if (!pauseDurationInput.matches(':focus')) {
    // pauseDurationInput.value = outputsState.pauseDuration || 0;
  // }

	  // تحديث حقول المدة (فقط إذا لم تكن قيد التحرير)
  for (let i = 1; i <= 10; i++) {
    const input = document.getElementById(`manual${i}Duration`);
    if (!input.matches(':focus')) {
      input.value = outputsState[`manual${i}Duration`] / 1000;
    }
  }
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
	document.querySelector('.dark-mode-toggle').onclick = toggleDarkMode; // خاص زر الوضع الداكن
	initDarkMode(); 
  };

  // ------ وظائف النظام ------
  // تبديل حالة المخرج
 // function toggleOutput(outputId) {
  //  fetch(`/${outputId}/toggle`, { method: 'POST' })
 //     .then(response => response.json())
  //    .then(data => {
  //      outputsState = data;
  //      updateAllControls();
  //    });
//  }

function toggleOutput(outputId) {
  fetch(`/${outputId}/toggle`, { method: 'POST' })
    .then(response => response.json())
    .then(data => {
      outputsState = data;
      
      // إعادة حساب الوقت المتبقي
      for (let i = 1; i <= 10; i++) {
        const activationTime = data[`manual${i}ActivationTime`];
        const duration = data[`manual${i}Duration`];
        
        if (activationTime > 0 && duration > 0) {
          updateManualRemainingTime(i, activationTime, duration, data.currentTime);
        }
      }
      
      updateAllControls();
    });
}

  // تشغيل/إيقاف النظام التبادلي
function toggleSystem() {
  const duration = document.getElementById('duration').value;
  const interval = document.getElementById('toggleInterval').value;
  const pauseDuration = document.getElementById('pauseDuration').value;

  fetch(`/toggle?duration=${duration}&interval=${interval}&pauseDuration=${pauseDuration}`, {
    method: 'POST'
  })
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

  // التحديث التلقائي كل ثانية
       setInterval(fetchStatus, 1000);
	   
  // // التحديث التلقائي كل ثانيتين
  //      setInterval(fetchStatus, 2000);

  // ---- قسم خاص بالمنسدلة القابلة للطي ---------
 function toggleSettings() {
  const content = document.getElementById('advancedSettings');
  const btnText = document.querySelector('.collapse-btn .btn-text');
  
  content.classList.toggle('active');
  
  // تحديث نص الزر بدقة
  if (content.classList.contains('active')) {
    btnText.innerHTML = 'إخفاء الإعدادات ▲';
  } else {
    btnText.innerHTML = 'إظهار الإعدادات ▼';
  }
}

// دالة جديدة لعرض قسم الرفع:
function toggleUploadSection() {
  const uploadSection = document.getElementById('uploadSection');
  uploadSection.style.display = uploadSection.style.display === 'none' ? 
    'block' : 'none';
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

// ----- دالة فحص وجود الملفات في السيرفر -----
function checkFiles() {
  fetch('/checkFiles')
    .then(response => {
      if (response.ok) {
        alert("✓ تم فحص الملفات - راجع السيريال مونيتور");
      } else {
        alert("❌ فشل في فحص الملفات!");
      }
    })
    .catch(error => {
      alert("❌ خطأ في الاتصال!");
    });
}

// ------ متغيرات لتتبع الحقول قيد التحرير ------
let isEditing = false;
let currentEditValue = null;

// ------ أحداث التركيز والخروج من الحقل ------
document.querySelectorAll('input[type="number"]').forEach(input => {
  input.addEventListener('focus', () => {
    isEditing = true;
    currentEditValue = input.value; // حفظ القيمة الحالية عند التركيز
  });  

  input.addEventListener('blur', () => {
    isEditing = false;
  });
});

// جلب حالة النظام
function fetchStatus() {
  if (isEditing) return; // لا تحدث إذا كان حقل في وضع التحرير
    fetch(`/status?nocache=${Math.random()}`)
    .then(response => response.json())
    .then(data => {
      outputsState = data;
      updateAllControls();
      document.getElementById('progressBar').style.width = data.progress + '%';
      document.getElementById('remainingTime').textContent = data.remainingTime + ' دقيقة';
      document.getElementById('elapsedTime').textContent = data.elapsedTime + ' دقيقة';
      document.getElementById('elapsedPercent').textContent = data.elapsedProgress + '%';
	    // تحديث الوقت المتبقي والمنقضي
  document.getElementById('remainingMinutes').textContent = data.remainingMinutes;
  document.getElementById('remainingSeconds').textContent = data.remainingSeconds;
  document.getElementById('elapsedMinutes').textContent = data.elapsedMinutes;
  document.getElementById('elapsedSeconds').textContent = data.elapsedSeconds;
	  
// إضافة حالة الاستراحة بين التبديلات
if (data.systemPaused && data.pauseStartTime && data.pauseDuration) {
  const currentTime = Date.now();
  const endTime = data.pauseStartTime + data.pauseDuration;
  const remainingPause = Math.max(0, endTime - currentTime);
  
  const seconds = Math.ceil(remainingPause / 1000);
  const formattedSeconds = seconds < 10 ? `0${seconds}` : seconds;
  
  document.getElementById('systemStatus').textContent = `استراحة: ${formattedSeconds} ثانية`;
  document.getElementById('systemStatus').style.color = '#f39c12'; // لون مختلف للاستراحة
}
	  
      // تحديث العد التنازلي لكل المخارج (مع استثناء الحقل قيد التحرير)
   //   for (let i = 1; i <= 10; i++) {
   //     const input = document.getElementById(`manual${i}Duration`);
    //    if (!input.matches(':focus')) { // لا تحدث إذا الحقل تحت التركيز
    //      updateManualRemainingTime(i, data[`manual${i}ActivationTime`], data[`manual${i}Duration`]);
    //    }
    //  }
      // تحديث الوقت المتبقي للمخارج
      for (let i = 1; i <= 10; i++) {
        const activationTime = data[`manual${i}ActivationTime`];
        const duration = data[`manual${i}Duration`];
        const serverTime = data.currentTime;
        
        if (activationTime > 0 && duration > 0) {
          updateManualRemainingTime(i, activationTime, duration, serverTime);
        }
      }
    });
}

// دالة محدثة مع تعريف صحيح للمتغيرات
/* function updateManualRemainingTime(pinIndex, activationTime, durationMs) { // ----- دالة العد التنازلي لإغلاق المخارج -------
  const remainingElement = document.getElementById(`manual${pinIndex}Remaining`);
  if (activationTime > 0 && durationMs > 0) {
    const currentTime = Date.now();
    const elapsedTime = currentTime - activationTime;
    const remainingTime = Math.max(0, durationMs - elapsedTime);
	
    if (remainingTime > 0) {
      const remainingSeconds = Math.ceil(remainingTime / 1000);
      remainingElement.textContent = `${remainingSeconds} ثانية`;
      remainingElement.style.color = remainingSeconds <= 5 ? "#e74c3c" : "#2ecc71";
    } else {
      remainingElement.textContent = "انتهى";
      remainingElement.style.color = "#e74c3c";
    }
  } else {
    remainingElement.textContent = "";
  }
} */
function updateManualRemainingTime(pinIndex, activationTime, durationMs, serverTime) {
  const remainingElement = document.getElementById(`manual${pinIndex}Remaining`);
  
  if (activationTime > 0 && durationMs > 0) {
    const currentTime = Date.now();
    const serverCurrentTime = serverTime + (currentTime - data.receivedTime);
    const elapsed = serverCurrentTime - activationTime;
    const remainingTime = Math.max(0, durationMs - elapsed);

    if (remainingTime > 0) {
      const totalSeconds = Math.ceil(remainingTime / 1000);
      const minutes = Math.floor(totalSeconds / 60);
      const seconds = totalSeconds % 60;
      const formattedMinutes = minutes.toString().padStart(2, '0');
      const formattedSeconds = seconds.toString().padStart(2, '0');
      
      remainingElement.textContent = `${formattedMinutes} د ${formattedSeconds} ث`;
      remainingElement.style.color = totalSeconds <= 10 ? "#e74c3c" : "#2ecc71";
    } else {
      remainingElement.textContent = "انتهى";
      remainingElement.style.color = "#e74c3c";
    }
  } else {
    remainingElement.textContent = "";
  }
}

// ------ لتعيين مدة تشغيل المخرج ---------
function setDuration(pinNumber) {  
  const parsedPin = parseInt(pinNumber); // --- تحويل النص إلى رقم ---
  const durationInput = document.getElementById(`manual${pinNumber}Duration`);
  const durationMs = durationInput.value * 1000;

  fetch(`/setDuration?pin=${pinNumber}&duration=${durationMs}`, { // ---- هذا الطلب يستدعي دالة في كود ESP32 (مثل /setDuration) تقوم بحفظ القيمة في الذاكرة (SPIFFS) ----
    method: 'POST' 
  })
  .then(response => response.json())
  .then(data => {
    outputsState = data;
    durationInput.value = data[`manual${pinNumber}Duration`] / 1000; // تحديث فوري
    showSaveNotification();
  });
}

// ---- دالة حفظ الإعدادات --------
function saveSystemSettings() {
  fetch('/saveSystemSettings', { method: 'POST' })
    .then(response => response.text())
    .then(data => alert(data));
}

/// --- دالة استدعاء الإعدادات ------
function loadSystemSettings() {
  fetch('/loadSystemSettings')
    .then(response => response.json())
    .then(data => {
      outputsState = data;
      updateAllControls();
      
      // عرض رسالة للمستخدم
      if (data.message) {
        alert(data.message);
      } else {
        alert("✓ تم تحميل الإعدادات بنجاح!");
      }
    })
    .catch(error => {
      console.error("Error:", error);
      alert("❌ حدث خطأ أثناء تحميل الإعدادات");
    });
}

function uploadFile() {
  const fileInput = document.getElementById('fileInput');
  const file = fileInput.files[0];
  if (!file) return;

  // التحقق من الملفات الحساسة
  const sensitiveFiles = ['wifi_config.bin', 'system_settings.bin'];
  if (sensitiveFiles.includes(file.name)) {
    const username = prompt('اسم المستخدم:');
    const password = prompt('كلمة المرور:');
    
    if (username !== 'admin' || password !== 'admin') {
      alert('صلاحية غير كافية لرفع هذا الملف');
      return;
    }
  }

  // التحقق من الملفات المسموحة
  const allowedTypes = {
    'text/css': '/css/',
    'application/javascript': '/js/',
    'font/woff2': '/fonts/',
    'text/plain': '/',
    'application/octet-stream': '/'
  };

  const formData = new FormData();
  formData.append('file', file);

  fetch('/upload', {
    method: 'POST',
    body: formData
  })
  .then(response => response.text())
  .then(data => {
    showUploadStatus('✓ ' + data, 'success');
    setTimeout(() => location.reload(), 1500);
  })
  .catch(error => {
    showUploadStatus('❌ ' + error, 'error');
  });
}

function showUploadStatus(message, type) {
  const statusDiv = document.getElementById('uploadStatus');
  statusDiv.innerHTML = `<div class="alert alert-${type}">${message}</div>`;
  setTimeout(() => statusDiv.innerHTML = '', 3000);
}

// دالة مساعدة لتنزيل ملف إعدادات الشبكة 
async function downloadFile(filename, customName = null) {
  if (!confirm('هل أنت متأكد من تنزيل إعدادات الشبكة؟')) return;
  
  try {
    const response = await fetch(`/download?file=${encodeURIComponent(filename)}`, {
      headers: {
        'Authorization': 'Basic ' + btoa('admin:admin')
      }
    });
    
    if (response.status === 401) {
      alert('يجب تسجيل الدخول لتنزيل الإعدادات');
      return;
    }
    
    const blob = await response.blob();
    
    // إنشاء رابط مؤقت للتنزيل
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = customName || filename; // اسم الملف المطلوب
    document.body.appendChild(a);
    a.click();
    
    // التنظيف
    window.URL.revokeObjectURL(url);
    document.body.removeChild(a);
  } catch (error) {
    console.error('فشل التنزيل:', error);
    alert('❌ فشل في تنزيل الملف!');
  }
}

// --- دالة الاستعادة و الحذف لإعدادات الشبكة ----
window.resetConfig = function(action) { // جعلها متاحة عالمياً
  const actions = {
    'default': {
      name: 'استعادة الإعدادات الافتراضية',
      endpoint: '/resetConfigDefault'
    },
    'delete': {
      name: 'حذف جميع الإعدادات',
      endpoint: '/resetConfigDelete'
    }
  };
  
  if (!actions[action]) return;
  
  if (!confirm(`⚠️ هل أنت متأكد من ${actions[action].name}؟ لا يمكن التراجع عن هذا الإجراء!`)) {
    return;
  }
  
  fetch(actions[action].endpoint, { 
    method: 'POST',
    headers: {
      'Cache-Control': 'no-cache',
      'Authorization': 'Basic ' + btoa('admin:admin')
    }
  })
  .then(handleResponse)
  .catch(handleError);
  
  function handleResponse(response) {
    if (response.status === 401) {
      alert('يجب تسجيل الدخول لإجراء هذه العملية');
      return;
    }
    
    if (response.ok) {
      showAlert('✅ تمت العملية بنجاح! جارِ إعادة التحميل...', 'success');
      setTimeout(() => {
        window.location.href = '/?nocache=' + Math.random();
      }, 2000);
    } else {
      throw new Error('فشلت العملية');
    }
  }
  
  function handleError(error) {
    console.error('Error:', error);
    showAlert('❌ حدث خطأ في الاتصال بالسيرفر!', 'error');
  }
  
  function showAlert(message, type) {
    const alertDiv = document.createElement('div');
    alertDiv.className = `alert alert-${type}`;
    alertDiv.textContent = message;
    document.body.appendChild(alertDiv);
    setTimeout(() => alertDiv.remove(), 3000);
  }
};

// Dark Mode Toggle
function toggleDarkMode() {
  const body = document.body;
  body.classList.toggle('dark-mode');
  
  const isDarkMode = body.classList.contains('dark-mode');
  localStorage.setItem('darkMode', isDarkMode);
  
  const icon = document.getElementById('darkModeIcon');
  icon.className = isDarkMode ? 'fas fa-sun' : 'fas fa-moon';
}

// Initialize Dark Mode from localStorage
function initDarkMode() {
  const isDarkMode = localStorage.getItem('darkMode') === 'true';
  document.body.classList.toggle('dark-mode', isDarkMode);
  
  const icon = document.getElementById('darkModeIcon');
  if (icon) {
    icon.className = isDarkMode ? 'fas fa-sun' : 'fas fa-moon';
  }
}

// التحديث اليدوي و وقف التحديث الآلي
if (!window.stopAutoRefresh) {
  // وضع التحديث العادي
  setInterval(fetchStatus, 2000);
} else {
  // وضع App Inventor: تحديث يدوي فقط
  document.getElementById('manualRefreshBtn').style.display = 'block';
}

// دالة التحديث اليدوي
function manualRefresh() {
  fetchStatus();
  showNotification("تم التحديث");
}
// --- دالة تسجيل الخروج ---
function logout() {
  fetch('/logout', { method: 'POST' })
    .then(() => {
      window.location.href = '/?logout=' + Date.now();
    });
}

// إضافة دالة للتحقق من المصادقة عند التحميل
function checkAuth() {
  fetch('/checkAuth')
    .then(response => {
      if (response.status === 401) {
        alert('يجب تسجيل الدخول أولاً');
        window.location.href = '/';
      }
    });
}

