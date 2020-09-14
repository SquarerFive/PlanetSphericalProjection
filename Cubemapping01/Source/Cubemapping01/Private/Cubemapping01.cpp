// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cubemapping01.h"

#define LOCTEXT_NAMESPACE "FCubemapping01Module"

void FCubemapping01Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FCubemapping01Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCubemapping01Module, Cubemapping01)