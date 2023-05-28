// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkMesh.h"




// Sets default values
AChunkMesh::AChunkMesh()
{
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	SetRootComponent(ProceduralMesh);

	ProceduralMesh->SetCastShadow(false);
	
	ProceduralMesh->SetVisibility(true);

}


// Called when the game starts or when spawned
void AChunkMesh::BeginPlay()
{
	Super::BeginPlay();
}

void AChunkMesh::EndPlay(EEndPlayReason::Type EndPlayReason) {

	Super::EndPlay(EndPlayReason);
}

void AChunkMesh::CreateMesh(FMapData* mapData)
{

    ProceduralMesh->CreateMeshSection_LinearColor(
		0,
		mapData->Vertices,
		mapData->Triangles,
		mapData->Normals,
		mapData->UVs,
		mapData->VertexColor,
		mapData->Tangents,
		true
	);
	ProceduralMesh->SetMaterial(0, Material);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("done")));
}

void AChunkMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

