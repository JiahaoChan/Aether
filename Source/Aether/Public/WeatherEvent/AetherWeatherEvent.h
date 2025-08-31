/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "AetherTypes.h"

#include "AetherWeatherEvent.generated.h"

/*
 * Weather Event:
 * 下雨 Rainy:
 * 毛毛雨 Drizzle: < 0.1mm/24h
 * 小雨 Light Rain: 0.1mm/24h ~ 10mm/24h
 * 中雨 Moderate Rain: 10mm/24h ~ 25mm/24h
 * 大雨 Heavy Rain: 25mm/24h ~ 50mm/24h
 * 暴雨 Rainstorm: 50mm/24h ~ 100mm/24h
 * 阵雨 Rain Shower: 突发性强、骤起骤停，常伴雷电（如夏季午后）
 * 
 * 雪 Snowy
 * 零星小雪 Slight Snow: < 0.1mm/24h
 * 小雪 Light Snow: 0.1mm/24h ~ 2.5mm/24h
 * 中雪 Moderate Snow: 2.5mm/24h ~ 5mm/24h
 * 大雪 Heavy Snow: 5mm/24h ~ 10mm/24h
 * 暴雪 Snowstorm: 10mm/24h ~ 20mm/24h
 * 阵雪 Snow Shower: 突发性降雪，强度变化剧烈
 * 
 * 积雪（Snow Cover）与吹雪（Drifting Snow）：地表持续积雪，或强风卷雪贴地运行（水平能见度不受影响）。
 * 雪暴（Snowstorm）：强风裹挟大量雪花狂舞，能见度<1公里，常与暴雪共生。
 * 结冰（Freezing）：低温导致地表或物体表面覆冰，影响交通与设施安全。
 * 冰雹（Hail）：强烈对流云中形成的层状冰球（＞5mm），破坏力强
 * 
 * 露 Dew: 水汽在夜间遇冷后于地面或物体表面凝结成的水珠，多出现于晴朗微风的夜晚。
 * 霜 Frost: 气温低于0℃时，水汽直接凝华成白色松脆冰晶，附着于地表或植物表面，常见于秋冬清晨。
 * 雾凇 Rime: 过冷雾滴在树枝、电线等物体上冻结形成的乳白色冰晶，呈毛茸状或粒状，多发生于高湿度低温环境。
 * 雨凇 Glaze: 过冷雨滴接触低温物体后瞬间冻结成透明或毛玻璃状冰层，易导致电线压断、路面结冰。
 * 
 * 雾（Fog）：近地面微小水滴悬浮使水平能见度<1公里，分平流雾（暖湿空气遇冷地表）和辐射雾（地表辐射冷却形成）。
 * 霾（Haze）：干尘粒均匀悬浮导致能见度<10公里，天空呈黄灰色，与污染密切相关。
 * 浮尘（Suspended Dust ：无风或微风时尘沙悬浮，能见度<10公里。
 * 扬沙（Blowing Sand）：风卷地面沙尘，能见度1~10公里。
 * 沙尘暴（Sand and Dust Storm）：强风卷沙致能见度<1公里，超强沙尘暴能见度可低于50米。
 * 
 * 雷暴（Thunderstorm）：积雨云内放电现象，伴随闪电与雷声，常引发短时强降水或冰雹。
 * 闪电（Lightning）：云间或云地放电产生的强光，可独立于雷声出现。
 * 极光（Aurora）：高纬地区晴夜大气电离层辉光，呈彩色光弧或光幕，中纬度罕见。
 * 虹（Rainbow）与晕（Halo）：阳光经水滴折射形成彩虹，或透过冰晶产生日晕光环。
 * 
 * 大风（Gale）：瞬时风速≥17米/秒（8级风），可摧毁临时建筑。
 * 飑（Squall）：突发性强风，风速骤增伴风向突变，常与雷暴共生。
 * 龙卷（Tornado）：积雨云下垂的漏斗状强旋风，中心风速可达100~200米/秒，破坏力极强。
 * 尘卷风（Dust Devil）：地面局部受热形成小旋风，卷起尘沙形成尘柱，多见于干燥午后。
 */

USTRUCT(BlueprintType)
struct FWeatherEventDescription
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UAetherWeatherEvent> Event;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeatherTriggerSource TriggerSource;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, meta = (EditCondition = "TriggerSource == EWeatherTriggerSource::AetherController", EditConditionHides))
	TMap<EAetherMonth, float> HappeningMonthsProbability;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, VisibleAnywhere, DisplayName="Happening Month", meta = (EditCondition = "TriggerSource == EWeatherTriggerSource::AetherController", EditConditionHides))
	FString HappeningMonthDisplay;
#endif
	
	FWeatherEventDescription()
	{
		Event = nullptr;
		TriggerSource = EWeatherTriggerSource::AetherController;
		HappeningMonthsProbability.Add(EAetherMonth::January, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::February, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::March, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::April, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::May, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::June, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::July, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::August, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::September, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::October, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::November, 0.0f);
		HappeningMonthsProbability.Add(EAetherMonth::December, 0.0f);
		
#if WITH_EDITORONLY_DATA
		HappeningMonthDisplay = FString();
#endif
	}
	
#if WITH_EDITORONLY_DATA
	void UpdateHappeningMonthDisplayString()
	{
		HappeningMonthDisplay = "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::January] > 0.0f ? "Jan. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::February] > 0.0f ? "Feb. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::March] > 0.0f ? "Mar. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::April] > 0.0f ? "Apr. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::May] > 0.0f ? "May. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::June] > 0.0f ? "Jun. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::July] > 0.0f ? "Jul. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::August] > 0.0f ? "Aug. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::September] > 0.0f ? "Sept. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::October] > 0.0f ? "Oct. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::November] > 0.0f ? "Nov. " : "";
		HappeningMonthDisplay += HappeningMonthsProbability[EAetherMonth::December] > 0.0f ? "Dec. " : "";
	}
#endif
	
#if WITH_EDITOR
	void FixProbability()
	{
		for (auto It = HappeningMonthsProbability.CreateIterator(); It; ++It)
		{
			It.Value() = FMath::Clamp(It.Value(), 0.0f, 1.0f);
		}
	}
#endif
};

/* EventClass */
UCLASS(Abstract, Blueprintable)
class UAetherWeatherEvent : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	FGameplayTagContainer EventTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	FGameplayTagContainer BlockWeatherEventsWithTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	FGameplayTagContainer CancelWeatherEventsWithTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	EWeatherEventRangeType EventRange;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	EWeatherEventType EventType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = 0))
	int32 Priority;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	EWeatherEventDuration DurationType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides, ClampMin = "0.1"))
	float DurationMax;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides))
	float DurationMin;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides, ClampMin = "0.1"))
	float BlendInTimeMax;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides))
	float BlendInTimeMin;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides, ClampMin = "0.1"))
	float BlendOutTimeMax;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather", meta = (EditCondition = "DurationType == EWeatherEventDuration::Duration", EditConditionHides))
	float BlendOutTimeMin;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weather")
	FText Description;
#endif
	
	bool bMakeInstanceHasBlueprintImpl;
	
	UAetherWeatherEvent();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	class UAetherWeatherEventInstance* MakeInstance_Route(class AAetherAreaController* Outer);
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) { return nullptr; }
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Aether Weather Event", DisplayName = "MakeInstance")
	UAetherWeatherEventInstance* K2_MakeInstance(AAetherAreaController* Outer);
};

/* EventInstance */
UCLASS(MinimalAPI, Blueprintable)
class UAetherWeatherEventInstance : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<UAetherWeatherEvent> EventClass;
	
	UPROPERTY()
	EWeatherEventExecuteState State;
	
	UPROPERTY()
	float CurrentStateLastTime;
	
	UPROPERTY()
	float Duration;
	
	UPROPERTY()
	float BlendInTime;
	
	UPROPERTY()
	float BlendOutTime;
	
	UAetherWeatherEventInstance()
	{
		EventClass = nullptr;
		State = EWeatherEventExecuteState::Finished;
		CurrentStateLastTime = 0.0f;
		Duration = 0.0f;
		BlendInTime = 0.0f;
		BlendOutTime = 0.0f;
	}
	
	UFUNCTION(BlueprintNativeEvent, Category = "Aether Weather Event")
	EWeatherEventExecuteState BlendIn(float DeltaTime, AAetherAreaController* AetherController);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Aether Weather Event")
	EWeatherEventExecuteState Run(float DeltaTime, AAetherAreaController* AetherController);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Aether Weather Event")
	EWeatherEventExecuteState BlendOut(float DeltaTime, AAetherAreaController* AetherController);
};

UCLASS()
class UAetherWeatherEvent_Custom : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UAetherWeatherEvent_Custom()
	{
		EventType = EWeatherEventType::Custom;
	}
};