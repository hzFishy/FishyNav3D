#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Nav3DPathLibrary.generated.h"
class ANav3DData;


UCLASS()
class NAV3D_API UNav3DPathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Simplified Blueprints-friendly pathfinder - just get a path from A to B
	UFUNCTION(BlueprintCallable, Category = "Nav3D|Pathfinding", meta = (WorldContext = "WorldContextObject"))
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	static bool FindNav3DPath(
		const UObject* WorldContextObject,
		FVector StartLocation,
		FVector EndLocation,
		float AgentRadius,
		TArray<FVector>& OutPathPoints);
	
	/** Fishy improvements */
	UFUNCTION(BlueprintCallable, Category="Nav3D|Pathfinding", meta=(WorldContext="WorldContextObject"))
	static bool FindNav3DPathExtended(const UObject* WorldContextObject, const FVector& StartLocation, const FVector& EndLocation, float AgentRadius, TArray<FVector>& OutPathPoints);
	
	/** Fishy improvements */
	static bool FindNav3DPathExtended(const UObject* WorldContextObject, const FVector& StartLocation, const FVector& EndLocation, float AgentRadius, TArray<FNavPathPoint>& OutNavPathPoints);

	/** 
	 *  Test if there is any collisions between the two points.
	 *  @returns true if there is no blocking collisions.
	 */
	static bool TestDirectTraversal(UWorld* World, const ANav3DData* Nav3DData, const FVector& Start, const FVector& End, float Radius);
};
