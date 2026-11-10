// 過去AHCメモを検査する。
// - 各回（## AHCxxx 名前 {#ahcxxx}）に、データのコメント（<!-- ahc: 長期 | 年-月 | 題材 | 解法 -->）と、
//   その直後の要点の段落があること
// - 「なぜ強いか」と「出典」の小見出しがあり、出典にリンクがあること（「未調査」の回は除く）
// - 分類のラベルとコロンで文をつなぐ書き方（「- 要点:」など）が残っていないこと
// - アンカーが番号と一致し、重複していないこと
// - 横断メモと一覧からのリンク先（/ahc/xxx-yyy#ahcNNN）が実在すること
import { readFileSync, readdirSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = join(dirname(fileURLToPath(import.meta.url)), '..')
const dir = join(root, 'docs', 'ahc')
const errors = []
const anchors = new Set() // "001-010#ahc001"
const seen = new Map()
const LABELS = /^\s*- (要点|問題|解法|なぜ強い|効かなかった|転用|出典|備考|未確認|検証):/m

const pages = readdirSync(dir).filter((f) => /^\d{3}-\d{3}\.md$/.test(f))
for (const file of pages) {
  const src = readFileSync(join(dir, file), 'utf8')
  const page = file.replace(/\.md$/, '')
  if (LABELS.test(src)) errors.push(`${file}: 分類のラベルとコロンの行が残っています「${src.match(LABELS)[0].trim()}」`)
  for (const block of src.split(/^## /m).slice(1)) {
    const head = block.match(/^AHC(\d+)\s+(.+?)\s*\{#(ahc\d+)\}/)
    if (!head) {
      errors.push(`${file}: 見出しの形が違います「${block.split('\n')[0]}」`)
      continue
    }
    const [, no, , id] = head
    if (id !== `ahc${no}`) errors.push(`${file}: アンカーと番号が一致しません AHC${no} {#${id}}`)
    if (seen.has(id)) errors.push(`${file}: ${id} が重複しています（${seen.get(id)}）`)
    seen.set(id, file)
    anchors.add(`${page}#${id}`)
    const [from, to] = page.split('-').map(Number)
    if (Number(no) < from || Number(no) > to) errors.push(`${file}: AHC${no} はこのページの範囲外です`)

    if (!/<!--\s*ahc:\s*(長期|短期)\s*\|\s*\d{4}-\d{2}\s*\|[^|]+\|[^|]+-->/.test(block)) errors.push(`${file}: AHC${no} にデータのコメントがありません`)
    const after = (block.split(/<!--[\s\S]*?-->/)[1] ?? '').trim()
    if (!after || after.startsWith('#')) errors.push(`${file}: AHC${no} に要点の段落がありません`)
    if (/^### 未調査/m.test(block)) continue
    if (!/^### なぜ強いか/m.test(block)) errors.push(`${file}: AHC${no} に「なぜ強いか」がありません`)
    const srcPart = block.split(/^### 出典/m)[1]
    if (!srcPart || !/\]\(https?:\/\//.test(srcPart)) errors.push(`${file}: AHC${no} に出典のリンクがありません`)
  }
}

for (const file of readdirSync(dir).filter((f) => f.endsWith('.md'))) {
  const src = readFileSync(join(dir, file), 'utf8')
  for (const m of src.matchAll(/\]\(\/ahc\/([^)#]+)#(ahc\d+)\)/g)) {
    const key = `${m[1]}#${m[2]}`
    if (!anchors.has(key)) errors.push(`${file}: リンク先がありません /ahc/${key}`)
  }
}

console.log(`過去AHC ${seen.size}回 / ${pages.length}ページ`)
if (errors.length) {
  console.error(errors.join('\n'))
  process.exit(1)
}
