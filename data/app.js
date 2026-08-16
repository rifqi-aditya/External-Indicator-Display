document.addEventListener('DOMContentLoaded', () => {
    const liveWeight = document.getElementById('liveWeight');
    const rawStream = document.getElementById('rawStream');
    const statusBadge = document.getElementById('statusBadge');
    const statusText = document.getElementById('statusText');
    const configForm = document.getElementById('configForm');
    const toast = document.getElementById('toast');

    // 1. Function untuk Polling Data Berat Realtime (Setiap 200ms)
    async function pollScaleData() {
        try {
            const response = await fetch('/api/weight');
            if (response.ok) {
                const data = await response.json();
                
                // Update Nilai Berat
                if (data.weight) {
                    // Ambil hanya komponen angka numerik dari weight string
                    const numPart = data.weight.replace(/[^0-9.-]/g, '');
                    liveWeight.innerText = numPart || '0.00';
                }

                // Update Raw Stream
                if (data.raw) {
                    rawStream.innerText = `"${data.raw}"`;
                } else {
                    rawStream.innerText = 'Menunggu stream RS232...';
                }

                // Update Connection Status Badge
                if (data.connected) {
                    statusBadge.classList.add('online');
                    statusText.innerText = 'INDICATOR ONLINE';
                } else {
                    statusBadge.classList.remove('online');
                    statusText.innerText = 'NO SIGNAL';
                }
            }
        } catch (err) {
            statusBadge.classList.remove('online');
            statusText.innerText = 'DISCONNECTED';
        }
    }

    // Jalankan Polling setiap 200ms
    setInterval(pollScaleData, 200);
    pollScaleData();

    // 2. Load Existing Config dari Server saat halaman dibuka
    async function loadCurrentConfig() {
        try {
            const response = await fetch('/api/config');
            if (response.ok) {
                const cfg = await response.json();
                if (cfg.mode !== undefined) document.getElementById('mode').value = cfg.mode;
                if (cfg.stext !== undefined) document.getElementById('stext').value = cfg.stext;
                if (cfg.rtext !== undefined) document.getElementById('rtext').value = cfg.rtext;
                if (cfg.baud !== undefined) document.getElementById('baud').value = cfg.baud;
            }
        } catch (err) {
            console.log('Using default form values');
        }
    }
    loadCurrentConfig();

    // 3. Handle Form Submit via AJAX (Tanpa Reload Halaman!)
    configForm.addEventListener('submit', async (e) => {
        e.preventDefault();

        const formData = new FormData(configForm);
        const params = new URLSearchParams(formData);

        try {
            const response = await fetch('/save', {
                method: 'POST',
                body: params
            });

            if (response.ok) {
                showToast('Konfigurasi Berhasil Disimpan!');
            } else {
                showToast('Gagal Menyimpan Konfigurasi!');
            }
        } catch (err) {
            showToast('Error Koneksi ke Server!');
        }
    });

    function showToast(message) {
        toast.innerText = message;
        toast.classList.add('show');
        setTimeout(() => {
            toast.classList.remove('show');
        }, 3000);
    }
});
