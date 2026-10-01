#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <DHT.h>
#include <ArduinoJson.h>

// WiFi Access Point credentials
const char* ssid = "FarmX_AP";
const char* password = "farmx123";

// DHT11 Sensor setup
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Sensor setup
#define SOIL_MOISTURE_PIN 34

// Motor control pins
#define MOTOR_IN1 14
#define MOTOR_IN2 27
#define MOTOR_IN3 26
#define MOTOR_IN4 25
const unsigned long MOTOR_PULSE_DURATION_MS = 300;

// Web server and WebSocket server
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Language definitions
enum Language {
    ENGLISH = 0,
    HINDI = 1,
    TAMIL = 2,
    TELUGU = 3,
    MARATHI = 4,
    BENGALI = 5,
    GUJARATI = 6
};

// Current selected language
Language currentLanguage = ENGLISH;

// Language names in their native scripts
const char* languageNames[] PROGMEM = {
    "English",
    "हिंदी",
    "தமிழ்",
    "తెలుగు", 
    "मराठी",
    "বাংলা",
    "ગુજરાતી"
};

// Multi-language strings stored in PROGMEM
const char* welcomeMessages[] PROGMEM = {
    "Welcome to FarmX Smart Agriculture System",
    "FarmX स्मार्ट कृषि प्रणाली में आपका स्वागत है",
    "FarmX ஸ்மார்ட் விவசாய அமைப்பில் உங்களை வரவேற்கிறோம்",
    "FarmX స్మార్ట్ వ్యవసాయ వ్యవస్థకు మిమ్మల్ని స్వాగతం",
    "FarmX स्मार्ट शेती प्रणालीमध्ये आपले स्वागत आहे",
    "FarmX স্মার্ট কৃষি সিস্টেমে আপনাকে স্বাগতম",
    "FarmX સ્માર્ટ કૃષિ સિસ્ટમમાં તમારું સ્વાગત છે"
};

const char* temperatureLabels[] PROGMEM = {
    "Temperature",
    "तापमान",
    "வெப்பநிலை",
    "ఉష్ణోగ్రత",
    "तापमान",
    "তাপমাত্রা",
    "તાપમાન"
};

const char* humidityLabels[] PROGMEM = {
    "Humidity",
    "आर्द्रता",
    "ஈரப்பதம்",
    "తేమ",
    "आर्द्रता",
    "আর্দ্রতা",
    "ભેજ"
};

const char* soilMoistureLabels[] PROGMEM = {
    "Soil Moisture",
    "मिट्टी की नमी",
    "மண் ஈரப்பதம்",
    "నేల తేమ",
    "मातीची ओलावा",
    "মাটির আর্দ্রতা",
    "માટીની ભેજ"
};

const char* nitrogenLabels[] PROGMEM = {
    "Nitrogen (N)",
    "नाइट्रोजन (N)",
    "நைட்ரஜன் (N)",
    "నైట్రోజన్ (N)",
    "नायट्रोजन (N)",
    "নাইট্রোজেন (N)",
    "નાઈટ્રોજન (N)"
};

const char* phosphorusLabels[] PROGMEM = {
    "Phosphorus (P)",
    "फास्फोरस (P)",
    "பாஸ்பரஸ் (P)",
    "ఫాస్ఫరస్ (P)",
    "फॉस्फरस (P)",
    "ফসফরাস (P)",
    "ફોસ્ફરસ (P)"
};

const char* potassiumLabels[] PROGMEM = {
    "Potassium (K)",
    "पोटैशियम (K)",
    "பொட்டாசியம் (K)",
    "పొటాషియం (K)",
    "पोटॅशियम (K)",
    "পটাসিয়াম (K)",
    "પોટેશિયમ (K)"
};

// NPK Advice messages for different ranges
const char* npkAdviceLow[][7] PROGMEM = {
    // Low Nitrogen
    {
        "Low nitrogen detected. Consider adding organic compost or urea fertilizer.",
        "कम नाइट्रोजन का स्तर। जैविक खाद या यूरिया उर्वरक डालें।",
        "குறைந்த நைட்ரஜன் கண்டறியப்பட்டது. கரிம உரம் அல்லது யூரியா உரம் சேர்க்கவும்।",
        "తక్కువ నైట్రోజన్ లెవల్. సేంద్రిය ఎరువులు లేదా యూరియా ఎరువు వేయండి।",
        "कमी नायट्रोजन पातळी. सेंद्रिय खत किंवा युरिया खत टाका।",
        "কম নাইট্রোজেনের মাত্রা। জৈব সার বা ইউরিয়া সার দিন।",
        "ઓછા નાઈટ્રોજનનું સ્તર. કાર્બનિક ખાતર અથવા યુરિયા ખાતર નાખો।"
    },
    // Low Phosphorus
    {
        "Low phosphorus levels. Add bone meal or DAP fertilizer for better root development.",
        "कम फास्फोरस का स्तर। जड़ों के विकास के लिए हड्डी का चूर्ण या डीएपी खाद डालें।",
        "குறைந்த பாஸ்பரஸ் அளவு. வேர் வளர்ச்சிக்காக எலும்பு மாவு அல்லது டிஏபி உரம் சேர்க்கவும்।",
        "తక్కువ ఫాస్ఫరస్ స్థాయిలు. వేర్ల అభివృద్ధికి ఎముక పొడి లేదా డిఏపి ఎరువు వేయండి।",
        "कमी फॉस्फरसची पातळी. मुळांच्या विकासासाठी हाडांचे पूड किंवा डीएपी खत टाका।",
        "কম ফসফরাসের মাত্রা। শিকড়ের বিকাশের জন্য হাড়ের গুঁড়া বা ডিএপি সার দিন।",
        "ઓછા ફોસ્ફરસનું સ્તર. મૂળના વિકાસ માટે હાડકાનો પાવડર અથવા ડીએપી ખાતર નાખો।"
    },
    // Low Potassium
    {
        "Low potassium detected. Apply potash fertilizer or wood ash to improve fruit quality.",
        "कम पोटैशियम का स्तर। फलों की गुणवत्ता के लिए पोटाश खाद या लकड़ी की राख डालें।",
        "குறைந்த பொட்டாசியம் கண்டறியப்பட்டது. பழங்களின் தரத்திற்காக பொட்டாஷ் உரம் அல்லது மரச்சாம்பல் சேர்க்கவும்।",
        "తక్కువ పొటాషియం లెవల్. పండ్ల నాణ్యత కోసం పొటాష్ ఎరువు లేదా కలప బూడిద వేయండి।",
        "कमी पोटॅशियमची पातळी. फळांच्या गुणवत्तेसाठी पोटाश खत किंवा लाकडाची राख टाका।",
        "কম পটাসিয়ামের মাত্রা। ফলের মানের জন্য পটাশ সার বা কাঠের ছাই দিন।",
        "ઓછા પોટેશિયમનું સ્તર. ફળોની ગુણવત્તા માટે પોટાશ ખાતર અથવા લાકડાની રાખ નાખો।"
    }
};

const char* npkAdviceHigh[][7] PROGMEM = {
    // High Nitrogen
    {
        "High nitrogen levels. Reduce nitrogen fertilizers and increase potassium for balanced growth.",
        "अधिक नाइट्रोजन का स्तर। संतुलित विकास के लिए नाइट्रोजन कम करें और पोटैशियम बढ़ाएं।",
        "அதிக நைட்ரஜன் அளவு. சமச்சீர் வளர்ச்சிக்காக நைட்ரஜனைக் குறைத்து பொட்டாசியத்தை அதிகரிக்கவும்।",
        "అధిక నైట్రోజన్ స్థాయిలు. సమతుల్య వృద్ధికి నైట్రోజన్ తగ్గించి పొటాషియం పెంచండి।",
        "जास्त नायट्रोजनची पातळी. संतुलित वाढीसाठी नायट्रोजन कमी करा आणि पोटॅशियम वाढवा।",
        "উচ্চ নাইট্রোজেনের মাত্রা। সুষম বৃদ্ধির জন্য নাইট্রোজেন কমান এবং পটাসিয়াম বাড়ান।",
        "વધારે નાઈટ્રોજનનું સ્તર. સંતુલિત વૃદ્ધિ માટે નાઈટ્રોજન ઘટાડો અને પોટેશિયમ વધારો।"
    },
    // High Phosphorus
    {
        "High phosphorus detected. Avoid phosphate fertilizers and focus on organic matter.",
        "अधिक फास्फोरस का स्तर। फॉस्फेट खाद से बचें और जैविक तत्वों पर ध्यान दें।",
        "அதிக பாஸ்பரஸ் கண்டறியப்பட்டது. பாஸ்பேட் உரங்களைத் தவிர்த்து கரிமப் பொருட்களில் கவனம் செலுத்துங்கள்।",
        "అధిక ఫాస్ఫరస్ లెవల్. ఫాస్ఫేట్ ఎరువులను తగ్గించి సేంద్రిய పదార్థాలపై దృష్టి పెట్టండి।",
        "जास्त फॉस्फरसची पातळी. फॉस्फेट खत टाळा आणि सेंद्रिय घटकांवर लक्ष द्या।",
        "উচ্চ ফসফরাসের মাত্রা। ফসফেট সার এড়িয়ে চলুন এবং জৈব পদার্থের দিকে মনোযোগ দিন।",
        "વધારે ફોસ્ફરસનું સ્તર. ફોસ્ફેટ ખાતર ટાળો અને કાર્બનિક પદાર્થો પર ધ્યાન આપો।"
    },
    // High Potassium
    {
        "High potassium levels. Reduce potash application and ensure proper calcium balance.",
        "अधिक पोटैशियम का स्तर। पोटाश का उपयोग कम करें और कैल्शियम संतुलन बनाए रखें।",
        "அதிக பொட்டாசியம் அளவு. பொட்டாஷ் பயன்பாட்டைக் குறைத்து சரியான கால்சியம் சமநிலையை உறுதிசெய்யுங்கள்।",
        "అధిక పొటాషియం స్థాయిలు. పొటాష్ వాడకం తగ్గించి కాల్షియం సమతుల్యత కాపాడండి।",
        "जास्त पोटॅशियमची पातळी. पोटाश वापर कमी करा आणि योग्य कॅल्शियम संतुलन राखा।",
        "উচ্চ পটাসিয়ামের মাত্রা। পটাশের ব্যবহার কমান এবং সঠিক ক্যালসিয়াম ভারসাম্য বজায় রাখুন।",
        "વધારે પોટેશિયમનું સ્તર. પોટાશનો ઉપયોગ ઘટાડો અને યોગ્ય કેલ્શિયમ સંતુલન જાળવો।"
    }
};

// General farming advice based on seasons and regions
const char* seasonalAdvice[][7] PROGMEM = {
    {
        "Monitor soil pH regularly. Ideal range is 6.0-7.5 for most crops.",
        "मिट्टी का पीएच नियमित रूप से जांचें। अधिकांश फसलों के लिए 6.0-7.5 आदर्श है।",
        "மண்ணின் பிஹெச் அளவை தொடர்ந்து கண்காணிக்கவும். பெரும்பாலான பயிர்களுக்கு 6.0-7.5 சிறந்தது।",
        "మట్టి పిహెచ్ను క్రమం తప్పకుండా పరిశీలించండి। చాలా పంటలకు 6.0-7.5 అనువైనది।",
        "मातीचा पीएच नियमितपणे तपासा. बहुतेक पिकांसाठी 6.0-7.5 योग्य आहे।",
        "মাটির পিএইচ নিয়মিত পরীক্ষা করুন। বেশিরভাগ ফসলের জন্য 6.0-7.5 আদর্শ।",
        "માટીનો પિએચ નિયમિત તપાસો. મોટાભાગના પાકો માટે 6.0-7.5 આદર્શ છે।"
    }
};

// NPK Sensor Simulation Class
class NPKSensor {
private:
    float nitrogen;
    float phosphorus;
    float potassium;
    unsigned long lastUpdate;
    const unsigned long updateInterval = 5000; // Update every 5 seconds
    
public:
    NPKSensor() {
        nitrogen = 0;
        phosphorus = 0;
        potassium = 0;
        lastUpdate = 0;
        randomSeed(analogRead(0));
    }
    
    void update() {
        if (millis() - lastUpdate >= updateInterval) {
            // Simulate realistic NPK values for Indian soil conditions
            // Nitrogen: 150-300 kg/ha (low: <150, high: >300)
            nitrogen = random(100, 400) / 10.0; // 10.0 to 40.0
            
            // Phosphorus: 10-25 kg/ha (low: <10, high: >25)
            phosphorus = random(5, 35) / 10.0; // 0.5 to 3.5
            
            // Potassium: 100-200 kg/ha (low: <100, high: >200)
            potassium = random(80, 250) / 10.0; // 8.0 to 25.0
            
            lastUpdate = millis();
        }
    }
    
    float getNitrogen() { return nitrogen; }
    float getPhosphorus() { return phosphorus; }
    float getPotassium() { return potassium; }
    
    // Get NPK status (0=low, 1=normal, 2=high)
    int getNitrogenStatus() {
        if (nitrogen < 15.0) return 0;
        if (nitrogen > 30.0) return 2;
        return 1;
    }
    
    int getPhosphorusStatus() {
        if (phosphorus < 1.0) return 0;
        if (phosphorus > 2.5) return 2;
        return 1;
    }
    
    int getPotassiumStatus() {
        if (potassium < 10.0) return 0;
        if (potassium > 20.0) return 2;
        return 1;
    }
};

// Language Manager Class
class LanguageManager {
public:
    static String getWelcomeMessage(Language lang) {
        return String(welcomeMessages[lang]);
    }
    
    static String getTemperatureLabel(Language lang) {
        return String(temperatureLabels[lang]);
    }
    
    static String getHumidityLabel(Language lang) {
        return String(humidityLabels[lang]);
    }
    
    static String getSoilMoistureLabel(Language lang) {
        return String(soilMoistureLabels[lang]);
    }
    
    static String getNitrogenLabel(Language lang) {
        return String(nitrogenLabels[lang]);
    }
    
    static String getPhosphorusLabel(Language lang) {
        return String(phosphorusLabels[lang]);
    }
    
    static String getPotassiumLabel(Language lang) {
        return String(potassiumLabels[lang]);
    }
    
    static String getLanguageName(Language lang) {
        return String(languageNames[lang]);
    }
    
    static String getNPKAdvice(int nutrient, int status, Language lang) {
        if (status == 0) { // Low
            return String(npkAdviceLow[nutrient][lang]);
        } else if (status == 2) { // High
            return String(npkAdviceHigh[nutrient][lang]);
        }
        return "";
    }
    
    static String getSeasonalAdvice(Language lang) {
        return String(seasonalAdvice[0][lang]);
    }
};

// Global instances
NPKSensor npkSensor;

// Enhanced HTML with language support and NPK display
const char* HTML_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>FarmX Multi-Language Smart Agriculture</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: linear-gradient(135deg, #74b9ff, #0dff97); }
        .container { max-width: 1000px; margin: 0 auto; background: white; padding: 20px; border-radius: 15px; box-shadow: 0 8px 32px rgba(0,0,0,0.1); }
        .header { text-align: center; margin-bottom: 30px; }
        .language-selector { display: flex; flex-wrap: wrap; gap: 10px; justify-content: center; margin: 20px 0; }
        .lang-btn { padding: 10px 15px; border: 2px solid #007bff; background: white; color: #007bff; border-radius: 25px; cursor: pointer; transition: all 0.3s; }
        .lang-btn.active { background: #007bff; color: white; }
        .lang-btn:hover { transform: translateY(-2px); box-shadow: 0 4px 8px rgba(0,123,255,0.3); }
        .status { padding: 15px; margin: 10px 0; border-radius: 10px; text-align: center; font-weight: bold; }
        .connected { background: #4CAF50; color: white; }
        .disconnected { background: #f44336; color: white; }
        .connecting { background: #ff9800; color: white; }
        .sensor-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 20px 0; }
        .sensor-card { background: #f8f9fa; padding: 20px; border-radius: 12px; border-left: 5px solid #007bff; }
        .sensor-card.npk { border-left-color: #28a745; }
        .sensor-value { font-size: 24px; font-weight: bold; color: #333; margin: 10px 0; }
        .controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin: 20px 0; }
        .btn { padding: 15px; border: none; border-radius: 8px; font-size: 16px; cursor: pointer; font-weight: bold; transition: all 0.3s; }
        .btn:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.2); }
        .btn-forward { background: #4CAF50; color: white; grid-column: 2; }
        .btn-left { background: #2196F3; color: white; }
        .btn-right { background: #2196F3; color: white; }
        .btn-stop { background: #f44336; color: white; grid-column: 2; }
        .btn-backward { background: #ff9800; color: white; grid-column: 2; }
        .advice-section { background: #e8f5e8; padding: 20px; border-radius: 12px; margin: 20px 0; border-left: 5px solid #28a745; }
        .advice-title { font-size: 18px; font-weight: bold; color: #155724; margin-bottom: 15px; }
        .advice-item { background: white; padding: 15px; margin: 10px 0; border-radius: 8px; border-left: 3px solid #28a745; }
        .debug { background: #f5f5f5; padding: 15px; border-radius: 8px; margin: 20px 0; font-family: monospace; font-size: 12px; }
        .npk-indicators { display: flex; gap: 15px; margin: 15px 0; }
        .npk-indicator { flex: 1; text-align: center; padding: 10px; border-radius: 8px; }
        .npk-low { background: #ffebee; color: #c62828; }
        .npk-normal { background: #e8f5e8; color: #2e7d32; }
        .npk-high { background: #fff3e0; color: #f57c00; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1 id="welcomeMsg">🚜 FarmX Multi-Language Smart Agriculture</h1>
            <div class="language-selector">
                <button class="lang-btn active" onclick="changeLanguage(0)">English</button>
                <button class="lang-btn" onclick="changeLanguage(1)">हिंदी</button>
                <button class="lang-btn" onclick="changeLanguage(2)">தமிழ்</button>
                <button class="lang-btn" onclick="changeLanguage(3)">తెలుగు</button>
                <button class="lang-btn" onclick="changeLanguage(4)">मराठी</button>
                <button class="lang-btn" onclick="changeLanguage(5)">বাংলা</button>
                <button class="lang-btn" onclick="changeLanguage(6)">ગુજરાતી</button>
            </div>
        </div>
        
        <div id="status" class="status disconnected">Initializing...</div>
        
        <div class="sensor-grid">
            <div class="sensor-card">
                <h3>🌡️ <span id="tempLabel">Temperature</span></h3>
                <div class="sensor-value" id="temp">--°C</div>
                <p><strong>🌡️ <span id="humidityLabel">Humidity</span>:</strong> <span id="humidity">--%</span></p>
            </div>
            
            <div class="sensor-card">
                <h3>💧 <span id="moistureLabel">Soil Moisture</span></h3>
                <div class="sensor-value" id="moisture">--</div>
            </div>
            
            <div class="sensor-card npk">
                <h3>🧪 NPK Sensor Data</h3>
                <div class="npk-indicators">
                    <div class="npk-indicator" id="nIndicator">
                        <div><span id="nitrogenLabel">N</span></div>
                        <div id="nitrogen">--</div>
                    </div>
                    <div class="npk-indicator" id="pIndicator">
                        <div><span id="phosphorusLabel">P</span></div>
                        <div id="phosphorus">--</div>
                    </div>
                    <div class="npk-indicator" id="kIndicator">
                        <div><span id="potassiumLabel">K</span></div>
                        <div id="potassium">--</div>
                    </div>
                </div>
                <p><strong>Last Update:</strong> <span id="lastUpdate">Never</span></p>
            </div>
        </div>
        
        <div class="advice-section">
            <div class="advice-title">🌱 Agricultural Advice</div>
            <div id="adviceContainer"></div>
        </div>
        
        <div class="controls">
            <button class="btn btn-left" onclick="sendCommand('L')">LEFT</button>
            <button class="btn btn-forward" onclick="sendCommand('F')">FORWARD</button>
            <button class="btn btn-right" onclick="sendCommand('R')">RIGHT</button>
            <button class="btn btn-stop" onclick="sendCommand('S')">STOP</button>
            <button class="btn btn-backward" onclick="sendCommand('B')">BACKWARD</button>
        </div>
        
        <div class="debug">
            <h4>🐛 Debug Log</h4>
            <div id="debugLog" style="height: 200px; overflow-y: scroll; background: white; padding: 10px; border: 1px solid #ddd;"></div>
        </div>
    </div>

    <script>
        let socket = null;
        let reconnectAttempts = 0;
        const maxReconnects = 5;
        let currentLang = 0;
        
        function log(message) {
            const now = new Date().toLocaleTimeString();
            const logDiv = document.getElementById('debugLog');
            logDiv.innerHTML += [${now}] ${message}<br>;
            logDiv.scrollTop = logDiv.scrollHeight;
            console.log(message);
        }
        
        function updateStatus(status, className) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = status;
            statusDiv.className = status ${className};
        }
        
        function changeLanguage(langId) {
            currentLang = langId;
            sendCommand(LANG:${langId});
            
            // Update active language button
            document.querySelectorAll('.lang-btn').forEach((btn, index) => {
                btn.classList.toggle('active', index === langId);
            });
            
            log(Language changed to: ${langId});
        }
        
        function connectWebSocket() {
            const host = window.location.hostname;
            const port = 81;
            const wsUrl = ws://${host}:${port}/;
            
            log(🔌 Attempting connection to: ${wsUrl});
            updateStatus('Connecting...', 'connecting');
            
            try {
                socket = new WebSocket(wsUrl);
                
                socket.onopen = function(event) {
                    log('✅ WebSocket connected successfully!');
                    updateStatus('Connected to FarmX', 'connected');
                    reconnectAttempts = 0;
                    sendCommand('LANG:0'); // Request English by default
                };
                
                socket.onclose = function(event) {
                    log(❌ WebSocket closed. Code: ${event.code});
                    updateStatus('Disconnected', 'disconnected');
                    
                    if (reconnectAttempts < maxReconnects) {
                        reconnectAttempts++;
                        log(🔄 Reconnecting... (${reconnectAttempts}/${maxReconnects}));
                        setTimeout(connectWebSocket, 3000);
                    }
                };
                
                socket.onerror = function(error) {
                    log(💥 WebSocket error: ${error.message || 'Unknown error'});
                    updateStatus('Connection Error', 'disconnected');
                };
                
                socket.onmessage = function(event) {
                    try {
                        log(📨 Received: ${event.data});
                        const data = JSON.parse(event.data);
                        updateSensorData(data);
                    } catch (e) {
                        log(❌ Failed to parse message: ${e.message});
                    }
                };
                
            } catch (e) {
                log(💥 Failed to create WebSocket: ${e.message});
                updateStatus('Failed to Create Connection', 'disconnected');
            }
        }
        
        function updateSensorData(data) {
            if (data.temperature !== undefined) {
                document.getElementById('temp').textContent = ${data.temperature.toFixed(1)}°C;
            }
            if (data.humidity !== undefined) {
                document.getElementById('humidity').textContent = ${data.humidity.toFixed(1)}%;
            }
            if (data.moisture !== undefined) {
                document.getElementById('moisture').textContent = data.moisture;
            }
            if (data.nitrogen !== undefined) {
                document.getElementById('nitrogen').textContent = data.nitrogen.toFixed(1);
                updateNPKIndicator('nIndicator', data.nitrogenStatus);
            }
            if (data.phosphorus !== undefined) {
                document.getElementById('phosphorus').textContent = data.phosphorus.toFixed(1);
                updateNPKIndicator('pIndicator', data.phosphorusStatus);
            }
            if (data.potassium !== undefined) {
                document.getElementById('potassium').textContent = data.potassium.toFixed(1);
                updateNPKIndicator('kIndicator', data.potassiumStatus);
            }
            if (data.labels) {
                updateLabels(data.labels);
            }
            if (data.advice) {
                updateAdvice(data.advice);
            }
            document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
        }
        
        function updateNPKIndicator(elementId, status) {
            const element = document.getElementById(elementId);
            element.className = 'npk-indicator';
            if (status === 0) element.classList.add('npk-low');
            else if (status === 1) element.classList.add('npk-normal');
            else if (status === 2) element.classList.add('npk-high');
        }
        
        function updateLabels(labels) {
            document.getElementById('tempLabel').textContent = labels.temperature;
            document.getElementById('humidityLabel').textContent = labels.humidity;
            document.getElementById('moistureLabel').textContent = labels.moisture;
            document.getElementById('nitrogenLabel').textContent = labels.nitrogen;
            document.getElementById('phosphorusLabel').textContent = labels.phosphorus;
            document.getElementById('potassiumLabel').textContent = labels.potassium;
            document.getElementById('welcomeMsg').textContent = labels.welcome;
        }
        
        function updateAdvice(advice) {
            const container = document.getElementById('adviceContainer');
            container.innerHTML = '';
            advice.forEach(item => {
                const div = document.createElement('div');
                div.className = 'advice-item';
                div.textContent = item;
                container.appendChild(div);
            });
        }
        
        function sendCommand(cmd) {
            if (socket && socket.readyState === WebSocket.OPEN) {
                const message = JSON.stringify({command: cmd});
                socket.send(message);
                log(📤 Sent command: ${cmd});
            } else {
                log(❌ Cannot send command '${cmd}' - WebSocket not connected);
            }
        }
        
        // Initialize connection when page loads
        window.addEventListener('load', function() {
            log('🚀 Page loaded, initializing WebSocket connection...');
            setTimeout(connectWebSocket, 1000);
        });
        
        // Keyboard controls
        document.addEventListener('keydown', function(e) {
            if (e.key.toLowerCase() === 'f') sendCommand('F');
            if (e.key.toLowerCase() === 'b') sendCommand('B');
            if (e.key.toLowerCase() === 'l') sendCommand('L');
            if (e.key.toLowerCase() === 'r') sendCommand('R');
            if (e.key.toLowerCase() === 's') sendCommand('S');
        });
    </script>
</body>
</html>
)rawliteral";

// Function prototypes
void sendSensorData();
void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void controlMotor(char command);
void stopMotors();
void printSystemInfo();
String generateAdvice();

unsigned long motorStopDeadlineMs = 0;
bool motorPulseActive = false;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=====================================");
    Serial.println("🚜 FarmX Multi-Language System Starting...");
    Serial.println("=====================================");
    
    // Print system information
    printSystemInfo();
    
    // Initialize DHT sensor
    dht.begin();
    Serial.println("✅ DHT sensor initialized");
    
    // Initialize NPK sensor simulation
    npkSensor.update();
    Serial.println("✅ NPK sensor simulation initialized");
    
    // Initialize motor pins
    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(MOTOR_IN3, OUTPUT);
    pinMode(MOTOR_IN4, OUTPUT);
    
    // Ensure all motors are off
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, LOW);
    Serial.println("✅ Motor pins initialized");
    
    // Start WiFi Access Point
    Serial.println("📡 Starting WiFi Access Point...");
    bool apResult = WiFi.softAP(ssid, password);
    
    if (apResult) {
        Serial.println("✅ WiFi AP started successfully!");
    } else {
        Serial.println("❌ Failed to start WiFi AP!");
        return;
    }
    
    delay(2000);
    
    IPAddress apIP = WiFi.softAPIP();
    Serial.println("📍 Access Point Details:");
    Serial.println("   SSID: " + String(ssid));
    Serial.println("   Password: " + String(password));
    Serial.println("   IP Address: " + apIP.toString());
    
    // Start HTTP server
    Serial.println("🌐 Starting HTTP server...");
    server.on("/", HTTP_GET, []() {
        Serial.println("📄 Web page requested from: " + server.client().remoteIP().toString());
        server.send(200, "text/html", HTML_CONTENT);
    });
    
    server.onNotFound([]() {
        Serial.println("❌ 404 Not Found: " + server.uri());
        server.send(404, "text/plain", "File not found");
    });
    
    server.begin();
    Serial.println("✅ HTTP server started on port 80");
    
    // Start WebSocket server
    Serial.println("🔌 Starting WebSocket server...");
    webSocket.begin();
    webSocket.onEvent(handleWebSocketMessage);
    Serial.println("✅ WebSocket server started on port 81");
    
    Serial.println("=====================================");
    Serial.println("🎉 FarmX Multi-Language System Ready!");
    Serial.println("📱 Connect to WiFi: " + String(ssid));
    Serial.println("🌐 Open browser: http://" + apIP.toString());
    Serial.println("=====================================");
}

void loop() {
    // Handle web server
    server.handleClient();
    
    // Handle WebSocket
    webSocket.loop();

    if (motorPulseActive && (long)(millis() - motorStopDeadlineMs) >= 0) {
        stopMotors();
        motorPulseActive = false;
    }
    
    // Update NPK sensor simulation
    npkSensor.update();
    
    // Send sensor data every 3 seconds
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate >= 3000) {
        sendSensorData();
        lastSensorUpdate = millis();
    }
}

void sendSensorData() {
    // Read DHT sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    // Read soil moisture (simulate if sensor not connected)
    int moistureRaw = analogRead(SOIL_MOISTURE_PIN);
    int moisturePercent = map(moistureRaw, 0, 4095, 100, 0); // Convert to percentage
    
    // Check for DHT sensor errors
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("❌ Failed to read from DHT sensor!");
        humidity = 0;
        temperature = 0;
    }
    
    // Create JSON object
    DynamicJsonDocument doc(1024);
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["moisture"] = moisturePercent;
    doc["nitrogen"] = npkSensor.getNitrogen();
    doc["phosphorus"] = npkSensor.getPhosphorus();
    doc["potassium"] = npkSensor.getPotassium();
    doc["nitrogenStatus"] = npkSensor.getNitrogenStatus();
    doc["phosphorusStatus"] = npkSensor.getPhosphorusStatus();
    doc["potassiumStatus"] = npkSensor.getPotassiumStatus();
    
    // Add language-specific labels
    JsonObject labels = doc.createNestedObject("labels");
    labels["welcome"] = LanguageManager::getWelcomeMessage(currentLanguage);
    labels["temperature"] = LanguageManager::getTemperatureLabel(currentLanguage);
    labels["humidity"] = LanguageManager::getHumidityLabel(currentLanguage);
    labels["moisture"] = LanguageManager::getSoilMoistureLabel(currentLanguage);
    labels["nitrogen"] = LanguageManager::getNitrogenLabel(currentLanguage);
    labels["phosphorus"] = LanguageManager::getPhosphorusLabel(currentLanguage);
    labels["potassium"] = LanguageManager::getPotassiumLabel(currentLanguage);
    
    // Generate advice
    JsonArray adviceArray = doc.createNestedArray("advice");
    
    // Add NPK-specific advice
    int nStatus = npkSensor.getNitrogenStatus();
    int pStatus = npkSensor.getPhosphorusStatus();
    int kStatus = npkSensor.getPotassiumStatus();
    
    if (nStatus != 1) {
        adviceArray.add(LanguageManager::getNPKAdvice(0, nStatus, currentLanguage));
    }
    if (pStatus != 1) {
        adviceArray.add(LanguageManager::getNPKAdvice(1, pStatus, currentLanguage));
    }
    if (kStatus != 1) {
        adviceArray.add(LanguageManager::getNPKAdvice(2, kStatus, currentLanguage));
    }
    
    // Add seasonal advice
    adviceArray.add(LanguageManager::getSeasonalAdvice(currentLanguage));
    
    // Send to all connected clients
    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.broadcastTXT(jsonString);
    
    // Debug output
    Serial.println("📊 Sensor Data:");
    Serial.println("   Temperature: " + String(temperature) + "°C");
    Serial.println("   Humidity: " + String(humidity) + "%");
    Serial.println("   Moisture: " + String(moisturePercent) + "%");
    Serial.println("   NPK: N=" + String(npkSensor.getNitrogen()) + 
                   ", P=" + String(npkSensor.getPhosphorus()) + 
                   ", K=" + String(npkSensor.getPotassium()));
    Serial.println("   Language: " + LanguageManager::getLanguageName(currentLanguage));
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("🔌 Client [%u] disconnected\n", num);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("🔌 Client [%u] connected from %s\n", num, ip.toString().c_str());
            // Send welcome data immediately
            sendSensorData();
            break;
        }
        
        case WStype_TEXT: {
            Serial.printf("📨 Client [%u] sent: %s\n", num, payload);
            
            // Parse JSON message
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, payload);
            
            if (error) {
                Serial.println("❌ Failed to parse JSON message");
                return;
            }
            
            String command = doc["command"];
            
            // Handle language change commands
            if (command.startsWith("LANG:")) {
                int langId = command.substring(5).toInt();
                if (langId >= 0 && langId <= 6) {
                    currentLanguage = (Language)langId;
                    Serial.println("🌐 Language changed to: " + LanguageManager::getLanguageName(currentLanguage));
                    // Send updated data with new language
                    sendSensorData();
                }
            }
            // Handle motor commands
            else if (command.length() == 1) {
                controlMotor(command.charAt(0));
            }
            break;
        }
        
        default:
            break;
    }
}

void stopMotors() {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, LOW);
}

void controlMotor(char command) {
    Serial.println("🤖 Motor command: " + String(command));
    
    // Stop any current motion before applying the new command
    stopMotors();
    motorPulseActive = false;
    
    switch (command) {
        case 'F': // Forward
            digitalWrite(MOTOR_IN1, HIGH);
            digitalWrite(MOTOR_IN3, HIGH);
            motorStopDeadlineMs = millis() + MOTOR_PULSE_DURATION_MS;
            motorPulseActive = true;
            Serial.println("⬆️ Moving Forward");
            break;
            
        case 'B': // Backward
            digitalWrite(MOTOR_IN2, HIGH);
            digitalWrite(MOTOR_IN4, HIGH);
            motorStopDeadlineMs = millis() + MOTOR_PULSE_DURATION_MS;
            motorPulseActive = true;
            Serial.println("⬇️ Moving Backward");
            break;
            
        case 'L': // Left
            digitalWrite(MOTOR_IN1, HIGH);
            digitalWrite(MOTOR_IN4, HIGH);
            motorStopDeadlineMs = millis() + MOTOR_PULSE_DURATION_MS;
            motorPulseActive = true;
            Serial.println("⬅️ Turning Left");
            break;
            
        case 'R': // Right
            digitalWrite(MOTOR_IN2, HIGH);
            digitalWrite(MOTOR_IN3, HIGH);
            motorStopDeadlineMs = millis() + MOTOR_PULSE_DURATION_MS;
            motorPulseActive = true;
            Serial.println("➡️ Turning Right");
            break;
            
        case 'S': // Stop
            Serial.println("⏹️ Stopped");
            break;
            
        default:
            Serial.println("❓ Unknown command: " + String(command));
            break;
    }
}

void printSystemInfo() {
    Serial.println("💻 System Information:");
    Serial.println("   Chip Model: " + String(ESP.getChipModel()));
    Serial.println("   Chip Cores: " + String(ESP.getChipCores()));
    Serial.println("   CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("   Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB");
    Serial.println("   Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("   SDK Version: " + String(ESP.getSdkVersion()));
}
