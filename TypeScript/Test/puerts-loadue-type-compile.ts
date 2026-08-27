import * as UE from 'ue';
import { FinalClass, Final } from './final-class-decorators';

declare const puerts: {
    loadUEType(typeName: string): unknown;
};

const LyraPlayerPrefs = puerts.loadUEType("LyraPlayerPrefs") as typeof UE.LyraPlayerPrefs;

export function verifyPuertsLoadUETypeCompile(): typeof UE.LyraPlayerPrefs {
    return LyraPlayerPrefs;
}

export function verifyLyraPlayerPrefsApiShape(): number {
    LyraPlayerPrefs.LoadSaveGame();
    return LyraPlayerPrefs.GetInt("Automation.IntKey", 7);
}

let overrideErrorMessage = '';
let finalClassErrorMessage = '';

@FinalClass()
class BaseEntity {
    @Final()
    public Destroy(): void {
        console.log("BaseEntity 销毁");
    }

    public Update(deltaTime: number): void {
        console.log(`BaseEntity 帧更新 ${deltaTime}`);
    }
}

try {
    @FinalClass()
    class HeroEntity extends BaseEntity {
        public Destroy(): void {
            console.log("HeroEntity 销毁（非法重写）");
        }

        public Update(deltaTime: number): void {
            console.log(`HeroEntity 帧更新 ${deltaTime}`);
        }
    }

    new HeroEntity();
} catch (error) {
    overrideErrorMessage = (error as Error).message;
    console.error(overrideErrorMessage);
}

@FinalClass(true)
class GameGlobalUtils {
    public static GetGameTime(): number {
        return Date.now();
    }
}

try {
    @FinalClass()
    class ExtendedUtils extends GameGlobalUtils {
        public static GetAccurateTime(): number {
            return Date.now();
        }
    }

    void ExtendedUtils;
} catch (error) {
    finalClassErrorMessage = (error as Error).message;
    console.error(finalClassErrorMessage);
}

export function verifyFinalClassDecoratorRuntime(): string {
    return `${overrideErrorMessage} | ${finalClassErrorMessage}`;
}
