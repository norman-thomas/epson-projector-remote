const int LED = D7;

void setup()
{
	Serial1.begin(9600, SERIAL_8N1);
	RGB.control(true);
	pinMode(LED, OUTPUT);

	Particle.function("power", power);
	Particle.function("power_on", powerOn);
	Particle.function("power_off", powerOff);
	Particle.function("power_status", powerStatus);
	Particle.function("error", checkError);
	Particle.function("ok", ok);
}

void loop() {}


// **************

enum POWER {
	UNKNOWN = -1,
	OFF = 0,
	ON = 1,
	WARMUP = 2,
	COOLINGDOWN = 3,
	STANDBYNETWORKON = 4,
	ABNORMALSTANDBY = 5
};
// enum SOURCE { MEDIAPLAYER = 30 };
// enum LUMINANCE { NORMAL = 0, ECO = 1 };


POWER getPower(); // necessary for the Particle cloud to be able to compile the following function
POWER getPower()
{
	String result = sendCommand("PWR?");
	if (detectError(result, "projector status check failed"))
		return UNKNOWN;

	if (result.equals("PWR=00"))
		return OFF;
	if (result.equals("PWR=01"))
		return ON;
	if (result.equals("PWR=02"))
		return WARMUP;
	if (result.equals("PWR=03"))
		return COOLINGDOWN;
	if (result.equals("PWR=04"))
		return STANDBYNETWORKON;
	if (result.equals("PWR=05"))
		return ABNORMALSTANDBY;

	return UNKNOWN;
}

int power(String arg)
{
	if (arg.equals("ON"))
		return powerOn(arg);
	if (arg.equals("OFF"))
		return powerOff(arg);
	if (arg.equals("?"))
		return powerStatus(arg);
	return -1;
}

int powerOn(String arg)
{
	const POWER status = getPower();
	if (status == ON || status == WARMUP)
		return 0;

	const String result = sendCommand("PWR ON");
	if (detectError(result, "projector turn on failed"))
		return -1;
	report("PWR ON: " + result);
	const bool cond = result.equals("");
	cond ? report("projector turned on") : report("projector turn on failed");
	return cond ? 0 : -1;
}

int powerOff(String arg)
{
	const POWER status = getPower();
	if (status == OFF || status == STANDBYNETWORKON)
		return 0;

	const String result = sendCommand("PWR OFF");
	if (detectError(result, "projector turn off failed"))
		return -1;
	report("PWR OFF: " + result);
	const bool cond = result.equals("");
	cond ? report("projector turned off") : report("projector turn off failed");
	return cond ? 0 : -1;
}

int powerStatus(String arg)
{
	const POWER status = getPower();
	if (status == UNKNOWN)
		return -1;
	return (status == ON || status == WARMUP) ? 1 : 0;
}

int checkError(String arg)
{
	const String result = sendCommand("ERR?");
	if (result.startsWith("ERR="))
		result.substring(4);
	return result.toInt();
}

int ok(String arg)
{
	return checkError(arg) == 0 ? 0 : -1;
}


bool detectError(String result, String errorMessage)
{
	const bool error = result.equals("ERR");
	if (error)
		report(errorMessage);
	return error;
}

void report(String descr)
{
	Particle.publish(descr);
}

// **************

void write(String cmd)
{
	RGB.color(255, 0, 0);
	digitalWrite(LED, HIGH);
	Serial1.print(cmd);
	Serial1.print("\r");
	delay(100);
	digitalWrite(LED, LOW);
	RGB.color(0, 0, 0);
}

String read()
{
	RGB.color(0, 255, 0);
	digitalWrite(LED, HIGH);
	String result = "";
	while (Serial1.available() > 0)
	{
		result.concat((char) Serial1.read());
	}
	delay(100);
	digitalWrite(LED, LOW);
	RGB.color(0, 0, 0);
	return result;
}

String sendCommand(String cmd)
{
	write(cmd);
	String result = read();
	result.trim();
	if (result.endsWith(":"))
		result.remove(result.length() - 1, 1);
	result.trim();
	return result;
}


