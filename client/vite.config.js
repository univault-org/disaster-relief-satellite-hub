import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import { VitePWA } from 'vite-plugin-pwa'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    react(),
    VitePWA({
      registerType: 'autoUpdate',
      includeAssets: ['favicon.ico', 'apple-touch-icon.png', 'masked-icon.svg'],
      manifest: {
        name: 'Disaster Relief Order System',
        short_name: 'Relief Orders',
        description: 'PWA for placing orders in disaster relief situations',
        theme_color: '#ffffff',
        icons: [
          {
            src: 'pwa-192x192.png',
            sizes: '192x192',
            type: 'image/png'
          },
          {
            src: 'pwa-512x512.png',
            sizes: '512x512',
            type: 'image/png'
          }
        ]
      }
    }),
    {
      name: 'handle-ggwave-import',
      transform(code, id) {
        if (id.endsWith('ggwave.js')) {
          return {
            code: `${code}
              export default ggwave_factory;
            `,
            map: null
          }
        }
      }
    }
  ],
  resolve: {
    alias: {
      'ggwave':'../lib/ggwave/bindings/javascript/ggwave.js',
    },
  },
  build: {
    rollupOptions: {
      external: ['ggwave'],
    },
  },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: './src/test/setup.js',
    include: ['src/**/*.{test,spec}.{js,mjs,cjs,ts,mts,cts,jsx,tsx}'],
    exclude: ['node_modules', 'dist', '.idea', '.git', '.cache'],
    coverage: {
      reporter: ['text', 'json', 'html'],
      exclude: [
        'node_modules/',
        'src/test/setup.js',
      ],
    },
  },
})
