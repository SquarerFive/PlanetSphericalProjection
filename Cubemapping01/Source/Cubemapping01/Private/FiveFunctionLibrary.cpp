// Copyright (C) Aiden Soedjarwo. 2019 - 2020


#include "FiveFunctionLibrary.h"

#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTargetCube.h"

#include "Voxel/Public/VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelSharedPtr.h"
#include "Kismet/KismetRenderingLibrary.h"


// Texture caching 

inline auto& GetRenderTargetMap()
{
	check(IsInGameThread());
	static TMap<FString, UTextureRenderTarget*> Map;
	return Map;
}

inline auto& GetVoxelTextureMap()
{
	check(IsInGameThread());
	static TMap<UTextureRenderTarget*, TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData>> Map;
	return Map;
}

FVoxelFloatTexture UFiveFunctionLibrary::CreateVoxelFloatTextureFromRenderTargetChannel(UObject* WorldContext, UTextureRenderTarget2D* RT, EVoxelRGBA Channel, int MipLevel)
{
	auto& Data = GetVoxelTextureMap().FindOrAdd(RT);
	if (Data)
	{
		return TVoxelTexture<float>(Data.ToSharedRef());
	}
	const auto ColorTexture = FVoxelTextureUtilities::CreateFromTexture_Color(RT);

	
	Data = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	Data->SetSize(ColorTexture.GetSizeX(), ColorTexture.GetSizeY());

	const int32 Num = ColorTexture.GetSizeX() * ColorTexture.GetSizeY();
	const int32 Width = ColorTexture.GetSizeX();
	const int32 Height = ColorTexture.GetSizeY();
	for (int32 X = 0; X < Width; X++)
	{
		for (int32 Y = 0; Y < Height; Y++)
		{
			int32 Index = Y * Width + X;
			const FColor Color = ColorTexture.GetTextureData()[Index];
			uint8 Result = 0;
			switch (Channel)
			{
			case EVoxelRGBA::R:
				Result = Color.R;
				break;
			case EVoxelRGBA::G:
				Result = Color.G;
				break;
			case EVoxelRGBA::B:
				Result = Color.B;
				break;
			case EVoxelRGBA::A:
				Result = Color.A;
				break;
			}
			const float Value = FVoxelUtilities::UINT8ToFloat(Result);
			Data->SetValue(Index, Value);
		}
	}

	return TVoxelTexture<float>(Data.ToSharedRef());
}

UTextureRenderTargetCube* UFiveFunctionLibrary::CreateRenderTargetCube(UObject* WorldContext, int32 Width, TextureMipGenSettings MipSettings, FLinearColor ClearColor, TextureCompressionSettings CompressionSettings, bool bHDR, FString TextureKey)
{
	UTextureRenderTarget* Data = GetRenderTargetMap().FindOrAdd(TextureKey);
	if (Data->IsValidLowLevel())
	{
		return (UTextureRenderTargetCube*)Data;
	}
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	int32 Height = (int32)Width / 2;
	if (Width > 0 && Height > 0 && World)
	{
		UTextureRenderTargetCube* NewRenderTargetCube = NewObject<UTextureRenderTargetCube>(WorldContext, FName(*TextureKey));
		check(NewRenderTargetCube);
		NewRenderTargetCube->ClearColor = ClearColor;
		NewRenderTargetCube->InitAutoFormat(Width);
		NewRenderTargetCube->bHDR = bHDR;

		NewRenderTargetCube->CompressionSettings = CompressionSettings;
		NewRenderTargetCube->UpdateResource();
		Data = NewRenderTargetCube;
		return NewRenderTargetCube;
	}
	return nullptr;
}


/*
UTexture2D* UFiveFunctionLibrary::TextureFromRenderTarget2D(UObject* WorldContext, UTextureRenderTarget2D* RT, TextureMipGenSettings MipSettings, TextureCompressionSettings CompressionSettings)
{
	UTexture2D* Texture = RT->ConstructTexture2D(WorldContext, "Heightmap", EObjectFlags::RF_NoFlags, CTF_Default);
	Texture->CompressionSettings = CompressionSettings;
	Texture->UpdateResource();
	return Texture;
}
*/

UTextureRenderTarget* UFiveFunctionLibrary::GetCachedRT(FString TextureKey, bool& bSuccess)
{
	UTextureRenderTarget* Texture = GetRenderTargetMap().FindRef(TextureKey);
	bSuccess = Texture != nullptr;
	return Texture;
}

void UFiveFunctionLibrary::ClearCache(bool bRenderTargetsOnly, bool bVoxelTexturesOnly)
{
	if (bVoxelTexturesOnly || (!bRenderTargetsOnly && !bVoxelTexturesOnly))
	{
		TArray<TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData>> VTArray;
		GetVoxelTextureMap().GenerateValueArray(VTArray);
		
		//for (TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData> VTP : VTArray)
		for (int i = 0; i < VTArray.Num(); i++)
		{
			TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData> VTP = VTArray[i];
			
			//VTP->~FTextureData();
			if (VTP)
			{
				VTP.Reset();
			}
		}
		
		GetVoxelTextureMap().Empty();
	}

	if (bRenderTargetsOnly || (!bRenderTargetsOnly && !bVoxelTexturesOnly))
	{
		TArray<UTextureRenderTarget*> RTArray;
		GetRenderTargetMap().GenerateValueArray(RTArray);
		for (int i = 0; i < RTArray.Num(); i++)
		{
			if (RTArray[i])
			RTArray[i]->ReleaseResource();
		}
		GetRenderTargetMap().Empty();
	}
}

void UFiveFunctionLibrary::ReleaseTextureResource(UTextureRenderTarget* RT)
{
	if (RT)RT->ReleaseResource();
}
