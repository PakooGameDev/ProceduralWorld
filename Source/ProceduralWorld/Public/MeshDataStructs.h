// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "MeshDataStructs.generated.h"


USTRUCT()
struct FMapData
{
	GENERATED_BODY()
	

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColor;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;

	FORCEINLINE FMapData();

	explicit FORCEINLINE FMapData(
		TArray<FVector> Vertices,
		TArray<int32> Triangles,
		TArray<FVector2D> UVs,
		TArray<FLinearColor> VertexColor,
		TArray<FVector> Normals,
		TArray<struct FProcMeshTangent> Tangents
	);

	void Serialize(FArchive& Archive);
};
/*
void FMapData::Serialize(FArchive& Archive)
{
	Archive << Vertices;
	Archive << Triangles;
	Archive << UVs;
	Archive << VertexColor;
	Archive << Normals;
	Archive << Tangents;
}
*/
FORCEINLINE FMapData::FMapData() {};

FORCEINLINE FMapData::FMapData(
	TArray<FVector> Vertices,
	TArray<int32> Triangles,
	TArray<FVector2D> UVs,
	TArray<FLinearColor> VertexColor,
	TArray<FVector> Normals,
	TArray<struct FProcMeshTangent> Tangents
) : Vertices(Vertices), Triangles(Triangles), UVs(UVs), VertexColor(VertexColor), Normals(Normals), Tangents(Tangents)
{}



