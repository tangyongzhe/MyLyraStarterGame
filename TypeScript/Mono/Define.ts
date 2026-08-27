import * as UE from 'ue';

export class Define {

    public static Game: UE.LyraGameInstance;
    public static DeltaTime: number;

    private static readonly dWidth = 768;
    private static readonly dHeight = 1366;

    public static ScreenWidth: number;
    public static ScreenHeight: number;

    public static readonly DesignScreenWidth =
        Define.ScreenWidth > Define.ScreenHeight ? Math.max(Define.dWidth, Define.dHeight) : Math.min(Define.dWidth, Define.dHeight);
    public static readonly DesignScreenHeight =
        Define.ScreenWidth > Define.ScreenHeight ? Math.min(Define.dWidth, Define.dHeight) : Math.max(Define.dWidth, Define.dHeight);
    public static LogLevel = 1;

    public static Process = 1;

    public static readonly MinRepeatedTimerInterval: number = 100;

    public static get IsEditor(): boolean {
        return Define.Game ? Define.Game.IsEditorEnvironment() : false;
    }

    public static get Debug(): boolean {
        return Define.IsEditor || (Define.Game ? Define.Game.IsDebugPackage() : false);
    }

    public static get Networked(): boolean {
        const Status = Define.Game ? Define.Game.GetNetworkConnectionStatus() : 0;
        return Status !== 1;
    }

    public static readonly ForceUpdate = true;

    public static isSH: boolean = false;
}
