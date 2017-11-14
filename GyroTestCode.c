float driftRate;

float gyroValue = 0;

void gyroCorrect()
{
	long currentTime = time1[T1];
	if (currentTime < 5)
	{
		return;
	}
	time1[T1] = 0;
	float currentRate = getGyroRateFloat(port4) - driftRate;
	gyroValue += currentRate/(1000 / (float)currentTime);
	if(gyroValue > 360)
	{
		gyroValue -= 360.0;
	}
	else if(gyroValue < -360)
	{
		gyroValue += 360;
	}
}

void gyroCalibration()
{
	time1[T1] = 0;
	float sum = 0;
	int count = 0;
	while(time1[T1] < 5000)
	{
		sum += getGyroRateFloat(port4);
		count++;
		delay(5);
	}
	driftRate = sum / count;
	SensorValue(port4) = 0;
	time1[T1] = 0;
}

task main()
{
	gyroCalibration();
	while(true)
	{
		gyroCorrect();
		displayTextLine(2, "GyroValue: %f", gyroValue);
	}
}
