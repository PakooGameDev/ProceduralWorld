// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "ProceduralMeshComponent.h"

#include "ChunkGen.h"
#include "MeshDataStructs.h"

#include "KismetProceduralMeshLibrary.h"

#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Kismet/GameplayStatics.h"

#include "Async/Async.h"

#include <fstream>
#include <iostream>
#include <string>

#include "ChunkMesh.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class PROCEDURALWORLD_API AChunkMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunkMesh();

	UProceduralMeshComponent* ProceduralMesh;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// EndPlay event
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void CreateMesh(FMapData* mapData);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private: 
	// Terrain
	UPROPERTY(EditAnywhere, Category = "Terrain Material")
		UMaterialInterface* Material;

};


