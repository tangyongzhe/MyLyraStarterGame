// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"
#include "Tickable.h"
#include "PuertsAutoMixinLibrary.h"
#include "SourceFileWatcher.h"

#include "LyraGameInstance.generated.h"

#define UE_API LYRAGAME_API

namespace puerts
{
	class FJsEnv;
}

class ALyraPlayerController;
class UObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyUpdate);

UCLASS(MinimalAPI, Config = Game)
class ULyraGameInstance : public UCommonGameInstance, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UE_API ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API ALyraPlayerController* GetPrimaryPlayerController() const;
	
	UE_API virtual bool CanJoinRequestedSession() const override;
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	UE_API virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	UE_API virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;
	UE_API virtual void OnStart() override;
	UE_API virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "MetaGame|MetaGameInstance")
	void BindMixin(const FPuertsAutoMixinDelegate& BindCallback);

	/**
	 * 重启整个 JS 虚拟机（Puerts JsEnv）。
	 * 热更下载新 Code 后调用：先解绑 Tick 委托，再异步销毁旧 JsEnv 并以相同参数重建、
	 * 重新 Start("Start")，新虚拟机 require 缓存清空、加载已挂载的最新 Code。
	 * 必须异步执行（JS 调用栈 unwind 后），同步销毁会崩溃。
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|JsEnv")
	UE_API void RestartJsEnv();

	/**
	 * 获取当前网络连接状态（通过 FGenericPlatformMisc::GetNetworkConnectionStatus()）。
	 * 返回 ENetworkConnectionStatus：0=Unknown 1=Disabled 2=Local 3=Connected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Network")
	int32 GetNetworkConnectionStatus() const;

	/**
	 * 当前是否编辑器环境（GIsEditor：编辑器内运行/PIE 为 true，打包游戏为 false）。
	 * TS 层 Define.IsEditor() 由此获取，替代原先的硬编码常量。
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Environment")
	bool IsEditorEnvironment() const;

	/**
	 * 当前包是否为打包面板选择的 Debug 包（读取 DefaultGame.ini 的 [Lyra] IsDebugPackage，
	 * 由打包面板整包前固化：Debug 包=true，Release 包=false，无配置/旧包回退 false）。
	 * TS 层 Define.Debug 由此获取，用于调试模式（记忆服务器/跳过更新检查等）。
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Environment")
	bool IsDebugPackage() const;

protected:

	UE_API virtual void Init() override;
	UE_API virtual void Tick(float DeltaTime) override;
	UE_API virtual bool IsTickable() const override;
	UE_API virtual TStatId GetStatId() const override;

	UE_API void OnPreClientTravelToSession(FString& URL);
	UE_API void StartLyraScriptRuntime();

	void HotReloadJavaScriptEnv(const FString& Path);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;

	UPROPERTY()
	FNotifyUpdate NotifyUpdate;

	UPROPERTY()
	float GameDeltaTime = 0.0f;
	TSharedPtr<puerts::FJsEnv> GameScript;

#if WITH_EDITOR
	TSharedPtr<PUERTS_NAMESPACE::FSourceFileWatcher> SourceFileWatcher;
#endif
};

#undef UE_API
