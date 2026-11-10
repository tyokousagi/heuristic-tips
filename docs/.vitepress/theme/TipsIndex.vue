<script setup lang="ts">
import { withBase } from 'vitepress'
import { data } from './tips.data'
</script>

<template>
  <table class="tips-index">
    <thead>
      <tr>
        <th>カテゴリ</th>
        <th class="num">tips</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="p in data.pages" :key="p.url">
        <td><a :href="withBase(p.url)">{{ p.title }}</a></td>
        <td class="num">{{ p.count }}</td>
      </tr>
      <tr>
        <td>合計</td>
        <td class="num">{{ data.total }}</td>
      </tr>
    </tbody>
  </table>
</template>
