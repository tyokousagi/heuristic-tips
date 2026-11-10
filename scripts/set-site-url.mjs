// スニペットの先頭コメントにある公開先URLを書き換える。
// 使い方: node scripts/set-site-url.mjs https://ユーザー名.github.io/リポジトリ名
// 最初は __SITE_URL__ という仮の値が入っている。2回目からは、前に設定したURLを新しいURLに置き換える。
import { existsSync, readFileSync, readdirSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = join(dirname(fileURLToPath(import.meta.url)), '..')
const dir = join(root, 'snippets')
const record = join(root, '.site-url')

const next = (process.argv[2] ?? '').replace(/\/+$/, '')
if (!/^https?:\/\//.test(next)) {
  console.error('公開先のURLを http:// か https:// から指定してください')
  process.exit(1)
}
const prev = existsSync(record) ? readFileSync(record, 'utf8').trim() : '__SITE_URL__'

let changed = 0
for (const f of readdirSync(dir)) {
  if (!/\.(hpp|cpp|py)$/.test(f)) continue
  const path = join(dir, f)
  const src = readFileSync(path, 'utf8')
  const out = src.split(`${prev}/snippets/`).join(`${next}/snippets/`)
  if (out !== src) {
    writeFileSync(path, out)
    changed++
  }
}
writeFileSync(record, next + '\n')
console.log(`${changed} 個のファイルのURLを ${next} に書き換えました`)
