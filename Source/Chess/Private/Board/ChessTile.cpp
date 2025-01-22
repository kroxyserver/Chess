// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessTile.h"

#include "Core/ChessPlayerController.h"

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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChessTileMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Assets/Meshes/SM_ChessTile.SM_ChessTile'"));
	if (ChessTileMeshAsset.Succeeded()) ChessTileMesh->SetStaticMesh(ChessTileMeshAsset.Object);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TileMaterialAsset(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/+Chess/Materials/ChessTile/M_ChessTile.M_ChessTile'"));
	if (TileMaterialAsset.Succeeded()) TileMaterial = TileMaterialAsset.Object;
}

void AChessTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (TileMaterial)
	{
		TileMaterialInstanceDynamic = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, TileMaterial);
		if (TileMaterialInstanceDynamic)
		{
			ChessTileMesh->SetMaterial(0, TileMaterialInstanceDynamic);

			TileMaterialInstanceDynamic->SetVectorParameterValue("Colour", (ChessTileInfo.bIsWhite) ? FLinearColor::White : FLinearColor::Black);
		}
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
	if (bHighlight)
		TileMaterialInstanceDynamic->SetVectorParameterValue("Colour", FLinearColor::Green);
	else
		TileMaterialInstanceDynamic->SetVectorParameterValue("Colour", (ChessTileInfo.bIsWhite) ? FLinearColor::White : FLinearColor::Black);

	ChessTileInfo.bIsHighlighted = bHighlight;
}