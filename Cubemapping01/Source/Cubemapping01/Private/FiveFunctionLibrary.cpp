// Copyright (C) SquarerFive. 2019 - 2020


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
	static TMap<FString, FPlanetResourceKey> Map;
	return Map;
}

inline auto& GetVoxelTextureMap()
{
	check(IsInGameThread());
	static TMap<UTextureRenderTarget*, TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData>> Map;
	return Map;
	
}

template<typename T>
inline auto& GetVoxelTextureTypeMap()
{
	check(IsInGameThread());
	static TMap<FString, TVoxelSharedPtr<typename TVoxelTexture<T>::FTextureData>> Map;
	return Map;
}

// ref: VoxelPlugin ; VoxelTexture.cpp

inline void ExtractTextureData(UTexture* Texture, int32& OutSizeX, int32& OutSizeY, TArray<FColor>& OutData)
{
	VOXEL_FUNCTION_COUNTER();

	check(IsInGameThread());

	if (auto* Texture2D = Cast<UTexture2D>(Texture))
	{
		FTexture2DMipMap& Mip = Texture2D->PlatformData->Mips[0];
		OutSizeX = Mip.SizeX;
		OutSizeY = Mip.SizeY;

		const int32 Size = OutSizeX * OutSizeY;
		OutData.SetNumUninitialized(Size);

		auto& BulkData = Mip.BulkData;
		if (!ensureAlways(BulkData.GetBulkDataSize() > 0))
		{
			OutSizeX = 1;
			OutSizeY = 1;
			OutData.SetNum(1);
			return;
		}

		void* Data = BulkData.Lock(LOCK_READ_ONLY);
		if (!ensureAlways(Data))
		{
			Mip.BulkData.Unlock();
			OutSizeX = 1;
			OutSizeY = 1;
			OutData.SetNum(1);
			return;
		}

		FMemory::Memcpy(OutData.GetData(), Data, Size * sizeof(FColor));
		Mip.BulkData.Unlock();
		return;
	}

	if (auto* TextureRenderTarget = Cast<UTextureRenderTarget2D>(Texture))
	{
		FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
		if (ensure(RenderTarget))
		{
			const auto Format = TextureRenderTarget->GetFormat();

			OutSizeX = TextureRenderTarget->GetSurfaceWidth();
			OutSizeY = TextureRenderTarget->GetSurfaceHeight();

			const int32 Size = OutSizeX * OutSizeY;
			OutData.SetNumUninitialized(Size);

			switch (Format)
			{
			case PF_B8G8R8A8:
			{
				if (ensure(RenderTarget->ReadPixels(OutData))) return;
				break;
			}
			case PF_R8G8B8A8:
			{
				if (ensure(RenderTarget->ReadPixels(OutData))) return;
				break;
			}
			case PF_FloatRGBA:
			{
				TArray<FLinearColor> LinearColors;
				LinearColors.SetNumUninitialized(Size);
				if (ensure(RenderTarget->ReadLinearColorPixels(LinearColors)))
				{
					for (int32 Index = 0; Index < Size; Index++)
					{
						OutData[Index] = LinearColors[Index].ToFColor(false);
					}
					return;
				}
				break;
			}
			default:
				ensure(false);
			}
		}
	}

	ensure(false);
	OutSizeX = 1;
	OutSizeY = 1;
	OutData.SetNum(1);
}

TVoxelTexture<FColor> CreateVoxelTexture_Colour(UTexture* Texture, FString& TextureKey)
{
	auto& Data = GetVoxelTextureTypeMap<FColor>().FindOrAdd(TextureKey);
	if (!Data.IsValid())
	{
		int32 SizeX = -1;
		int32 SizeY = -1;
		TArray<FColor>TextureData;
		ExtractTextureData(Texture, SizeX, SizeY, TextureData);

		Data = MakeVoxelShared<TVoxelTexture<FColor>::FTextureData>();
		Data->SetSize(SizeX, SizeY);
		for (int32 Index = 0; Index < SizeX * SizeY; ++Index)
		{
			Data->SetValue(Index, TextureData[Index]);
		}
	}
	return TVoxelTexture<FColor>(Data.ToSharedRef());
}

FVoxelFloatTexture UFiveFunctionLibrary::CreateVoxelFloatTextureFromRenderTargetChannel(UObject* WorldContext, FPlanetResource Resource, EVoxelRGBA Channel, int MipLevel)
{
	FPlanetResourceKey* RTData = GetRenderTargetMap().Find(Resource.TextureKey);
	UTextureRenderTarget* RT = RTData->Value;

	auto& Data = GetVoxelTextureTypeMap<float>().FindOrAdd(Resource.TextureKey);
	if (Data)
	{
		return TVoxelTexture<float>(Data.ToSharedRef());
	}
	
	const auto ColorTexture = CreateVoxelTexture_Colour(RT, Resource.TextureKey);

	
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
	FPlanetResourceKey& Data = GetRenderTargetMap().FindOrAdd(TextureKey);
	if (Data.bValid)
	{
		return (UTextureRenderTargetCube*)Data.Value;
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
		Data = FPlanetResourceKey(NewRenderTargetCube);
		// Data = NewRenderTargetCube;
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
	FPlanetResourceKey* Texture = GetRenderTargetMap().Find(TextureKey);
	bSuccess = Texture != nullptr;
	if (Texture)
		return Texture->Value;
	return nullptr;
	
}

void UFiveFunctionLibrary::ClearCache(bool bRenderTargetsOnly, bool bVoxelTexturesOnly)
{
	if (bVoxelTexturesOnly || (!bRenderTargetsOnly && !bVoxelTexturesOnly))
	{
		// Workaround for the error.
		// FVoxelTextureHelpers::ClearCache();
		// FVoxelTextureHelpers::ClearIdCache();

		

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
		TArray<FPlanetResourceKey> RTArray;
		GetRenderTargetMap().GenerateValueArray(RTArray);
		for (int i = 0; i < RTArray.Num(); i++)
		{
			if (RTArray[i].Value)
			RTArray[i].Value->ReleaseResource();
		}
		GetRenderTargetMap().Empty();
	}
}

void UFiveFunctionLibrary::ReleaseTextureResource(UTextureRenderTarget* RT)
{
	if (RT)RT->ReleaseResource();
}

FPlanetResource UFiveFunctionLibrary::CreatePlanetResource(UObject* WorldContext, FString CubemapKey, FString TextureKey, int32 Width)
{
	check(IsInGameThread());
	ensure(WorldContext);
	FPlanetResource Resource = FPlanetResource();
	Resource.TextureKey = TextureKey;
	Resource.CubemapKey = CubemapKey;

	FPlanetResourceKey& Data = GetRenderTargetMap().FindOrAdd(TextureKey);
	if (Data.bValid) { Data.Value->ReleaseResource(); }

	//UTextureRenderTarget2D* RT = UKismetRenderingLibrary::CreateRenderTarget2D(WorldContext, Width, Width, ETextureRenderTargetFormat::RTF_RGBA16f);

	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(WorldContext, (FName)*TextureKey);
	check(RT);
	RT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	RT->ClearColor = FLinearColor::Black;
	RT->bAutoGenerateMips = false;
	RT->InitAutoFormat(Width, int32(Width/2));
	RT->UpdateResourceImmediate(true);

	ensure(RT != nullptr);

	CreateRenderTargetCube(WorldContext, Width, TextureMipGenSettings::TMGS_Sharpen10, FLinearColor(FColor::Black),
		TextureCompressionSettings::TC_VectorDisplacementmap, true, CubemapKey);

	if (!RT)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("RT is invalid (failed to create)"));
	}
	Data = FPlanetResourceKey(RT);
	

	return Resource;
}

void UFiveFunctionLibrary::ReleasePlanetResource(FPlanetResource Resource)
{
	FPlanetResourceKey* RT = GetRenderTargetMap().Find(Resource.TextureKey);
	if (RT)
	{
		RT->bValid = false;
		RT->Value->ReleaseResource();
	}
	auto* Data = GetVoxelTextureTypeMap<FColor>().Find(Resource.TextureKey);
	if (Data) {
		Data->Reset();
	}
	FPlanetResourceKey* RTCube = GetRenderTargetMap().Find(Resource.CubemapKey);
	if (RTCube)
	{
		RTCube->bValid = false;
		RTCube->Value->ReleaseResource();
		//RTCube->Reset();
	}
	GetRenderTargetMap().Reset();
}

UTextureRenderTarget2D* UFiveFunctionLibrary::GetRenderTarget2DFromResource(FPlanetResource Resource, bool& bSuccess)
{
	FPlanetResourceKey* RT = GetRenderTargetMap().Find(Resource.TextureKey);
	if (RT)
	{
		bSuccess = true;
		return Cast<UTextureRenderTarget2D>(RT->Value);
	}
	bSuccess = false;
	return nullptr;
}

UTextureRenderTargetCube* UFiveFunctionLibrary::GetRenderTargetCubeFromResource(FPlanetResource Resource, bool& bSuccess)
{
	FPlanetResourceKey* RT = GetRenderTargetMap().Find(Resource.CubemapKey);
	if (RT)
	{
		
		bSuccess = true;
		return Cast<UTextureRenderTargetCube>(RT->Value);
	}
	bSuccess = false;
	return nullptr;
}

bool UFiveFunctionLibrary::IsPlanetResourceValid(FPlanetResource Resource)
{
	FPlanetResourceKey* RT2D = GetRenderTargetMap().Find(Resource.TextureKey);
	FPlanetResourceKey* RTCube = GetRenderTargetMap().Find(Resource.CubemapKey);
	return RT2D != nullptr && RTCube != nullptr;
}

TMap<FString, FPlanetResourceKey> UFiveFunctionLibrary::GetCache()
{
	return GetRenderTargetMap();
}

UWorld* UFiveFunctionLibrary::GetWorld(UObject* WorldContext)
{
	return GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
}
