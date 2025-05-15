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
