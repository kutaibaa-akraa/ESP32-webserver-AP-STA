
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
//يمكن تفعيل هذا الشرط إن أردت أن يعمل أحدهما و الثاني معطل  تماماً 
//(لا يعمل حتى ينتهي الأول )و هو خيار لتأكيد الحالة
//يتبع هذا الخيار للأزرار اليدوية التبادلية
// else {
 // document.getElementById('manual1Btn').disabled = false;
 // document.getElementById('manual2Btn').disabled = false;
// }

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

  // التحديث التلقائي كل ثانيتين
//  setInterval(fetchStatus, 2000);

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


