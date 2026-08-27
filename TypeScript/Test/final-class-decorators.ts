import FinalClassLib = require('final-class');

type Constructor<T = object> = new (...args: any[]) => T;

type FinalMethodMetadata = {
    propertyKey: string | symbol;
};

const finalMethodRegistry = new WeakMap<Function, FinalMethodMetadata[]>();
const finalClassRegistry = new WeakSet<Function>();

function getOwnMethodNames(target: object): string[] {
    return Object.getOwnPropertyNames(target).filter((name) => name !== 'constructor');
}

function collectFinalMethods(ctor: Function): Array<string | symbol> {
    const inherited = new Set<string | symbol>();
    let current = Object.getPrototypeOf(ctor.prototype) as object | null;

    while (current && current !== Object.prototype) {
        const parentCtor = (current as { constructor?: Function }).constructor;
        const metadata = parentCtor ? finalMethodRegistry.get(parentCtor) : undefined;
        if (metadata) {
            for (const item of metadata) {
                inherited.add(item.propertyKey);
            }
        }
        current = Object.getPrototypeOf(current) as object | null;
    }

    return [...inherited];
}

function getParentConstructor(ctor: Constructor): Constructor | undefined {
    const parentPrototype = Object.getPrototypeOf(ctor.prototype) as { constructor?: Constructor } | null;
    const parentCtor = parentPrototype?.constructor;
    if (!parentCtor || parentCtor === Object) {
        return undefined;
    }
    return parentCtor;
}

function assertNoFinalOverrides(ctor: Constructor): void {
    const inheritedFinalMethods = collectFinalMethods(ctor);
    if (inheritedFinalMethods.length === 0) {
        return;
    }

    const ownMethods = new Set(getOwnMethodNames(ctor.prototype));
    const parentCtor = getParentConstructor(ctor);
    const parentName = parentCtor?.name ?? 'UnknownParent';

    for (const propertyKey of inheritedFinalMethods) {
        if (typeof propertyKey === 'string' && ownMethods.has(propertyKey)) {
            throw new Error(`Cannot override final method "${propertyKey}" in class "${parentName}"`);
        }
    }
}

function assertDoesNotExtendFinalClass(ctor: Constructor): void {
    const parentCtor = getParentConstructor(ctor);
    if (parentCtor && finalClassRegistry.has(parentCtor)) {
        throw new Error(`Cannot extend final class "${parentCtor.name}"`);
    }
}

function bindToFinalClassLibrary(ctor: Constructor): void {
    const parentCtor = getParentConstructor(ctor);

    FinalClassLib.create({
        className: ctor.name || 'AnonymousFinalClass',
        parents: parentCtor ? [parentCtor] : [],
        descriptor: {
            [ctor.name || 'AnonymousFinalClass']: {
                value: function FinalClassValidationBridge(this: object) {
                    return this;
                },
            },
        },
    });
}

export function Final(): MethodDecorator {
    return (target, propertyKey, descriptor) => {
        if (typeof descriptor?.value !== 'function') {
            throw new Error(`@Final can only be applied to methods: ${String(propertyKey)}`);
        }

        const ctor = (target as { constructor: Function }).constructor;
        const existing = finalMethodRegistry.get(ctor) ?? [];
        existing.push({ propertyKey });
        finalMethodRegistry.set(ctor, existing);
    };
}

export function FinalClass(): ClassDecorator;
export function FinalClass(isFinalClass: boolean): ClassDecorator;
export function FinalClass(isFinalClass = false): ClassDecorator {
    return <TFunction extends Function>(target: TFunction) => {
        const ctor = target as unknown as Constructor;

        bindToFinalClassLibrary(ctor);
        assertDoesNotExtendFinalClass(ctor);
        assertNoFinalOverrides(ctor);

        if (isFinalClass) {
            finalClassRegistry.add(ctor);
        }
    };
}
