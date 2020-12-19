// Copyright (C) SquarerFive. 2019 - 2020

#pragma once

//#include "Core.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTexture.h"
//#include "VoxelNodes/VoxelNodeHelpers.h"
#include "FiveFunctionLibrary.generated.h"

/**
 * 
 */


class UTextureRenderTargetCube;
class UTexture2D;

USTRUCT(BlueprintType)
struct FPlanetResource
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString CubemapKey;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString TextureKey; // 2d
};

USTRUCT(BlueprintType)
struct FPlanetResourceKey {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UTextureRenderTarget* Value;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bValid = false;
	FPlanetResourceKey() {

	}
	FPlanetResourceKey(UTextureRenderTarget* InValue) : Value(InValue) { bValid = true; }
};

UCLASS()
class CUBEMAPPING01_API UFiveFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static FVoxelFloatTexture CreateVoxelFloatTextureFromRenderTargetChannel(UObject* WorldContext, FPlanetResource Resource, EVoxelRGBA Channel, int MipLevel);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static UTextureRenderTargetCube* CreateRenderTargetCube(UObject* WorldContext, int32 Width, TextureMipGenSettings MipSettings, FLinearColor ClearColor, TextureCompressionSettings CompressionSettings, bool bHDR, FString TextureKey);
	/*
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"), Category = "FiveFunctionLibrary | Texture Utilities")
		static UTexture2D* TextureFromRenderTarget2D(UObject* WorldContext, UTextureRenderTarget2D* RT, TextureMipGenSettings MipSettings, TextureCompressionSettings CompressionSettings);
	*/

	UFUNCTION(BlueprintCallable, BlueprintPure)
		static UTextureRenderTarget* GetCachedRT(FString TextureKey, bool &bSuccess);
	UFUNCTION(BlueprintCallable)
		static void ClearCache(bool bRenderTargetsOnly, bool bVoxelTexturesOnly);
	UFUNCTION(BlueprintCallable)
		static void ReleaseTextureResource(UTextureRenderTarget* RT);
	UFUNCTION(BlueprintCallable)
		static FPlanetResource CreatePlanetResource(UObject* WorldContext, FString CubemapKey, FString TextureKey, int32 Width);
	UFUNCTION(BLueprintCallable, meta = (WorldContext = "WorldContext"))
		static void ReleasePlanetResource(FPlanetResource Resource);

	UFUNCTION(BLueprintCallable, BlueprintPure)
		static UTextureRenderTarget2D* GetRenderTarget2DFromResource(FPlanetResource Resource, bool& bSuccess);
	UFUNCTION(BLueprintCallable, BlueprintPure)
		static UTextureRenderTargetCube* GetRenderTargetCubeFromResource(FPlanetResource Resource, bool& bSuccess);
	UFUNCTION(BLueprintCallable, BlueprintPure)
		static bool IsPlanetResourceValid(FPlanetResource Resource);

	/* Debug */
	UFUNCTION(BlueprintCallable)
		static TMap<FString, FPlanetResourceKey> GetCache();
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContext"))
		static UWorld* GetWorld(UObject* WorldContext);

};



