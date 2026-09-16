// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/MissionRegistry.h"
#include "NeonDistrict/NeonDistrictMission.h"

ANeonDistrictMission* UMissionRegistry::GetActiveMission(EMissionCategory Category) const
{
	for (const TWeakObjectPtr<ANeonDistrictMission>& Entry : ActiveMissions)
	{
		ANeonDistrictMission* Mission = Entry.Get();
		if (Mission && Mission->GetCategory() == Category)
		{
			return Mission;
		}
	}
	return nullptr;
}

void UMissionRegistry::GetActiveMissions(EMissionCategory Category, TArray<ANeonDistrictMission*>& OutMissions) const
{
	OutMissions.Reset();
	
	for (const TWeakObjectPtr<ANeonDistrictMission>& Entry : ActiveMissions)
	{
		ANeonDistrictMission* Mission = Entry.Get();
		if (Mission && Mission->GetCategory() == Category)
		{
			OutMissions.Add(Mission);
		}
	}
}

void UMissionRegistry::Register(ANeonDistrictMission* Mission)
{
	if (!Mission || ActiveMissions.Contains(Mission)) return;
	
	ActiveMissions.Add(Mission);
	OnActiveMissionsChanged.Broadcast(Mission, true);
}

void UMissionRegistry::Unregister(ANeonDistrictMission* Mission)
{
	// 없던 걸 빼면 방송하지 않는다
	if (ActiveMissions.Remove(Mission) == 0) return;
	
	OnActiveMissionsChanged.Broadcast(Mission, false);
}
