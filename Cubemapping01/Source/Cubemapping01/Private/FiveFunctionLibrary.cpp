// Copyright (C) Aiden Soedjarwo. 2019 - 2020


#include "FiveFunctionLibrary.h"

#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTargetCube.h"

#include "Voxel/Public/VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelSharedPtr.h"
#include "Kismet/KismetRenderingLibrary.h"

FVoxelFloatTexture UFiveFunctionLibrary::CreateVoxelFloatTextureFromRenderTargetChannel(UObject* WorldContext, UTextureRenderTarget2D* RT, EVoxelRGBA Channel)
{
	// convert rt to texture2d
	/*
	UTexture2D* Texture = RT->ConstructTexture2D(WorldContext, "AlphaTex", EObjectFlags::RF_NoFlags, CTF_DeferCompression);
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	Texture->SRGB = 0;
	
	
	**/

	// TVoxelTexture<float> VoxelT = FVoxelTextureUtilities::CreateFromTexture_Float(RT, Channel);

	const auto ColorTexture = FVoxelTextureUtilities::CreateFromTexture_Color(RT);

	TVoxelSharedPtr<typename TVoxelTexture<float>::FTextureData> Data;

	Data = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	Data->SetSize(ColorTexture.GetSizeX(), ColorTexture.GetSizeY());

	const int32 Num = ColorTexture.GetSizeX() * ColorTexture.GetSizeY();
	//const FColor* FormattedImageData = static_cast<cosnt FColor*>(cast<UTexture2D>(Texture)->PlatformData)
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

	//Texture->ReleaseResource(); // release it before deleting.
	//delete Texture;
	return TVoxelTexture<float>(Data.ToSharedRef());
}

UTextureRenderTargetCube* UFiveFunctionLibrary::CreateRenderTargetCube(UObject* WorldContext, int32 Width, TextureMipGenSettings MipSettings, FLinearColor ClearColor, TextureCompressionSettings CompressionSettings, bool bHDR)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	int32 Height = (int32)Width / 2;
	if (Width > 0 && Height > 0 && World)
	{
		UTextureRenderTargetCube* NewRenderTargetCube = NewObject<UTextureRenderTargetCube>(WorldContext);
		check(NewRenderTargetCube);
		NewRenderTargetCube->ClearColor = ClearColor;
		NewRenderTargetCube->InitAutoFormat(Width);
		NewRenderTargetCube->bHDR = bHDR;
		NewRenderTargetCube->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		NewRenderTargetCube->CompressionSettings = CompressionSettings;
		NewRenderTargetCube->UpdateResource();
		return NewRenderTargetCube;
	}
	return nullptr;
}

UTexture2D* UFiveFunctionLibrary::TextureFromRenderTarget2D(UObject* WorldContext, UTextureRenderTarget2D* RT, TextureMipGenSettings MipSettings, TextureCompressionSettings CompressionSettings)
{
	UTexture2D* Texture = RT->ConstructTexture2D(WorldContext, "Heightmap", EObjectFlags::RF_NoFlags, CTF_Default);
	Texture->CompressionSettings = CompressionSettings;


#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = MipSettings;
#endif
	Texture->UpdateResource();
	return Texture;
}
