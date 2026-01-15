/* ===============================
Background Music Utilities
================================ */
let bgMusic = null;
let fadeInterval = null;

function startMusic() {
    if (!bgMusic) {
        bgMusic = new Audio("https://phangchunhoe.github.io/smart_recycling_bin/audio/game_music.mp3"); // <-- your music file
        bgMusic.loop = true;
        bgMusic.volume = 0.35;
    }

    bgMusic.currentTime = 0;
    bgMusic.play().catch(() => {
        // autoplay might fail if not user-initiated — safe to ignore
    });
}

function startMusicFadeIn() {
    if (!bgMusic) {
        bgMusic = new Audio("./audio/game_music.mp3"); // your file
        bgMusic.loop = true;
    }

    clearInterval(fadeInterval);

    bgMusic.volume = 0;
    bgMusic.currentTime = 0;
    bgMusic.play().catch(() => {});

    fadeInterval = setInterval(() => {
        if (bgMusic.volume < 0.35) {
            bgMusic.volume = Math.min(bgMusic.volume + 0.02, 0.35);
        } else {
            clearInterval(fadeInterval);
        }
    }, 80); // smooth fade (~1.5s)
}

function stopMusicFadeOut() {
    if (!bgMusic) return;

    clearInterval(fadeInterval);

    fadeInterval = setInterval(() => {
        if (bgMusic.volume > 0.02) {
            bgMusic.volume -= 0.02;
        } else {
            bgMusic.volume = 0;
            bgMusic.pause();
            bgMusic.currentTime = 0;
            clearInterval(fadeInterval);
        }
    }, 80);
}



function stopMusic() {
    if (bgMusic) {
        bgMusic.pause();
        bgMusic.currentTime = 0;
    }
}



/* ===============================
Inject Tailwind CSS (CDN)
================================ */
(function injectTailwind() {
    if (!document.getElementById("tailwindcdn")) {
        const link = document.createElement("script");
        link.src = "https://cdn.tailwindcss.com";
        link.id = "tailwindcdn";
        document.head.appendChild(link);
    }
})();

/* ===============================
EcoCoin Button Logic
================================ */
const ecoBtn = document.querySelector(".ecocoin");

ecoBtn.addEventListener("click", showCodePopup);

function showCodePopup() {
    const overlay = document.createElement("div");
    overlay.className =
        "fixed inset-0 bg-black/60 flex items-center justify-center z-50";

    overlay.innerHTML = `
        <div id="popupCard" class="relative glass-card w-[90%] max-w-md p-6 text-center animate-scaleIn">


            <!-- ✕ Close Button -->
            <button id="closePopup"
                class="absolute top-3 right-3 w-8 h-8
                    flex items-center justify-center
                    rounded-full
                    text-gray-400 hover:text-gray-600
                    hover:bg-gray-100
                    transition">
                ✕
            </button>

            <h2 class="text-2xl font-bold text-green-600 mb-2">
                EcoCoin Access
            </h2>

            <p class="text-gray-600 mb-4">
                Enter your 4-digit EcoCode 🌱
            </p>

            <input id="ecoCodeInput"
                type="password"
                maxlength="4"
                class="w-full text-center text-2xl tracking-widest border rounded-lg p-3 focus:outline-none focus:ring-2 focus:ring-green-400"
                placeholder="••••">

            <p id="errorMsg" class="text-red-500 text-sm mt-2 hidden">
                Invalid code. Try again.
            </p>

            <button id="submitCode"
                class="mt-5 w-full bg-green-500 hover:bg-green-600 text-white font-semibold py-3 rounded-lg transition">
                Enter Game
            </button>
        </div>
    `;

    document.body.appendChild(overlay);
    
    // to dismiss the popup
    document.getElementById("closePopup").onclick = () => {
        overlay.remove();
    };

    document.getElementById("popupCard").addEventListener("click", (e) => {
        e.stopPropagation();
    });

    overlay.addEventListener("click", (e) => {
        if (e.target === overlay) {
            overlay.remove();
        }
    });



    document.getElementById("submitCode").onclick = () => {
        const code = document.getElementById("ecoCodeInput").value;
        console.log("Code entered:", code, "Length:", code.length);
        if (code === "1234" || code === "8888" || code === "5601" || code === "8716") {
            console.log("Code correct! Starting quiz...");
            overlay.remove();
            startMusicFadeIn(); 
            startEcoQuiz();
        } else {
            console.log("Code incorrect");
            document.getElementById("errorMsg").classList.remove("hidden");
        }
    };
}

/* ===============================
Quiz Data
================================ */
const questions = [
    {
        q: "Which of these items can be recycled?",
        options: ["Plastic Bottle", "Food Waste", "Used Tissue", "Styrofoam"],
        answer: 0
    },
    {
        q: "What color bin is usually used for paper?",
        options: ["Blue", "Green", "Red", "Black"],
        answer: 0
    },
    {
        q: "Why should we recycle?",
        options: [
            "To save energy",
            "To reduce pollution",
            "To conserve resources",
            "All of the above"
        ],
        answer: 3
    },
    {
        q: "Which material takes the longest to decompose?",
        options: ["Paper", "Banana Peel", "Plastic", "Cardboard"],
        answer: 2
    },
    {
        q: "What should you do before recycling a bottle?",
        options: [
            "Throw it as is",
            "Wash and empty it",
            "Crush it with trash",
            "Fill it with water"
        ],
        answer: 1
    }
];

/* ===============================
Start Quiz
================================ */
let currentQ = 0;
let score = 0;

function startEcoQuiz() {
    document.body.innerHTML = `
        <div class="min-h-screen quiz-background">
            <div class="min-h-screen quiz-overlay flex items-center justify-center p-4">
                <div class="glass-card w-full max-w-2xl p-6">                             
                    <div class="mb-4">
                        <div class="w-full glass-progress rounded-full h-3">
                            <div id="progressBar"
                                class="bg-green-500 h-3 rounded-full transition-all"
                                style="width: 0%"></div>
                        </div>
                        <p class="text-sm text-gray-500 mt-1">
                            Question <span id="qNum">1</span> / 5
                        </p>
                    </div>

                    <h2 id="questionText"
                        class="text-2xl font-bold text-gray-800 mb-6">
                    </h2>

                    <div id="options"
                        class="grid grid-cols-1 sm:grid-cols-2 gap-4">
                    </div>
                </div>
            </div>
        </div>    
    `;

    renderQuestion();
}

/* ===============================
Render Question
================================ */
function renderQuestion() {
    const q = questions[currentQ];

    document.getElementById("questionText").textContent = q.q;
    document.getElementById("qNum").textContent = currentQ + 1;

    document.getElementById("progressBar").style.width =
        ((currentQ) / questions.length) * 100 + "%";

    const optionsDiv = document.getElementById("options");
    optionsDiv.innerHTML = "";

    q.options.forEach((opt, index) => {
        const btn = document.createElement("button");
        btn.className =
            "glass-option p-4 text-lg font-semibold text-gray-800";

        btn.textContent = opt;

        btn.onclick = () => handleAnswer(index);
        optionsDiv.appendChild(btn);
    });
}

/* ===============================
Handle Answer
================================ */
function handleAnswer(selected) {
    if (selected === questions[currentQ].answer) {
        score++;
    }

    currentQ++;

    if (currentQ < questions.length) {
        renderQuestion();
    } else {
        showResults();
    }
}

/* ===============================
Results Screen
================================ */
function showResults() {
    stopMusicFadeOut();

    if (score === 0) {
        alert("Try again to earn a voucher 🌱");
        location.reload();
        return;
    }



    const voucherValue = score; // $1 per correct

    document.body.innerHTML = `
        <div class="min-h-screen quiz-background results-page">
            <div class="min-h-screen quiz-overlay flex flex-col items-center justify-center p-6 text-center">

                <h1 class="text-4xl font-extrabold text-white mb-2 animate-float">
                    🎉 Congratulations! 🎉
                </h1>

                <p class="text-white/90 mb-6 text-lg animate-float">
                    You’ve earned a FairPrice voucher
                </p>

                <div class="voucher-container mb-8">
                    <div id="voucherCard" class="voucher-card glass-card voucher-glow">
                        <div id="voucherInner" class="voucher-inner">

                            <!-- FRONT -->
                            <div class="voucher-face voucher-front flex flex-col items-center justify-center gap-4">
                                <img src="https://phangchunhoe.github.io/smart_recycling_bin/images/ntuc_logo_noBackground.png"
                                    class="w-32 object-contain"
                                    alt="FairPrice Logo">

                                <p class="text-gray-800 font-semibold">
                                    Voucher Value
                                </p>

                                <p class="text-4xl font-extrabold text-green-600">
                                    $${voucherValue}
                                </p>

                                <p class="text-sm text-gray-600">
                                    Tap to reveal 🎁
                                </p>
                            </div>

                            <!-- BACK -->
                            <div class="voucher-face voucher-back flex flex-col items-center justify-center gap-3">
                                <img src="https://phangchunhoe.github.io/smart_recycling_bin/images/ntuc_logo_noBackground.png"
                                    class="w-24 object-contain"
                                    alt="FairPrice Logo">

                                <img src="https://phangchunhoe.github.io/smart_recycling_bin/images/ntuc_qr.svg"
                                    class="w-24 h-24 rounded-xl bg-white p-2"
                                    alt="Voucher QR Code">

                                <p class="text-lg font-bold text-gray-800">
                                    $${voucherValue} FairPrice Voucher
                                </p>

                            </div>
                        </div>
                    </div>
                </div>

                <button onclick="location.reload()"
                    class="glass-option px-6 py-3 text-white bg-green-500 hover:bg-green-600 font-semibold rounded-xl">
                    Back to Bin Dashboard
                </button>
            </div>
        </div>
    `;

    document.getElementById("voucherInner").onclick = () => {
        document.getElementById("voucherInner").classList.toggle("flipped");
    };

    shootConfetti();
}


function shootConfetti(duration = 5000) {
    const canvas = document.createElement("canvas");
    canvas.style.position = "fixed";
    canvas.style.inset = "0";
    canvas.style.pointerEvents = "none";
    canvas.style.zIndex = "9999";
    canvas.style.transition = "opacity 1s ease";
    canvas.style.opacity = "1";
    document.body.appendChild(canvas);

    const ctx = canvas.getContext("2d");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    const pieces = [];
    const colors = ["#22c55e", "#16a34a", "#4ade80", "#bbf7d0", "#ffffff"];

    for (let i = 0; i < 180; i++) {
        pieces.push({
            x: Math.random() * canvas.width,
            y: Math.random() * canvas.height - canvas.height,
            size: Math.random() * 6 + 4,
            speed: Math.random() * 4 + 2,
            rotation: Math.random() * 360,
            color: colors[Math.floor(Math.random() * colors.length)]
        });
    }

    const start = performance.now();

    function animate(now) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        pieces.forEach(p => {
            p.y += p.speed;
            p.rotation += 4;

            ctx.save();
            ctx.translate(p.x, p.y);
            ctx.rotate(p.rotation * Math.PI / 180);
            ctx.fillStyle = p.color;
            ctx.fillRect(-p.size / 2, -p.size / 2, p.size, p.size);
            ctx.restore();
        });

        if (now - start < duration) {
            requestAnimationFrame(animate);
        } else {
            // 🌫 Fade out smoothly
            canvas.style.opacity = "0";

            setTimeout(() => {
                canvas.remove();
            }, 1000);
        }
    }

    requestAnimationFrame(animate);
}

/* ===============================
Small Animation (Tailwind helper)
================================ */
const style = document.createElement("style");
style.textContent = `
@keyframes scaleIn {
    from { transform: scale(0.9); opacity: 0 }
    to { transform: scale(1); opacity: 1 }
}
.animate-scaleIn {
    animation: scaleIn 0.25s ease-out;
}
`;
document.head.appendChild(style);
