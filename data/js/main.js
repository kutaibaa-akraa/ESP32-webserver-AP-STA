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
  { primary: 'manual1', secondary: 'manual2' }
  // ,
  // { primary: 'manual3', secondary: 'manual4' },
  // { primary: 'manual5', secondary: 'manual6' },
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
  
    // تحديث القيم المدخلة
    document.getElementById('toggleInterval').value = outputsState.toggleInterval;
    document.getElementById('duration').value = outputsState.duration;
	
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

/*   // جلب حالة النظام
function fetchStatus() {
  fetch(`/status?nocache=${Math.random()}`)
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
} */

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

      // تحديث العد التنازلي لكل المخارج (مع استثناء الحقل قيد التحرير)
      for (let i = 1; i <= 10; i++) {
        const input = document.getElementById(`manual${i}Duration`);
        if (!input.matches(':focus')) { // لا تحدث إذا الحقل تحت التركيز
          updateManualRemainingTime(i, data[`manual${i}ActivationTime`], data[`manual${i}Duration`]);
        }
      }
    });
}

// دالة محدثة مع تعريف صحيح للمتغيرات
function updateManualRemainingTime(pinIndex, activationTime, durationMs) { // ----- دالة العد التنازلي لإغلاق المخارج -------
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
}

function setDuration(pinNumber) {  // ------ لتعيين مدة تشغيل المخرج ---------
  const durationInput = document.getElementById(`manual${pinNumber}Duration`);
  const durationMs = durationInput.value * 1000;

  fetch(`/setDuration?pin=${pinNumber}&duration=${durationMs}`, { 
    method: 'POST' 
  })
  .then(response => response.json())
  .then(data => {
    outputsState = data;
    durationInput.value = data[`manual${pinNumber}Duration`] / 1000; // تحديث فوري
    showSaveNotification();
  });
}