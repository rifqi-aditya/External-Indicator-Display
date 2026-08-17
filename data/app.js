document.addEventListener('DOMContentLoaded', () => {
    // Parameter P10 Display (2 panel horizontal = 64 piksel)
    const DISPLAY_WIDTH_PX = 64;

    // Elemen UI Utama
    const liveWeight = document.getElementById('liveWeight');
    const statusBadge = document.getElementById('statusBadge');
    const statusText = document.getElementById('statusText');
    const activeBaudTag = document.getElementById('activeBaudTag');
    const activeModeText = document.getElementById('activeModeText');
    const activeSignalText = document.getElementById('activeSignalText');
    const rawStreamBox = document.getElementById('rawStreamBox');
    const toast = document.getElementById('toast');

    // Elemen Input Teks & Smart Badge
    const stextInput = document.getElementById('stext');
    const modeSelect = document.getElementById('mode');
    const charCountLabel = document.getElementById('charCount');
    const smartBadge = document.getElementById('smartBadge');
    const badgeIcon = document.getElementById('badgeIcon');
    const badgeText = document.getElementById('badgeText');
    const groupCustomText = document.getElementById('groupCustomText');

    // State Debug Pause
    let isDebugPaused = false;
    const btnTogglePause = document.getElementById('btnTogglePause');

    // ==========================================
    // 1. MANAJEMEN TAB NAVIGASI
    // ==========================================
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabContents = document.querySelectorAll('.tab-content');

    tabButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const targetTab = btn.getAttribute('data-tab');

            tabButtons.forEach(b => b.classList.remove('active'));
            tabContents.forEach(c => c.classList.remove('active'));

            btn.classList.add('active');
            const activeContent = document.getElementById(targetTab);
            if (activeContent) {
                activeContent.classList.add('active');
            }
        });
    });

    if (btnTogglePause) {
        btnTogglePause.addEventListener('click', () => {
            isDebugPaused = !isDebugPaused;
            btnTogglePause.innerText = isDebugPaused ? 'Lanjutkan Stream' : 'Pause Stream';
            btnTogglePause.style.background = isDebugPaused ? 'rgba(239, 68, 68, 0.2)' : '';
        });
    }

    // ==========================================
    // 2. LOGIKA SMART AUTO STATIC / RUNNING TEXT
    // ==========================================
    // Mengukur lebar piksel string teks (Font 5x7: 5px per char + 1px spacing)
    function calculateTextPixelWidth(text) {
        let width = 0;
        for (let i = 0; i < text.length; i++) {
            const char = text[i];
            if (char === ' ') {
                width += 3; // Spasi = 3px
            } else {
                width += 6; // 5px huruf + 1px spasi
            }
        }
        return width > 0 ? width - 1 : 0;
    }

    function updateSmartTextPreview() {
        const text = stextInput.value;
        const mode = parseInt(modeSelect.value, 10);

        // Jika mode 0 (Live Weight), sembunyikan atau biarkan pasif
        if (mode === 0) {
            groupCustomText.style.opacity = '0.5';
        } else {
            groupCustomText.style.opacity = '1';
        }

        const pxWidth = calculateTextPixelWidth(text);
        charCountLabel.innerText = `${text.length} Karakter (${pxWidth}px / ${DISPLAY_WIDTH_PX}px)`;

        if (mode === 2) {
            // Paksa Running Text
            smartBadge.className = 'smart-badge running';
            badgeIcon.innerText = '📜';
            badgeText.innerText = 'Paksa Mode Berjalan (Continuous Running Text)';
        } else {
            // Mode 1: Smart Auto
            if (pxWidth <= DISPLAY_WIDTH_PX) {
                smartBadge.className = 'smart-badge static';
                badgeIcon.innerText = '📌';
                badgeText.innerText = 'Teks Muat (Tampil Statis Ditengah Layar)';
            } else {
                smartBadge.className = 'smart-badge running';
                badgeIcon.innerText = '📜';
                badgeText.innerText = 'Teks Panjang (Otomatis Berjalan / Running Text)';
            }
        }
    }

    if (stextInput && modeSelect) {
        stextInput.addEventListener('input', updateSmartTextPreview);
        modeSelect.addEventListener('change', updateSmartTextPreview);
    }

    // ==========================================
    // 3. POLLING DATA BEBAN REALTIME (/api/weight)
    // ==========================================
    let isFetchingScaleData = false;

    async function pollScaleData() {
        if (isFetchingScaleData || document.hidden) return;
        isFetchingScaleData = true;

        try {
            const response = await fetch('/api/weight');
            if (response.ok) {
                const data = await response.json();
                
                // Update Angka Beban
                if (data.weight) {
                    const numPart = data.weight.replace(/[^0-9.-]/g, '');
                    liveWeight.innerText = numPart || '0.00';
                }

                // Update Raw Stream Terminal
                if (!isDebugPaused && rawStreamBox) {
                    if (data.raw) {
                        rawStreamBox.innerText = `"${data.raw}"`;
                    } else {
                        rawStreamBox.innerText = 'Menunggu data stream RS232...';
                    }
                }

                // Update Status Badge
                if (data.connected) {
                    statusBadge.classList.add('online');
                    statusText.innerText = 'TERHUBUNG';
                    activeSignalText.innerText = 'Normal (Online)';
                    activeSignalText.className = 'status-item-val green';
                } else {
                    statusBadge.classList.remove('online');
                    statusText.innerText = 'TANPA SINYAL';
                    activeSignalText.innerText = 'No Signal (RS232)';
                    activeSignalText.className = 'status-item-val';
                }
            }
        } catch (err) {
            statusBadge.classList.remove('online');
            statusText.innerText = 'TERPUTUS';
            activeSignalText.innerText = 'Terputus (Offline)';
            activeSignalText.className = 'status-item-val';
        } finally {
            isFetchingScaleData = false;
        }
    }

    setInterval(pollScaleData, 300);
    pollScaleData();

    // ==========================================
    // 4. MEMUAT KONFIGURASI DARI SERVER
    // ==========================================
    async function loadCurrentConfig() {
        try {
            const response = await fetch('/api/config');
            if (response.ok) {
                const cfg = await response.json();
                if (cfg.mode !== undefined) {
                    modeSelect.value = cfg.mode;
                    updateActiveModeText(cfg.mode);
                }
                if (cfg.stext !== undefined) {
                    stextInput.value = cfg.stext;
                }
                if (cfg.baud !== undefined) {
                    document.getElementById('baud').value = cfg.baud;
                    if (activeBaudTag) activeBaudTag.innerText = `RS232 @ ${cfg.baud} BAUD`;
                }
                updateSmartTextPreview();
            }
        } catch (err) {
            console.log('Menggunakan nilai form bawaan');
        }
    }
    loadCurrentConfig();

    function updateActiveModeText(modeVal) {
        const m = parseInt(modeVal, 10);
        if (m === 0) activeModeText.innerText = 'Live Weight (Timbangan)';
        else if (m === 1) activeModeText.innerText = 'Teks Custom (Auto Statis/Running)';
        else if (m === 2) activeModeText.innerText = 'Paksa Running Text';
    }

    // ==========================================
    // 5. PENGIRIMAN FORM VIA AJAX (/save)
    // ==========================================
    async function sendConfig(params) {
        try {
            const response = await fetch('/save', {
                method: 'POST',
                body: params
            });
            if (response.ok) {
                showToast('Pengaturan Berhasil Disimpan!');
                loadCurrentConfig();
            } else {
                showToast('Gagal Menyimpan Pengaturan!');
            }
        } catch (err) {
            showToast('Koneksi Error ke Kontroler!');
        }
    }

    const displayForm = document.getElementById('displayForm');
    if (displayForm) {
        displayForm.addEventListener('submit', e => {
            e.preventDefault();
            const formData = new FormData(displayForm);
            const params = new URLSearchParams(formData);
            sendConfig(params);
        });
    }

    const serialForm = document.getElementById('serialForm');
    if (serialForm) {
        serialForm.addEventListener('submit', e => {
            e.preventDefault();
            const formData = new FormData(serialForm);
            const params = new URLSearchParams(formData);
            sendConfig(params);
        });
    }

    function showToast(message) {
        toast.innerText = message;
        toast.classList.add('show');
        setTimeout(() => {
            toast.classList.remove('show');
        }, 3000);
    }
});
