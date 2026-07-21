// Copyright © 2026 Gazda-dev. All Rights Reserved.


#include "Gameplay/Actors/Characters/BaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/MPTestTaskGameMode.h"
#include "Core/MPTestTaskPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Gameplay/Components/InteractionComponent.h"
#include "Gameplay/Components/WeaponComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "UI/HealthBarWidget.h"

DEFINE_LOG_CATEGORY(LogPlayer);

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(GetMesh());
	HealthBarWidget->SetRelativeLocation(FVector(0.0, 0.0, 120.0));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawSize(FVector2D(130.0, 20.0));
	
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(20.f);
	
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	GetMesh()->bEnableUpdateRateOptimizations = true;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthComponent)
	{
		if (HasAuthority())
		{
			HealthComponent->OnDamagedServer.AddUObject(this, &ABaseCharacter::HandleDamagedServer);
			HealthComponent->OnDeath.AddDynamic(this, &ABaseCharacter::HandleDeathServer);	
		}
		
		HealthComponent->OnDeath.AddDynamic(this, &ABaseCharacter::HandleDeathCosmetic);
		
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABaseCharacter::HandleHealthChanged);
		HandleHealthChanged(nullptr
			, HealthComponent->GetHealth()
			, 0.f
			, nullptr
			, nullptr);
	}
}

void ABaseCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(!IsLocallyControlled());
	}
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (GetController())
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (GetController())
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABaseCharacter::DebugDamageSelf(float Amount)
{
	if (HasAuthority())
	{
		UGameplayStatics::ApplyDamage(this
			, Amount
			, GetController()
			, this
			, UDamageType::StaticClass());
	}
}

void ABaseCharacter::Fire()
{
	WeaponComponent->TryFire();
}

void ABaseCharacter::Interact()
{
	InteractionComponent->TryInteract();
}

void ABaseCharacter::HandleDamagedServer(float Amount, AController* InstigatedBy)
{
	if (!IsValid(InstigatedBy) || InstigatedBy == GetController())
	{
		return;
	}
	
	if (AMPTestTaskPlayerState* AttackerPlayerState = InstigatedBy->GetPlayerState<AMPTestTaskPlayerState>())
	{
		AttackerPlayerState->AddDamageDealt(Amount);
	}
}

void ABaseCharacter::HandleDeathServer(UHealthComponent* InHealthComponent
	, AController* InstigatedBy
	, AActor* DamageCauser)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	if (IsValid(InstigatedBy))
	{
		if (const AMPTestTaskPlayerState* PS = InstigatedBy->GetPlayerState<AMPTestTaskPlayerState>())
		{
			UE_LOG(LogPlayer, Display, TEXT("Player died %s"), *PS->GetPlayerName());
		}
	}
	
	if (AMPTestTaskGameMode* GameMode = World->GetAuthGameMode<AMPTestTaskGameMode>())
	{
		GameMode->NotifyPlayerDied(GetController(), InstigatedBy);
	}
}

void ABaseCharacter::HandleDeathCosmetic(UHealthComponent* InHealthComponent, AController* InstigatedBy,
	AActor* DamageCauser)
{
	//dedicated server has no local viewer -> skip
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this
			, DeathSound
			, GetActorLocation());
	}
	else
	{
		UE_LOG(LogPlayer, Error, TEXT("Death sound not set %s"), *GetName());
	}
	
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		GetCharacterMovement()->DisableMovement();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
	}
	
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void ABaseCharacter::HandleHealthChanged(UHealthComponent* InHealthComponent
	, float NewHealth
	, float Delta
	, AController* InstigatedBy
	, AActor* DamageCauser)
{
	if (!HealthBarWidget)
	{
		return;
	}
	
	UHealthBarWidget* HealthBar = Cast<UHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!IsValid(HealthBar))
	{
		return;
	}
	
	const float MaxP = FMath::Max(HealthComponent->GetMaxHealth(), 1.f);
	HealthBar->UpdateHealthPercent(FMath::Clamp(NewHealth / MaxP, 0.f, 1.f));
}


void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
		
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABaseCharacter::Fire);
		
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABaseCharacter::Interact);
	}
}

