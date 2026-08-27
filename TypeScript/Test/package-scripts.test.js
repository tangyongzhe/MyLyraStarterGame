const fs = require('fs');
const path = require('path');
const assert = require('assert');

const packageJsonPath = path.resolve(__dirname, '..', '..', 'package.json');
const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));

assert.ok(packageJson.scripts, 'package.json should define scripts');
assert.strictEqual(
  packageJson.scripts.build,
  'npx tsc -p tsconfig.json',
  'package.json should expose npm run build for TypeScript compilation'
);
assert.strictEqual(
  packageJson.scripts.typecheck,
  'npx tsc -p tsconfig.json --noEmit',
  'package.json should expose npm run typecheck for no-emit verification'
);

console.log('package scripts verified');
