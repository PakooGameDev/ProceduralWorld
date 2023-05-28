// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessGenerator.h"


// Sets default values
AEndlessGenerator::AEndlessGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEndlessGenerator::BeginPlay()
{
	Super::BeginPlay();
	//Continentalness
	Continentalness = new FastNoiseLite();
	SetupNoise(Continentalness, worldSeed, ContinentalnessFrequency, ContinentalnessOctaves);
	Continentalness->SetFractalType(FastNoiseLite::FractalType_FBm);
	//Erosian
	Erosian = new FastNoiseLite();
	SetupNoise(Erosian, worldSeed + 7281, ErosianFrequency, ErosianOctaves);
	Erosian->SetFractalType(FastNoiseLite::FractalType_FBm);
	//Peaks
	Peaks = new FastNoiseLite();
	SetupNoise(Peaks, worldSeed + 666, PeaksFrequency, PeakesOctaves);
	Peaks->SetFractalType(FastNoiseLite::FractalType_FBm);

	GenerateChunks();
	//UE_LOG(LogTemp, Warning, TEXT("Started"));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("debug %f"), chunkSize));
}

// Called every frame
void AEndlessGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CreateClassAndMesh();

	if (!CoordsChanged()) {
		LastCoords.X = GetCurrentCoords().X;
		LastCoords.Y = GetCurrentCoords().Y;
		//UpdateChunkVisibility();
		GenerateChunks();
	}
}

void AEndlessGenerator::SetupNoise(FastNoiseLite* noise, int seed, float frequency, int octaves)
{
	noise->SetSeed(seed);
	noise->SetFrequency(frequency);
	noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise->SetFractalOctaves(octaves);
}

void AEndlessGenerator::GenerateChunks() 
{
	FIntVector ChunkCoords;
	TArray<FVector> Locations;
	FVector PlayerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	
	int32 verticesPerLine = FMath::RoundToInt(chunkSize / GridSpacing) + 1;
	
	/* generate chunk locations and coords in the main thread */

	for (int32 x = -RenderRange; x <= RenderRange; x++) {
		for (int32 y = -RenderRange; y <= RenderRange; y++) {		

			ChunkCoords.X = x + GetCurrentCoords().X;
			ChunkCoords.Y = y + GetCurrentCoords().Y;

			FVector chunkLoc = FVector(ChunkCoords.X * chunkSize, ChunkCoords.Y * chunkSize, 0.f);

			float ChunkVectorLength = FVector(x * chunkSize, y * chunkSize, 0.f).Size();
			float RenderRangeRadius = chunkSize * RenderRange;
			float DistanceToChunk = FVector::Dist(chunkLoc, PlayerLoc);


			if (!MapCache.Contains(chunkLoc) && (ChunkVectorLength <= RenderRangeRadius)) {
				Locations.Add(chunkLoc);
			}

		}
	}

	Locations.Sort([&](const FVector& A, const FVector& B) {
		return FVector::Dist(A, PlayerLoc) < FVector::Dist(B, PlayerLoc);
	});
	
	for (int32 i = 0; i < Locations.Num(); i++)
	{
		FMapData* mapData = new FMapData();
		MapCache.Add(Locations[i], mapData);
	//	RenderedChunks.Add(Locations[i]);
		SpawnQueue.Enqueue(Locations[i]);
	}

	/*lock access to specific data and blocks any tries to change data*/
	FCriticalSection Mutex; 

	/* generate mesh data in other threads inside thread pool */
	ParallelFor(Locations.Num(), [&](int32 Idx) {
			
		auto loc = Locations[Idx]; 

		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector2D> UVs;
		TArray<FLinearColor> VertexColor;
		TArray<FVector> Normals;
		TArray<struct FProcMeshTangent> Tangents;

		int32 Vertex = 0;

		int32 meshSimplificationIncrement = (levelOfDetail == 0) ? 1 : levelOfDetail * 2;
	//	int verticesPerLineL = (verticesPerLine - 1) / meshSimplificationIncrement + 1;// RENAME

		for (int32 X = 0; X <= verticesPerLine; X += meshSimplificationIncrement)
		{
			for (int32 Y = 0; Y <= verticesPerLine; Y += meshSimplificationIncrement)
			{

				float PosX = loc.X + X * GridSpacing;
				float PosY = loc.Y + Y * GridSpacing;
				float height = CalculateHeight(PosX, PosY);

				if (height < 0) {
					VertexColor.Add(FLinearColor(0.0f, 0.0f, 1.0f, 1.0f));
					
				}
				else {
					VertexColor.Add(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f));
				}

				Vertices.Add(FVector(X * GridSpacing, Y * GridSpacing, height));
				UVs.Add(FVector2D(X * UVScale, Y  * UVScale));

				if (X < verticesPerLine - 1 && Y < verticesPerLine - 1) {
					Triangles.Add(Vertex);
					Triangles.Add(Vertex + verticesPerLine + 2);
					Triangles.Add(Vertex + verticesPerLine + 1);
					Triangles.Add(Vertex + verticesPerLine + 2);
					Triangles.Add(Vertex);
					Triangles.Add(Vertex + 1);
				}
				Vertex++;
			}
		}

		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);
				
		Mutex.Lock();
			MapCache[loc]->Vertices = Vertices;
			MapCache[loc]->Triangles = Triangles;
			MapCache[loc]->UVs = UVs;
			MapCache[loc]->VertexColor = VertexColor;
			MapCache[loc]->Normals = Normals;
			MapCache[loc]->Tangents = Tangents;
		Mutex.Unlock();
		
	}); 
}


/*
void AEndlessGenerator::WriteToFile(FVector location)
{
	FString VectorAsName = FString::Printf(TEXT("%f_%f_%f.txt"), location.X, location.Y, location.Z);
	std::ofstream OutFile;
	OutFile.open(TCHAR_TO_UTF8(*VectorAsName));

	if (OutFile.is_open())
	{

		FMapData* meshData = MapCache[location];
		// Записываем строку в файл
		OutFile << "\n";
		// Закрываем файл
		OutFile.close();
	}
}



void AEndlessGenerator::ReadFromFile(FVector location)
{
	FString VectorAsName = FString::Printf(TEXT("%f_%f_%f.txt"), location.X, location.Y, location.Z);
	std::ifstream InFile;
	InFile.open(TCHAR_TO_UTF8(*VectorAsName));

	if (InFile.is_open())
	{
		// Читаем файл построчно
		std::string Line;
		while (std::getline(InFile, Line))
		{
			// чет делаем с Line
		}
		// Закрываем файл
		InFile.close();
	}
}
*/

/*
void AEndlessGenerator::WriteToCacheFile(const FString& Filename, const FMapData* Data)
{
	FString AbsoluteFilePath = FPaths::ProjectSavedDir() / Filename; // получаем абсолютный путь до кэш файла

	TArray<uint8> DataArray; // создаем массив байт для записи в файл
	
	UStruct::Serialize();
		
		
	Serialize();

	Data->Vertices;
	Triangles;
	UVs;
	VertexColor;
	Normals;
	Tangents;
	

	const TCHAR* DataRaw = *Data; // конвертируем строку в массив TCHAR

	int32 DataSize = FCString::Strlen(DataRaw); // определяем размер данных
	DataArray.SetNum(DataSize); // устанавливаем размер массива

	FMemory::Memcpy(DataArray.GetData(), DataRaw, DataSize); // копируем данные в массив

	FFileHelper::SaveArrayToFile(DataArray, *AbsoluteFilePath); // записываем массив в кэш файл
}

FString AEndlessGenerator::ReadFromCacheFile(const FString & Filename)
{
	FString AbsoluteFilePath = FPaths::ProjectSavedDir() / Filename; // получаем абсолютный путь до кэш файла

	TArray<uint8> DataArray; // создаем массив байт для чтения из файла
	FFileHelper::LoadFileToArray(DataArray, *AbsoluteFilePath); // читаем данные из кэш файла

	const TCHAR* DataRaw = reinterpret_cast<const TCHAR*>(DataArray.GetData()); // приводим массив байт к массиву TCHAR
	FString Data(DataRaw); // создаем строку из массива TCHAR

	return Data;
}

void CacheMapData(FMapData& MapData, const FString& CacheFilePath)
{
	FBufferArchive BufferArchive;

	MapData.Serialize(BufferArchive);

	// Write buffer to file
	FFileHelper::SaveArrayToFile(BufferArchive, *CacheFilePath);
}

bool LoadCachedMapData(FMapData& OutMapData, const FString& CacheFilePath)
{
	TArray<uint8> Data;

	if (!FFileHelper::LoadFileToArray(Data, *CacheFilePath))
	{
		return false;
	}

	FMemoryReader MemoryReader(Data, true);
	MemoryReader.Seek(0);

	OutMapData.Serialize(MemoryReader);

	return true;
}


class FMyArchive : public FArchive
{
public:
	FMyArchive Ar( additional parameters if needed );

	MyStruct MyObject;

	MyObject.Serialize(Ar);

	//MyObject.Serialize(Ar, EArchive::ET_Load);

};


WriteToCacheFile("mycachefile.txt", "Hello, Cache!"); // записываем данные в кэш файл
FString CachedData = ReadFromCacheFile("mycachefile.txt"); // получаем данные из кэш файла
UE_LOG(LogTemp, Log, TEXT("Cached Data: %s"), *CachedData); // выводим полученные данные
*/


void AEndlessGenerator::UpdateChunkVisibility()
{


	/*FVector PlayerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	float DistanceToChunk = FVector::Dist(chunkLoc, PlayerLoc);
	float RenderRangeRadius = chunkSize * RenderRange;

	if (DistanceToChunk > RenderRangeRadius) {
		ChunksVisibility.Add(false);
	}
	else {
		ChunksVisibility.Add(true);
	}*/
	//RenderedChunks.Add(spawnedActor);
	//spawnedActor->ProceduralMesh->SetVisibility(ChunksVisibility[Counter]);
	//spawnedActor->ProceduralMesh->SetHiddenInGame(!ChunksVisibility[Counter]);
	//RenderedChunks[Counter]->ProceduralMesh->SetVisibility(ChunksVisibility[Counter]);
	//RenderedChunks[Counter]->ProceduralMesh->SetHiddenInGame(!ChunksVisibility[Counter]);
}

void AEndlessGenerator::CreateClassAndMesh() {
	FVector QueueItem;
	if (!SpawnQueue.IsEmpty() && SpawnQueue.Dequeue(QueueItem)) {
		spawnedActor = GetWorld()->SpawnActor<AChunkMesh>(Chunk, QueueItem, FRotator(0.f, 0.f, 0.f));
		if (spawnedActor != nullptr) {
			spawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			spawnedActor->CreateMesh(MapCache[QueueItem]);
			//RenderedChunks[QueueItem] = spawnedActor;
		}
	}
}	

//helping functions

float AEndlessGenerator::CalculateHeight(float X, float Y)
{
	float c = Continentalness->GetNoise(X, Y) / 2.0f + 0.5f;
	float e = Erosian->GetNoise(X, Y) / 2.0f + 0.5f;
	float p = Peaks->GetNoise(X, Y) / 2.0f + 0.5f;
	if ((c > 0.85f || c < 0.15f) || (e > 0.85f || e < 0.15f) || (p > 0.85f || p < 0.15f)) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("C:%f, E:%f, PV:%f"), c, e, p));
	}
	return CCurve->GetFloatValue(c) * ContinentalnessHeightMultiplicator + ECurve->GetFloatValue(e) * ErosianHeightMultiplicator + PCurve->GetFloatValue(p) * PeaksHeightMultiplicator;
}

bool AEndlessGenerator::CoordsChanged() {
	return LastCoords.X == GetCurrentCoords().X && LastCoords.Y == GetCurrentCoords().Y;
}

FIntVector AEndlessGenerator::GetCurrentCoords() {
	FVector player = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	int32 X = FMath::RoundToInt(player.X / chunkSize);
	int32 Y = FMath::RoundToInt(player.Y / chunkSize);
	
	return FIntVector(X, Y, 0);
}

