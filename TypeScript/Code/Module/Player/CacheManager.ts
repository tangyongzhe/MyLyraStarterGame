import { IManager } from "../../../Mono/Core/Manager/IManager";
import { JsonHelper } from "../../../Mono/Helper/JsonHelper";
import * as UE from 'ue';

declare const puerts: {
    loadUEType(typeName: string): unknown;
};

export class CacheManager implements IManager {

    private static _instance: CacheManager;

    public static get instance(): CacheManager {
        return CacheManager._instance;
    }

    private cacheObj: Map<string, any>;
    private playerPrefs!: typeof UE.LyraPlayerPrefs;

    public init() {
        const lyraPlayerPrefs = puerts.loadUEType("LyraPlayerPrefs") as typeof UE.LyraPlayerPrefs | undefined;
        if (!lyraPlayerPrefs) {
            throw new Error("LyraPlayerPrefs type is unavailable. Regenerate Puerts bindings or ensure the C++ class is loaded before CacheManager.init().");
        }

        this.playerPrefs = lyraPlayerPrefs;
        this.cacheObj = new Map<string, any>();
        this.playerPrefs.LoadSaveGame();
        CacheManager._instance = this;
    }

    public destroy() {
        CacheManager._instance = null;
    }

    public getString(key: string, defaultValue: string = null): string {
        return this.playerPrefs.GetString(key, defaultValue);
    }

    public getInt(key: string, defaultValue: number = 0): number {
        return this.playerPrefs.GetInt(key, defaultValue);
    }

    public getValue<T extends object>(type: new (...args: any[]) => T, key: string): T {
        let data: any = this.cacheObj.get(key);
        if (!!data) {
            return data as T;
        }
        const jStr = this.playerPrefs.GetString(key, null);
        if (jStr == null) return null;
        const res = JsonHelper.fromJson<T>(type, jStr);
        this.cacheObj.set(key, res);
        return res;
    }

    public setString(key: string, value: string) {
        this.playerPrefs.SetString(key, value);
    }

    public setInt(key: string, value: number) {
        this.playerPrefs.SetInt(key, value);
    }

    public setValue<T extends object>(key: string, value: T) {
        this.cacheObj.set(key, value);
        const jStr = JsonHelper.toJson(value);
        this.playerPrefs.SetString(key, jStr);
    }

    public deleteKey(key: string) {
        if (this.cacheObj.has(key)) {
            this.cacheObj.delete(key);
        }
        this.playerPrefs.Delete(key);
    }
}