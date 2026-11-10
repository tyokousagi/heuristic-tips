import { createContentLoader } from 'vitepress'

export interface TipsData {
  total: number
  pages: { url: string; title: string; count: number }[]
  bySource: Record<string, number>
}

declare const data: TipsData
export { data }

// サイドバーと同じ並びにするための順序
const ORDER = [
  'thinking', 'greedy', 'sa-basic', 'sa-neighbor', 'sa-temp', 'sa-speed', 'sa-misc',
  'beam', 'search-other', 'interactive', 'evaluation', 'speed', 'data-structures',
  'topics', 'testing', 'time', 'debug',
]

export default createContentLoader('tips/*.md', {
  includeSrc: true,
  transform(raw): TipsData {
    const bySource: Record<string, number> = {}
    const pages = raw
      .filter((p) => !/\/tips\/(index)?$/.test(p.url.replace(/\.html$/, '')))
      .map((p) => {
        const src = p.src ?? ''
        // tips は「### 見出し」から次の見出しまでを1件と数える
        const blocks = src.split(/^### /m).slice(1)
        for (const b of blocks) {
          const ids = new Set<string>()
          for (const m of b.matchAll(/<Src id="([^"]+)"/g)) {
            for (const id of m[1].split(',')) ids.add(id.trim())
          }
          for (const id of ids) bySource[id] = (bySource[id] ?? 0) + 1
        }
        const title = (src.match(/^# (.+)$/m)?.[1] ?? p.url).trim()
        return { url: p.url, title, count: blocks.length }
      })
      .sort((a, b) => rank(a.url) - rank(b.url))
    return { total: pages.reduce((s, p) => s + p.count, 0), pages, bySource }
  },
})

function rank(url: string): number {
  const name = url.replace(/\.html$/, '').split('/').pop() ?? ''
  const i = ORDER.indexOf(name)
  return i < 0 ? ORDER.length : i
}
