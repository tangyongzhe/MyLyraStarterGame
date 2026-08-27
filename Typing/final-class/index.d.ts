declare module 'final-class' {
    type FinalClassConstructor = new (...args: any[]) => any;

    interface FinalClassCreateOptions {
        className?: string;
        parents?: FinalClassConstructor[];
        tagName?: string;
        descriptor?: PropertyDescriptorMap;
    }

    interface FinalClassStatic {
        create(options: FinalClassCreateOptions): FinalClassConstructor;
    }

    const FinalClass: FinalClassStatic;
    export = FinalClass;
}
