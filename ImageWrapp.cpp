#include "KroozCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Async/Async.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "Components/TimelineComponent.h"
#include "Json.h"
#include "JsonUtilities.h"


AKroozCharacter::AKroozCharacter()
{
    PrimaryActorTick.bCanEverTick = true; 
    bReplicates = true;
    LastCreatedTexture = nullptr;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 0.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    CameraBoom->bUsePawnControlRotation = true;
    
    Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
    Camera->SetupAttachment(CameraBoom);
    Camera->SetFieldOfView(100.0f);
    
    FPS = CreateDefaultSubobject<USkeletalMeshComponent>("FPS");
    FPS->SetupAttachment(Camera);
    FPS->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    FPS->SetRelativeLocation(FVector(0.0f, 0.0f, -180.0f));

    GetCharacterMovement()->MaxWalkSpeed = 700.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    bUseControllerRotationYaw = true;

    MyTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
    CameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraCurve"));

    bCanFire = true;
    
}

void AKroozCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (CurveCam)
    {
        FOnTimelineFloat TimelineFloat;
        TimelineFloat.BindUFunction(this, "CamUpdate");
        CameraTimeline->AddInterpFloat(CurveCam , TimelineFloat);
        CameraTimeline->SetLooping(false);
        CameraTimeline->SetPlayRate(1.0f);

    }
    
    if (CurveFloat && MyTimeline)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineUpdate"));
        MyTimeline->AddInterpFloat(CurveFloat, TimelineProgress);


        FOnTimelineEvent TimelineEnd;
        TimelineEnd.BindUFunction(this, FName("TimelineFinished"));
        MyTimeline->SetTimelineFinishedFunc(TimelineEnd);
    }
}

void AKroozCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FVector Forward = GetActorForwardVector();
        AddMovementInput(Forward, Value);
        
        if (CameraTimeline && !CameraTimeline->IsPlaying() && !CameraTimeline->IsReversing())  
        {
            CameraTimeline->Play();  // Play normally instead of PlayFromStart()
        }
    }
    else if (Controller && Value == 0.0f)
    {
        if (CameraTimeline && !CameraTimeline->IsReversing() && GetMovementComponent()->Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
        {
            CameraTimeline->Reverse();
        }
    }
}

void AKroozCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FVector Right = GetActorRightVector();
        AddMovementInput(Right, Value);
    }
}

void AKroozCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AKroozCharacter::Around(float Value)
{
    if (Controller)
    {
        AddControllerPitchInput(Value);
    }
}

void AKroozCharacter::Turn(float Value)
{
    if (Controller)
    {
        AddControllerYawInput(Value);
    }
}

void AKroozCharacter::Dashing(){
    if (Controller){
        GetCharacterMovement()->MaxWalkSpeed = 850.0f;
    }
}

void AKroozCharacter::StopDashing(){
    if (Controller){
        GetCharacterMovement()->MaxWalkSpeed = 700.0f;
    }
}

// Replicated Fire
void AKroozCharacter::Firing()
{
    // Check if controller exists AND if player can fire
    if (!Controller || !bCanFire)
    {
        // Player cannot fire, so exit the function immediately
        return;
    }
     
    // If we reach here, player can fire, so set bCanFire to false
    bCanFire = false;
    
    UWorld* World = GetWorld();
    if (!World) return;

    FHitResult Hit;
    FVector StartLocation = GetActorLocation()+FVector(0.0,0.0,20.0);
    FVector ForwardDirection = Camera->GetForwardVector();

    FVector EndLocation = StartLocation + (ForwardDirection * 1200.0f);
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    bool HitResult = World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

    //Debug Lines
    FColor DebugColor = HitResult ? FColor::Red : FColor::Magenta;
    DrawDebugLine(World , StartLocation , EndLocation , DebugColor,false ,3.0f);

    if (HitResult && Hit.GetActor())
    {
        AActor* HitActor = Hit.GetActor();
        UMeshComponent* MeshComp = Cast<UMeshComponent>(HitActor->GetComponentByClass(UMeshComponent::StaticClass()));
        UMaterialInterface* Material = MeshComp->GetMaterial(0);
        UTexture* theTexture = nullptr;
        bool istexture  = Material->GetTextureParameterValue(FName("Texture"),theTexture);


        if (istexture)
        {
            KroozServer = FHttpModule::Get().CreateRequest();
            KroozServer->SetVerb("GET");
            KroozServer->SetURL("https://api.waifu.pics/sfw/waifu");
            KroozServer->OnProcessRequestComplete().BindLambda([this, HitActor](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
            {
                if (!bWasSuccessful || !Response.IsValid())
                {
                    return;
                }

                TSharedPtr<FJsonObject> JsonObject;
                FString ReceivedURL = Response->GetContentAsString();
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedURL);

                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    FString MainURL = JsonObject->GetStringField("url");
                    UE_LOG(LogTemp , Error , TEXT("URL: %s"), *MainURL);
            
                    if (HasAuthority())
                    {
                        Multicast_HitActorData(MainURL, HitActor);
                    }
                    else
                    {
                        Server_HitActorData(MainURL, HitActor);
                    }

                }
            });
                
        
            KroozServer->ProcessRequest();
        }
        }
    
    if (HasAuthority())
    {
        Multicast_FireMontage();
    }
    else
    {
        Server_PlayFireMontage();
    }
}

void AKroozCharacter::Server_PlayFireMontage_Implementation()
{
    Multicast_FireMontage();
}

void AKroozCharacter::Multicast_FireMontage_Implementation()
{
    if (FireMontage)
    {
        UAnimInstance* AnimInstance = FPS->GetAnimInstance();
        if (AnimInstance )
        {
            AnimInstance->Montage_Play(FireMontage, 1.0f);

            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
               {
                bCanFire = true;
               }, FireMontage->GetPlayLength(), false);

        }
    }
    else return;
    
}

void AKroozCharacter::Server_HitActorData_Implementation(const FString &Url, AActor* HitActor)
{
    if (IsValid(HitActor))
    {
        Multicast_HitActorData(Url , HitActor);
    }
}

void AKroozCharacter::Multicast_HitActorData_Implementation(const FString &Url , AActor* HitActor)
{
    if (IsValid(HitActor))
    {
        WeChangedHim = HitActor;
        ApiRequest(Url,HitActor);
    }
    
}

void AKroozCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AKroozCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AKroozCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &AKroozCharacter::Turn);
    PlayerInputComponent->BindAxis("Around", this, &AKroozCharacter::Around);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AKroozCharacter::Dashing);
    PlayerInputComponent->BindAction("Dash", IE_Released, this, &AKroozCharacter::StopDashing);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AKroozCharacter::Firing);

}

void AKroozCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // DOREPLIFETIME_CONDITION_NOTIFY(AKroozCharacter, URL, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(AKroozCharacter,WeChangedHim , COND_None, REPNOTIFY_OnChanged);

}

void AKroozCharacter::ApiRequest(const FString &url , AActor* actor)
{
    if (!IsValid(actor)) return;
    
    ImageServer = FHttpModule::Get().CreateRequest();
    ImageServer->SetURL(url);
    ImageServer->SetVerb("GET");
    ImageServer->OnProcessRequestComplete().BindLambda([this, actor](FHttpRequestPtr Request2, FHttpResponsePtr Response2, bool bWasSuccessful2)
    {
        if (!bWasSuccessful2) return;
        TArray<uint8> ResponseData = Response2->GetContent();
        CreateTexture(ResponseData, actor);
    });

    ImageServer->ProcessRequest();
            
}

void AKroozCharacter::CreateTexture(const TArray<uint8>& ImageData, AActor* actor)
{
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ImageData, actor]()
    {
        IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
        EImageFormat Format = Module.DetectImageFormat(ImageData.GetData(),ImageData.Num());
        
        switch (Format)
        {
            case EImageFormat::PNG:
                break;
            case EImageFormat::JPEG:
                break;
            default:
                UE_LOG(LogTemp, Warning, TEXT("Not Suported Image Format"));
                break;
        }

        TSharedPtr<IImageWrapper> ImageWrapper = Module.CreateImageWrapper(Format);

        if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
        {
            TArray<uint8> UncompressedBGRA;
            if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
            {
                int32 Width = ImageWrapper->GetWidth();
                int32 Height = ImageWrapper->GetHeight();

                if (Width <= 0 || Height <= 0) return; 

                UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height,PF_B8G8R8A8);
                if (NewTexture && NewTexture->GetPlatformData())
                {
                    NewTexture->SRGB = true;
                    NewTexture->CompressionSettings = TC_Default;
                    NewTexture->MipGenSettings = TMGS_NoMipmaps;
                    NewTexture->LODGroup = TEXTUREGROUP_World;
                    NewTexture->Filter = TF_Default;
                    NewTexture->AddToRoot();
                    NewTexture->LODBias = 1;
                    
                    NewTexture->NeverStream = false;
                    NewTexture->LODGroup = TEXTUREGROUP_World;
                    

                    // Lock texture data
                    void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                    if (TextureData)
                    {
                        FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
                        NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
                    }

                    // Move to GameThread and handle both clearing and setting in sequence
                    AsyncTask(ENamedThreads::GameThread, [this, NewTexture, actor]()
                    {
                        if (!NewTexture || !actor) return;
                        
                        // Clean up previous texture before creating a new one
                        if (LastCreatedTexture)
                        {
                            LastCreatedTexture->RemoveFromRoot();
                            LastCreatedTexture->MarkAsGarbage();
                        }
                        
                        // Update to the new texture
                        LastCreatedTexture = NewTexture;
                        NewTexture->UpdateResource();

                        UMeshComponent* KroozMesh = actor->FindComponentByClass<UMeshComponent>();
                        if (!KroozMesh) return;

                        UMaterialInstanceDynamic* Mat = nullptr;
                        if (KroozMesh->GetMaterial(0) && KroozMesh->GetMaterial(0)->IsA(UMaterialInstanceDynamic::StaticClass()))
                        {
                            Mat = Cast<UMaterialInstanceDynamic>(KroozMesh->GetMaterial(0));
                        }
                        else
                        {
                            Mat = KroozMesh->CreateAndSetMaterialInstanceDynamic(0);
                        }

                        if (Mat)
                        {
                            Mat->SetTextureParameterValue("Texture", nullptr);
                            KroozMesh->MarkRenderStateDirty();
                            
                            StartTimeline();
                            Mat->SetTextureParameterValue("Texture", NewTexture);
                            Mat->SetScalarParameterValue("Dif", ChangedValue);
                        }
                    });
                }
            }
        }
    });
}

void AKroozCharacter::TimelineUpdate(float Value)
{
    ChangedValue = Value;
    UMaterialInstanceDynamic* Mat = nullptr;
    UMeshComponent* KroozMesh = WeChangedHim->FindComponentByClass<UMeshComponent>();
    
    if (KroozMesh->GetMaterial(0) && KroozMesh->GetMaterial(0)->IsA(UMaterialInstanceDynamic::StaticClass()))
    {
        Mat = Cast<UMaterialInstanceDynamic>(KroozMesh->GetMaterial(0));
    }
    else
    {
        Mat = KroozMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
    if (Mat)
    {
        Mat->SetScalarParameterValue("Dif", ChangedValue);   
    }
}


void AKroozCharacter::BeginDestroy()
{
    if (LastCreatedTexture)
    {
        LastCreatedTexture->RemoveFromRoot();
        LastCreatedTexture->MarkAsGarbage();
        LastCreatedTexture = nullptr;
    }
    
    if (KroozServer.IsValid())
    {
        if (KroozServer->GetStatus() != EHttpRequestStatus::NotStarted &&
        KroozServer->GetStatus() != EHttpRequestStatus::Failed)
        {
            KroozServer->CancelRequest();
        }
        
    }
    

    if (ImageServer.IsValid())
    {
        if (ImageServer->GetStatus() != EHttpRequestStatus::NotStarted &&
        ImageServer->GetStatus() != EHttpRequestStatus::Failed)
        {
            ImageServer->CancelRequest();
        }
       
    }
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    }

    WeChangedHim = nullptr;

    Super::BeginDestroy();
}


void AKroozCharacter::TimelineFinished()
{
    ChangedValue = 0;
}

void AKroozCharacter::StartTimeline()
{
    if (MyTimeline)
    {
        MyTimeline->PlayFromStart();
    }
}

void AKroozCharacter::CamUpdate(float Value)
{
    
   Camera->SetFieldOfView(Value);
    if (CameraTimeline->IsPlaying() && CameraTimeline->GetPlaybackPosition() >= CameraTimeline->GetTimelineLength())
    {
        CameraTimeline->Stop();
    }
}


