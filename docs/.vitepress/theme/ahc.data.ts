import { createContentLoader } from 'vitepress'

export interface AhcEntry {
  id: string // ahc001
  no: number
  name: string
  url: string // /ahc/001-010#ahc001
  length: string // 長期 / 短期
  month: string // 2021-03
  topic: string
  method: string
  point: string
}

declare const data: AhcEntry[]
export { data }

// 各ページの「## AHC001 名前 {#ahc001}」と、その下のコメント
// 「<!-- ahc: 長期 | 2021-03 | 題材 | 解法 -->」、続く最初の段落（要点）から一覧を作る
export default createContentLoader('ahc/[0-9]*.md', {
  includeSrc: true,
  transform(raw): AhcEntry[] {
    const list: AhcEntry[] = []
    for (const page of raw) {
      const src = page.src ?? ''
      const base = page.url.replace(/\.html$/, '')
      for (const block of src.split(/^## /m).slice(1)) {
        const head = block.match(/^AHC(\d+)\s+(.+?)\s*\{#(ahc\d+)\}/)
        if (!head) continue
        const meta = (block.match(/<!--\s*ahc:\s*(.+?)\s*-->/)?.[1] ?? '').split('|').map((s) => s.trim())
        // コメントの次の段落を要点とする
        const after = block.split(/<!--[\s\S]*?-->/)[1] ?? ''
        const point = (after.trim().split(/\n\s*\n/)[0] ?? '').replace(/\s+/g, ' ').trim()
        list.push({
          id: head[3],
          no: Number(head[1]),
          name: head[2],
          url: `${base}#${head[3]}`,
          length: meta[0] ?? '',
          month: meta[1] ?? '',
          topic: meta[2] ?? '',
          method: meta[3] ?? '',
          point: point.startsWith('#') ? '' : point,
        })
      }
    }
    return list.sort((a, b) => a.no - b.no)
  },
})
