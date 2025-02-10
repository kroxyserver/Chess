// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessTile.h"

#include "Core/ChessPlayerController.h"
#include "Data/ChessBoardData.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessTile::AChessTile()
{
	PrimaryActorTick.bCanEverTick = true;

	// DefaultSceneRootComponent
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRootComponent"));
	SetRootComponent(DefaultSceneRootComponent);

	// ChessTileMesh
	ChessTileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChessTileMesh"));
	ChessTileMesh->SetupAttachment(DefaultSceneRootComponent);
	ChessTileMesh->SetCollisionProfileName("ChessTile");

	// ChessTileHighlightMesh
	ChessTileHighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChessTileHighlightMesh"));
	ChessTileHighlightMesh->SetupAttachment(DefaultSceneRootComponent);
	ChessTileHighlightMesh->SetCollisionProfileName("ChessTile");
	ChessTileHighlightMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UChessBoardData> ChessBoardDataAsset(TEXT("/Script/Chess.ChessBoardData'/Game/+Chess/Data/DA_ChessBoardData.DA_ChessBoardData'"));
	if (ChessBoardDataAsset.Succeeded()) ChessBoardData = ChessBoardDataAsset.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChessTileMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Assets/Meshes/ChessTile/SM_ChessTile.SM_ChessTile'"));
	if (ChessTileMeshAsset.Succeeded()) ChessTileMesh->SetStaticMesh(ChessTileMeshAsset.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChessTileHighlightMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Assets/Meshes/ChessTile/SM_ChessTile_HighlightBorder.SM_ChessTile_HighlightBorder'"));
	if (ChessTileHighlightMeshAsset.Succeeded()) ChessTileHighlightMesh->SetStaticMesh(ChessTileHighlightMeshAsset.Object);
}

void AChessTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ChessBoardData)
	{
		if (ChessTileInfo.bIsWhite)
		{
			if (ChessBoardData->WhiteTileMaterial) TileMaterial = ChessBoardData->WhiteTileMaterial;
		}
		else
		{
			if (ChessBoardData->BlackTileMaterial) TileMaterial = ChessBoardData->BlackTileMaterial;
		}

		if (TileMaterial)
		{
			TileMaterialInstanceDynamic = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, TileMaterial);
			if (TileMaterialInstanceDynamic)
			{
				ChessTileMesh->SetMaterial(0, TileMaterialInstanceDynamic);

				TileMaterialInstanceDynamic->SetScalarParameterValue("RotationAngle", FMath::FRand());
			}
		}

		TileHighlightMaterial = ChessBoardData->TileHighlightMaterial;
	}
}

void AChessTile::BeginPlay()
{
	Super::BeginPlay();
}

void AChessTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChessTile::HighlightTile(bool bHighlight)
{
	if (ChessTileHighlightMesh) ChessTileHighlightMesh->SetVisibility(bHighlight);
	
	ChessTileInfo.bIsHighlighted = bHighlight;
}