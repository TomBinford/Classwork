#pragma config(Sensor, port4,  gyro,           sensorVexIQ_Gyro)
#pragma config(Sensor, port10, touchLED,       sensorVexIQ_LED)
#pragma config(Motor,  motor6,          leftMotor,     tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor7,          intakeMotor,   tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor12,         rightMotor,    tmotorVexIQ, PIDControl, reversed, encoder)float driftRate = 0;

float driftRate;

float gyroValue = 0;

struct Side
{
	tMotor sideMotor;
};

struct Chassis
{
	Side leftSide;
	Side rightSide;
};

struct Robot
{
	Chassis chassis;
};

Robot robot;

void setChassisPowers(int leftPower, int rightPower)
{
	motor[robot.chassis.leftSide] = leftPower;
	motor[robot.chassis.rightSide] = rightPower;
}

void gyroCorrect()
{
	long currentTime = time1[T1];
	if (currentTime < 5)
	{
		return;
	}
	time1[T1] = 0;
	float currentRate = getGyroRateFloat(gyro) - driftRate;
	gyroValue += currentRate/(1000 / (float)currentTime);
}

void gyroCalibration()
{
	setTouchLEDBlinkTime(touchLED, 20, 20);
	time1[T1] = 0;
	float sum = 0;
	int count = 0;
	while(time1[T1] < 5000)
	{
		if(time1[T1] > 0 && time1[T1] < 1000)
		{
			setTouchLEDColor(touchLED, colorRedViolet);
		}
		else if(time1[T1] > 1000 && time1[T1] < 2000)
		{
			setTouchLEDColor(touchLED, colorRed);
		}
		else if(time1[T1] > 2000 && time1[T1] < 3000)
		{
			setTouchLEDColor(touchLED, colorOrange);
		}
		else if(time1[T1] > 3000 && time1[T1] < 4000)
		{
			setTouchLEDColor(touchLED, colorYellow);
		}
		else if(time1[T1] > 4000 && time1[T1] < 5000)
		{
			setTouchLEDColor(touchLED, colorGreen);
		}
		sum += getGyroRateFloat(gyro);
		count++;
		delay(5);
	}
	setTouchLEDColor(touchLED, colorLimeGreen);
	driftRate = sum / count;
	time1[T1] = 0;
}

bool hasTurnedProportionally(float goalDegrees, float changeDegrees)
{
	if(goalDegrees - 1 > gyroValue)
	{
		setChassisPowers((-(int)(((changeDegrees + gyroValue) / changeDegrees) * 100) - 5), ((int)(((changeDegrees + gyroValue) / changeDegrees) * 100) + 5));
		if(((changeDegrees + gyroValue) / changeDegrees) * 100 > 10)
		{
			return false;
		}
		return true;
	}
	else if(goalDegrees + 1 < gyroValue)
	{
		setChassisPowers(((int)(((changeDegrees - gyroValue) / changeDegrees) * 100) + 5), (-(int)(((changeDegrees - gyroValue) / changeDegrees) * 100) - 5));
		if(changeDegrees - gyroValue / changeDegrees * 100 < -10)
		{
			return false;
		}
		return true;
	}
	return true;
}

void init()
{
	gyroCalibration();
	robot.chassis.leftSide =  leftMotor;
	robot.chassis.rightSide = rightMotor;
	time1[T1] = 0;
}

task main()
{
	while(getTouchLEDValue(touchLED) == 0)
	{
	}
	init();
	while(true)
	{
		gyroCorrect();
		if(hasTurnedProportionally(-90, -90))
		{
			setChassisPowers(0, 0);
			setTouchLEDColor(touchLED, colorBlue);
			displayTextLine(2, "GyroValue: %f", gyroValue);
		}
		displayTextLine(2, "GyroValue: %f", gyroValue);
	}
}
