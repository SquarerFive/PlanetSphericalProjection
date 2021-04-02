// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetManagerSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

void UPlanetManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

}

void UPlanetManagerSubsystem::Deinitialize()
{
	TArray<FRenderTargetSlot> Values;
	RenderTargetStorage.GenerateValueArray(Values);
	for (FRenderTargetSlot Slot : Values)
	{
		if (Slot.bValid)
		{
			Slot.Texture->ReleaseResource();
		}
	}
	Values.Empty();
	RenderTargetStorage.Reset();
}

void UPlanetManagerSubsystem::SetupTexturesIfNot(FRenderTargetConfig Config)
{
	if (!bInitialized)
	{
		// int32 id = 0;
		for (FRenderTargetLOD LOD : Config.LODs)
		{
			for (int id = 0; id < LOD.Count; ++id) {
				UTextureRenderTarget2D* RT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), LOD.Width, int32(LOD.Width / 2));
				FRenderTargetSlot Slot(RT);
				RenderTargetStorage.Add(
					FIntVector(id, LOD.LOD, 0),
					Slot
				);
			}
		}
		bInitialized = true;
	}
}
