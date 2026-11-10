import DefaultTheme from 'vitepress/theme'
import type { Theme } from 'vitepress'
import Src from './Src.vue'
import SourceTable from './SourceTable.vue'
import TipsIndex from './TipsIndex.vue'
import AhcIndex from './AhcIndex.vue'
import './custom.css'

export default {
  extends: DefaultTheme,
  enhanceApp({ app }) {
    app.component('Src', Src)
    app.component('SourceTable', SourceTable)
    app.component('TipsIndex', TipsIndex)
    app.component('AhcIndex', AhcIndex)
  },
} satisfies Theme
