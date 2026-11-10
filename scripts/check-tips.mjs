// tipsページを検査する。
// - すべてのtips（### 見出し）に <Src id="..."> があること
// - 出典のidが docs/.vitepress/sources.json に登録されていること
// - 同じ見出しのtipsが重複していないこと
// - 使われていない出典を知らせること
import { readFileSync, readdirSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = join(dirname(fileURLToPath(import.meta.url)), '..')
const tipsDir = join(root, 'docs', 'tips')
const sources = JSON.parse(readFileSync(join(root, 'docs', '.vitepress', 'sources.json'), 'utf8'))
const known = new Set(sources.map((s) => s.id))

const errors = []
const used = new Map()
const titles = new Map()
let total = 0

for (const file of readdirSync(tipsDir).filter((f) => f.endsWith('.md') && f !== 'index.md')) {
  const src = readFileSync(join(tipsDir, file), 'utf8')
  const blocks = src.split(/^### /m).slice(1)
  for (const block of blocks) {
    total++
    const title = block.split('\n')[0].trim()
    if (titles.has(title)) errors.push(`${file}: 見出しが重複しています「${title}」（${titles.get(title)}）`)
    titles.set(title, file)
    const ids = [...block.matchAll(/<Src id="([^"]+)"/g)].flatMap((m) => m[1].split(',').map((s) => s.trim()))
    if (ids.length === 0) errors.push(`${file}: 出典がありません「${title}」`)
    for (const id of ids) {
      if (!known.has(id)) errors.push(`${file}: 未登録の出典 ${id}「${title}」`)
      used.set(id, (used.get(id) ?? 0) + 1)
    }
  }
  console.log(`${String(blocks.length).padStart(4)}  ${file}`)
}

const ids = sources.map((s) => s.id)
const dup = ids.filter((id, i) => ids.indexOf(id) !== i)
for (const id of dup) errors.push(`sources.json: idが重複しています ${id}`)

const unused = sources.filter((s) => !used.has(s.id)).map((s) => s.id)
console.log(`tips ${total}件 / 出典 ${sources.length}件（tipsで使用 ${used.size}件）`)
if (unused.length) console.log(`tipsで使っていない出典: ${unused.join(', ')}`)

if (errors.length) {
  console.error(errors.join('\n'))
  process.exit(1)
}
