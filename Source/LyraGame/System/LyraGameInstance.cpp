// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameInstance.h"
#include "Extern/UeDownloadHelper.h"

#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "HAL/IConsoleManager.h"
#include "LyraGameplayTags.h"
#include "Misc/Paths.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraLocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Async/Async.h"
#include "JsEnv.h"
#include "PuertsSetting.h"
#include "PuertsAutoMixinModule.h"
#include "SourceFileWatcher.h"

#if UE_WITH_DTLS
#include "DTLSCertStore.h"
#include "DTLSHandlerComponent.h"
#include "Misc/FileHelper.h"
#endif // UE_WITH_DTLS

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameInstance)

namespace Lyra
{
	static bool bTestEncryption = false;
	static FAutoConsoleVariableRef CVarLyraTestEncryption(
		TEXT("Lyra.TestEncryption"),
		bTestEncryption,
		TEXT("If true, clients will send an encryption token with their request to join the server and attempt to encrypt the connection using a debug key. This is NOT SECURE and for demonstration purposes only."),
		ECVF_Default);

#if UE_WITH_DTLS
	static bool bUseDTLSEncryption = false;
	static FAutoConsoleVariableRef CVarLyraUseDTLSEncryption(
		TEXT("Lyra.UseDTLSEncryption"),
		bUseDTLSEncryption,
		TEXT("Set to true if using Lyra.TestEncryption and the DTLS packet handler."),
		ECVF_Default);

	/* Intended for testing with multiple game instances on the same device (desktop builds) */
	static bool bTestDTLSFingerprint = false;
	static FAutoConsoleVariableRef CVarLyraTestDTLSFingerprint(
		TEXT("Lyra.TestDTLSFingerprint"),
		bTestDTLSFingerprint,
		TEXT("If true and using DTLS encryption, generate unique cert per connection and fingerprint will be written to file to simulate passing through an online service."),
		ECVF_Default);

#if !UE_BUILD_SHIPPING
	static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(
		TEXT("GenerateDTLSCertificate"),
		TEXT("Generate a DTLS self-signed certificate for testing and export to PEM."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString> &InArgs, UWorld *InWorld)
															  {
				if (InArgs.Num() == 1)
				{
					const FString& CertName = InArgs[0];

					FTimespan CertExpire = FTimespan::FromDays(365);
					TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().CreateCert(CertExpire, CertName);
					if (Cert.IsValid())
					{
						const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / FPaths::MakeValidFileName(FString::Printf(TEXT("%s.pem"), *CertName));

						if (!Cert->ExportCertificate(CertPath))
						{
							UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to export certificate."));
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to generate certificate."));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Invalid argument(s)."));
				} }));
#endif // UE_BUILD_SHIPPING
#endif // UE_WITH_DTLS
};

ULyraGameInstance::ULyraGameInstance(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager *ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataAvailable, false, LyraGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataInitialized, false, LyraGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_GameplayReady, false, LyraGameplayTags::InitState_DataInitialized);
	}

	// Initialize the debug key with a set value for AES256. This is not secure and for example purposes only.
	DebugTestEncryptionKey.SetNum(32);

	for (int32 i = 0; i < DebugTestEncryptionKey.Num(); ++i)
	{
		DebugTestEncryptionKey[i] = uint8(i);
	}

	if (UCommonSessionSubsystem *SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
	}
}

void ULyraGameInstance::OnStart()
{
	Super::OnStart();

	// 二次启动：先挂载本地已下载的 CDN pak（PakOrder=100+，高于首包），
	// 保证 JsEnv 加载的是最新 Code，而非首包旧代码
	UUeDownloadHelper::GetInstance()->MountLocalCdnPaks();
	UE_LOG(LogTemp, Log, TEXT("[HotUpdate] OnStart: 本地 CDN pak 挂载完成，创建 JsEnv(8080)"));

	StartLyraScriptRuntime();
}

void ULyraGameInstance::Shutdown()
{
	if (UCommonSessionSubsystem *SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		SessionSubsystem->OnPreClientTravelEvent.RemoveAll(this);
	}

	UE_LOG(LogTemp, Log, TEXT("[LyraPuerts] Shutdown: JsEnv will be released with the game instance"));

	if (GameScript.IsValid())
	{
		GameScript.Reset();
	}

	if (SourceFileWatcher.IsValid())
	{
		SourceFileWatcher.Reset();
	}

	Super::Shutdown();
}

void ULyraGameInstance::StartLyraScriptRuntime()
{
#if WITH_EDITOR

	std::function<void(const FString&)> SourceLoadedCallback = nullptr;

	SourceFileWatcher = MakeShared<PUERTS_NAMESPACE::FSourceFileWatcher>(
		[this](const FString& InPath)
		{
			HotReloadJavaScriptEnv(InPath);
		});
	SourceLoadedCallback = [this](const FString& InPath)
		{
			if (SourceFileWatcher.IsValid())
			{
				SourceFileWatcher->OnSourceLoaded(InPath);
			}
		};
#endif

	if (GameScript.IsValid())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[LyraPuerts] StartLyraScriptRuntime skipped because JsEnv already exists"));
		return;
	}

	const UPuertsSetting& PuertsSettings = *GetDefault<UPuertsSetting>();
	const int32 DebugPort = PuertsSettings.DebugEnable ? PuertsSettings.DebugPort : -1;

	UE_LOG(LogTemp, Log, TEXT("[LyraPuerts] Creating JsEnv and starting entry module 'Start' (DebugEnable=%s, Port=%d, WaitDebugger=%s, Timeout=%.3f)"),
		PuertsSettings.DebugEnable ? TEXT("true") : TEXT("false"),
		DebugPort,
		PuertsSettings.WaitDebugger ? TEXT("true") : TEXT("false"),
		PuertsSettings.WaitDebuggerTimeout);
	GameScript = MakeShared<puerts::FJsEnv>(
		std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
		std::make_shared<puerts::FDefaultLogger>(),
		DebugPort,
		[](const FString& ScriptUrl)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[LyraPuerts] Source loaded: %s"), *ScriptUrl);
		});

	if (PuertsSettings.WaitDebugger)
	{
		UE_LOG(LogTemp, Log, TEXT("[LyraPuerts] Waiting for debugger before Start(\"Start\")"));
		GameScript->WaitDebugger(PuertsSettings.WaitDebuggerTimeout);
	}

	TArray<TPair<FString, UObject *>> Arguments;
	Arguments.Emplace(TEXT("GameInstance"), this);

	GameScript->Start("Start", Arguments);
}

void ULyraGameInstance::BindMixin(const FPuertsAutoMixinDelegate& BindCallback)
{
	IPuertsAutoMixinModule& Module = FModuleManager::LoadModuleChecked<IPuertsAutoMixinModule>("PuertsAutoMixin");
	Module.RegisterBindDelegate(GameScript, BindCallback);
}

void ULyraGameInstance::RestartJsEnv()
{
	UE_LOG(LogTemp, Log, TEXT("[LyraPuerts] RestartJsEnv requested"));

	// 先解绑旧 JsEnv 的 Tick 委托，防止广播到已销毁对象
	NotifyUpdate.Clear();

	AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<ULyraGameInstance>(this)]()
			  {
		if (!WeakThis.IsValid())
		{
			return;
		}

		ULyraGameInstance* Self = WeakThis.Get();
		// 仅回收 UMG Widget UObject，不调用 RemoveAllViewportWidgets（会清除 SGameLayerManager
		// 导致 AddToViewport 失效）。Slate Widget 树由 JS 侧 UIManager.destroyLayer 清理。
		CollectGarbage(RF_NoFlags);
		CollectGarbage(RF_NoFlags);

		// 销毁旧 JsEnv（v8 隔离区，require 缓存清空，所有 JS 均为最新）
		Self->GameScript.Reset();
		UE_LOG(LogTemp, Log, TEXT("[HotUpdate] RestartJsEnv: 旧 JsEnv 已销毁，重建新虚拟机"));

		// 按 OnStart 相同参数重建 JsEnv，新虚拟机加载已挂载的最新 Code
		const UPuertsSetting& PuertsSettings = *GetDefault<UPuertsSetting>();
		const int32 DebugPort = PuertsSettings.DebugEnable ? PuertsSettings.DebugPort : -1;
		Self->GameScript = MakeShared<puerts::FJsEnv>(
			std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
			std::make_shared<puerts::FDefaultLogger>(),
			DebugPort,
			[](const FString& ScriptUrl)
			{
				UE_LOG(LogTemp, Verbose, TEXT("[LyraPuerts] Source loaded: %s"), *ScriptUrl);
			});

		if (PuertsSettings.WaitDebugger)
		{
			UE_LOG(LogTemp, Log, TEXT("[LyraPuerts] Waiting for debugger before RestartJsEnv Start(\"Start\")"));
			Self->GameScript->WaitDebugger(PuertsSettings.WaitDebuggerTimeout);
		}

		TArray<TPair<FString, UObject*>> Arguments;
		Arguments.Add(TPair<FString, UObject*>(TEXT("GameInstance"), Self));
		Self->GameScript->Start("Start", Arguments);
		UE_LOG(LogTemp, Log, TEXT("[HotUpdate] RestartJsEnv: 新 JsEnv 已 Start(\"Start\")，最新 Code 生效")); });
}

int32 ULyraGameInstance::GetNetworkConnectionStatus() const
{
	return static_cast<int32>(FGenericPlatformMisc::GetNetworkConnectionStatus());
}

bool ULyraGameInstance::IsEditorEnvironment() const
{
	return GIsEditor;
}

bool ULyraGameInstance::IsDebugPackage() const
{
	// 打包类型由打包面板在整包前固化到 DefaultGame.ini 的 [Lyra] IsDebugPackage：
	// Debug 包=true，Release 包=false。无配置（旧包）回退 false，与 Release 行为一致。
	int32 IsDebugPackage = 0;
	GConfig->GetInt(TEXT("Lyra"), TEXT("IsDebugPackage"), IsDebugPackage, GGameIni);
	return IsDebugPackage == 1;
}

void ULyraGameInstance::Tick(float DeltaTime)
{
	GameDeltaTime = DeltaTime;
	NotifyUpdate.Broadcast();
}

bool ULyraGameInstance::IsTickable() const
{
	return IsValid(GetWorld());
}

TStatId ULyraGameInstance::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULyraGameInstance, STATGROUP_Tickables);
}

ALyraPlayerController *ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}

bool ULyraGameInstance::CanJoinRequestedSession() const
{
	// Temporary first pass:  Always return true
	// This will be fleshed out to check the player's state
	if (!Super::CanJoinRequestedSession())
	{
		return false;
	}
	return true;
}

void ULyraGameInstance::HandlerUserInitialized(const UCommonUserInfo *UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	// If login succeeded, tell the local player to load their settings
	if (bSuccess && ensure(UserInfo))
	{
		ULyraLocalPlayer *LocalPlayer = Cast<ULyraLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex));

		// There will not be a local player attached to the dedicated server user
		if (LocalPlayer)
		{
			LocalPlayer->LoadSharedSettingsFromDisk();
		}
	}
}

void ULyraGameInstance::ReceivedNetworkEncryptionToken(const FString &EncryptionToken, const FOnEncryptionKeyResponse &Delegate)
{
	// This is a simple implementation to demonstrate using encryption for game traffic using a hardcoded key.
	// For a complete implementation, you would likely want to retrieve the encryption key from a secure source,
	// such as from a web service over HTTPS. This could be done in this function, even asynchronously - just
	// call the response delegate passed in once the key is known. The contents of the EncryptionToken is up to the user,
	// but it will generally contain information used to generate a unique encryption key, such as a user and/or session ID.

	FEncryptionKeyResponse Response(EEncryptionResponse::Failure, TEXT("Unknown encryption failure"));

	if (EncryptionToken.IsEmpty())
	{
		Response.Response = EEncryptionResponse::InvalidToken;
		Response.ErrorMsg = TEXT("Encryption token is empty.");
	}
	else
	{
#if UE_WITH_DTLS
		if (Lyra::bUseDTLSEncryption)
		{
			TSharedPtr<FDTLSCertificate> Cert;

			if (Lyra::bTestDTLSFingerprint)
			{
				// Generate server cert for this identifier, post the fingerprint
				FTimespan CertExpire = FTimespan::FromHours(4);
				Cert = FDTLSCertStore::Get().CreateCert(CertExpire, EncryptionToken);
			}
			else
			{
				// Load cert from disk for testing purposes (never in production)
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("LyraTest.pem");

				Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);

				if (!Cert.IsValid())
				{
					Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
				}
			}

			if (Cert.IsValid())
			{
				if (Lyra::bTestDTLSFingerprint)
				{
					// Fingerprint should be posted to a secure web service for discovery
					// Writing to disk for local testing
					TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();

					FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");

					FString FingerprintStr = BytesToHex(Fingerprint.GetData(), Fingerprint.Num());
					FFileHelper::SaveStringToFile(FingerprintStr, *DebugFile);
				}

				// Server currently only needs the identifier
				Response.EncryptionData.Identifier = EncryptionToken;
				Response.EncryptionData.Key = DebugTestEncryptionKey;

				Response.Response = EEncryptionResponse::Success;
			}
			else
			{
				Response.Response = EEncryptionResponse::Failure;
				Response.ErrorMsg = TEXT("Unable to obtain certificate.");
			}
		}
		else
#endif // UE_WITH_DTLS
		{
			Response.Response = EEncryptionResponse::Success;
			Response.EncryptionData.Key = DebugTestEncryptionKey;
		}
	}

	Delegate.ExecuteIfBound(Response);
}

void ULyraGameInstance::ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse &Delegate)
{
	// This is a simple implementation to demonstrate using encryption for game traffic using a hardcoded key.
	// For a complete implementation, you would likely want to retrieve the encryption key from a secure source,
	// such as from a web service over HTTPS. This could be done in this function, even asynchronously - just
	// call the response delegate passed in once the key is known.

	FEncryptionKeyResponse Response;

#if UE_WITH_DTLS
	if (Lyra::bUseDTLSEncryption)
	{
		Response.Response = EEncryptionResponse::Failure;

		APlayerController *const PlayerController = GetFirstLocalPlayerController();

		if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
		{
			const FUniqueNetIdRepl &PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();

			// Ideally the encryption token is passed in directly rather than having to attempt to rebuild it
			const FString EncryptionToken = PlayerUniqueId.ToString();

			Response.EncryptionData.Identifier = EncryptionToken;

			// Server's fingerprint should be pulled from a secure service
			if (Lyra::bTestDTLSFingerprint)
			{
				// But for testing purposes...
				FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");
				FString FingerprintStr;
				FFileHelper::LoadFileToString(FingerprintStr, *DebugFile);

				Response.EncryptionData.Fingerprint.AddUninitialized(FingerprintStr.Len() / 2);
				HexToBytes(FingerprintStr, Response.EncryptionData.Fingerprint.GetData());
			}
			else
			{
				// Pulling expected fingerprint from disk for testing, this should come from a secure service
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("LyraTest.pem");

				TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);
				if (!Cert.IsValid())
				{
					Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
				}

				if (Cert.IsValid())
				{
					TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();

					Response.EncryptionData.Fingerprint = Fingerprint;
				}
				else
				{
					Response.Response = EEncryptionResponse::Failure;
					Response.ErrorMsg = TEXT("Unable to obtain certificate.");
				}
			}

			Response.EncryptionData.Key = DebugTestEncryptionKey;

			Response.Response = EEncryptionResponse::Success;
		}
	}
	else
#endif // UE_WITH_DTLS
	{
		Response.Response = EEncryptionResponse::Success;
		Response.EncryptionData.Key = DebugTestEncryptionKey;
	}

	Delegate.ExecuteIfBound(Response);
}

void ULyraGameInstance::OnPreClientTravelToSession(FString &URL)
{
	// Add debug encryption token if desired.
	if (Lyra::bTestEncryption)
	{
#if UE_WITH_DTLS
		if (Lyra::bUseDTLSEncryption)
		{
			APlayerController *const PlayerController = GetFirstLocalPlayerController();

			if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
			{
				const FUniqueNetIdRepl &PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();
				const FString EncryptionToken = PlayerUniqueId.ToString();

				URL += TEXT("?EncryptionToken=") + EncryptionToken;
			}
		}
		else
#endif // UE_WITH_DTLS
		{
			// This is just a value for testing/debugging, the server will use the same key regardless of the token value.
			// But the token could be a user ID and/or session ID that would be used to generate a unique key per user and/or session, if desired.
			URL += TEXT("?EncryptionToken=1");
		}
	}
}

void ULyraGameInstance::HotReloadJavaScriptEnv(const FString& Path)
{
	if (GameScript.IsValid())
	{
		TArray<uint8> Source;
		if (FFileHelper::LoadFileToArray(Source, *Path))
		{
			UE_LOG(LogTemp, Display, TEXT("read file success for %s"), *Path);
			GameScript->ReloadSource(Path, puerts::PString(reinterpret_cast<const char*>(Source.GetData()), Source.Num()));
			UE_LOG(LogTemp, Display, TEXT("read file success for %s"), *Path);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("read file fail for %s"), *Path);
		}
	}
}
