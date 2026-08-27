# LyraStarterGame Tools

## Excel toolchain

Migrated into and adapted for the LyraStarterGame directory layout.

### Prerequisites
- .NET SDK installed
- Excel workbook sources under `LyraStarterGame/Excel`
- Built tool output under `LyraStarterGame/Bin`

### Build tools
```powershell
dotnet build .\Tools\ExcelExport\ExcelExport.csproj -nologo
dotnet build .\Tools\FileServer\FileServer.csproj -nologo
```

### Run from `LyraStarterGame/Excel`
- Export all:
  - `win_startExportAll.bat`
- Export config JSON/classes only:
  - `win_startExcelExport.bat`
- Export i18n constants/json:
  - `win_startI18NExport.bat`
- Export numeric attribute constants:
  - `win_startAttrExport.bat`
- Export one target workbook:
  - `win_startExportTarget.bat <full-or-relative-xlsx-path>`

### Generated outputs
- Config TypeScript classes:
  - `TypeScript/Code/Module/Generate/Config`
- Config JSON:
  - `Content/AssetsPackage/Config`
- I18N constants:
  - `TypeScript/Code/Module/Const/LangType.ts`
  - `TypeScript/Code/Module/Const/I18NKey.ts`
- Attribute constants:
  - `TypeScript/Code/Module/Const/NumericType.ts`

### Notes
- `AttrExporter` requires an `AttributeConfig*.xlsx` workbook in `Excel/`.
- The current Lyra seed workbook is `Excel/AttributeConfig@c.xlsx` and exists to keep the full export chain operational.
- Tool source still contains legacy internal namespaces from the migrated codebase; runtime behavior has been adapted to LyraStarterGame paths.
