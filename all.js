let downloadBox;

const downloadData = {
    'vscode': {
        title: 'Visual Studio Code Downloads',
        links: [
            { name: 'Windows', url: 'https://code.visualstudio.com/docs/?dv=win'},
            { name: 'macOS', url: 'https://code.visualstudio.com/docs/?dv=osx'},
            { name: 'Linux', url: 'https://code.visualstudio.com/docs/?dv=linux'}
        ]
    },
    'arduino': {
        title: 'Arduino IDE Downloads',
        links: [
            { name: 'Windows', url: 'https://www.arduino.cc/en/software'},
            { name: 'macOS', url: 'https://www.arduino.cc/en/software'},
            { name: 'Linux', url: 'https://www.arduino.cc/en/software'}
        ]
    }
};


function showDownloadLinks(tool) {
    if (!downloadBox) return;

    const data = downloadData[tool];
    if (!data) return;

    if (downloadBox.classList.contains('show') && downloadBox.dataset.currentTool === tool) {
        downloadBox.classList.remove('show');
        setTimeout(() => {
            downloadBox.innerHTML = '';
            delete downloadBox.dataset.currentTool;
        }, 500); 
        return;
    }
    const wasShowing = downloadBox.classList.contains('show');
    if (wasShowing) {
        downloadBox.classList.remove('show');
    }
    setTimeout(() => {
        let content = `<h3>${data.title}</h3>`;
        data.links.forEach(link => {
            content += `<a class="download-link" href="${link.url}" target="_blank">${link.name}</a>`;
        });
        downloadBox.innerHTML = content;
        downloadBox.dataset.currentTool = tool; 
        requestAnimationFrame(() => {
            downloadBox.classList.add('show');
        });
    }, wasShowing ? 500 : 0);
}


function toggleImageZoom(img) {
    const container = img.parentElement;
    const downloadBtn = container.querySelector(".download-btn");

    if (img.classList.contains("zoomed")) {
        img.classList.remove("zoomed");
        downloadBtn.style.display = "none";
    } else {
        img.classList.add("zoomed");
        downloadBtn.style.display = "block";
    }
}

function toggleDescription(btn) {
    const box = btn.closest('.description-box');
    const content = box.querySelector('.description-content');
    const svgPath = btn.querySelector('svg path');

    if (content.classList.contains("open")) {
        content.classList.remove("open");
        btn.style.transform = "rotate(0deg)";
        svgPath.setAttribute("d", "M7 10l5 5 5-5H7z");
    } else {
        content.classList.add("open");
        btn.style.transform = "rotate(180deg)";
        svgPath.setAttribute("d", "M7 14l5-5 5 5H7z");
    }
}


document.addEventListener('DOMContentLoaded', () => {

    menuBtn = document.getElementById("menu-btn");
    sideMenu = document.getElementById("side-menu");
    closeBtn = document.getElementById("close-btn");
 
    downloadBox = document.getElementById("download-box");
    const loader = document.getElementById('loader');

    document.addEventListener('click', (event) => {
        const isToolIcon = event.target.closest('.tool-icon');
        const isDownloadBox = event.target.closest('#download-box');

        if (!isToolIcon && !isDownloadBox && downloadBox && downloadBox.classList.contains('show')) {
            downloadBox.classList.remove('show');
            setTimeout(() => {
                 downloadBox.innerHTML = '';
                 delete downloadBox.dataset.currentTool;
            }, 500); 
        }
    });

    window.addEventListener('load', () => {
        if (loader) {
             setTimeout(() => {
                loader.classList.add('hidden');
            }, 400);
        }
    });

    document.querySelectorAll("a").forEach(link => {
        link.addEventListener("click", function(e) {
            const href = this.getAttribute("href");
            if (!href.endsWith(".html") && href !== '/') return;

            e.preventDefault();
            const currentLoader = document.getElementById("loader");
            if (currentLoader) currentLoader.classList.remove('hidden');
            setTimeout(() => {
                window.location.href = href;
            }, 400);
        });
    });
});