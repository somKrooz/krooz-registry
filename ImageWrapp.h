// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "KroozCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;

class UTimelineComponent;
class UCurveFloat;


UCLASS()
class SATSUJIN_API AKroozCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKroozCharacter();

protected:
	virtual void BeginPlay() override;
	FTimerHandle TimerHandle;
	virtual void BeginDestroy() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Dashing();
	void StopDashing();

	UPROPERTY(EditAnywhere , BlueprintReadWrite)	
	TObjectPtr<USkeletalMeshComponent> FPS;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	TObjectPtr<UCameraComponent> Camera;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	
	void Around(float Value);
	void Turn(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> KroozServer;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ImageServer;
	
	// We Are Not Using This Shit AnyMore
	UPROPERTY(Replicated)
	FString URL = "https://cdn.waifu.im/6664.jpeg";
	
	
	void Firing();
	
	//Replicate Fire Montage
	UFUNCTION(Server, Reliable)
	void Server_PlayFireMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireMontage();
	
	// Replicate Actor Change
	UFUNCTION(Server, Reliable)
	void Server_HitActorData(const FString &thing , AActor* Actor);

	UPROPERTY(EditAnywhere , BlueprintReadWrite , Replicated)
	TWeakObjectPtr<AActor> WeChangedHim;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HitActorData(const FString &thing, AActor* Actor);

	// This Keeps Track Of The Replicated Properties
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Texture
	void ApiRequest(const FString &thing, AActor* actor);
	void CreateTexture(const TArray<uint8> &data ,AActor* actor);

// Update Some Value Overtime
public:
	UTexture2D* LastCreatedTexture;
	
	UFUNCTION()
	void TimelineUpdate(float Value);

	UFUNCTION()
	void TimelineFinished();
	
	UFUNCTION()
	void StartTimeline();

	
	
	UPROPERTY()
	UTimelineComponent* MyTimeline;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* CurveFloat;


	UFUNCTION()
	void CamUpdate(float Value);
	
	bool bCanFire;


	UPROPERTY()
	UTimelineComponent* CameraTimeline;

		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* CurveCam;
	
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	float ChangedValue;

	
};
