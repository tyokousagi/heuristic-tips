<script setup lang="ts">
import { computed, ref } from 'vue'
import { withBase } from 'vitepress'
import { data } from './ahc.data'

const query = ref('')
const length = ref('')

// 空白区切りの語をすべて含む行だけ残す（題材・解法・要点・名前が対象）
const rows = computed(() => {
  const words = query.value.toLowerCase().split(/\s+/).filter(Boolean)
  return data.filter((e) => {
    if (length.value && e.length !== length.value) return false
    const text = `${e.name} ${e.topic} ${e.method} ${e.point}`.toLowerCase()
    return words.every((w) => text.includes(w))
  })
})
</script>

<template>
  <div class="ahc-filter">
    <input v-model="query" type="search" placeholder="絞り込み（例: 焼きなまし グリッド）" aria-label="絞り込み" />
    <select v-model="length" aria-label="期間">
      <option value="">長期・短期</option>
      <option value="長期">長期</option>
      <option value="短期">短期</option>
    </select>
    <span class="count">{{ rows.length }} / {{ data.length }}</span>
  </div>
  <table class="ahc-index">
    <thead>
      <tr>
        <th>回</th>
        <th>期間</th>
        <th>題材</th>
        <th>解法</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="e in rows" :key="e.id">
        <td class="no">
          <a :href="withBase(e.url)" :title="e.point">{{ String(e.no).padStart(3, '0') }}</a>
          <div class="name">{{ e.name }}</div>
        </td>
        <td class="len">{{ e.length }}<div class="month">{{ e.month }}</div></td>
        <td>{{ e.topic }}</td>
        <td>{{ e.method }}</td>
      </tr>
    </tbody>
  </table>
</template>
