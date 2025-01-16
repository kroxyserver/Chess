// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Core/ChessPlayer.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AChessPlayer::AChessPlayer()
{
	// DefaultSceneRootComponent
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRootComponent"));
	SetRootComponent(DefaultSceneRootComponent);

	// SpringArmComponent
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeRotation(FRotator(-65.f, 0.f, 0.f));
	SpringArmComponent->TargetArmLength = 3000.f;
	SpringArmComponent->bDoCollisionTest = false;

	// CameraComponent
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
}

void AChessPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AChessPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChessPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}