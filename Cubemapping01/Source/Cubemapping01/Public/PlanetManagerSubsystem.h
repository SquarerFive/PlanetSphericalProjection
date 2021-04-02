// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlanetManagerSubsystem.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FRenderTargetLOD
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 Width;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 LOD;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 Count = 3;
};

USTRUCT(BlueprintType, Blueprintable)
struct FRenderTargetConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FRenderTargetLOD> LODs;
};

USTRUCT(BlueprintType, Blueprintable)
struct FRenderTargetSlot
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bInUse = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bValid = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UTextureRenderTarget2D* Texture;

	FRenderTargetSlot( ) {}
	FRenderTargetSlot(UTextureRenderTarget2D* InTexture) : Texture(InTexture) { bValid = true; }
};

UCLASS()
class CUBEMAPPING01_API UPlanetManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
public:
	UFUNCTION(BlueprintCallable)
		void SetupTexturesIfNot(FRenderTargetConfig Config);
private:
	bool bInitialized = false;

	// index - lod - extra
	TMap<FIntVector, FRenderTargetSlot> RenderTargetStorage;

};
