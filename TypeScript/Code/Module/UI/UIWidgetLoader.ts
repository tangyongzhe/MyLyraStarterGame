import UE = require('ue');
import { Log } from '../../../Mono/Module/Log/Log';

function tryResolveGeneratedClass(widgetAsset: UE.Object, assetPath: string): UE.Class | null {
    // GetClass() describes the blueprint asset itself, not its generated widget class.
    if (widgetAsset instanceof UE.BlueprintCore && widgetAsset.GeneratedClass) {
        return widgetAsset.GeneratedClass;
    }

    Log.warning(`Loaded widget asset has no generated class: ${assetPath}`);
    return null;
}

export function loadWidgetClass(path: string): UE.Class | null {
    const widgetClass = UE.Class?.Load?.(path);
    if (widgetClass) {
        return widgetClass;
    }

    Log.warning(`UE.Class.Load failed while loading widget class: ${path}`);

    const assetPath = path.endsWith("_C")
        ? path.slice(0, -2).replace(/\.([^./]+)$/, '')
        : path;
    const widgetAsset = UE.Object?.Load?.(assetPath);
    if (!widgetAsset) {
        Log.warning(`UE.Object.Load failed while loading widget asset: ${assetPath}`);
        return null;
    }

    return tryResolveGeneratedClass(widgetAsset, assetPath);
}
