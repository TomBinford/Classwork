#pragma config(Sensor, port1,  color,          sensorVexIQ_ColorHue)
#pragma config(Sensor, port2,  rightGrayscale, sensorVexIQ_ColorGrayscale)
#pragma config(Sensor, port4,  gyro,           sensorVexIQ_Gyro)
#pragma config(Sensor, port9,  leftGrayscale,  sensorVexIQ_ColorGrayscale)
#pragma config(Sensor, port10, touchLED,       sensorVexIQ_LED)
#pragma config(Motor,  motor6,          leftMotor,     tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor7,          intakeMotor,   tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor12,         rightMotor,    tmotorVexIQ, PIDControl, reversed, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

float driftRate = 0;

float gyroValue = 0;

float tempGyro;

bool blackSeen = false;

enum RingColors
{
	RedOrGreen,
	Blue,
	None
};

enum States
{
	DrivingForward,
	CollectingRing,
	ScanningRing,
	DepositingRing,
	DirectTurn,
	DirectDrive,
	DirectReTurn,
	AlignmentState,
	Idle
};

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
	States state;
};

Robot robot;

void init()
{
	robot.chassis.leftSide =  leftMotor;
	robot.chassis.rightSide = rightMotor;
	robot.state = AlignmentState;
	time1[T1] = 0;
	setColorMode(color, colorTypeRGB_Raw_Reflected);
	setColorMode(leftGrayscale, colorTypeGrayscale_Reflected);
	setColorMode(rightGrayscale, colorTypeGrayscale_Reflected);
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
	if(gyroValue < -360)
	{
		gyroValue += 360;
	}
	else if(gyroValue > 360)
	{
		gyroValue -= 360;
	}
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
		else if(time1[T1] > 2000 && time1[T1] < 3000)
		{
			setTouchLEDColor(touchLED, colorOrange);
		}
		else if(time1[T1] > 4000 && time1[T1] < 5000)
		{
			setTouchLEDColor(touchLED, colorGreen);
		}
		sum += getGyroRateFloat(gyro);
		count++;
		delay(5);
	}
	driftRate = sum / count;
	time1[T1] = 0;
}

int inchesToDegs(float inches)
{
	//2 in ~ 100 degs
	return inches * 34.0;
}

void setChassisPowers(int leftPower, int rightPower)
{
	motor[robot.chassis.leftSide] = leftPower;
	motor[robot.chassis.rightSide] = rightPower;
}

int getRingColor()
{
	int red = getColorRedChannel(color);
	int green = getColorGreenChannel(color);
	int blue = getColorBlueChannel(color);

	if(red > green && red > blue)
	{
		return 0;
	}
	if(green > red && green > blue)
	{
		return 0;
	}
	if(blue > red && blue > green)
	{
		return 1;
	}
	return 0;
}

bool hasPassedBlackLine()
{
	if(getColorGrayscale(leftGrayscale) < 30 || getColorGrayscale(rightGrayscale) < 30)
	{
		blackSeen = true;
	}
	if((getColorGrayscale(leftGrayscale) > 150 || getColorGrayscale(rightGrayscale) > 150) && blackSeen)
	{
		blackSeen = false;
		return true;
	}
	return false;
}

bool hasTurnedProportionally(float goalDegrees, float changeDegrees, int maxPower = 60, int minPower = 13)
{
	if(goalDegrees > 360)
	{
		goalDegrees -= 360;
	}
	if(goalDegrees < -360)
	{
		goalDegrees += 360;
	}
	int power = (abs(gyroValue - goalDegrees) - 10) / abs(changeDegrees) * maxPower + minPower;
	if(goalDegrees - 1 > gyroValue)
	{
		setChassisPowers(-power, power);
		return false;
	}
	else if(goalDegrees + 1 < gyroValue)
	{
		setChassisPowers(power, -power);
		return false;
	}
	return true;
}

int absEncoder(Side m)
{
	return abs(nMotorEncoder[m]);
}
//
//
//
//task main
//
//
//
task main()
{
	bool exitProgram = false;

	bool hasTurnedAround = false;

	int directTurnDegrees;

	RingColors ringColor = None;

	int currentRingSpot = 0;

	bool enteringState = true;

	int alignmentStage = 0;

	while(getTouchLEDValue(touchLED) != 1)
	{
	}
	init();
	gyroCalibration();
	while(true)
	{
		if(ringColor == RedOrGreen)
		{
			displayTextLine(3, "Red/Green");
		}
		if(ringColor == Blue)
		{
			displayTextLine(3, "Blue");
		}
		if(ringColor == None)
		{
			displayTextLine(3, "None");
		}
		displayTextLine(2, "Gyro: %f", gyroValue);
		gyroCorrect();
		switch(robot.state)
		{
		case Idle:
			setChassisPowers(0, 0);
			displayTextLine(3, "State: Idle");
			break;
		case AlignmentState:
			displayTextLine(3, "State: AlignmentState");
			if(enteringState)
			{
				gyroValue = 0;
				enteringState = false;
			}
			if(absEncoder(robot.chassis.leftSide) < 100 && absEncoder(robot.chassis.rightSide) < 100 && alignmentStage == 0)
			{
				setChassisPowers(50, 50);
			}
			else if(alignmentStage == 0)
			{
				alignmentStage++;
				setChassisPowers(0, 0);
				nMotorEncoder[robot.chassis.leftSide] = 0;
				nMotorEncoder[robot.chassis.rightSide] = 0;
			}
			else if(!hasTurnedProportionally(-90, -90, 100) && alignmentStage == 1)
			{
			}
			else if(alignmentStage == 1)
			{
				setChassisPowers(0, 0);
				alignmentStage++;
				nMotorEncoder[robot.chassis.leftSide] = 0;
				nMotorEncoder[robot.chassis.rightSide] = 0;
			}
			else if(absEncoder(robot.chassis.leftSide) < 175 && absEncoder(robot.chassis.rightSide) < 165 && alignmentStage == 2)
			{
				setChassisPowers(50, 50);
			}
			else if(alignmentStage == 2)
			{
				setChassisPowers(0, 0);
				alignmentStage++;
				nMotorEncoder[robot.chassis.leftSide] = 0;
				nMotorEncoder[robot.chassis.rightSide] = 0;
			}
			else if(!hasTurnedProportionally(0, 90) && alignmentStage == 3)
			{
			}
			else if(alignmentStage == 3)
			{
				alignmentStage++;
				setChassisPowers(0, 0);
				robot.state = DrivingForward;
			}
			break;
		case DrivingForward:
			displayTextLine(3, "State: DrivingForward");
			if(!hasPassedBlackLine())
			{
				if(gyroValue > 0.5)
				{
					setChassisPowers(55, 50);
				}
				else if(gyroValue < -0.5)
				{
					setChassisPowers(50, 55);
				}
				else
				{
					setChassisPowers(50, 50);
				}
				//go forward straight
			}
			else
			{
				setChassisPowers(0, 0);
				currentRingSpot++;
				robot.state = CollectingRing;
				if(currentRingSpot == 4)
				{
					exitProgram = true;
				}
			}
			break;
		case CollectingRing:
			if(enteringState)
			{
				nMotorEncoder[intakeMotor]  = 0;
				enteringState = false;
			}
			if(nMotorEncoder[intakeMotor] > -250)
			{
				motor[intakeMotor] = -50;
			}
			else
			{
				motor[intakeMotor] = 0;
				robot.state = ScanningRing;
				enteringState = true;
			}
			break;
		case ScanningRing:
			int temp = getRingColor();
			if(temp == 0)
			{
				ringColor = RedOrGreen;
			}
			else
			{
				ringColor = Blue;
			}
			enteringState = true;
			robot.state = DirectTurn;
			break;
		case DepositingRing:
			displayTextLine(3, "State: DepositingRing");
			if(nMotorEncoder[intakeMotor] < 0)
			{
				motor[intakeMotor] = 50;
			}
			else
			{
				motor[intakeMotor] = 0;
				robot.state = DirectDrive;
				hasTurnedAround = true;
			}
			break;
			//
			//
			//
			//DirectTurn
			//
			//
			//
		case DirectTurn:
			displayTextLine(3, "State: DirectTurn");
			if(enteringState)
			{
				gyroValue = 0;
				enteringState = false;
			}
			if(ringColor == RedOrGreen)
			{
				if(currentRingSpot == 1)
				{
					directTurnDegrees = radiansToDegrees(asin(18 / 19.0));
					if(hasTurnedProportionally(directTurnDegrees, directTurnDegrees))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
				else if(currentRingSpot == 2)
				{
					directTurnDegrees = 180 - radiansToDegrees(asin(18 / 19.0));
					if(hasTurnedProportionally(180 - radiansToDegrees(asin(18 / 19.0)), 180 - radiansToDegrees(asin(18 / 19.0))))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
				else
				{
					directTurnDegrees = 180 - radiansToDegrees(asin(18 / 25.0));
					displayTextLine(1, "%f", 180 - radiansToDegrees(asin(18/25.0)));
					if(hasTurnedProportionally(180 - radiansToDegrees(asin(18 / 25.0)), 180 - radiansToDegrees(asin(18 / 25.0))))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
			}
			else if(ringColor == Blue)
			{
				if(currentRingSpot == 1)
				{
					directTurnDegrees = radiansToDegrees(asin(18 / 28.0));
					if(hasTurnedProportionally(radiansToDegrees(asin(18 / 28.0)), radiansToDegrees(asin(18 / 28.0))))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
				else if(currentRingSpot == 2)
				{
					directTurnDegrees = radiansToDegrees(asin(18 / 19.0));
					if(hasTurnedProportionally(radiansToDegrees(asin(18 / 19.0)), radiansToDegrees(asin(18 / 19.0))))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
				else
				{
					directTurnDegrees = 180 - radiansToDegrees(asin(18 / 19.0));
					if(hasTurnedProportionally(180 - radiansToDegrees(asin(18 / 19.0)), 180 - radiansToDegrees(asin(18 / 19.0))))
					{
						robot.state = DirectDrive;
						setChassisPowers(0, 0);
						enteringState = true;
					}
				}
			}
			break;
			//
			//
			//
			//Direct Drive
			//
			//
			//
		case DirectDrive:
			displayTextLine(3, "State: DirectDrive");
			if(enteringState)
			{
				nMotorEncoder[robot.chassis.leftSide] = 0;
				nMotorEncoder[robot.chassis.rightSide] = 0;
				enteringState = false;
			}
			if(ringColor == RedOrGreen)
			{
				if(currentRingSpot == 1)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(12) && absEncoder(robot.chassis.rightSide) < inchesToDegs(12))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(11) && absEncoder(robot.chassis.rightSide) < inchesToDegs(11))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
				else if(currentRingSpot == 2)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(12) && absEncoder(robot.chassis.rightSide) < inchesToDegs(12))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(11) && absEncoder(robot.chassis.rightSide) < inchesToDegs(11))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
				else if(currentRingSpot == 3)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(25) && absEncoder(robot.chassis.rightSide) < inchesToDegs(25))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(24) && absEncoder(robot.chassis.rightSide) < inchesToDegs(24))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
			}
			else if(ringColor == Blue)
			{
				if(currentRingSpot == 1)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(24) && absEncoder(robot.chassis.rightSide) < inchesToDegs(24))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(21) && absEncoder(robot.chassis.rightSide) < inchesToDegs(21))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
				else if(currentRingSpot == 2)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(19) && absEncoder(robot.chassis.rightSide) < inchesToDegs(19))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(11) && absEncoder(robot.chassis.rightSide) < inchesToDegs(11))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
				else if(currentRingSpot == 3)
				{
					if(hasTurnedAround)
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(25) && absEncoder(robot.chassis.rightSide) < inchesToDegs(25))
						{
							setChassisPowers(-50, -50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DirectReTurn;
							enteringState = true;
							hasTurnedAround = false;
						}
					}
					else
					{
						if(absEncoder(robot.chassis.leftSide) < inchesToDegs(24) && absEncoder(robot.chassis.rightSide) < inchesToDegs(24))
						{
							setChassisPowers(50, 50);
						}
						else
						{
							setChassisPowers(0, 0);
							robot.state = DepositingRing;
							enteringState = true;
						}
					}
				}
			}
			break;
			//
			//
			//
			//Direct Re-Turn
			//
			//
			//
		case DirectReTurn:
			displayTextLine(3, "State: DirectReTurn");
			if(enteringState)
			{
				tempGyro = gyroValue;
				enteringState = false;
			}
			if(hasTurnedProportionally(0, -tempGyro - directTurnDegrees))
			{
				setChassisPowers(0, 0);
				robot.state = DrivingForward;
				enteringState = true;
			}
			break;
		}
		if(exitProgram)
		{
			setChassisPowers(0, 0);
			break;
		}
	}
}
