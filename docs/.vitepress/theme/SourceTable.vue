<script setup lang="ts">
import sources from '../sources.json'
import { data } from './tips.data'

type Source = { id: string; author: string; title: string; url: string; note?: string }
const rows = (sources as Source[]).map((s) => ({ ...s, count: data.bySource[s.id] ?? 0 }))
</script>

<template>
  <table class="source-table">
    <thead>
      <tr>
        <th>資料</th>
        <th>内容</th>
        <th class="num">tips</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="s in rows" :key="s.id">
        <td>
          <a :href="s.url" target="_blank" rel="noreferrer">{{ s.title }}</a>
          <div class="author">{{ s.author }}</div>
        </td>
        <td>{{ s.note }}</td>
        <td class="num">{{ s.count }}</td>
      </tr>
    </tbody>
  </table>
</template>
