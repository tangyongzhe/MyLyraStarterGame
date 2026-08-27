declare function require(moduleName: string): unknown;

declare module "ue" {
    interface LyraGameInstance {
        BindMixin(BindCallback: unknown): void;
    }
}
