#pragma config(Sensor, port1,  rightSonar,     sensorVexIQ_Distance)
#pragma config(Sensor, port7,  leftSonar,      sensorVexIQ_Distance)
#pragma config(Sensor, port9,  color,          sensorVexIQ_ColorGrayscale)
#pragma config(Motor,  motor6,          leftMotor,     tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor12,         rightMotor,    tmotorVexIQ, PIDControl, reversed, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

bool attending[3][2];

int row = 0;

int studentsPresent = 0;

bool atStart = true;

bool onBlack = false;
bool onWhite = true;

enum State
{
	Scanning,
	DrivingForward,
	TurningAround,
	NudgingLeft,
	NudgingRight
};

struct Side
{
	tMotor m;
};

struct Chassis
{
	Side leftSide;
	Side rightSide;
};

struct Robot
{
	Chassis chassis;
	State state;
};

Robot robot;

bool isAtNextLine()
{
	if(getColorValue(color) < 30)
	{
		onBlack = true;
		onWhite = false;
		return false;
	}
	if(getColorValue(color) > 70)
	{
		if(onBlack)
		{
			return true;
		}
		else
		{
			return false;
		}
		onBlack = false;
		onWhite = true;
	}
	return true;
}

void init()
{
	robot.chassis.leftSide = leftMotor;
	robot.chassis.rightSide = rightMotor;
	robot.state = DrivingForward;
	setColorMode(color, colorTypeGrayscale_Reflected);
}

void setChassisPowers(int leftPower, int rightPower)
{
	motor[robot.chassis.leftSide] = leftPower;
	motor[robot.chassis.rightSide] = rightPower;
}

task main()
{
	init();
	while(true)
	{
		switch(robot.state)
		{
		case Scanning:
			if(SensorValue[rightSonar] < 200)
			{
				attending[row][1] = true;
				studentsPresent++;
			}
			else
			{
				attending[row][1] = false;
			}
			if(SensorValue[leftSonar] < 200)
			{
				attending[row][0] = true;
				studentsPresent++;
			}
			else
			{
				attending[row][0] = false;
			}
			if(row < 2)
			{
				robot.state = DrivingForward;
			}
			else
			{
				robot.state = TurningAround;
			}
			break;
		case DrivingForward:
			if(atStart)
			{
				if(isAtNextLine())
				{
					setChassisPowers(0, 0);
					atStart = false;
				}
				else
				{
					setChassisPowers(75, 75);
				}
			}
			else
			{
				if(isAtNextLine())
				{
					setChassisPowers(0, 0);
					robot.state = Scanning;
					row++;
				}
				else
				{
					setChassisPowers(75, 75);
				}
			}
			break;
		case NudgingLeft:
			break;
		case NudgingRight:
			break;
		}
		displayTextLine(3, "Present: %d", studentsPresent);
	}
}