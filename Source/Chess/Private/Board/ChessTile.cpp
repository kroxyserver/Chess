// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessTile.h"

#include "Core/ChessPlayerController.h"
#include "Data/ChessBoardData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

	// HighlightFX
	HighlightFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HighlightFX"));
	HighlightFX->SetupAttachment(ChessTileMesh);
	HighlightFX->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<UChessBoardData> ChessBoardDataAsset(TEXT("/Script/Chess.ChessBoardData'/Game/+Chess/Data/DA_ChessBoardData.DA_ChessBoardData'"));
	if (ChessBoardDataAsset.Succeeded()) ChessBoardData = ChessBoardDataAsset.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChessTileMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Assets/Meshes/ChessTile/SM_ChessTile.SM_ChessTile'"));
	if (ChessTileMeshAsset.Succeeded()) ChessTileMesh->SetStaticMesh(ChessTileMeshAsset.Object);

	//static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HighlightFXAsset(TEXT("/Script/Niagara.NiagaraSystem'/Game/+Chess/Niagara/NS_ChessTile_Highlight.NS_ChessTile_Highlight'"));
	//if (HighlightFXAsset.Succeeded()) HighlightFX->SetAsset(HighlightFXAsset.Object);
}

void AChessTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ChessBoardData)
	{
		if (ChessTileInfo.bIsWhite)
		{
			if (ChessBoardData->WhiteTileMaterial.LoadSynchronous()) TileMaterial = ChessBoardData->WhiteTileMaterial.LoadSynchronous();
		}
		else
		{
			if (ChessBoardData->BlackTileMaterial.LoadSynchronous()) TileMaterial = ChessBoardData->BlackTileMaterial.LoadSynchronous();
		}

		if (TileMaterial)
		{
			TileMaterialInstanceDynamic = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, TileMaterial);
			if (TileMaterialInstanceDynamic)
			{
				for (int32 ElementIndex = 0; ElementIndex < ChessTileMesh->GetMaterials().Num(); ElementIndex++)
					ChessTileMesh->SetMaterial(ElementIndex, TileMaterialInstanceDynamic);

				TileMaterialInstanceDynamic->SetScalarParameterValue("RotationAngle", FMath::FRand());
			}
		}

		if (ChessBoardData->ValidMovesHighlightFX)
		{
			HighlightFX->SetAsset(ChessBoardData->ValidMovesHighlightFX);
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
	ChessTileInfo.bIsHighlighted = bHighlight;

	if (!HighlightFX) return PRINTSTRING(FColor::Red, "HighlightFX Invalid");

	if (bHighlight)
	{
		HighlightFX->Activate();
	}
	else
	{
		HighlightFX->Deactivate();
	}
}