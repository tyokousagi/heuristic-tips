<script setup lang="ts">
import { computed } from 'vue'
import sources from '../sources.json'

// tipsの出典を表示する。id はカンマ区切りで複数指定できる
const props = defineProps<{ id: string }>()

type Source = { id: string; author: string; title: string; url: string }
const byId = new Map((sources as Source[]).map((s) => [s.id, s]))

const list = computed(() =>
  props.id
    .split(',')
    .map((s) => s.trim())
    .filter(Boolean)
    .map((id) => byId.get(id) ?? { id, author: '', title: `未登録の出典 ${id}`, url: '' }),
)
</script>

<template>
  <p class="tip-src">
    出典:
    <template v-for="(s, i) in list" :key="s.id">
      <a v-if="s.url" :href="s.url" target="_blank" rel="noreferrer">{{ s.author }}「{{ s.title }}」</a>
      <span v-else>{{ s.title }}</span>
      <span v-if="i < list.length - 1">、</span>
    </template>
  </p>
</template>
