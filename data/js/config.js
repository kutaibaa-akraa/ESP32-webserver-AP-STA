// ======== ثوابت وإعدادات ========
const MAX_NETWORKS = 5;
let networks = [];
let currentNetworkIndex = 0;

function logout() {
  // إرسال طلب لإنهاء الجلسة
  fetch('/logout', { method: 'POST' })
    .then(() => {
      // إعادة تحميل الصفحة مع إضافة بارامتر لمنع الكاش
      window.location.href = '/?logout=' + Date.now();
    });
}

// ======== دوال إدارة الواجهة ========
function renderNetworks() {
  const container = document.getElementById('networkContainer');
  container.innerHTML = '';
  
  networks.forEach((net, index) => {
    const isActive = index === currentNetworkIndex;
    const networkCard = document.createElement('div');
    networkCard.className = `network-card ${isActive ? 'active' : ''}`;
    networkCard.dataset.index = index;
    
    networkCard.innerHTML = `
      <div class="network-header">
        <h3>${net.ssid || 'شبكة جديدة'}</h3>
        <span class="priority-badge">الأولوية: ${net.priority || 1}</span>
        ${isActive ? '<span class="active-badge">✔ نشطة</span>' : ''}
      </div>
      
      <div class="form-group">
        <label>اسم الشبكة (SSID)</label>
        <input type="text" name="ssid_${index}" value="${net.ssid || ''}" 
               placeholder="أدخل اسم شبكة الواي فاي">
      </div>
      
      <div class="form-group">
        <label>كلمة المرور</label>
        <input type="password" name="password_${index}" value="${net.password || ''}" 
               placeholder="أدخل كلمة مرور الشبكة">
      </div>
      
      <div class="form-group checkbox-group">
        <label>
          <input type="checkbox" name="useStaticIP_${index}" ${net.useStaticIP ? 'checked' : ''}>
          <span class="checkbox-custom"></span>
          استخدام عنوان IP ثابت
        </label>
      </div>
      
      <div id="staticIPFields_${index}" style="display:${net.useStaticIP ? 'block' : 'none'}">
        <div class="ip-grid">
          <div class="form-group">
            <label>العنوان المحلي (IP)</label>
            <input type="text" name="localIP_${index}" value="${net.localIP || '192.168.1.100'}" 
                   placeholder="192.168.1.100">
          </div>
          <div class="form-group">
            <label>البوابة (Gateway)</label>
            <input type="text" name="gateway_${index}" value="${net.gateway || '192.168.1.1'}" 
                   placeholder="192.168.1.1">
          </div>
          <div class="form-group">
            <label>القناع (Subnet)</label>
            <input type="text" name="subnet_${index}" value="${net.subnet || '255.255.255.0'}" 
                   placeholder="255.255.255.0">
          </div>
        </div>
      </div>
      
      <div class="network-actions">
        <button type="button" class="button button-danger" data-action="remove" data-index="${index}">
          <i class="fas fa-trash"></i> حذف
        </button>
        <button type="button" class="button button-info" data-action="set-active" data-index="${index}">
          <i class="fas fa-check"></i> تعيين نشطة
        </button>
      </div>
    `;
    
    container.appendChild(networkCard);
    
    // إضافة مستمع للأحداث بشكل ديناميكي
    const checkbox = networkCard.querySelector(`input[name="useStaticIP_${index}"]`);
    checkbox.addEventListener('change', () => toggleStaticIP(index));
  });
}

function toggleStaticIP(index) {
  const fields = document.getElementById(`staticIPFields_${index}`);
  const checkbox = document.querySelector(`input[name="useStaticIP_${index}"]`);
  fields.style.display = checkbox.checked ? 'block' : 'none';
}

// ======== إدارة بيانات الشبكات ========
async function loadNetworks() {
  try {
	  
    const response = await fetch('/getNetworks', {
      headers: {
        'Authorization': 'Basic ' + btoa('admin:admin')
      }
    });
    
    if (response.status === 401) {
      showAlert('❌ يجب تسجيل الدخول أولاً', 'error');
      return;
    }

    if (!response.ok) throw new Error('Failed to load networks');
    
    networks = await response.json();
    renderNetworks();
  } catch (error) {
    console.error('فشل في تحميل الشبكات:', error);
    showAlert('❌ فشل في تحميل إعدادات الشبكات!', 'error');
    
    // إضافة شبكة افتراضية
    networks = [{
      ssid: 'ESP32-Control',
      password: '12345678',
      useStaticIP: false,
      priority: 1
    }];
    
    renderNetworks();
  }
}

function addNetwork() {
  if (networks.length >= MAX_NETWORKS) {
    showAlert(`❌ الحد الأقصى ${MAX_NETWORKS} شبكات فقط!`, 'error');
    return;
  }
  
  networks.push({
    ssid: '',
    password: '',
    useStaticIP: false,
    priority: networks.length + 1
  });
  
  renderNetworks();
}

function removeNetwork(index) {
  if (networks.length <= 1) {
    showAlert('❌ يجب أن يكون هناك شبكة واحدة على الأقل!', 'error');
    return;
  }
  
  networks.splice(index, 1);
  
  // إعادة تعيين الأولويات
  networks.forEach((net, i) => {
    net.priority = i + 1;
  });
  
  // تحديث الشبكة النشطة
  if (currentNetworkIndex >= networks.length) {
    currentNetworkIndex = networks.length - 1;
  }
  
  renderNetworks();
}

function setActiveNetwork(index) {
  currentNetworkIndex = index;
  renderNetworks();
}

// ======== التحقق من الصحة والإرسال ========
function validateForm(e) {
  e.preventDefault();
  
  // التحقق من وجود شبكات
  if (networks.length === 0) {
    showAlert('❌ يجب إدخال شبكة واحدة على الأقل', 'error');
    return false;
  }
  
  // التحقق من كل شبكة
  for (let i = 0; i < networks.length; i++) {
    const ssid = document.querySelector(`input[name="ssid_${i}"]`).value;
    const password = document.querySelector(`input[name="password_${i}"]`).value;
    
    if (!ssid || ssid.trim().length < 2) {
      showAlert(`❌ اسم الشبكة قصير جداً في الشبكة ${i+1}`, 'error');
      return false;
    }
    
    if (!password || password.length < 8) {
      showAlert(`❌ كلمة المرور يجب أن تكون 8 أحرف على الأقل في الشبكة ${i+1}`, 'error');
      return false;
    }
    
    const useStaticIP = document.querySelector(`input[name="useStaticIP_${i}"]`).checked;
    if (useStaticIP) {
      const localIP = document.querySelector(`input[name="localIP_${i}"]`).value;
      const gateway = document.querySelector(`input[name="gateway_${i}"]`).value;
      const subnet = document.querySelector(`input[name="subnet_${i}"]`).value;
      
      if (!isValidIP(localIP) || !isValidIP(gateway) || !isValidIP(subnet)) {
        showAlert(`❌ عنوان IP غير صالح في الشبكة ${i+1}`, 'error');
        return false;
      }
    }
  }
  
  submitForm();
}

function submitForm() {
  const formData = new FormData();
  
  networks.forEach((net, index) => {
    formData.append(`ssid_${index}`, document.querySelector(`input[name="ssid_${index}"]`).value);
    formData.append(`password_${index}`, document.querySelector(`input[name="password_${index}"]`).value);
    formData.append(`useStaticIP_${index}`, document.querySelector(`input[name="useStaticIP_${index}"]`).checked ? 'on' : 'off');
    formData.append(`localIP_${index}`, document.querySelector(`input[name="localIP_${index}"]`)?.value || '');
    formData.append(`gateway_${index}`, document.querySelector(`input[name="gateway_${index}"]`)?.value || '');
    formData.append(`subnet_${index}`, document.querySelector(`input[name="subnet_${index}"]`)?.value || '');
    formData.append(`priority_${index}`, index + 1);
	formData.append(`priority_${index}`, net.priority || index + 1);
  });
  
  fetch('/saveConfig', {
    method: 'POST',
    headers: {
      'Authorization': 'Basic ' + btoa('admin:admin')
    },
    body: formData
  })
  .then(response => {
    if (!response.ok) throw new Error('Network response was not ok');
    return response.text();
  })
  .then(data => {
    showAlert('✓ تم حفظ الإعدادات بنجاح، جارِ إعادة التوجيه...', 'success');
    setTimeout(() => { window.location.href = '/'; }, 3000);
  })
  .catch(error => {
    console.error('Error:', error);
    showAlert('❌ فشل في حفظ الإعدادات!', 'error');
  });
}

// ======== دوال مساعدة ========
function isValidIP(ip) {
  const pattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
  if (!pattern.test(ip)) return false;
  
  const parts = ip.split('.');
  return parts.every(part => {
    const num = parseInt(part);
    return num >= 0 && num <= 255;
  });
}

function showAlert(message, type) {
  const alertDiv = document.getElementById('alert');
  alertDiv.style.display = 'block';
  alertDiv.className = `alert alert-${type}`;
  alertDiv.textContent = message;
  
  setTimeout(() => {
    alertDiv.style.display = 'none';
  }, 5000);
}

// ======== تهيئة الصفحة ========
document.addEventListener('DOMContentLoaded', function() {
  // تحميل الشبكات
  loadNetworks();
  
  // إعداد مستمعي الأحداث
  document.getElementById('addNetworkBtn').addEventListener('click', addNetwork);
  document.getElementById('wifiForm').addEventListener('submit', validateForm);
  
  // مستمع للأحداث على الحاويات
  document.getElementById('networkContainer').addEventListener('click', function(e) {
    if (e.target.dataset.action === 'remove') {
      removeNetwork(parseInt(e.target.dataset.index));
    } else if (e.target.dataset.action === 'set-active') {
      setActiveNetwork(parseInt(e.target.dataset.index));
    }
  });
});

// إصلاح إرسال الأولوية
networks.forEach((net, index) => {
  const priorityInput = document.querySelector(`input[name="priority_${index}"]`);
  if (priorityInput) {
    net.priority = parseInt(priorityInput.value) || (index + 1);
  }
});

// في ملف main.js
function toggleDarkMode() {
  const body = document.body;
  const darkModeIcon = document.getElementById('darkModeIcon');
  
  body.classList.toggle('dark-mode');
  
  if (body.classList.contains('dark-mode')) {
    darkModeIcon.className = 'fas fa-sun';
    localStorage.setItem('darkMode', 'enabled');
  } else {
    darkModeIcon.className = 'fas fa-moon';
    localStorage.setItem('darkMode', 'disabled');
  }
}

// عند تحميل الصفحة، تحقق من وضع التخزين المحلي
document.addEventListener('DOMContentLoaded', function() {
  const darkMode = localStorage.getItem('darkMode');
  const body = document.body;
  const darkModeIcon = document.getElementById('darkModeIcon');
  
  if (darkMode === 'enabled') {
    body.classList.add('dark-mode');
    darkModeIcon.className = 'fas fa-sun';
  } else {
    body.classList.remove('dark-mode');
    darkModeIcon.className = 'fas fa-moon';
  }
});