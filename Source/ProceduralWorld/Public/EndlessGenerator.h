// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkMesh.h"
#include "MeshDataStructs.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "FastNoiseLite.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"
#include "EndlessGenerator.generated.h"

class AChunkMesh;
class FastNoiseLite;

UCLASS()
class PROCEDURALWORLD_API AEndlessGenerator : public AActor
{
	GENERATED_BODY()

/******************VARIABLES********************/

public:	
	// Sets default values for this actor's properties
	AEndlessGenerator();

	UPROPERTY(EditAnywhere, Category = "Chunk Data")
		TSubclassOf<AChunkMesh> Chunk;
		
	UPROPERTY(EditAnywhere, Category = "Chunk Data", Meta = (ClampMin = 1.0))
		float chunkSize = 1000.f;
	UPROPERTY(EditAnywhere, Category = "Chunk Data", Meta = (ClampMin = 1.0))
		float GridSpacing = 100.f;
	UPROPERTY(EditAnywhere, Category = "Chunk Data", Meta = (ClampMin = 0.000001))
		float UVScale = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		int32 worldSeed = 1133;

	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float ContinentalnessHeightMultiplicator = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float ContinentalnessFrequency = 0.000001;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		int32 ContinentalnessOctaves = 9;

	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float ErosianHeightMultiplicator = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float ErosianFrequency = 0.000001f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		int32 ErosianOctaves = 7;

	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float PeaksHeightMultiplicator = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		float PeaksFrequency = 0.000003f;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		int32 PeakesOctaves = 8;


	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		int32 RenderRange = 30;

		int32 levelOfDetail = 0;

		AChunkMesh* spawnedActor = nullptr;

		// TerrainData

		FIntVector LastCoords;

		TMap<FVector, FMapData*> MapCache;

		TQueue<FVector> SpawnQueue;

		TMap<FVector, AChunkMesh*> RenderedChunks;
		TArray<bool> ChunksVisibility;

protected:
	// Terrain Curves
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		UCurveFloat* CCurve;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		UCurveFloat* ECurve;
	UPROPERTY(EditAnywhere, Category = "Terrain Data")
		UCurveFloat* PCurve;

	// Terrain 
	FastNoiseLite* Continentalness;
	FastNoiseLite* Erosian;
	FastNoiseLite* Peaks;

/******************FUNCTIONS********************/

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	void UpdateChunkVisibility();

	void GenerateChunks();

	void CreateClassAndMesh();

	void SetupNoise(FastNoiseLite* noise, int seed, float frequency, int octaves);
	
	float CalculateHeight(float X, float Y);

	bool CoordsChanged();

	void WriteToCacheFile(const FString& Filename, const FMapData* Data);

	void ReadFromCacheFile(const FString& Filename);

	//void WriteToFile(FVector location);

	//void ReadFromFile(FVector location);

	FIntVector GetCurrentCoords();

};
