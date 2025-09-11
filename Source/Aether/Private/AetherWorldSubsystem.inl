/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#define SECONDS_PER_HOUR 3600.0f
#define SECONDS_PER_DAY_EARTH 86400.0f
#define SECONDS_PER_YEAR_EARTH 31536000.0f
#define DAYS_PER_YEAR_EARTH 365

#define OBLIQUITY 23.45f				// 黄赤交角（度）
#define STANDARD_MERIDIAN 120.0f		// 标准时区经度（东八区）
#define DEG_TO_HOUR 15.0f				// 度到小时转换系数（1小时=15度）

#define J2000_JD 2451545
#define DAYS_PER_CENTURY 36525

//#define DEG_TO_RAD (PI / 180.0f)
//#define RAD_TO_DEG (180.0f / PI)

/**
 * 计算指定地理位置和时间的太阳位置（高度角、方位角）及极地条件
 * @param Latitude 纬度（度），北纬为正
 * @param Longitude 经度（度），东经为正
 * @param TimeStampOfEarthDay 当日累计秒数（0~86400）
 * @param TimeStampOfEarthYear 当年累计秒数（0~31536000）
 * @param SunElevation [输出] 太阳高度角（度），地平线以上为正值
 * @param SunAzimuth [输出] 太阳方位角（度），从正北顺时针测量
 * @param PolarCondition [输出] 极地条件标志：0=正常，1=极昼，-1=极夜
 */
inline void CalculateSunPosition(
    const float& Latitude,
    const float& Longitude,
    const float& TimeStampOfEarthDay,
    const float& TimeStampOfEarthYear,
    float& SunElevation,
    float& SunAzimuth,
    int32& PolarCondition)
{
    // ===== 1. 输入预处理 =====
    // 计算年序日（Day of Year, 1-365）
    float DateOfYear = FMath::Fmod(TimeStampOfEarthYear / SECONDS_PER_DAY_EARTH, DAYS_PER_YEAR_EARTH);
    
    // 将秒转换为小时（地方时）
    float LocalHour = TimeStampOfEarthDay / SECONDS_PER_HOUR;
    
    // ===== 2. 天文参数计算 =====
    // 计算太阳赤纬角（δ），单位：弧度（使用标准近似公式）
    float SolarDeclination = FMath::DegreesToRadians(OBLIQUITY * FMath::Sin(2 * PI * (284.0f + DateOfYear) / DAYS_PER_YEAR_EARTH));
    
    // 计算时角（H），单位：弧度
    // a. 经度转换为时区修正（15度=1小时）
    float TimeZoneCorrection = (Longitude - STANDARD_MERIDIAN) / DEG_TO_HOUR;
    // b. 计算真太阳时
    float LocalSolarTime = LocalHour + TimeZoneCorrection;
    // c. 计算时角（度）并转换为弧度
    float HourAngleDeg = DEG_TO_HOUR * (LocalSolarTime - 12.0f);
    float HourAngleRad = FMath::DegreesToRadians(HourAngleDeg);
    
    // 纬度转换为弧度
    float LatRad = FMath::DegreesToRadians(Latitude);
    
    // ===== 3. 太阳高度角计算 =====
    // 使用标准球面三角公式：sin(α) = sin(φ)sin(δ) + cos(φ)cos(δ)cos(H)
    float SinElevation = FMath::Sin(LatRad) * FMath::Sin(SolarDeclination) 
                       + FMath::Cos(LatRad) * FMath::Cos(SolarDeclination) * FMath::Cos(HourAngleRad);
    
    // 处理浮点精度溢出，确保值在[-1, 1]范围内
    SinElevation = FMath::Clamp(SinElevation, -1.0f, 1.0f);
    
    // 计算高度角弧度值（后续计算会复用）
    float ElevationRad = FMath::Asin(SinElevation);
    SunElevation = FMath::RadiansToDegrees(ElevationRad);
    
    // ===== 4. 太阳方位角计算 =====
    // 公式：cos(Az) = [sin(δ) - sin(α)sin(φ)] / [cos(α)cos(φ)]
	// 计算方位角的正弦和余弦分量
	float sinAzimuth = -FMath::Cos(SolarDeclination) * FMath::Sin(HourAngleRad);
	float cosAzimuth = FMath::Sin(SolarDeclination) * FMath::Cos(LatRad) 
					 - FMath::Cos(SolarDeclination) * FMath::Sin(LatRad) * FMath::Cos(HourAngleRad);
	
	// 使用atan2计算方位角（弧度）
	float azimuthRad = FMath::Atan2(sinAzimuth, cosAzimuth);
	
	// 转换为度数并确保在0-360度范围内
	SunAzimuth = FMath::RadiansToDegrees(azimuthRad);
	SunAzimuth = FMath::Fmod(SunAzimuth + 360.0f, 360.0f);
	
    // ===== 5. 极昼/极夜检测 =====
    // 计算临界角余弦值（用于判断太阳是否可能升起）
    float CriticalAngleCos = -FMath::Sin(LatRad) * FMath::Sin(SolarDeclination) / (FMath::Cos(LatRad) * FMath::Cos(SolarDeclination));
    
    // 判断逻辑
    PolarCondition = 0; // 默认正常
    if (CriticalAngleCos > 1.0f)
    {
        PolarCondition = -1; // 极夜（全天无日出）
        SunElevation = -90.0f; // 太阳位于地平线以下
    }
	else if (CriticalAngleCos < -1.0f)
	{
        PolarCondition = 1;  // 极昼（全天有日照）
        SunElevation = 90.0f; // 太阳位于头顶
    }
}

/**
 * 计算指定地理位置和时间的月球位置（高度角、方位角）- 简化版
 * @param Latitude 纬度（度），北纬为正
 * @param Longitude 经度（度），东经为正
 * @param TimeStampOfEarthDay 当日累计秒数（0~86400）
 * @param TimeStampOfEarthYear 当年累计秒数（0~31536000）
 * @param MoonElevation [输出] 月球高度角（度）
 * @param MoonAzimuth [输出] 月球方位角（度），从正北顺时针测量
 */
inline void CalculateMoonPosition(
    const float& Latitude,
    const float& Longitude,
    const float& TimeStampOfEarthDay,
    const float& TimeStampOfEarthYear,
    float& MoonElevation,
    float& MoonAzimuth)
{
	double totalSecondsSinceJ2000Epoch = TimeStampOfEarthYear;
    
	// 将总秒数转换为从J2000起算的天数
	double daysSinceJ2000Epoch = totalSecondsSinceJ2000Epoch / SECONDS_PER_DAY_EARTH;
    
	// 计算当前时刻的儒略日
	// 因为J2000的儒略日是2451545.0（对应当年中午12点），而我们的时间原点是当天0点，所以有0.5天的偏移
	double currentJulianDay = J2000_JD - 0.5 + daysSinceJ2000Epoch;
    
	// ===== 2. 计算儒略世纪数T =====
	// T 是从J2000（2451545.0 JD）起算的儒略世纪数
	double T = (currentJulianDay - J2000_JD) / DAYS_PER_CENTURY;
	
	float daysSince2000 = currentJulianDay - J2000_JD;
    // ===== 1. 时间参数计算 =====
    // 计算当前时间相对于J2000的儒略世纪数
	//float daysSince2000 = TimeStampOfEarthYear / SECONDS_PER_DAY_EARTH + TimeStampOfEarthDay / SECONDS_PER_DAY_EARTH;
    //float T = (daysSince2000) / 36525.0f; // 儒略世纪数
	
    // ===== 2. 计算月球轨道参数（简化平根数） =====
    // 月球平黄经（度）
    float meanLongitude = 218.3164477f + 481267.88123421f * T;
    meanLongitude = FMath::Fmod(meanLongitude, 360.0f);
    if (meanLongitude < 0) meanLongitude += 360.0f;
	
    // 月球平近点角（度）
    float meanAnomaly = 134.9633964f + 477198.8675055f * T;
    meanAnomaly = FMath::Fmod(meanAnomaly, 360.0f);
    if (meanAnomaly < 0) meanAnomaly += 360.0f;
	
    // 升交点平黄经（度）
    float ascendingNode = 125.0445550f - 1934.1361849f * T;
    ascendingNode = FMath::Fmod(ascendingNode, 360.0f);
    if (ascendingNode < 0) ascendingNode += 360.0f;
	
    // ===== 3. 计算月球黄经和黄纬（简化版） =====
    // 只考虑最大的摄动项（约6.3度振幅）
    float longitude = meanLongitude + 6.289f * FMath::Sin(FMath::DegreesToRadians(meanAnomaly));
    longitude = FMath::Fmod(longitude, 360.0f);
    if (longitude < 0) longitude += 360.0f;
	
    // 黄纬（主要受交点影响，约5.1度振幅）
    float latitude = 5.128f * FMath::Sin(FMath::DegreesToRadians(ascendingNode));
    latitude = FMath::Fmod(latitude, 360.0f);
    if (latitude < 0) latitude += 360.0f;
	
    // ===== 4. 坐标转换：黄道坐标 -> 赤道坐标 =====
    float eclipticObliquity = FMath::DegreesToRadians(OBLIQUITY);
	
	float Rad_longitude = FMath::DegreesToRadians(longitude);
	float Rad_latitude = FMath::DegreesToRadians(latitude);
	
    float sinLongitude = FMath::Sin(Rad_longitude);
    float cosLongitude = FMath::Cos(Rad_longitude);
    float sinLatitude = FMath::Sin(Rad_latitude);
    float cosLatitude = FMath::Cos(Rad_latitude);
    float sinObliquity = FMath::Sin(eclipticObliquity);
    float cosObliquity = FMath::Cos(eclipticObliquity);
	
    // 赤经计算
    float ra = FMath::RadiansToDegrees(
    	FMath::Atan2(sinLongitude * cosObliquity - FMath::Tan(Rad_latitude) * sinObliquity, cosLongitude));
    ra = FMath::Fmod(ra + 360.0f, 360.0f);
	
    // 赤纬计算
    float dec = FMath::RadiansToDegrees(
    	FMath::Asin(
			sinLatitude * cosObliquity + cosLatitude * sinObliquity * sinLongitude
    ));
	
    // ===== 5. 计算时角 =====
    // 计算格林尼治恒星时（简化公式）
    float greenwichSiderealTime = 280.46061837f + 360.98564736629f * daysSince2000;
    greenwichSiderealTime = FMath::Fmod(greenwichSiderealTime, 360.0f);
    if (greenwichSiderealTime < 0) greenwichSiderealTime += 360.0f;
	
    // 本地恒星时
    float localSiderealTime = greenwichSiderealTime + Longitude;
    localSiderealTime = FMath::Fmod(localSiderealTime, 360.0f);
    if (localSiderealTime < 0) localSiderealTime += 360.0f;
	
    // 月球时角
    float hourAngle = localSiderealTime - ra;
    hourAngle = FMath::Fmod(hourAngle + 360.0f, 360.0f);
    if (hourAngle > 180.0f) hourAngle -= 360.0f;
	
    // ===== 6. 计算月球高度角和方位角 =====
    float latRad = FMath::DegreesToRadians(Latitude);
    float decRad = FMath::DegreesToRadians(dec);
    float haRad = FMath::DegreesToRadians(hourAngle);
	
    // 高度角计算
    float sinAlt = FMath::Sin(latRad) * FMath::Sin(decRad) + 
                  FMath::Cos(latRad) * FMath::Cos(decRad) * FMath::Cos(haRad);
    sinAlt = FMath::Clamp(sinAlt, -1.0f, 1.0f);
    MoonElevation = FMath::RadiansToDegrees(FMath::Asin(sinAlt));
	
    // 方位角计算（使用atan2确保正确的象限）
    float sinAz = -FMath::Cos(decRad) * FMath::Sin(haRad);
    float cosAz = FMath::Sin(decRad) * FMath::Cos(latRad) - 
                 FMath::Cos(decRad) * FMath::Sin(latRad) * FMath::Cos(haRad);
    
    MoonAzimuth = FMath::RadiansToDegrees(FMath::Atan2(sinAz, cosAz));
    MoonAzimuth = FMath::Fmod(MoonAzimuth + 360.0f, 360.0f);
}

/* Vector Pivot: Local Position, Vector Point at: Planet. */
inline FRotator ConvertPlanetRotation(const float& ElevationDegree, const float& AzimuthDegree)
{
	FRotator Rotator = FRotator::ZeroRotator;
	Rotator.Pitch = ElevationDegree;
	Rotator.Yaw = AzimuthDegree;
	return Rotator;
}

/* Vector Pivot: Local Position, Vector Point at: Planet. */
inline FVector ConvertPlanetDirection(const float& ElevationDegree, const float& AzimuthDegree)
{
	return ConvertPlanetRotation(ElevationDegree, AzimuthDegree).Vector();
}

/* Vector Pivot: Planet, Vector Point at: Local Position. */
inline FVector ConvertPlanetLightDirection(const float& ElevationDegree, const float& AzimuthDegree)
{
	return -ConvertPlanetDirection(ElevationDegree, AzimuthDegree);
}