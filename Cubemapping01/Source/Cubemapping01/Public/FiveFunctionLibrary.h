// Copyright (C) Aiden Soedjarwo. 2019 - 2020

#pragma once

//#include "Core.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTexture.h"
#include "FiveFunctionLibrary.generated.h"

/**
 * 
 */


class UTextureRenderTargetCube;
class UTexture2D;


UCLASS()
class CUBEMAPPING01_API UFiveFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static FVoxelFloatTexture CreateVoxelFloatTextureFromRenderTargetChannel(UObject* WorldContext, UTextureRenderTarget2D* RT, EVoxelRGBA Channel);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static UTextureRenderTargetCube* CreateRenderTargetCube(UObject* WorldContext, int32 Width, TextureMipGenSettings MipSettings, FLinearColor ClearColor, TextureCompressionSettings CompressionSettings, bool bHDR);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static UTexture2D* TextureFromRenderTarget2D(UObject* WorldContext, UTextureRenderTarget2D* RT, TextureMipGenSettings MipSettings, TextureCompressionSettings CompressionSettings);
	
};
