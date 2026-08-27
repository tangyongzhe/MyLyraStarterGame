const fs = require('fs');
const path = require('path');
const assert = require('assert');

const rootDir = path.resolve(__dirname, '..', '..');
const tsconfigPath = path.join(rootDir, 'tsconfig.json');
const tsconfig = JSON.parse(fs.readFileSync(tsconfigPath, 'utf8'));
const declarationPath = path.join(rootDir, 'Typing', 'lyra-project.d.ts');
const declarationText = fs.readFileSync(declarationPath, 'utf8');

assert.ok(Array.isArray(tsconfig.exclude), 'tsconfig.json should define an exclude list');
assert.ok(
  tsconfig.exclude.includes('TypeScript/TsTemplates/**/*'),
  'tsconfig.json should exclude placeholder TsTemplates sources from compilation'
);
assert.ok(
  declarationText.includes('declare function require(moduleName: string): unknown;'),
  'project declaration should provide a require() typing for CommonJS module loading'
);
assert.ok(
  declarationText.includes('interface LyraGameInstance'),
  'project declaration should augment the LyraGameInstance type'
);
assert.ok(
  declarationText.includes('BindMixin(BindCallback: unknown): void;'),
  'project declaration should declare the BindMixin API exposed by the C++ game instance'
);

console.log('tsconfig build setup verified');
