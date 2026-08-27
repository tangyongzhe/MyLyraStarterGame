const fs = require('fs');
const path = require('path');
const assert = require('assert');

const rootDir = path.resolve(__dirname, '..', '..');
const adapterPath = path.join(rootDir, 'TypeScript', 'Test', 'final-class-decorators.ts');
const compileTestPath = path.join(rootDir, 'TypeScript', 'Test', 'puerts-loadue-type-compile.ts');
const finalClassDeclarationPath = path.join(rootDir, 'Typing', 'final-class', 'index.d.ts');

const adapterSource = fs.readFileSync(adapterPath, 'utf8');
const compileTestSource = fs.readFileSync(compileTestPath, 'utf8');
const finalClassDeclaration = fs.readFileSync(finalClassDeclarationPath, 'utf8');

assert.ok(
  adapterSource.includes("import FinalClassLib = require('final-class');"),
  'decorator adapter should be implemented on top of the installed final-class CommonJS package'
);
assert.ok(
  adapterSource.includes('export function FinalClass(): ClassDecorator;'),
  'decorator adapter should expose the zero-argument FinalClass overload'
);
assert.ok(
  adapterSource.includes('export function FinalClass(isFinalClass: boolean): ClassDecorator;'),
  'decorator adapter should expose the boolean FinalClass overload for final classes'
);
assert.ok(
  adapterSource.includes('export function Final(): MethodDecorator'),
  'decorator adapter should export a Final method decorator factory'
);
assert.ok(
  adapterSource.includes('FinalClassLib.create({'),
  'decorator adapter should exercise final-class.create to stay grounded in the npm package API'
);
assert.ok(
  compileTestSource.includes("import { FinalClass, Final } from './final-class-decorators';"),
  'runtime test should consume the local decorator adapter instead of nonexistent final-class decorator exports'
);
assert.ok(
  compileTestSource.includes('@FinalClass()'),
  'runtime test should verify the class decorator syntax remains available'
);
assert.ok(
  compileTestSource.includes('@FinalClass(true)'),
  'runtime test should verify the final-class syntax remains available'
);
assert.ok(
  compileTestSource.includes('@Final()'),
  'runtime test should verify the method decorator syntax remains available'
);
assert.ok(
  compileTestSource.includes('Cannot override final method'),
  'runtime test should document the expected final-method override failure'
);
assert.ok(
  compileTestSource.includes('Cannot extend final class'),
  'runtime test should document the expected final-class inheritance failure'
);
assert.ok(
  !finalClassDeclaration.includes('Final():'),
  'final-class package declaration should not advertise a nonexistent Final export'
);

console.log('final-class decorator adapter verified');
