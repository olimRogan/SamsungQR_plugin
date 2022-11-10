// Copyright Epic Games, Inc. All Rights Reserved.

#include "Samsung_QR.h"

#define LOCTEXT_NAMESPACE "FSamsung_QRModule"

void FSamsung_QRModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSamsung_QRModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSamsung_QRModule, Samsung_QR)