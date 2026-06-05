#include "Pathfinding/Core/Nav3DPathLibrary.h"
#include "EngineUtils.h"
#include "KismetTraceUtils.h"
#include "Nav3D.h"
#include "Nav3DData.h"
#include "NavigationSystem.h"
#include "Pathfinding/Core/Nav3DPath.h"
#include "Pathfinding/Core/Nav3DPathCoordinator.h"
#include "Pathfinding/Core/Nav3DPathingTypes.h"
#include "Nav3D.h"
#if NAV3D_WITH_DEBUG
#include "Draw/FUDraw.h"
#endif


namespace NAV3D
{
#if NAV3D_WITH_DEBUG
	namespace Debug
	{
		FU_CMD_AUTOVAR(DebugDisplayFindPathExtendedCmd, 
			"nav3d.Fishy.Debug.DisplayFindPathExtended", "Show debug data",
			int32, DebugDisplayFindPathExtended, 0
		);
	}
#endif
}


bool UNav3DPathLibrary::FindNav3DPath(
    const UObject* WorldContextObject,
    FVector StartLocation,
    FVector EndLocation,
    float AgentRadius,
    TArray<FVector>& OutPathPoints)
{
    OutPathPoints.Empty();
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogNav3D, Error, TEXT("FindNav3DPathSimple: No world context"));
        return false;
    }
    
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        UE_LOG(LogNav3D, Error, TEXT("FindNav3DPathSimple: No navigation system"));
        return false;
    }
    
    // Find the FIRST Nav3DData actor in the world
    const ANav3DData* Nav3dData = nullptr;
    for (TActorIterator<ANav3DData> It(World); It;)
    {
        Nav3dData = *It;
        break;
    }
    
    if (!Nav3dData)
    {
        UE_LOG(LogNav3D, Error, TEXT("FindNav3DPathSimple: No Nav3DData actor found in world"));
        return false;
    }
    
    UE_LOG(LogNav3D, Log, TEXT("FindNav3DPathSimple: Found Nav3DData: %s"), *Nav3dData->GetName());
    
    // Build request - use the NavData's own agent properties
    FNav3DPathingRequest Request;
    Request.StartLocation = StartLocation;
    Request.EndLocation = EndLocation;
    Request.NavData = Nav3dData;
    Request.AgentProperties = Nav3dData->GetNavAgentProperties();  // Use NavData's properties!
    Request.AgentProperties.AgentRadius = AgentRadius;  // Override just the radius
    Request.LogVerbosity = ENav3DPathingLogVerbosity::Verbose;
    
    UE_LOG(LogNav3D, Log, TEXT("FindNav3DPathSimple: Calling FindPath from %s to %s with radius %.2f"), 
        *StartLocation.ToString(), *EndLocation.ToString(), AgentRadius);
    
    // Find path
    FNav3DPath Path;
    ENavigationQueryResult::Type Result = FNav3DPathCoordinator::FindPath(Path, Request);
    
    UE_LOG(LogNav3D, Log, TEXT("FindNav3DPathSimple: Result = %d"), (int32)Result);
    
    if (Result == ENavigationQueryResult::Success)
    {
        const TArray<FNavPathPoint>& PathPoints = Path.GetPathPoints();
        UE_LOG(LogNav3D, Log, TEXT("FindNav3DPathSimple: Got %d path points"), PathPoints.Num());
        OutPathPoints.Reserve(PathPoints.Num());
        for (const FNavPathPoint& Point : PathPoints)
        {
            OutPathPoints.Add(Point.Location);
        }
        return true;
    }
    
    return false;
}

bool UNav3DPathLibrary::FindNav3DPathExtended(const UObject* WorldContextObject, const FVector& StartLocation, const FVector& EndLocation, float AgentRadius, TArray<FVector>& OutPathPoints)
{
	TArray<FNavPathPoint> OutNavPathPoint;
    if (FindNav3DPathExtended(WorldContextObject, StartLocation, EndLocation, AgentRadius, OutNavPathPoint))
    {
    	OutPathPoints.Reserve(OutNavPathPoint.Num());
    	for (const FNavPathPoint& Point : OutNavPathPoint)
    	{
    		OutPathPoints.Emplace(Point.Location);
    	}
    	
	    return true;
    }
	
	return false;
}

bool UNav3DPathLibrary::FindNav3DPathExtended(const UObject* WorldContextObject, const FVector& StartLocation, const FVector& EndLocation, float AgentRadius, TArray<FNavPathPoint>& OutNavPathPoints)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}
	
    const ANav3DData* Nav3dData = nullptr;
    for (TActorIterator<ANav3DData> It(World); It;)
    {
        Nav3dData = *It;
        break;
    }
	
	if (!IsValid(Nav3dData)) { return false; }
    
    FNav3DPathingRequest Request;
    Request.StartLocation = StartLocation;
    Request.EndLocation = EndLocation;
    Request.NavData = Nav3dData;
    Request.AgentProperties = Nav3dData->GetNavAgentProperties();
    Request.AgentProperties.AgentRadius = AgentRadius;
    Request.LogVerbosity = ENav3DPathingLogVerbosity::Verbose;
    
    // Find path
    FNav3DPath Path;
    ENavigationQueryResult::Type Result = FNav3DPathCoordinator::FindPath(Path, Request);
    
    if (Result == ENavigationQueryResult::Success)
    {
        const TArray<FNavPathPoint>& PathPoints = Path.GetPathPoints();
    	OutNavPathPoints.Reserve(PathPoints.Num());
    	
    	int32 SourceIndex = 0;
    	int32 NextIndex = 1;
    	// always add first point
    	OutNavPathPoints.Emplace(PathPoints[SourceIndex]);
    	
#if NAV3D_WITH_DEBUG
    	if (NAV3D::Debug::DebugDisplayFindPathExtended)
    	{
    		for (const FNavPathPoint& Point : PathPoints)
    		{
    			FU::Draw::DrawDebugSphere(
					World,
					Point.Location,
					20,
					FColor::Yellow,
					10,
					2
				);
    		}
    	}
#endif
    	
    	while (SourceIndex < PathPoints.Num() && NextIndex < PathPoints.Num())
    	{
    		const FNavPathPoint& CurrPathPoint = PathPoints[SourceIndex];
    		const FNavPathPoint& NextPathPoint = PathPoints[NextIndex];
    		
    		// test if we can skip the next point
    		if (TestDirectTraversal(World, Nav3dData, CurrPathPoint.Location, NextPathPoint.Location, AgentRadius))
    		{
    			// there is a direct path, go check next one and dont add current point, keep source index
    			NextIndex++;
    		}
    		else
    		{
    			// we got blocked, keep the previous point and update source point
    			if (NextIndex > 1)
    			{
    				const int32 PreviousValidIndex = NextIndex - 1;
    				const FNavPathPoint& PreviousPoint = PathPoints[PreviousValidIndex];
    				OutNavPathPoints.Emplace(PreviousPoint);
    				SourceIndex = PreviousValidIndex;
    			}
    			else
    			{
    				NextIndex++;
    				SourceIndex++;
    			}
    		}
    	}
    	
    	// always add last point
    	OutNavPathPoints.Emplace(PathPoints.Last());
    	
    	for (const FNavPathPoint& Point : OutNavPathPoints)
    	{
#if NAV3D_WITH_DEBUG
    		if (NAV3D::Debug::DebugDisplayFindPathExtended)
    		{
    			FU::Draw::DrawDebugSphere(
					World,
					Point.Location,
					20,
					FColor::Cyan,
					20,
					2
				);
    		}
#endif
    	}
    	
    	return true;
    }
	
	return false;
}

bool UNav3DPathLibrary::TestDirectTraversal(UWorld* World, const ANav3DData* Nav3DData, const FVector& Start, const FVector& End, float Radius)
{
	FHitResult Result;
	const bool bHit = World->SweepSingleByChannel(
		Result,
		Start, End, FQuat::Identity, 
		Nav3DData->GetGenerationSettings().CollisionChannel, 
		FCollisionShape::MakeSphere(Radius)
	);
	
#if NAV3D_WITH_DEBUG
	if (NAV3D::Debug::DebugDisplayFindPathExtended)
	{
		DrawDebugSweptSphere(
			World,
			Start,
			End,
			Radius,
			bHit ? FColor::Red : FColor::Blue,
			false,
			10
		);
	}
#endif
	
	return !bHit; 
}
