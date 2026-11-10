// スニペットを検査する。
// - snippets/*.hpp が単独でコンパイルできること（ほかのファイルに依存しないこと）
// - snippets/template.cpp がコンパイルできること
// - snippets/tests/*.cpp をコンパイルして実行し、すべて成功すること
// - snippets/run_tests.py が構文として正しいこと（Python が見つかる場合）
import { spawnSync } from 'node:child_process'
import { mkdirSync, readdirSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = join(dirname(fileURLToPath(import.meta.url)), '..')
const dir = join(root, 'snippets')
const build = join(root, 'build')
mkdirSync(build, { recursive: true })

const FLAGS = ['-std=c++20', '-O2', '-Wall', '-Wextra']
let failed = 0

function run(label, cmd, args) {
  const r = spawnSync(cmd, args, { encoding: 'utf8' })
  const ok = r.status === 0
  console.log(`${ok ? 'ok  ' : 'FAIL'}  ${label}`)
  if (!ok) {
    failed++
    console.log((r.stdout ?? '') + (r.stderr ?? '') + (r.error ? String(r.error) : ''))
  } else if (r.stdout?.trim()) {
    console.log(r.stdout.trim().split('\n').map((l) => `      ${l}`).join('\n'))
  }
  return ok
}

for (const f of readdirSync(dir).filter((f) => f.endsWith('.hpp')).sort()) {
  run(`compile ${f}`, 'g++', [...FLAGS, '-fsyntax-only', '-x', 'c++', join(dir, f)])
}
run('compile template.cpp', 'g++', [...FLAGS, '-fsyntax-only', join(dir, 'template.cpp')])

for (const f of readdirSync(join(dir, 'tests')).filter((f) => f.endsWith('.cpp')).sort()) {
  const exe = join(build, f.replace(/\.cpp$/, process.platform === 'win32' ? '.exe' : ''))
  if (run(`build   tests/${f}`, 'g++', [...FLAGS, '-o', exe, join(dir, 'tests', f)])) {
    run(`run     tests/${f}`, exe, [])
  }
}

const pythons = process.platform === 'win32' ? [['py', '-3'], ['python']] : [['python3'], ['python']]
const py = pythons.find(([cmd, ...pre]) => spawnSync(cmd, [...pre, '--version']).status === 0)
if (py) {
  const [cmd, ...pre] = py
  run('compile run_tests.py', cmd, [...pre, '-m', 'py_compile', join(dir, 'run_tests.py')])
} else {
  console.log('skip  run_tests.py（Python が見つかりません）')
}

if (failed) {
  console.error(`${failed} 件の検査が失敗しました`)
  process.exit(1)
}
console.log('スニペットの検査はすべて成功しました')
