🔧 PROJECT OVERVIEW
🧪 Features:
DHT11 (temperature & humidity)

MQ2 gas sensor (LPG, CO, Smoke)

Flood detection using a single probe (detects GND)

OLED display (0.96" I2C SSD1306)

HC-05 Bluetooth module (for data to phone via serial)

Buzzer + LED strip for flood or gas alerts

Power via USB or 5V battery pack

🪛 HARDWARE COMPONENTS
Component	Pin Count	Notes
Arduino Uno/Nano	14+	Main controller
DHT11	3	VCC, GND, DATA
MQ2	4	AOUT or DOUT
OLED (SSD1306 I2C)	4	VCC, GND, SDA, SCL
HC-05 Bluetooth	4	VCC, GND, TX, RX
Flood Probe	2	One goes to A1
LED strip	2	VCC, GND
Buzzer	2	Digital output pin

🔌 FULL CONNECTIONS
Component	Connect To Arduino	Notes
DHT11	Data → D2	Use 10kΩ resistor between VCC & Data if raw sensor
VCC → 5V	
GND → GND	
MQ2 (AOUT)	A0	Use analog output
VCC → 5V	
GND → GND	
Flood Probe	One wire → A1	Other wire to GND
OLED (I2C)	SDA → A4	On Uno/Nano
SCL → A5	
VCC → 3.3V or 5V	Depending on your OLED
GND → GND	
HC-05	TX → Arduino RX (D0)🟥	Use voltage divider for RX
RX → Arduino TX (D1)🟥	
VCC → 5V	
GND → GND	
Buzzer	+ve → D6	GND to GND
LED Strip	+ve → D7 (or MOSFET if long)	GND to GND
Power	USB or 5V battery	Can use power bank via USB

🔋 Voltage Divider for HC-05 RX pin:
Arduino TX → HC-05 RX needs 3.3V, so use resistors:

Arduino TX → 1kΩ resistor → junction → HC-05 RX

Junction → 2kΩ resistor → GND

✅ CODE INTEGRATION (Brief Structure)
Collect sensor data from DHT11, MQ2, Flood probe

Show on OLED using Adafruit_SSD1306

Send via Serial.println() for HC-05 to read on mobile

Activate buzzer & LED strip when gas/flood exceeds limit

📱 App to Receive Data on Phone
Use Serial Bluetooth Terminal (Android app)

Connect to HC-05 → view live values (flood, gas, temp)

